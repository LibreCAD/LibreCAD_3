#!/usr/bin/env python3
"""Restore the pinned R0 Mac Qt module inputs; never build dependencies."""

import argparse
import hashlib
import json
import os
import platform
import re
import subprocess
import sys
from pathlib import Path, PurePosixPath

PIN = Path(__file__).resolve().parents[1] / 'lcrenderer/rhi/providers/qt683-macos.json'


def digest(data):
    return hashlib.sha256(data).hexdigest()


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


def restore(archives, destination, pin_path=PIN, tar='/usr/bin/tar'):
    helper_sha = digest(Path(__file__).read_bytes())
    pin_bytes = pin_path.read_bytes()
    pin = json.loads(pin_bytes, object_pairs_hook=unique_object)
    if (pin.get('format') != 'librecad-r0-provider-v1' or pin.get('platform') != 'macos' or
            pin.get('qt_version') != '6.8.3' or
            [item['module'] for item in pin['archives']] != ['qtbase', 'qtshadertools', 'qtsvg', 'qttools']):
        raise ValueError('Expected the pinned Mac Qt 6.8.3 module inputs')
    references = pin['reference_files_sha256']
    if not references or references.get('bin/qsb') != pin['qsb_sha256']:
        raise ValueError('Missing or inconsistent provider reference identities')
    for name, expected in references.items():
        path = PurePosixPath(name)
        if (not name or path.is_absolute() or '..' in path.parts or '\\' in name or
                not re.fullmatch('[0-9a-f]{64}', expected)):
            raise ValueError('Invalid provider reference path/digest')
    inputs = []
    # Check every archive before creating a destination or invoking extraction/tools.
    for item in pin['archives']:
        raw_path = archives / (item['module'] + '.7z')
        path = raw_path.resolve(strict=True)
        if (raw_path.is_symlink() or type(item['bytes']) is not int or path.stat().st_size != item['bytes'] or
                file_digest(path) != item['sha256']):
            raise ValueError('Provider archive size/hash mismatch: ' + path.name)
        inputs.append((path, item['sha256']))
    destination = destination.absolute()
    destination.mkdir(mode=0o700)  # Also refuses an existing/dangling-symlink target.
    commands = []
    for path, expected in inputs:
        if file_digest(path) != expected:
            raise ValueError('Provider archive changed before extraction: ' + path.name)
        command = [tar, '-xf', str(path), '-C', str(destination), '--no-same-owner']
        subprocess.run(command, check=True, capture_output=True, timeout=120)
        commands.append(command)
        if file_digest(path) != expected:
            raise ValueError('Provider archive changed during extraction: ' + path.name)
    def check_references():
        for name, expected in references.items():
            path = destination / name
            if not path.resolve().is_relative_to(destination.resolve()) or file_digest(path) != expected:
                raise ValueError('Restored provider reference mismatch: ' + name)
    check_references()
    tool_env = {key: value for key, value in os.environ.items()
                if not key.startswith('DYLD_') and key not in ('QT_PLUGIN_PATH', 'QT_QPA_PLATFORM_PLUGIN_PATH',
                                                               'QML_IMPORT_PATH', 'QML2_IMPORT_PATH')}
    version = subprocess.run([str(destination / 'bin/qsb'), '--version'], check=True, env=tool_env,
                             capture_output=True, text=True, timeout=10).stdout.strip()
    if version != 'qsb 6.8.3':
        raise ValueError('Restored shader tool version mismatch: ' + version)
    check_references()
    if any(file_digest(path) != expected for path, expected in inputs):
        raise ValueError('Provider archive changed during restoration')
    if pin_path.read_bytes() != pin_bytes or digest(Path(__file__).read_bytes()) != helper_sha:
        raise ValueError('Provider pin/helper changed during restoration')
    return dict(schema_version=1, evidence_kind='r0-provider-restore',
                scope='Mac Qt required-module inputs; not graphics, system/deployment or release acceptance',
                pin_sha256=digest(pin_bytes), restore_helper_sha256=helper_sha, qt_version=pin['qt_version'],
                host_architecture=platform.machine(), destination=str(destination),
                archives={path.name: expected for path, expected in inputs},
                references=references, commands=commands, qsb_version=version)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--archives', type=Path, required=True)
    parser.add_argument('--destination', type=Path, required=True,
                        help='new SDK directory; failures preserve it for diagnosis, never reuse it')
    args = parser.parse_args()
    if sys.platform != 'darwin':
        parser.error('This pinned provider is for native macOS only')
    try:
        print(json.dumps(restore(args.archives, args.destination), indent=2, sort_keys=True))
    except subprocess.CalledProcessError as error:
        detail = error.stderr or ''
        if isinstance(detail, bytes):
            detail = detail.decode(errors='replace')
        parser.exit(1, 'R0 provider restore: FAIL: {}\n{}\n'.format(error, detail[-2000:]))
    except (OSError, ValueError, KeyError, TypeError, subprocess.SubprocessError) as error:
        parser.exit(1, 'R0 provider restore: FAIL: {}\n'.format(error))


if __name__ == '__main__':
    main()
