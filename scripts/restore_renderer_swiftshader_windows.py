#!/usr/bin/env python3
"""Restore four pinned R0 Windows Vulkan inputs offline; never install a driver."""

import argparse
import ctypes
import hashlib
import json
import re
import struct
import sys
import zipfile
from pathlib import Path

PIN = Path(__file__).resolve().parents[1] / 'lcrenderer/rhi/providers/swiftshader-windows-x64.json'
FILES = {'vulkan-1.dll', 'vk_swiftshader.dll', 'vk_swiftshader_icd.json', 'LICENSE.headless_shell'}


def digest(path):
    value = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            value.update(block)
    return value.hexdigest()


def unique_object(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError('Duplicate provider key: ' + key)
        result[key] = value
    return result


def pe_x64(raw):
    if len(raw) < 64 or raw[:2] != b'MZ':
        raise ValueError('Expected a Windows AMD64 PE32+ DLL')
    offset = struct.unpack_from('<I', raw, 60)[0]
    if (not 64 <= offset <= len(raw) - 26 or raw[offset:offset + 4] != b'PE\0\0' or
            struct.unpack_from('<H', raw, offset + 4)[0] != 0x8664 or
            not struct.unpack_from('<H', raw, offset + 22)[0] & 0x2000 or
            struct.unpack_from('<H', raw, offset + 24)[0] != 0x20b):
        raise ValueError('Expected a Windows AMD64 PE32+ DLL')


def check_files(prefix, files):
    if set(files) != FILES:
        raise ValueError('Expected exactly the four pinned Vulkan inputs')
    if {path.name for path in prefix.iterdir()} != FILES:
        raise ValueError('Restored Vulkan input prefix changed: unexpected entries')
    for name, item in files.items():
        path = prefix / name
        if (path.is_symlink() or path.resolve(strict=True).parent != prefix.resolve(strict=True) or
                path.stat().st_size != item['bytes'] or digest(path) != item['sha256']):
            raise ValueError('Restored Vulkan input changed: ' + name)
    for name in ('vulkan-1.dll', 'vk_swiftshader.dll'):
        pe_x64((prefix / name).read_bytes())


def native_loader_probe(prefix, files):
    """Only a native AMD64 loader call; never create an instance/device/window."""
    if sys.platform != 'win32' or ctypes.sizeof(ctypes.c_void_p) != 8:
        raise ValueError('Native Windows x64 is required for the loader probe')
    check_files(prefix, files)  # Verify every input before loading external code.
    kernel = ctypes.WinDLL('kernel32', use_last_error=True)
    kernel.GetCurrentProcess.argtypes, kernel.GetCurrentProcess.restype = [], ctypes.c_void_p
    kernel.IsWow64Process2.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_ushort), ctypes.POINTER(ctypes.c_ushort)]
    kernel.IsWow64Process2.restype = ctypes.c_int
    process, native = ctypes.c_ushort(), ctypes.c_ushort()
    if (not kernel.IsWow64Process2(kernel.GetCurrentProcess(), ctypes.byref(process), ctypes.byref(native)) or
            process.value != 0 or native.value != 0x8664):
        raise ValueError('Loader probe requires native AMD64, not WOW64/ARM64 emulation')
    path = (prefix / 'vulkan-1.dll').resolve(strict=True)
    loader = ctypes.WinDLL(str(path), winmode=0x900)  # DLL directory + System32 only.
    kernel.GetModuleFileNameW.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_wchar), ctypes.c_uint32]
    kernel.GetModuleFileNameW.restype = ctypes.c_uint32
    name = ctypes.create_unicode_buffer(32768)
    length = kernel.GetModuleFileNameW(loader._handle, name, len(name))
    if not 0 < length < len(name) or Path(name.value).resolve(strict=True) != path:
        raise ValueError('Foreign or unobserved Vulkan loader module')
    loader.vkEnumerateInstanceVersion.argtypes = [ctypes.POINTER(ctypes.c_uint32)]
    loader.vkEnumerateInstanceVersion.restype = ctypes.c_int32
    version = ctypes.c_uint32()
    if loader.vkEnumerateInstanceVersion(ctypes.byref(version)) != 0:
        raise ValueError('Vulkan loader version query failed')
    api = [(version.value >> 22) & 127, (version.value >> 12) & 1023, version.value & 4095]
    if version.value >> 29 or api[0] != 1 or api[1] < 1:
        raise ValueError('Unqualified Vulkan loader API version')
    check_files(prefix, files)
    return {'scope': 'Native Windows x64 loader only; no ICD/device/graphics qualification',
            'architecture': 'x86_64', 'loaded_module': str(path), 'loader_api_version': api,
            'loader_sha256': files['vulkan-1.dll']['sha256']}


def restore(archive, destination, pin_path=PIN, probe=False):
    helper_sha, pin_sha = digest(Path(__file__)), digest(pin_path)
    pin = json.loads(pin_path.read_bytes(), object_pairs_hook=unique_object)
    if (pin.get('format') != 'librecad-r0-provider-v1' or pin.get('provider') != 'chromium-swiftshader' or
            pin.get('platform') != 'windows' or pin.get('payload_architectures') != ['x86_64'] or
            pin.get('source_version') != '156.0.8078.47' or set(pin['files']) != FILES):
        raise ValueError('Expected the pinned Windows x64 SwiftShader candidate')
    item, files = pin['archive'], pin['files']
    for value in [item, *files.values()]:
        if (type(value['bytes']) is not int or not 0 < value['bytes'] <= 512 * 1024 * 1024 or
                not isinstance(value['sha256'], str) or not re.fullmatch('[0-9a-f]{64}', value['sha256'])):
            raise ValueError('Invalid Vulkan input size/digest')
    if item['filename'] != 'chrome-headless-shell-win64.zip' or item['prefix'] != 'chrome-headless-shell-win64':
        raise ValueError('Unexpected provider archive layout')
    archive = archive.absolute()
    if archive.is_symlink() or archive.stat().st_size != item['bytes'] or digest(archive) != item['sha256']:
        raise ValueError('Vulkan archive size/hash/path mismatch')
    with zipfile.ZipFile(archive) as source:
        names = [info.filename for info in source.infolist()]
        selected = {}
        for name, expected in files.items():
            member = item['prefix'] + '/' + name
            if names.count(member) != 1:
                raise ValueError('Missing or duplicate Vulkan archive member: ' + name)
            info = source.getinfo(member)
            if (info.file_size != expected['bytes'] or info.is_dir() or
                    info.external_attr >> 16 & 0o170000 not in (0, 0o100000)):
                raise ValueError('Invalid Vulkan archive member size/type: ' + name)
            raw = source.read(info)
            if hashlib.sha256(raw).hexdigest() != expected['sha256']:
                raise ValueError('Vulkan member hash mismatch: ' + name)
            selected[name] = raw
    for name in ('vulkan-1.dll', 'vk_swiftshader.dll'):
        pe_x64(selected[name])
    manifest = json.loads(selected['vk_swiftshader_icd.json'], object_pairs_hook=unique_object)
    if manifest != pin['icd_manifest'] or manifest.get('ICD', {}).get('library_path') != '.\\vk_swiftshader.dll':
        raise ValueError('Unexpected SwiftShader ICD manifest')
    destination = destination.absolute()
    destination.mkdir(mode=0o700)  # Fresh prefix only; never overwrite an existing installation.
    for name, raw in selected.items():
        (destination / name).write_bytes(raw)
    check_files(destination, files)
    result = {'evidence_kind': 'r0-windows-vulkan-inputs', 'scope': pin['scope'], 'prefix': str(destination.resolve()),
              'helper_sha256': helper_sha, 'pin_sha256': pin_sha, 'archive_sha256': item['sha256'],
              'files': files, 'native_loader_probe': native_loader_probe(destination, files) if probe else None}
    if digest(archive) != item['sha256'] or digest(pin_path) != pin_sha or digest(Path(__file__)) != helper_sha:
        raise ValueError('Vulkan provider inputs/helper changed during restore')
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__, allow_abbrev=False)
    parser.add_argument('--archive', type=Path, required=True)
    parser.add_argument('--destination', type=Path, required=True)
    parser.add_argument('--native-probe', action='store_true', help='Require native Windows x64 loader execution, not graphics')
    args = parser.parse_args()
    try:
        print(json.dumps(restore(args.archive, args.destination, probe=args.native_probe), indent=2))
    except (OSError, ValueError, KeyError, TypeError, AttributeError, struct.error, zipfile.BadZipFile) as error:
        parser.exit(1, 'R0 Windows Vulkan restore: FAIL: {}\n'.format(error))


if __name__ == '__main__':
    main()
