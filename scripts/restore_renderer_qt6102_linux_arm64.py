#!/usr/bin/env python3
"""Restore one pinned R0 Linux ARM64 Qt tuple offline; never install packages."""

import argparse
import hashlib
import json
import os
import platform
import re
import subprocess
import sys
from pathlib import Path, PurePosixPath

PIN = Path(__file__).resolve().parents[1] / 'lcrenderer/rhi/providers/qt6102-linux-arm64.json'
QSB = 'lib/qt6/bin/qsb'
LIB = 'lib/aarch64-linux-gnu'


def file_digest(path):
    value = hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            value.update(chunk)
    return value.hexdigest()


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError('Duplicate provider key: ' + key)
        result[key] = value
    return result


def restore(archives, destination, pin_path=PIN, extractor='/usr/bin/dpkg-deb'):
    if sys.platform != 'linux' or platform.machine().lower() not in ('aarch64', 'arm64'):
        raise ValueError('This pinned provider requires native Linux ARM64')
    helper_sha = file_digest(Path(__file__))
    pin_bytes = pin_path.read_bytes()
    pin = json.loads(pin_bytes, object_pairs_hook=unique_object)
    if (not isinstance(pin, dict) or pin.get('format') != 'librecad-r0-provider-v1' or pin.get('platform') != 'linux' or
            pin.get('qt_version') != '6.10.2' or pin.get('payload_architectures') != ['arm64']):
        raise ValueError('Expected the pinned Linux ARM64 Qt 6.10.2 tuple')
    packages, references = pin['packages'], pin['reference_files_sha256']
    if not isinstance(packages, list) or not 1 <= len(packages) <= 128 or not isinstance(references, dict) or QSB not in references:
        raise ValueError('Missing provider packages/references')
    for name, expected in references.items():
        path = PurePosixPath(name)
        if (not name or name == '.' or path.is_absolute() or '..' in path.parts or '\\' in name or
                ':' in name or not isinstance(expected, str) or not re.fullmatch('[0-9a-f]{64}', expected)):
            raise ValueError('Invalid provider reference path/digest')
    cache = archives.resolve(strict=True)
    inputs, names, filenames = [], set(), set()
    for item in packages:
        name, filename = item['package'], item['filename']
        if (not isinstance(name, str) or not re.fullmatch('[a-z0-9][a-z0-9+.-]*', name) or
                not isinstance(filename, str) or not re.fullmatch('[a-zA-Z0-9+._~-]+\\.deb', filename) or
                name in names or filename in filenames or item['architecture'] not in ('arm64', 'all') or
                not isinstance(item['version'], str) or not re.fullmatch(r'6\.10\.2(?:[+-].*)?', item['version']) or
                type(item['bytes']) is not int or item['bytes'] <= 0 or
                not isinstance(item['sha256'], str) or not re.fullmatch('[0-9a-f]{64}', item['sha256'])):
            raise ValueError('Invalid or duplicate provider package identity')
        names.add(name)
        filenames.add(filename)
        path = cache / filename
        if path.is_symlink() or path.stat().st_size != item['bytes'] or file_digest(path) != item['sha256']:
            raise ValueError('Provider package size/hash/path mismatch: ' + filename)
        inputs.append((path, item['sha256']))
    # No extraction or tool invocation until every raw package is verified.
    destination = destination.absolute()
    destination.mkdir(mode=0o700)
    prefix = destination / 'usr'
    commands = []
    for path, expected in inputs:
        if file_digest(path) != expected:
            raise ValueError('Provider package changed before extraction: ' + path.name)
        command = [extractor, '-x', str(path), str(destination)]
        subprocess.run(command, check=True, capture_output=True, timeout=120)
        commands.append(command)
        if file_digest(path) != expected:
            raise ValueError('Provider package changed during extraction: ' + path.name)

    def check_references():
        for name, expected in references.items():
            path = prefix / name
            if not path.resolve().is_relative_to(prefix.resolve()) or file_digest(path) != expected:
                raise ValueError('Restored provider reference mismatch: ' + name)

    check_references()
    env = os.environ.copy()
    for name in ('LD_PRELOAD', 'LD_AUDIT', 'QT_PLUGIN_PATH', 'QT_QPA_PLATFORM_PLUGIN_PATH',
                 'QML_IMPORT_PATH', 'QML2_IMPORT_PATH', 'LD_DEBUG_OUTPUT'):
        env.pop(name, None)
    env.update(LD_LIBRARY_PATH=str(prefix / LIB), LD_DEBUG='libs', QT_QPA_PLATFORM='offscreen')
    result = subprocess.run([str(prefix / QSB), '--version'], env=env, check=True,
                            capture_output=True, text=True, timeout=10)
    version = result.stdout.strip()
    if version != 'qsb 6.10.2':
        raise ValueError('Restored shader tool version mismatch: ' + version)
    loaded = sorted({str(Path(p.strip()).resolve()) for p in
                     re.findall(r'calling init:\s+([^\n]*libQt6[^\n]*)', result.stderr)})
    if (not loaded or any(not Path(p).is_relative_to(prefix.resolve()) for p in loaded) or
            any(not any(required in Path(p).name for p in loaded)
                for required in ('libQt6Core', 'libQt6Gui', 'libQt6ShaderTools'))):
        raise ValueError('Shader tool loaded missing/foreign Qt libraries')
    check_references()
    if any(file_digest(path) != expected for path, expected in inputs):
        raise ValueError('Provider package changed during restoration')
    if pin_path.read_bytes() != pin_bytes or file_digest(Path(__file__)) != helper_sha:
        raise ValueError('Provider pin/helper changed during restoration')
    return dict(schema_version=1, evidence_kind='r0-linux-provider-restore',
                scope='Pinned Qt inputs only; native module/graphics/deployment/release qualification is separate',
                qt_version=pin['qt_version'], host_architecture=platform.machine(),
                destination=str(destination), prefix=str(prefix), restore_helper_sha256=helper_sha,
                pin_sha256=hashlib.sha256(pin_bytes).hexdigest(), qsb_version=version,
                packages={path.name: expected for path, expected in inputs}, references=references,
                commands=commands, qsb_loader_log=result.stderr,
                loaded_qt_files={p: file_digest(Path(p)) for p in loaded})


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--archives', type=Path, required=True)
    parser.add_argument('--destination', type=Path, required=True,
                        help='new SDK directory; failures preserve it for diagnosis')
    args = parser.parse_args()
    try:
        print(json.dumps(restore(args.archives, args.destination), indent=2, sort_keys=True))
    except subprocess.CalledProcessError as error:
        detail = error.stderr or ''
        if isinstance(detail, bytes):
            detail = detail.decode(errors='replace')
        parser.exit(1, 'R0 Linux provider restore: FAIL: {}\n{}\n'.format(error, detail[-2000:]))
    except (OSError, ValueError, KeyError, TypeError, subprocess.SubprocessError) as error:
        parser.exit(1, 'R0 Linux provider restore: FAIL: {}\n'.format(error))


if __name__ == '__main__':
    main()
