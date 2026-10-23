"""CPU restoration failure fixtures; mocked extraction is not provider acceptance."""

import copy
import contextlib
import io
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import restore_renderer_qt683_macos as provider


class Qt683RestoreTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.archives = self.root / 'archives'
        self.archives.mkdir()
        self.destination = self.root / 'sdk'
        self.pin_path = self.root / 'pin.json'
        self.pin = json.loads(provider.PIN.read_text())
        self.files = {name: ('fixture ' + name).encode() for name in self.pin['reference_files_sha256']}
        self.pin['reference_files_sha256'] = {name: provider.digest(data) for name, data in self.files.items()}
        self.pin['qsb_sha256'] = self.pin['reference_files_sha256']['bin/qsb']
        for item in self.pin['archives']:
            data = item['module'].encode()
            (self.archives / (item['module'] + '.7z')).write_bytes(data)
            item.update(bytes=len(data), sha256=provider.digest(data))

    def invoke(self, run=None):
        self.pin_path.write_text(json.dumps(self.pin))
        with patch.object(provider.subprocess, 'run', side_effect=run or self.extract) as mock:
            result = provider.restore(self.archives, self.destination, self.pin_path)
        return result, mock

    def extract(self, command, **kwargs):
        if command[0] == '/usr/bin/tar':
            for name, data in self.files.items():
                path = self.destination / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(data)
            return subprocess.CompletedProcess(command, 0, b'', b'')
        return subprocess.CompletedProcess(command, 0, 'qsb 6.8.3\n', '')

    def test_restores_all_references_and_records_exact_inputs(self):
        result, mock = self.invoke()
        self.assertEqual('r0-provider-restore', result['evidence_kind'])
        self.assertEqual(self.pin['reference_files_sha256'], result['references'])
        self.assertEqual(4, len(result['archives']))
        self.assertEqual(5, mock.call_count)
        self.assertEqual(provider.digest(self.pin_path.read_bytes()), result['pin_sha256'])
        self.assertTrue(all('--no-same-owner' in cmd for cmd in result['commands']))

    def test_all_archives_checked_before_destination_or_execution(self):
        (self.archives / 'qtshadertools.7z').write_bytes(b'bad')
        with self.assertRaisesRegex(ValueError, 'archive size/hash'):
            self.invoke(lambda *a, **k: self.fail('must not execute'))
        self.assertFalse(self.destination.exists())

    def test_archive_hash_checked_even_at_same_size(self):
        path = self.archives / 'qtbase.7z'
        path.write_bytes(b'x' * path.stat().st_size)
        with self.assertRaisesRegex(ValueError, 'archive size/hash'):
            self.invoke()

    def test_added_module_is_required_before_any_extraction(self):
        (self.archives / 'qttools.7z').unlink()
        with self.assertRaises(FileNotFoundError):
            self.invoke(lambda *a, **k: self.fail('must not execute'))
        self.assertFalse(self.destination.exists())

    def test_duplicate_json_keys_are_rejected_before_extraction(self):
        raw = json.dumps(self.pin).replace('"qt_version": "6.8.3"',
                                         '"qt_version": "6.8.3", "qt_version": "6.8.3"')
        with patch.object(Path, 'read_bytes', return_value=raw.encode()), \
                self.assertRaisesRegex(ValueError, 'Duplicate provider key'):
            self.invoke(lambda *a, **k: self.fail('must not execute'))
        self.assertFalse(self.destination.exists())

    def test_wrong_archive_size_is_rejected_before_hashing(self):
        (self.archives / 'qtbase.7z').write_bytes(b'oversized input')
        with patch.object(provider, 'file_digest', side_effect=AssertionError('must not hash')), \
                self.assertRaisesRegex(ValueError, 'archive size/hash'):
            self.invoke()

    def test_streamed_digest_matches_bytes_digest(self):
        path = self.archives / 'large-fixture'
        data = b'x' * (1024 * 1024 + 17)
        path.write_bytes(data)
        self.assertEqual(provider.digest(data), provider.file_digest(path))

    def test_missing_archive_fails_without_destination(self):
        (self.archives / 'qtbase.7z').unlink()
        with self.assertRaises(FileNotFoundError):
            self.invoke()
        self.assertFalse(self.destination.exists())

    def test_raw_archive_symlink_is_not_followed(self):
        with patch.object(Path, 'is_symlink', return_value=True), \
                self.assertRaisesRegex(ValueError, 'archive size/hash'):
            self.invoke(lambda *a, **k: self.fail('must not execute'))
        self.assertFalse(self.destination.exists())

    def test_existing_destination_is_never_overwritten(self):
        self.destination.mkdir()
        sentinel = self.destination / 'owned'
        sentinel.write_text('keep')
        with self.assertRaises(FileExistsError):
            self.invoke(lambda *a, **k: self.fail('must not execute'))
        self.assertEqual('keep', sentinel.read_text())

    def test_dangling_destination_symlink_is_not_followed(self):
        try:
            self.destination.symlink_to(self.root / 'missing', target_is_directory=True)
        except OSError as error:
            if sys.platform != 'win32':
                raise
            # This provider is Mac-only. Without Windows symlink privilege,
            # exercise the same native mkdir refusal without a passing skip.
            with patch.object(Path, 'mkdir', side_effect=FileExistsError(str(error))):
                with self.assertRaises(FileExistsError):
                    self.invoke(lambda *a, **k: self.fail('must not execute'))
            return
        with self.assertRaises(FileExistsError):
            self.invoke()

    def test_invalid_provider_identity_or_module_list(self):
        original = copy.deepcopy(self.pin)
        for key, value in [('format', 'other'), ('qt_version', '6.12.0'), ('platform', 'linux'),
                           ('archives', list(reversed(self.pin['archives'])))]:
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.pin = copy.deepcopy(original)
                self.pin[key] = value
                self.invoke()
        self.assertFalse(self.destination.exists())

    def test_unsafe_reference_names_and_digests(self):
        original = copy.deepcopy(self.pin)
        for name, digest in [('../outside', 'a' * 64), ('/outside', 'a' * 64),
                             ('a\\b', 'a' * 64), ('lib/file', 'not-a-digest')]:
            with self.subTest(name=name), self.assertRaises(ValueError):
                self.pin = copy.deepcopy(original)
                self.pin['reference_files_sha256'][name] = digest
                self.invoke()
        self.assertFalse(self.destination.exists())

    def test_extraction_failure_or_timeout_cannot_publish_success(self):
        for error in (subprocess.CalledProcessError(1, ['tar']), subprocess.TimeoutExpired(['tar'], 120)):
            with self.subTest(error=error), tempfile.TemporaryDirectory() as root:
                self.destination = Path(root) / 'sdk'
                with self.assertRaises(subprocess.SubprocessError):
                    self.invoke(lambda *a, **k: (_ for _ in ()).throw(error))

    def test_restored_reference_mismatch_does_not_execute_qsb(self):
        self.files['bin/qsb'] = b'changed'
        with self.assertRaisesRegex(ValueError, 'reference mismatch'):
            self.invoke()

    def test_wrong_native_qsb_version_fails(self):
        def run(command, **kwargs):
            result = self.extract(command, **kwargs)
            if command[0] != '/usr/bin/tar':
                result.stdout = 'qsb 6.12.0\n'
            return result
        with self.assertRaisesRegex(ValueError, 'version mismatch'):
            self.invoke(run)

    def test_qsb_does_not_inherit_foreign_provider_overrides(self):
        with patch.dict(provider.os.environ, {'DYLD_FRAMEWORK_PATH': '/foreign', 'DYLD_INSERT_LIBRARIES': '/foreign',
                                             'QT_PLUGIN_PATH': '/foreign', 'QML_IMPORT_PATH': '/foreign'}):
            _, mock = self.invoke()
        env = mock.call_args.kwargs['env']
        self.assertFalse(any(name.startswith('DYLD_') for name in env))
        self.assertNotIn('QT_PLUGIN_PATH', env)
        self.assertNotIn('QML_IMPORT_PATH', env)

    def test_pin_change_during_restore_cannot_publish_success(self):
        def run(command, **kwargs):
            result = self.extract(command, **kwargs)
            if command[0] != '/usr/bin/tar':
                self.pin_path.write_text('{}')
            return result
        with self.assertRaisesRegex(ValueError, 'pin/helper changed'):
            self.invoke(run)

    def test_changed_reference_after_qsb_cannot_publish_success(self):
        def run(command, **kwargs):
            result = self.extract(command, **kwargs)
            if command[0] != '/usr/bin/tar':
                (self.destination / 'bin/qsb').write_bytes(b'changed')
            return result
        with self.assertRaisesRegex(ValueError, 'reference mismatch'):
            self.invoke(run)

    def test_changed_archive_after_qsb_cannot_publish_success(self):
        def run(command, **kwargs):
            result = self.extract(command, **kwargs)
            if command[0] != '/usr/bin/tar':
                (self.archives / 'qtbase.7z').write_bytes(b'changed')
            return result
        with self.assertRaisesRegex(ValueError, 'changed during restoration'):
            self.invoke(run)

    def test_changed_archive_during_extraction_fails_before_qsb(self):
        def run(command, **kwargs):
            self.assertEqual('/usr/bin/tar', command[0])
            result = self.extract(command, **kwargs)
            (self.archives / 'qtbase.7z').write_bytes(b'changed')
            return result
        with self.assertRaisesRegex(ValueError, 'changed during extraction'):
            self.invoke(run)

    def test_reference_resolving_outside_sdk_is_rejected(self):
        original = Path.resolve
        escaped = self.destination / 'bin/qsb'
        def resolve(path, *args, **kwargs):
            return self.root / 'outside' if path == escaped else original(path, *args, **kwargs)
        with patch.object(Path, 'resolve', resolve), self.assertRaisesRegex(ValueError, 'reference mismatch'):
            self.invoke()

    def test_inconsistent_reference_pin_is_rejected_before_extraction(self):
        self.pin['qsb_sha256'] = 'a' * 64
        with self.assertRaisesRegex(ValueError, 'reference identities'):
            self.invoke(lambda *a, **k: self.fail('must not execute'))
        self.assertFalse(self.destination.exists())

    def test_boolean_size_is_not_an_integer_pin(self):
        self.pin['archives'][0]['bytes'] = True
        with self.assertRaisesRegex(ValueError, 'archive size/hash'):
            self.invoke()
        self.assertFalse(self.destination.exists())

    def test_cli_keeps_native_tool_error_diagnostic(self):
        error = subprocess.CalledProcessError(-6, ['qsb'], stderr=b'CPU probe diagnostic')
        stderr = io.StringIO()
        argv = ['restore', '--archives', str(self.archives), '--destination', str(self.destination)]
        with patch.object(provider.sys, 'argv', argv), patch.object(provider.sys, 'platform', 'darwin'), \
                patch.object(provider, 'restore', side_effect=error), contextlib.redirect_stderr(stderr), \
                self.assertRaises(SystemExit) as caught:
            provider.main()
        self.assertEqual(1, caught.exception.code)
        self.assertIn('CPU probe diagnostic', stderr.getvalue())

    def test_cli_rejects_non_mac_execution_before_restoration(self):
        argv = ['restore', '--archives', str(self.archives), '--destination', str(self.destination)]
        with patch.object(provider.sys, 'argv', argv), patch.object(provider.sys, 'platform', 'linux'), \
                patch.object(provider, 'restore') as restore, contextlib.redirect_stderr(io.StringIO()), \
                self.assertRaises(SystemExit) as caught:
            provider.main()
        self.assertEqual(2, caught.exception.code)
        restore.assert_not_called()


if __name__ == '__main__':
    unittest.main()
