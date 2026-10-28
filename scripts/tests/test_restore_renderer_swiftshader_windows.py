"""CPU fixtures for the pinned Windows inputs/Win32 protocol; never execute a DLL."""

import copy
import ctypes
import hashlib
import importlib.util
import json
import struct
import subprocess
import sys
import tempfile
import unittest
import warnings
import zipfile
from pathlib import Path
from unittest.mock import Mock, patch

SCRIPT = Path(__file__).resolve().parents[1] / 'restore_renderer_swiftshader_windows.py'
SPEC = importlib.util.spec_from_file_location('win_vulkan_restore', SCRIPT)
restore = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(restore)


def dll(machine=0x8664, magic=0x20b, characteristics=0x2022):
    raw = bytearray(256)
    raw[:2] = b'MZ'
    struct.pack_into('<I', raw, 60, 128)
    raw[128:132] = b'PE\0\0'
    struct.pack_into('<H', raw, 132, machine)
    struct.pack_into('<H', raw, 150, characteristics)
    struct.pack_into('<H', raw, 152, magic)
    return bytes(raw)


class WindowsVulkanRestoreTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name).resolve()
        self.archive = self.root / 'chrome-headless-shell-win64.zip'
        self.destination = self.root / 'private inputs Ω'
        self.pin_path = self.root / 'pin.json'
        self.pin = json.loads(restore.PIN.read_text())
        self.payload = {'vulkan-1.dll': dll(), 'vk_swiftshader.dll': dll(),
            'vk_swiftshader_icd.json': json.dumps(self.pin['icd_manifest']).encode(),
            'LICENSE.headless_shell': b'Synthetic CPU-only SwiftShader/Vulkan notice fixture'}
        self.build_archive()

    def build_archive(self, omit=None, duplicate=None, symlink=None, extra=None):
        with warnings.catch_warnings(), zipfile.ZipFile(self.archive, 'w') as archive:
            warnings.simplefilter('ignore', UserWarning)
            for name, raw in self.payload.items():
                self.pin['files'][name] = {'bytes': len(raw), 'sha256': hashlib.sha256(raw).hexdigest()}
                if name == omit:
                    continue
                info = zipfile.ZipInfo('chrome-headless-shell-win64/' + name)
                info.external_attr = (0o120777 if name == symlink else 0o100600) << 16
                archive.writestr(info, raw)
                if name == duplicate:
                    archive.writestr(info, raw)
            if extra:
                archive.writestr(extra, b'Never extract or execute unrelated members')
        self.pin['archive'].update(bytes=self.archive.stat().st_size, sha256=restore.digest(self.archive))
        self.save_pin()

    def save_pin(self):
        self.pin_path.write_text(json.dumps(self.pin))

    def run_restore(self, probe=False):
        return restore.restore(self.archive, self.destination, self.pin_path, probe)

    def test_cross_platform_static_restore_has_exact_files_identities_and_no_native_claim(self):
        self.build_archive(extra='../outside.exe')
        with patch.object(restore, 'native_loader_probe') as native:
            result = self.run_restore()
        native.assert_not_called()
        self.assertEqual(restore.FILES, {p.name for p in self.destination.iterdir()})
        self.assertFalse((self.root / 'outside.exe').exists())
        self.assertEqual('r0-windows-vulkan-inputs', result['evidence_kind'])
        self.assertIsNone(result['native_loader_probe'])
        self.assertEqual(restore.digest(self.pin_path), result['pin_sha256'])
        self.assertEqual(restore.digest(SCRIPT), result['helper_sha256'])
        self.assertEqual(self.pin['icd_manifest'], json.loads((self.destination / 'vk_swiftshader_icd.json').read_text()))

    def test_real_pin_keeps_raw_architecture_notice_and_manifest_distinct_from_execution(self):
        pin = json.loads(restore.PIN.read_text())
        self.assertEqual(['x86_64'], pin['payload_architectures'])
        self.assertEqual(126051683, pin['archive']['bytes'])
        self.assertIn('?generation=1792453636893180', pin['archive']['url'])
        self.assertEqual('e0af9b23f8051cca4f37ca49cd425e8055701bfc0383c198d67f8d5cc5884249', pin['archive']['sha256'])
        self.assertEqual(restore.FILES, set(pin['files']))
        self.assertEqual('1.0.5', pin['icd_manifest']['ICD']['api_version'])
        self.assertIn('not an accepted ICD', pin['scope'])

    def test_archive_hash_size_and_symlink_fail_before_destination_or_native_code(self):
        for key, value in (('bytes', True), ('bytes', 1), ('sha256', '0' * 64)):
            original = copy.deepcopy(self.pin)
            self.pin['archive'][key] = value
            self.save_pin()
            with self.subTest(key=key, value=value), patch.object(restore, 'native_loader_probe') as native, self.assertRaises(ValueError):
                self.run_restore(True)
            native.assert_not_called()
            self.assertFalse(self.destination.exists())
            self.pin = original
        self.save_pin()
        with patch.object(Path, 'is_symlink', return_value=True), self.assertRaisesRegex(ValueError, 'path mismatch'):
            self.run_restore()

    def test_existing_destination_is_never_overwritten(self):
        self.destination.mkdir()
        sentinel = self.destination / 'keep'
        sentinel.write_bytes(b'user data')
        with self.assertRaises(FileExistsError):
            self.run_restore()
        self.assertEqual(b'user data', sentinel.read_bytes())

    def test_missing_duplicate_or_symlink_members_fail_before_extraction(self):
        for option in ('omit', 'duplicate', 'symlink'):
            self.build_archive(**{option: 'vulkan-1.dll'})
            with self.subTest(option=option), self.assertRaises(ValueError):
                self.run_restore()
            self.assertFalse(self.destination.exists())

    def test_member_hash_or_size_mismatch_fails_before_extraction(self):
        for key, value in (('bytes', 123), ('sha256', '0' * 64)):
            self.build_archive()
            self.pin['files']['vk_swiftshader.dll'][key] = value
            self.save_pin()
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.run_restore()
            self.assertFalse(self.destination.exists())

    def test_wrong_pe_machine_magic_non_dll_or_truncated_header_fail(self):
        for raw in (dll(machine=0xaa64), dll(machine=0x14c), dll(magic=0x10b), dll(characteristics=0x22), b'MZ', b'not PE'):
            self.payload['vulkan-1.dll'] = raw
            self.build_archive()
            with self.subTest(raw=raw[:4]), self.assertRaisesRegex(ValueError, 'PE32'):
                self.run_restore()
            self.assertFalse(self.destination.exists())

    def test_changed_or_foreign_icd_manifest_is_not_rewritten(self):
        for field, value in (('library_path', '../foreign.dll'), ('api_version', '1.3.0')):
            manifest = copy.deepcopy(self.pin['icd_manifest'])
            manifest['ICD'][field] = value
            self.payload['vk_swiftshader_icd.json'] = json.dumps(manifest).encode()
            self.build_archive()
            with self.subTest(field=field), self.assertRaisesRegex(ValueError, 'manifest'):
                self.run_restore()
            self.assertFalse(self.destination.exists())

    def test_pin_rejects_extra_output_keys_bad_tuple_and_duplicate_json_keys(self):
        for key, value in (('provider', 'foreign'), ('platform', 'linux'), ('payload_architectures', ['arm64']), ('source_version', 'moving')):
            original = copy.deepcopy(self.pin)
            self.pin[key] = value
            self.save_pin()
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.run_restore()
            self.pin = original
        self.pin['files']['../escape'] = self.pin['files']['vulkan-1.dll']
        self.save_pin()
        with self.assertRaises(ValueError):
            self.run_restore()
        self.pin_path.write_text('{"provider":"first","provider":"second"}')
        with self.assertRaisesRegex(ValueError, 'Duplicate'):
            self.run_restore()

    def test_cli_failure_has_no_success_json(self):
        self.archive.write_bytes(b'changed')
        result = subprocess.run([sys.executable, '-B', str(SCRIPT), '--archive', str(self.archive),
            '--destination', str(self.destination)], capture_output=True)
        self.assertNotEqual(0, result.returncode)
        self.assertEqual(b'', result.stdout)
        self.assertIn(b'FAIL', result.stderr)


    # Protocol mocks, not native execution or an actual Windows loader/provider.
    def fake_api(self, process=0, native=0x8664, arch_ok=True, version=(1 << 22) | (3 << 12) | 280,
                 query_status=0, module=None, path_length=None, on_load=None):
        kernel, loader = Mock(), Mock()
        loader._handle = 1234
        kernel.GetCurrentProcess.return_value = -1
        def architecture(handle, process_ptr, native_ptr):
            ctypes.cast(process_ptr, ctypes.POINTER(ctypes.c_ushort))[0] = process
            ctypes.cast(native_ptr, ctypes.POINTER(ctypes.c_ushort))[0] = native
            return int(arch_ok)
        kernel.IsWow64Process2.side_effect = architecture
        def filename(handle, buffer, limit):
            buffer.value = str(module or (self.destination / 'vulkan-1.dll'))
            return len(buffer.value) if path_length is None else path_length
        kernel.GetModuleFileNameW.side_effect = filename
        def query(ptr):
            ctypes.cast(ptr, ctypes.POINTER(ctypes.c_uint32))[0] = version
            return query_status
        loader.vkEnumerateInstanceVersion.side_effect = query
        def load(name, **kwargs):
            if name == 'kernel32':
                return kernel
            self.assertEqual(0x900, kwargs['winmode'])
            self.assertEqual((self.destination / 'vulkan-1.dll').resolve(), Path(name))
            if on_load:
                on_load()
            return loader
        return kernel, loader, load

    def test_native_success_records_observed_path_api_and_scope(self):
        kernel, loader, load = self.fake_api()
        with patch.object(restore.sys, 'platform', 'win32'), patch.object(restore.ctypes, 'WinDLL', side_effect=load, create=True):
            result = self.run_restore(True)
        self.assertEqual([1, 3, 280], result['native_loader_probe']['loader_api_version'])
        self.assertIn('no ICD/device/graphics', result['native_loader_probe']['scope'])
        loader.vkEnumerateInstanceVersion.assert_called_once()
        kernel.IsWow64Process2.assert_called_once()

    def test_non_windows_guard_never_loads_code(self):
        self.run_restore()
        with patch.object(restore.sys, 'platform', 'darwin'), patch.object(restore.ctypes, 'WinDLL', create=True) as load, self.assertRaises(ValueError):
            restore.native_loader_probe(self.destination, self.pin['files'])
        load.assert_not_called()

    def test_32bit_process_never_loads_code(self):
        self.run_restore()
        with patch.object(restore.sys, 'platform', 'win32'), patch.object(restore.ctypes, 'sizeof', return_value=4), \
             patch.object(restore.ctypes, 'WinDLL', create=True) as load, self.assertRaises(ValueError):
            restore.native_loader_probe(self.destination, self.pin['files'])
        load.assert_not_called()

    def test_unexpected_prefix_entries_fail_before_loading_any_code(self):
        self.run_restore()
        (self.destination / 'CFGMGR32.dll').write_bytes(b'unreviewed dependency')
        with patch.object(restore.sys, 'platform', 'win32'), patch.object(restore.ctypes, 'WinDLL', create=True) as load, self.assertRaisesRegex(ValueError, 'changed'):
            restore.native_loader_probe(self.destination, self.pin['files'])
        load.assert_not_called()

    def test_wow64_arm64_or_failed_arch_query_never_load_provider(self):
        for options in ({'process': 0x8664, 'native': 0xaa64}, {'process': 0x14c}, {'native': 0xaa64}, {'arch_ok': False}):
            kernel, loader, load = self.fake_api(**options)
            self.destination = self.root / ('probe-' + str(len(list(self.root.iterdir()))))
            with self.subTest(options=options), patch.object(restore.sys, 'platform', 'win32'), \
                 patch.object(restore.ctypes, 'WinDLL', side_effect=load, create=True) as execute, self.assertRaisesRegex(ValueError, 'native AMD64'):
                self.run_restore(True)
            self.assertEqual(1, execute.call_count)
            loader.vkEnumerateInstanceVersion.assert_not_called()

    def test_foreign_truncated_missing_module_or_failed_version_never_pass(self):
        foreign = self.root / 'foreign.dll'
        foreign.write_bytes(dll())
        for options in ({'module': foreign}, {'path_length': 0}, {'path_length': 32768}, {'query_status': -3}, {'version': 1 << 22}):
            self.destination = self.root / ('probe-' + str(len(list(self.root.iterdir()))))
            kernel, loader, load = self.fake_api(**options)
            with self.subTest(options=options), patch.object(restore.sys, 'platform', 'win32'), \
                 patch.object(restore.ctypes, 'WinDLL', side_effect=load, create=True), self.assertRaises(ValueError):
                self.run_restore(True)

    def test_changed_provider_after_native_load_fails(self):
        kernel, loader, load = self.fake_api(on_load=lambda: (self.destination / 'vk_swiftshader.dll').write_bytes(b'changed'))
        with patch.object(restore.sys, 'platform', 'win32'), patch.object(restore.ctypes, 'WinDLL', side_effect=load, create=True), self.assertRaisesRegex(ValueError, 'changed'):
            self.run_restore(True)

    def test_ci_requires_native_preflight_and_retains_only_input_metadata(self):
        workflow = (SCRIPT.parents[1] / '.github/workflows/renderer.yml').read_text()
        self.assertGreaterEqual(workflow.count('scripts.tests.test_restore_renderer_swiftshader_windows'), 2)
        block = workflow.split('- name: Restore pinned Windows Vulkan inputs and probe the native loader', 1)[1].split('- name:', 1)[0]
        self.assertIn("if: runner.os == 'Windows'", block)
        self.assertIn("'--native-probe'", block)
        self.assertIn('subprocess.run(command, check=True', block)
        self.assertNotIn('continue-on-error', block)
        upload = workflow.split('- name: Retain provider/build/CPU evidence; not graphics acceptance', 1)[1].split('  r0-scaffold:', 1)[0]
        self.assertIn('build-rhi/windows-vulkan-inputs/', upload)
        self.assertNotIn('chrome-headless-shell-win64.zip', upload)


if __name__ == '__main__':
    unittest.main()
