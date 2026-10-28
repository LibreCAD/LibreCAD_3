"""CPU fixtures for the targeted restorer; mocked tools are not native proof."""

import contextlib
import copy
import hashlib
import io
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import restore_renderer_qt6102_linux_arm64 as provider


class LinuxQtRestoreTests(unittest.TestCase):
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
        self.pin['reference_files_sha256'] = {name: self.sha(data) for name, data in self.files.items()}
        for item in self.pin['packages']:
            data = item['package'].encode()
            (self.archives / item['filename']).write_bytes(data)
            item.update(bytes=len(data), sha256=self.sha(data))

    @staticmethod
    def sha(data):
        return hashlib.sha256(data).hexdigest()

    def invoke(self, run=None, raw=None):
        self.pin_path.write_text(raw if raw is not None else json.dumps(self.pin))
        with patch.object(provider.sys, 'platform', 'linux'), \
                patch.object(provider.platform, 'machine', return_value='aarch64'), \
                patch.object(provider.subprocess, 'run', side_effect=run or self.extract) as mock:
            result = provider.restore(self.archives, self.destination, self.pin_path)
        return result, mock

    def extract(self, command, **kwargs):
        if command[0] == '/usr/bin/dpkg-deb':
            if not (self.destination / 'usr').exists():
                for name, data in self.files.items():
                    path = self.destination / 'usr' / name
                    path.parent.mkdir(parents=True, exist_ok=True)
                    path.write_bytes(data)
            return subprocess.CompletedProcess(command, 0, b'', b'')
        paths = [self.destination / 'usr' / provider.LIB / ('libQt6' + name + '.so.6.10.2')
                 for name in ('Core', 'Gui', 'ShaderTools')]
        trace = '\n'.join('1: calling init: ' + str(p.resolve()) for p in paths)
        return subprocess.CompletedProcess(command, 0, 'qsb 6.10.2\n', trace)

    def test_restores_inputs_and_records_private_tool_loader(self):
        result, mock = self.invoke()
        self.assertEqual('r0-linux-provider-restore', result['evidence_kind'])
        self.assertEqual(51, len(result['packages']))
        self.assertEqual(19, len(result['references']))
        self.assertEqual(3, len(result['loaded_qt_files']))
        self.assertEqual(52, mock.call_count)
        self.assertEqual(self.sha(self.pin_path.read_bytes()), result['pin_sha256'])
        self.assertTrue(all(cmd[1] == '-x' for cmd in result['commands']))
        env = mock.call_args.kwargs['env']
        self.assertEqual(str(self.destination / 'usr' / provider.LIB), env['LD_LIBRARY_PATH'])
        self.assertEqual('offscreen', env['QT_QPA_PLATFORM'])

    def test_last_corrupt_package_prevents_all_extraction(self):
        (self.archives / self.pin['packages'][-1]['filename']).write_bytes(b'bad')
        with self.assertRaisesRegex(ValueError, 'size/hash/path'):
            self.invoke(lambda *a, **k: self.fail('must not execute'))
        self.assertFalse(self.destination.exists())

    def test_same_size_corruption_is_rejected(self):
        item = self.pin['packages'][0]
        (self.archives / item['filename']).write_bytes(b'x' * item['bytes'])
        with self.assertRaisesRegex(ValueError, 'size/hash/path'):
            self.invoke()

    def test_wrong_pin_identity_and_top_level_are_rejected(self):
        original = copy.deepcopy(self.pin)
        for key, value in [('format', 'other'), ('platform', 'macos'), ('qt_version', '6.10.3'),
                           ('payload_architectures', ['x86_64'])]:
            self.pin = copy.deepcopy(original)
            self.pin[key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.invoke()
        with self.assertRaises(ValueError):
            self.invoke(raw='[]')
        self.assertFalse(self.destination.exists())

    def test_invalid_package_metadata_is_rejected(self):
        original = copy.deepcopy(self.pin)
        for key, value in [('filename', '../outside.deb'), ('filename', 'a\\b.deb'),
                           ('package', '-option'), ('architecture', 'amd64'), ('version', '6.10.20-1'),
                           ('bytes', True), ('bytes', 0), ('sha256', 'bad')]:
            self.pin = copy.deepcopy(original)
            self.pin['packages'][0][key] = value
            with self.subTest(key=key, value=value), self.assertRaises(ValueError):
                self.invoke()
        self.assertFalse(self.destination.exists())

    def test_duplicate_package_name_or_filename_is_rejected(self):
        original = copy.deepcopy(self.pin)
        for key in ('package', 'filename'):
            self.pin = copy.deepcopy(original)
            self.pin['packages'][1][key] = self.pin['packages'][0][key]
            with self.subTest(key=key), self.assertRaisesRegex(ValueError, 'duplicate'):
                self.invoke()

    def test_invalid_reference_paths_and_hashes_are_rejected(self):
        original = copy.deepcopy(self.pin)
        for name, expected in [('../outside', 'a' * 64), ('/outside', 'a' * 64),
                               ('a\\b', 'a' * 64), ('C:/outside', 'a' * 64), ('lib/file', 'bad')]:
            self.pin = copy.deepcopy(original)
            self.pin['reference_files_sha256'][name] = expected
            with self.subTest(name=name), self.assertRaises(ValueError):
                self.invoke()
        self.assertFalse(self.destination.exists())

    def test_missing_packages_or_tool_reference_are_rejected(self):
        original = copy.deepcopy(self.pin)
        for key, value in [('packages', []), ('reference_files_sha256', {})]:
            self.pin = copy.deepcopy(original)
            self.pin[key] = value
            with self.subTest(key=key), self.assertRaisesRegex(ValueError, 'Missing provider'):
                self.invoke()

    def test_missing_package_fails_before_destination(self):
        (self.archives / self.pin['packages'][0]['filename']).unlink()
        with self.assertRaises(FileNotFoundError):
            self.invoke()
        self.assertFalse(self.destination.exists())

    def test_existing_destination_is_preserved(self):
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
            with patch.object(Path, 'mkdir', side_effect=FileExistsError(str(error))), self.assertRaises(FileExistsError):
                self.invoke()
            return
        with self.assertRaises(FileExistsError):
            self.invoke()

    def test_package_symlink_is_rejected(self):
        target = (self.archives / self.pin['packages'][0]['filename']).resolve()
        original = Path.is_symlink
        with patch.object(Path, 'is_symlink', lambda p: p == target or original(p)), \
                self.assertRaisesRegex(ValueError, 'size/hash/path'):
            self.invoke()

    def test_streamed_hash_matches_bytes(self):
        data = b'x' * (1024 * 1024 + 19)
        path = self.root / 'large'
        path.write_bytes(data)
        self.assertEqual(self.sha(data), provider.file_digest(path))

    def test_extraction_failure_or_timeout_never_publishes_success(self):
        for i, error in enumerate((subprocess.CalledProcessError(1, ['dpkg-deb']),
                                   subprocess.TimeoutExpired(['dpkg-deb'], 120))):
            self.destination = self.root / ('sdk-' + str(i))
            with self.subTest(error=error), self.assertRaises(subprocess.SubprocessError):
                self.invoke(lambda *a, **k: (_ for _ in ()).throw(error))

    def test_reference_mismatch_prevents_tool_execution(self):
        self.files[provider.QSB] = b'changed'
        with self.assertRaisesRegex(ValueError, 'reference mismatch'):
            self.invoke(lambda command, **kw: self.extract(command, **kw) if command[0] == '/usr/bin/dpkg-deb'
                        else self.fail('qsb must not execute'))

    def test_wrong_qsb_version_is_rejected(self):
        def run(command, **kwargs):
            result = self.extract(command, **kwargs)
            if command[0] != '/usr/bin/dpkg-deb':
                result.stdout = 'qsb 6.12.0\n'
            return result
        with self.assertRaisesRegex(ValueError, 'version mismatch'):
            self.invoke(run)

    def test_missing_or_foreign_tool_libraries_are_rejected(self):
        for i, trace in enumerate(('', 'calling init: /usr/lib/libQt6Core.so.6')):
            self.destination = self.root / ('sdk-' + str(i))
            def run(command, **kwargs):
                result = self.extract(command, **kwargs)
                if command[0] != '/usr/bin/dpkg-deb':
                    result.stderr = trace
                return result
            with self.subTest(trace=trace), self.assertRaisesRegex(ValueError, 'missing/foreign'):
                self.invoke(run)

    def test_package_change_during_extraction_is_rejected(self):
        def run(command, **kwargs):
            result = self.extract(command, **kwargs)
            (self.archives / Path(command[2]).name).write_bytes(b'changed')
            return result
        with self.assertRaisesRegex(ValueError, 'changed during extraction'):
            self.invoke(run)

    def test_reference_escape_is_rejected_before_reading_it(self):
        original = Path.resolve
        escaped = self.destination / 'usr' / provider.QSB
        def resolve(path, *args, **kwargs):
            return self.root / 'outside' if path == escaped else original(path, *args, **kwargs)
        with patch.object(Path, 'resolve', resolve), self.assertRaisesRegex(ValueError, 'reference mismatch'):
            self.invoke()

    def test_pin_change_during_tool_execution_is_rejected(self):
        def run(command, **kwargs):
            result = self.extract(command, **kwargs)
            if command[0] != '/usr/bin/dpkg-deb':
                self.pin_path.write_text('{}')
            return result
        with self.assertRaisesRegex(ValueError, 'pin/helper changed'):
            self.invoke(run)

    def test_helper_change_during_tool_execution_is_rejected(self):
        helper = self.root / 'helper.py'
        helper.write_text('original')
        def run(command, **kwargs):
            result = self.extract(command, **kwargs)
            if command[0] != '/usr/bin/dpkg-deb':
                helper.write_text('changed')
            return result
        with patch.object(provider, '__file__', str(helper)), self.assertRaisesRegex(ValueError, 'pin/helper changed'):
            self.invoke(run)

    def test_reference_change_during_tool_execution_is_rejected(self):
        def run(command, **kwargs):
            result = self.extract(command, **kwargs)
            if command[0] != '/usr/bin/dpkg-deb':
                (self.destination / 'usr' / provider.QSB).write_bytes(b'changed')
            return result
        with self.assertRaisesRegex(ValueError, 'reference mismatch'):
            self.invoke(run)

    def test_duplicate_json_keys_are_rejected_before_extraction(self):
        with self.assertRaisesRegex(ValueError, 'Duplicate provider key'):
            self.invoke(raw='{"platform":"linux","platform":"linux"}')
        self.assertFalse(self.destination.exists())

    def test_wrong_native_platform_or_architecture_is_rejected(self):
        for system, machine in [('darwin', 'arm64'), ('linux', 'x86_64')]:
            with patch.object(provider.sys, 'platform', system), \
                    patch.object(provider.platform, 'machine', return_value=machine), \
                    patch.object(provider.subprocess, 'run') as run, self.assertRaisesRegex(ValueError, 'native Linux ARM64'):
                provider.restore(self.archives, self.destination, self.pin_path)
            run.assert_not_called()
        self.assertFalse(self.destination.exists())

    def test_cli_keeps_native_failure_diagnostic_and_no_success_json(self):
        error = subprocess.CalledProcessError(-6, ['qsb'], stderr=b'CPU probe diagnostic')
        stderr, stdout = io.StringIO(), io.StringIO()
        argv = ['restore', '--archives', str(self.archives), '--destination', str(self.destination)]
        with patch.object(provider.sys, 'argv', argv), patch.object(provider, 'restore', side_effect=error), \
                contextlib.redirect_stderr(stderr), contextlib.redirect_stdout(stdout), \
                self.assertRaises(SystemExit) as caught:
            provider.main()
        self.assertEqual(1, caught.exception.code)
        self.assertIn('CPU probe diagnostic', stderr.getvalue())
        self.assertEqual('', stdout.getvalue())


if __name__ == '__main__':
    unittest.main()
