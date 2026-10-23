"""Fail-closed R0 evidence/pixel and process-runner regressions; no GPU required."""

import argparse
import copy
import json
import os
import re
import struct
import subprocess
import sys
import tempfile
import textwrap
import unittest
import zlib
from pathlib import Path
from unittest.mock import Mock, patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import run_renderer_checks as runner  # noqa: E402
import validate_renderer_inventory as inventory  # noqa: E402


def png(width, height, rgba):
    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xffffffff)
    lines = b"".join(b"\0" + rgba[y * width * 4:(y + 1) * width * 4] for y in range(height))
    return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)) +
            chunk(b"IDAT", zlib.compress(lines)) + chunk(b"IEND", b""))


class MinimumLinuxCandidateTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.pin = inventory.read_json(inventory.REPO_ROOT / 'lcrenderer/rhi/providers/qt683-linux-arm64.json')

    def test_prebuilt_candidate_pins_exact_inputs_and_icu_layout(self):
        pin = self.pin
        self.assertEqual('librecad-r0-provider-v1', pin['format'])
        self.assertEqual('linux', pin['platform'])
        self.assertEqual(['arm64'], pin['payload_architectures'])
        self.assertEqual('6.8.3', pin['qt_version'])
        self.assertRegex(pin['metadata_sha256'], r'^[0-9a-f]{64}$')
        self.assertEqual(pin['qsb_sha256'], pin['reference_files_sha256']['bin/qsb'])
        self.assertEqual({'qtbase', 'icu', 'qtwayland', 'qtshadertools'}, {a['module'] for a in pin['archives']})
        for item in pin['archives']:
            self.assertGreater(item['bytes'], 0)
            self.assertRegex(item['sha256'], r'^[0-9a-f]{64}$')
            self.assertRegex(item['published_sha1'], r'^[0-9a-f]{40}$')
            self.assertTrue(item['url'].startswith('https://download.qt.io/online/qtsdkrepository/linux_arm64/'))
            self.assertEqual('lib' if item['module'] == 'icu' else '.', item['extract_subdir'])
        self.assertTrue({'lib/libicui18n.so.73.2', 'lib/libicuuc.so.73.2', 'lib/libicudata.so.73.2',
                         'include/QtGui/6.8.3/QtGui/rhi/qrhi.h', 'plugins/platforms/libqwayland-generic.so'}
                        <= pin['reference_files_sha256'].keys())

    def test_candidate_crash_cannot_be_relabeled_full_wayland_or_control_acceptance(self):
        result = self.pin['local_results']
        self.assertIn('Diagnostic-only', self.pin['scope'])
        self.assertIs(False, result['required_wayland_widgets_passed'])
        self.assertEqual(-11, result['widget_native_exit_status'])
        self.assertIs(True, result['qt_only_reproducer_crashes'])
        self.assertEqual({'positive_invocations': 24, 'positive_assertions': 1384,
                          'expected_failures': 6, 'failure_assertions': 18}, result['xcb'])
        self.assertEqual({'positive_invocations': 12, 'positive_assertions': 512}, result['wayland_offscreen'])
        self.assertEqual('6.10.2', result['current_qt_only_control']['qt_version'])
        self.assertEqual(0, result['current_qt_only_control']['native_exit_status'])
        self.assertIs(False, result['current_qt_only_control']['r0_smoke_qualified'])
        # A checked record is not native proof or authority to accept a provider.


    def test_validation_diagnostic_cannot_qualify_exit_zero_or_select_a_minimum(self):
        data = self.pin['local_results']['validation_diagnostic']
        self.assertIs(False, data['candidate_qualified'])
        self.assertIs(False, data['minimum_selection_changed'])
        self.assertEqual('VUID-vkDestroySurfaceKHR-surface-01266', data['surface_lifetime_vuid'])
        self.assertEqual('VUID-VkPresentInfoKHR-pImageIndices-01430', data['secondary_vuid'])
        provider = data['validation_provider']
        self.assertEqual(('1.4.341.0-1', 'arm64', 6698616), (provider['version'], provider['architecture'], provider['bytes']))
        self.assertTrue(provider['url'].startswith('https://ports.ubuntu.com/ubuntu-ports/pool/'))
        self.assertEqual('1.4.341', provider['api_version'])
        self.assertIs(True, provider['private_extraction_only'])
        self.assertEqual('045596d3ced7fb462f0afc6a8cc576606c50719ee1708f66f1a0e92e851bf9ad', provider['sha256'])
        cases = data['observations']
        self.assertEqual({'minimum-plain', 'minimum-validation', 'current-plain', 'current-validation'}, set(cases))
        self.assertEqual(0, cases['minimum-plain']['wrapper_exit_code'])
        self.assertIs(True, cases['minimum-plain']['mesa_queue_warning'])
        self.assertEqual(139, cases['minimum-validation']['wrapper_exit_code'])
        self.assertIn(data['surface_lifetime_vuid'], cases['minimum-validation']['vuid_ids'])
        for name, record in cases.items():
            self.assertIs(False, record['r0_smoke_qualified'])
            self.assertNotEqual(0, record['strict_r0_helper_exit'])
            self.assertEqual('validation' in name, record['validation_library_loaded'])
            if name.startswith('current'):
                self.assertEqual(0, record['wrapper_exit_code'])
                self.assertEqual([], record['vuid_ids'])
        for source in data['upstream_sources'].values():
            self.assertTrue(source['url'].startswith('https://raw.githubusercontent.com/qt/'))
            self.assertRegex(source['sha256'], r'^[0-9a-f]{64}$')
        # Metadata guard only; native validation messages and lifetime diagnosis are retained separately.


class MacModuleProviderTests(unittest.TestCase):
    def test_required_modules_and_official_binary_inputs_are_complete(self):
        pin = inventory.read_json(inventory.REPO_ROOT / 'lcrenderer/rhi/providers/qt683-macos.json')
        self.assertEqual('6.8.3', pin['qt_version'])
        self.assertEqual(['arm64', 'x86_64'], pin['payload_architectures'])
        self.assertEqual(['qtbase', 'qtshadertools', 'qtsvg', 'qttools'], [p['module'] for p in pin['archives']])
        self.assertTrue({'Svg', 'SvgWidgets', 'UiTools', 'Test', 'OpenGL', 'OpenGLWidgets', 'GuiPrivate'} <= set(pin['components']))
        for item in pin['archives']:
            self.assertTrue(item['url'].startswith('https://download.qt.io/online/qtsdkrepository/mac_x64/'))
            self.assertGreater(item['bytes'], 0)
            self.assertRegex(item['sha256'], r'^[0-9a-f]{64}$')
            self.assertRegex(item['published_sha1'], r'^[0-9a-f]{40}$')
        refs = pin['reference_files_sha256']
        for name in ['UiTools', 'Svg', 'SvgWidgets', 'Test', 'OpenGL', 'OpenGLWidgets']:
            self.assertIn('lib/Qt' + name + '.framework/Versions/A/Qt' + name, refs)
        for name in ['libexec/moc', 'libexec/rcc', 'libexec/uic', 'plugins/platforms/libqoffscreen.dylib',
                     'plugins/platforms/libqcocoa.dylib', 'plugins/imageformats/libqsvg.dylib',
                     'plugins/iconengines/libqsvgicon.dylib']:
            self.assertIn(name, refs)

    def test_native_probe_observes_frameworks_without_desktop_or_graphics(self):
        root = inventory.REPO_ROOT / 'lcrenderer/rhi/providerprobe'
        cpp = (root / 'providerprobe.cpp').read_text()
        cmake = (root / 'CMakeLists.txt').read_text()
        self.assertIn('_dyld_image_count()', cpp)
        self.assertIn('_dyld_get_image_name(index)', cpp)
        self.assertIn('r0-macos-qt-modules', cpp)
        self.assertIn('QSysInfo::currentCpuArchitecture()', cpp)
        self.assertIn('PROBE_OTHER_QT', cmake)
        self.assertIn('../spike/cmake/LegacyQtMacSdk.cmake', cmake)
        self.assertNotIn('QRhi::create(', cpp)
        self.assertNotIn('->show(', cpp)
        workflow = (inventory.REPO_ROOT / '.github/workflows/renderer.yml').read_text()
        mac = workflow.split('- name: Build native macOS targets with minimum Qt', 1)[1].split('- name:', 1)[0]
        self.assertIn('-DPROBE_QT_VERSION=6.8.3', mac)
        self.assertIn('-DBUILD_TESTING=ON', mac)
        self.assertIn('-DPROBE_OTHER_QT="$QT_COMPARE_ROOT_DIR/lib/cmake/Qt6"', mac)
        self.assertIn('build-rhi/provider-modules/source-inputs.sha256', mac)

    def test_native_negatives_do_not_assume_linux_loader_or_dev_full(self):
        helper = (inventory.REPO_ROOT / 'lcrenderer/rhi/providerprobe/check_config.py').read_text()
        self.assertIn('DYLD_FRAMEWORK_PATH', helper)
        self.assertIn('closed-pipe', helper)
        self.assertIn('os.close(read_fd)', helper)
        self.assertIn('Mixed-provider negative requires', helper)
        # CPU guards only; native restore/build/CTest and loader observations are separate.


class MacModuleCIResultTests(unittest.TestCase):
    """Execute the actual inline CI validator on synthetic CPU-only receipts."""

    files = {'QtCore', 'QtGui', 'QtWidgets', 'QtSvg', 'QtSvgWidgets', 'QtUiTools', 'QtTest',
             'QtOpenGL', 'QtOpenGLWidgets', 'libqoffscreen.dylib', 'libqsvg.dylib', 'libqsvgicon.dylib'}
    semantics = ['qt-build-runtime-version', 'cpu-offscreen-qpa', 'private-rhi-header-link',
                 'legacy-widget-link-only', 'legacy-opengl-link-only', 'svg-widget-link-only',
                 'form-buffer-open', 'uitools-form-load', 'uitools-properties', 'test-signal-spy-valid',
                 'test-signal-observed', 'svg-document', 'svg-cpu-pixel', 'svg-buffer-open',
                 'svg-image-plugin-pixel', 'private-asset-directory', 'svg-icon-write',
                 'svg-icon-plugin-pixel', 'native-loader-map-readable', 'loaded-qt-paths-private']

    @staticmethod
    def script():
        workflow = (inventory.REPO_ROOT / '.github/workflows/renderer.yml').read_text()
        block = workflow.split('- name: Require native macOS Qt modules without a graphics device', 1)[1].split('- name:', 1)[0]
        return textwrap.dedent(block.split("python3 -B - <<'PY'\n", 1)[1].rsplit('\n          PY', 1)[0])

    @staticmethod
    def change_json(root, name, change):
        path = root / name
        value = json.loads(path.read_text())
        change(value)
        path.write_text(json.dumps(value))

    def execute(self, change=None, machine='arm64', system='Darwin', translation_status=0, translation_value=0, translation_errno=0):
        script = self.script()
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp).resolve()
            prefix, compare = root / 'sdk', root / 'compare'
            build = root / 'build-rhi/provider-modules'
            build.mkdir(parents=True)
            pin = inventory.read_json(inventory.REPO_ROOT / 'lcrenderer/rhi/providers/qt683-macos.json')
            for name in pin['reference_files_sha256']:
                data = ('fixture ' + name).encode()
                pin['reference_files_sha256'][name] = inventory.sha256(data)
                for sdk in (prefix, compare):
                    path = sdk / name
                    path.parent.mkdir(parents=True, exist_ok=True)
                    path.write_bytes(data)
            pin_path = root / 'lcrenderer/rhi/providers/qt683-macos.json'
            pin_path.parent.mkdir(parents=True)
            pin_path.write_text(json.dumps(pin))
            helper = root / 'scripts/restore_renderer_qt683_macos.py'
            helper.parent.mkdir()
            helper.write_bytes(b'synthetic tool; never executed')
            for sdk, name in ((prefix, 'provider-restore.json'), (compare, 'provider-compare-restore.json')):
                value = dict(destination=str(sdk), qt_version='6.8.3', qsb_version='qsb 6.8.3',
                             pin_sha256=inventory.sha256(pin_path.read_bytes()),
                             restore_helper_sha256=inventory.sha256(helper.read_bytes()),
                             references=pin['reference_files_sha256'],
                             archives={p['module'] + '.7z': p['sha256'] for p in pin['archives']})
                (root / 'build-rhi' / name).write_text(json.dumps(value))
            names = ['r0-qt-required-modules', 'r0-qt-provider-negatives']
            (build / 'discovery.json').write_text(json.dumps(dict(tests=[dict(name=n) for n in names])))
            (build / 'results.xml').write_text('<testsuite>' + ''.join('<testcase name="' + n + '" status="run"/>' for n in names) + '</testsuite>')
            paths = [str(prefix / name) for name in pin['reference_files_sha256'] if Path(name).name in self.files]
            self.assertEqual(12, len(paths))
            report = dict(schema_version=1, evidence_kind='r0-macos-qt-modules', architecture='arm64',
                          qt_build_version='6.8.3', qt_runtime_version='6.8.3', qpa='offscreen',
                          qt_prefix=str(prefix), plugin_directory=str(prefix / 'plugins'),
                          application_created_graphics_device=False, legacy_context_qualified=False,
                          checks=self.semantics + sorted(self.files), loaded_qt_paths=paths)
            (build / 'modules.json').write_text(json.dumps(report))
            (build / 'lcrhiproviderprobe').write_bytes(b'synthetic producer; never executed')
            if change:
                change(root)
            previous = Path.cwd()
            os.chdir(root)
            try:
                def sysctl(name, value, size, unused, length):
                    self.assertEqual(b'sysctl.proc_translated', name)
                    value._obj.value = translation_value
                    return translation_status
                native = Mock(sysctlbyname=Mock(side_effect=sysctl))
                with patch.dict(os.environ, {'EXPECTED_RENDERER_ARCH': 'arm64', 'QT_ROOT_DIR': str(prefix),
                                             'QT_COMPARE_ROOT_DIR': str(compare)}), \
                        patch('platform.machine', return_value=machine), patch('platform.system', return_value=system), \
                        patch('ctypes.CDLL', return_value=native), patch('ctypes.get_errno', return_value=translation_errno):
                    try:
                        exec(compile(script, '<renderer.yml Mac module gate>', 'exec'), {'__name__': '__main__'})
                    except (Exception, SystemExit):
                        self.assertFalse((build / 'module-identity.json').exists())
                        raise
                return json.loads((build / 'module-identity.json').read_text())
            finally:
                os.chdir(previous)

    def reject(self, change=None, **kwargs):
        with self.assertRaises((SystemExit, ValueError, KeyError, TypeError, OSError)):
            self.execute(change, **kwargs)

    def test_accepts_exact_cpu_module_receipt_without_executing_native_tools(self):
        result = self.execute()
        self.assertEqual(32, result['checks'])
        self.assertEqual('arm64', result['architecture'])
        self.assertEqual(12, len(result['loaded_qt_files']))
        self.assertIn('not graphics/platform/release acceptance', result['scope'])

    def test_rejects_non_native_host(self):
        self.reject(machine='x86_64')
        self.reject(system='Linux')
        self.reject(translation_value=1)
        self.reject(translation_status=-1, translation_errno=1)
        self.assertEqual(32, self.execute(translation_status=-1, translation_errno=2)['checks'])

    def test_rejects_missing_duplicate_unexpected_failed_or_skipped_ctests(self):
        for tests in ([], [{'name': 'other'}], [{'name': 'r0-qt-required-modules'}] * 2):
            with self.subTest(discovery=tests):
                self.reject(lambda root: self.change_json(root, 'build-rhi/provider-modules/discovery.json', lambda x: x.update(tests=tests)))
        for detail in ('<skipped/>', '<failure/>', '<error/>', ''):
            status = 'notrun' if not detail else 'run'
            xml = '<testsuite><testcase name="r0-qt-required-modules" status="' + status + '">' + detail + '</testcase><testcase name="r0-qt-provider-negatives" status="run"/></testsuite>'
            with self.subTest(xml=xml):
                self.reject(lambda root: (root / 'build-rhi/provider-modules/results.xml').write_text(xml))

    def test_rejects_wrong_producer_architecture_scope_checks_or_loader_paths(self):
        cases = {'architecture': 'x86_64', 'evidence_kind': 'r0-linux-qt-modules', 'schema_version': True,
                 'qt_runtime_version': '6.12.0', 'qpa': 'cocoa', 'application_created_graphics_device': True,
                 'legacy_context_qualified': True, 'checks': ['synthetic'] * 32, 'loaded_qt_paths': []}
        for key, value in cases.items():
            with self.subTest(field=key):
                self.reject(lambda root: self.change_json(root, 'build-rhi/provider-modules/modules.json', lambda x: x.update({key: value})))
        self.reject(lambda root: self.change_json(root, 'build-rhi/provider-modules/modules.json',
                    lambda x: x['loaded_qt_paths'].__setitem__(0, x['loaded_qt_paths'][0].replace('/sdk/', '/compare/'))))
        self.reject(lambda root: self.change_json(root, 'build-rhi/provider-modules/modules.json',
                    lambda x: x['checks'].__setitem__(0, 'unknown-check')))

    def test_rejects_stale_restoration_and_late_sdk_changes(self):
        for name in ('provider-restore.json', 'provider-compare-restore.json'):
            for key, value in (('pin_sha256', 'a' * 64), ('restore_helper_sha256', 'a' * 64), ('qsb_version', 'qsb 6.12.0'), ('archives', {})):
                with self.subTest(restore=name, field=key):
                    self.reject(lambda root: self.change_json(root, 'build-rhi/' + name, lambda x: x.update({key: value})))
        self.reject(lambda root: (root / 'sdk/bin/qsb').write_bytes(b'late corruption'))

    def test_ci_gates_are_mac_only_and_never_upload_sdk_payloads(self):
        workflow = (inventory.REPO_ROOT / '.github/workflows/renderer.yml').read_text()
        block = workflow.split('- name: Require native macOS Qt modules without a graphics device', 1)[1].split('- name:', 1)[0]
        self.assertIn("if: runner.os == 'macOS'", block)
        self.assertIn('--no-tests=error', block)
        self.assertIn('EXPECTED_RENDERER_ARCH: ${{ matrix.cmake_arch }}', block)
        upload = workflow.split('- name: Retain provider/build/CPU evidence; not graphics acceptance', 1)[1]
        self.assertIn('provider-modules/module-identity.json', upload)
        self.assertIn('provider-compare-restore.json', upload)
        self.assertNotIn('provider_root', upload)
        self.assertNotIn('compare-sdk/', upload)


class CurrentLinuxProviderTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.pin = inventory.read_json(inventory.REPO_ROOT / 'lcrenderer/rhi/providers/qt6102-linux-arm64.json')

    def test_current_binary_inputs_are_pinned_without_selecting_a_minimum(self):
        pin = self.pin
        self.assertEqual('librecad-r0-provider-v1', pin['format'])
        self.assertEqual('6.10.2', pin['qt_version'])
        self.assertEqual(['arm64'], pin['payload_architectures'])
        self.assertIn('not a minimum selection', pin['scope'])
        self.assertEqual(51, len(pin['packages']))
        self.assertEqual(32346770, sum(p['bytes'] for p in pin['packages']))
        self.assertEqual(51, len({p['package'] for p in pin['packages']}))
        self.assertEqual(51, len({p['filename'] for p in pin['packages']}))
        for package in pin['packages']:
            self.assertIn(package['architecture'], ('arm64', 'all'))
            self.assertTrue(package['version'].startswith('6.10.2'))
            self.assertGreater(package['bytes'], 0)
            self.assertRegex(package['sha256'], r'^[0-9a-f]{64}$')
            self.assertEqual(Path(package['filename']).name, package['filename'])
            self.assertTrue(package['url'].startswith('https://ports.ubuntu.com/ubuntu-ports/pool/'))
            self.assertTrue(package['url'].endswith('/' + package['filename']))

    def test_required_modules_tools_and_plugins_have_reference_hashes(self):
        self.assertTrue({'Core', 'Gui', 'GuiPrivate', 'Widgets', 'Svg', 'UiTools', 'Test',
                         'OpenGL', 'OpenGLWidgets', 'ShaderTools', 'WaylandClient'}
                        <= set(self.pin['components']))
        references = self.pin['reference_files_sha256']
        self.assertEqual(19, len(references))
        self.assertEqual('de5a13ed06a4a47f985c78f6ce2bbf8418c0d3ac61352df834071cdf132a4a42',
                         references['lib/qt6/bin/qsb'])
        for name, digest in references.items():
            self.assertFalse(Path(name).is_absolute())
            self.assertNotIn('..', Path(name).parts)
            self.assertRegex(digest, r'^[0-9a-f]{64}$')
        for ending in ('libQt6UiTools.so.6.10.2', 'libQt6Test.so.6.10.2',
                       'libQt6OpenGLWidgets.so.6.10.2', 'rhi/qrhi.h',
                       'Qt6UiToolsDependencies.cmake', 'platforms/libqxcb.so',
                       'platforms/libqwayland.so', 'imageformats/libqsvg.so',
                       'iconengines/libqsvgicon.so'):
            self.assertTrue(any(name.endswith('/' + ending) for name in references), ending)

    def test_probe_keeps_cpu_and_native_qualification_boundaries_explicit(self):
        root = inventory.REPO_ROOT / 'lcrenderer/rhi/providerprobe'
        cmake = (root / 'CMakeLists.txt').read_text()
        cpp = (root / 'providerprobe.cpp').read_text()
        self.assertIn('This R0 provider probe qualifies Linux and macOS only', cmake)
        self.assertIn('r0-qt-required-modules', cmake)
        self.assertIn('r0-qt-provider-negatives', cmake)
        self.assertIn('r0-linux-qt-modules', cpp)
        self.assertIn('application_created_graphics_device', cpp)
        self.assertIn('legacy_context_qualified', cpp)
        self.assertIn('loaded-qt-paths-private', cpp)
        self.assertNotIn('QRhi::create(', cpp)
        self.assertNotIn('->show(', cpp)
        for name in ('CMakeLists.txt', 'providerprobe.cpp', 'check_config.py', 'README.md'):
            path = root / name
            result = subprocess.run(['git', 'check-ignore', str(path)],
                                    cwd=inventory.REPO_ROOT, capture_output=True, check=False)
            self.assertEqual(1, result.returncode, str(path))
        # These are CPU drift guards. Actual module/configuration proof is native CTest.

    def test_ci_uses_private_restore_only_for_the_qualified_arm64_tuple(self):
        workflow = (inventory.REPO_ROOT / '.github/workflows/renderer.yml').read_text()
        restore = workflow.split('- name: Cold restore pinned ARM64 Qt modules', 1)[1].split('- name:', 1)[0]
        modules = workflow.split('- name: Require restored ARM64 Qt modules', 1)[1].split('- name:', 1)[0]
        for block in (restore, modules):
            self.assertIn("if: matrix.arch == 'arm64'", block)
        self.assertIn('scripts/restore_renderer_qt6102_linux_arm64.py', restore)
        self.assertIn('r0-qt-required-modules', modules)
        self.assertIn('r0-qt-provider-negatives', modules)
        self.assertIn("('failure', 'error', 'skipped')", modules)
        self.assertIn('QT_ROOT_DIR: /usr', workflow)
        self.assertIn('-DCMAKE_PREFIX_PATH="$QT_ROOT_DIR"', workflow)
        self.assertIn('build-rhi/provider-restore.json', workflow)
        self.assertIn('build-rhi/provider-modules/results.xml', workflow)
        self.assertIn('scripts.tests.test_restore_renderer_qt6102_linux_arm64', workflow)


class RendererInventoryTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.path = self.root / "report.json"
        self.fixture = inventory.read_json(inventory.FIXTURE_PATH)
        self.rgba = bytearray(bytes((16, 24, 32, 255)) * (256 * 192))
        for point in self.fixture["landmarks"]:
            offset = (point["y"] * 256 + point["x"]) * 4
            self.rgba[offset:offset + 4] = bytes(point["rgba"])
        self.output = self.write_image("frame.png", bytes(self.rgba), "view-0")
        manifest = inventory.source_inputs()
        self.report = {
            "schema_version": 3, "evidence_kind": "r0-spike", "fixture": self.fixture["id"],
            "build": {
                "source_revision": "a" * 40, "source_dirty": True, "source_diff_sha256": "a" * 64,
                "source_inputs": manifest, "source_inputs_sha256": inventory.sha256(manifest.encode()),
                "qt_version": "6.8.3", "compile_qt_version": "6.8.3", "qsb_version": "qsb 6.8.3",
                "qsb_sha256": "b" * 64, "fixture_sha256": inventory.sha256(inventory.FIXTURE_PATH.read_bytes()),
                "shaders": {"scene.vert.qsb": "c" * 64, "scene.frag.qsb": "d" * 64},
                "compiler": "test compiler", "configuration": "Release", "generator": "test generator",
                "deployment_target": "", "target_arch": "arm64",
                "options": {"standalone_spike": True, "production_rhi": False, "widget": True}},
            "runtime": {
                "requested_api": "metal", "actual_api": "metal", "mode": "offscreen",
                "background": "opaque", "scenario": "standard",
                "device_name": "Test integrated device", "device_class": "hardware", "qt_version": "6.8.3",
                "architecture": "arm64", "os": "macos", "qpa": "cocoa", "format": "RGBA8", "sample_count": 1,
                "alpha": "opaque-premultiplied-blending", "color_space": "SDR-unorm-no-sRGB-transfer",
                "capabilities": {"min_texture_size": 1, "max_texture_size": 16384, "max_uniform_buffer_range": 16384,
                                 "rgba8": True, "rgba8_render_target": True, "sample_counts": [1, 4]},
                "vendor_id": "0", "device_id": "0", "driver_version": None, "metadata_note": "Not exposed by QRhi",
                "views": [{"id": "view-0", "dpr": 1, "rendered_frames": 1, "submitted_frames": 1,
                           "readback_frame": 1, "last_submitted_frame": 1,
                           "exposed_rendered_frames": 0, "owner_generation": 1, "exposed": False}]},
            "outputs": [self.output]}

    def write_image(self, name, rgba, ident, alpha="opaque"):
        image = png(256, 192, rgba)
        (self.root / name).write_bytes(image)
        (self.root / (name + ".rgba")).write_bytes(rgba)
        return {"id": ident, "image": name, "image_sha256": inventory.sha256(image),
                "rgba": name + ".rgba", "rgba_sha256": inventory.sha256(rgba),
                "width": 256, "height": 192, "stride": 1024, "channels": "RGBA", "origin": "top-left", "alpha": alpha}

    def validate(self, device="hardware", mode="offscreen", **kwargs):
        self.path.write_text(json.dumps(self.report), encoding="utf-8")
        return inventory.validate_report(self.path, "metal", device, mode, **kwargs)

    def test_valid_report_with_unavailable_driver_ids(self):
        self.assertEqual(len(self.fixture["landmarks"]), len(self.validate()))

    def test_painter_register_covers_the_actual_interface(self):
        root = inventory.SPIKE_ROOT.parents[2]
        header = (root / "lcviewernoqt/painters/lcpainter.h").read_text()
        header = re.sub(r"/\*.*?\*/|//[^\n]*", "", header, flags=re.S)
        declarations = re.findall(r"\bvirtual\s+[^;]*?\b(\w+)\s*\([^;]*\)\s*(?:const\s*)?=\s*0;", header)
        plan = (root / "VULKAN_RENDERING_MIGRATION_PLAN.md").read_text()
        register = plan.split("### Complete painter method register\n", 1)[1].split("\n## 4.", 1)[0]
        mapped = set(re.findall(r"`(\w+)`", register))
        self.assertEqual(set(declarations), mapped)
        expected = int(re.search(r"has (\d+) active pure-virtual declarations", plan)[1])
        self.assertEqual(expected, len(declarations))

    def test_every_spike_source_input_is_visible_to_git(self):
        root = inventory.SPIKE_ROOT.parents[2]
        paths = [(inventory.SPIKE_ROOT / line[66:]).relative_to(root).as_posix()
                 for line in inventory.source_inputs().splitlines()]
        result = subprocess.run(['git', 'check-ignore', '--no-index', '--stdin'], cwd=root,
                                input='\n'.join(paths) + '\n', text=True, capture_output=True)
        self.assertEqual(1, result.returncode,
                         'Required source inputs would be missing from a clean candidate: ' + result.stdout + result.stderr)
        self.assertEqual('', result.stdout)

    def test_backend_and_mode_mismatch_fail(self):
        for key, value in (("requested_api", "auto"), ("actual_api", "null"), ("mode", "widget")):
            with self.subTest(key=key), self.assertRaises(ValueError):
                original = self.report["runtime"][key]
                self.report["runtime"][key] = value
                try:
                    self.validate()
                finally:
                    self.report["runtime"][key] = original

    def test_unknown_device_cannot_pass_hardware_or_software(self):
        self.report["runtime"]["device_class"] = "unknown"
        for kind in ("hardware", "software"):
            with self.subTest(kind=kind), self.assertRaisesRegex(ValueError, "device"):
                self.validate(device=kind)
        self.validate(device="any")

    def test_software_cannot_pass_hardware(self):
        self.report["runtime"]["device_class"] = "software"
        with self.assertRaises(ValueError):
            self.validate()
        self.validate(device="software")

    def test_wrong_revision_and_architecture(self):
        for expected in ({"expected_revision": "b" * 40}, {"expected_arch": "x86_64"}):
            with self.subTest(expected=expected), self.assertRaises(ValueError):
                self.validate(**expected)

    def test_requested_dpr_is_checked_against_observed_view(self):
        self.validate(expected_dpr=1)
        with self.assertRaisesRegex(ValueError, "observed DPR"):
            self.validate(expected_dpr=1.25)

    def test_requested_qpa_is_checked_against_observed_session(self):
        self.validate(expected_qpa="cocoa")
        with self.assertRaisesRegex(ValueError, "observed QPA"):
            self.validate(expected_qpa="xcb")

    def test_runtime_build_architecture_must_agree(self):
        self.report["runtime"]["architecture"] = "x86_64"
        with self.assertRaisesRegex(ValueError, "architecture"):
            self.validate()

    def test_stale_build_with_valid_internal_manifest_hash_fails(self):
        manifest = self.report["build"]["source_inputs"].replace("scene.cpp", "deleted.cpp")
        self.report["build"].update(source_inputs=manifest, source_inputs_sha256=inventory.sha256(manifest.encode()))
        with self.assertRaisesRegex(ValueError, "stale or incomplete"):
            self.validate()

    def test_boolean_sample_count_is_not_a_capability(self):
        self.report["runtime"]["capabilities"]["sample_counts"] = [True]
        with self.assertRaisesRegex(ValueError, "capabilities"):
            self.validate()

    def test_required_target_uniform_and_texture_capabilities_fail_closed(self):
        caps = self.report["runtime"]["capabilities"]
        for key, value in (("rgba8_render_target", False), ("rgba8_render_target", 1),
                           ("max_uniform_buffer_range", 63), ("max_uniform_buffer_range", True),
                           ("min_texture_size", 32), ("min_texture_size", None), ("max_texture_size", 255)):
            original = caps[key]
            caps[key] = value
            with self.subTest(key=key, value=value), self.assertRaises(ValueError):
                self.validate()
            caps[key] = original

    def test_pixels_cannot_exceed_the_reported_device_limit(self):
        rgba = bytes((16, 24, 32, 255)) * (257 * 192)
        self.report["outputs"] = [self.write_image("oversized.png", rgba, "view-0")]
        # Supply coherent PNG/sidecar dimensions to isolate the capability check.
        image = png(257, 192, rgba)
        (self.root / "oversized.png").write_bytes(image)
        self.report["outputs"][0].update(width=257, stride=257 * 4, image_sha256=inventory.sha256(image))
        self.report["runtime"]["capabilities"]["max_texture_size"] = 256
        with self.assertRaisesRegex(ValueError, "texture limits"):
            self.validate()

    def test_provider_shader_fixture_and_source_digest_changes_fail(self):
        for key, value in (("qt_version", "6.7.0"), ("qsb_version", "qsb 6.9.0"),
                           ("fixture_sha256", "0" * 64), ("source_inputs_sha256", "0" * 64),
                           ("shaders", {}), ("source_dirty", "true")):
            original = self.report["build"][key]
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.report["build"][key] = value
                try:
                    self.validate()
                finally:
                    self.report["build"][key] = original

    def test_legacy_agl_compatibility_is_only_qualified_for_old_mac_metal(self):
        options = self.report["build"]["options"]
        options["legacy_agl_link_workaround"] = True
        self.validate()
        for value in (1, "true"):
            options["legacy_agl_link_workaround"] = value
            with self.assertRaisesRegex(ValueError, "AGL compatibility"):
                self.validate()
        options["legacy_agl_link_workaround"] = True
        self.report["runtime"]["os"] = "linux"
        with self.assertRaisesRegex(ValueError, "AGL compatibility"):
            self.validate()

    def test_cmake_compatibility_module_is_in_the_source_input_manifest(self):
        self.assertIn("  cmake/LegacyQtMacSdk.cmake\n", inventory.source_inputs())

    def test_legacy_agl_cmake_fix_preserves_every_other_dependency(self):
        module = inventory.SPIKE_ROOT / "cmake/LegacyQtMacSdk.cmake"
        cases = [(True, "6.8.3", "false", True, True), (True, "6.9.3", "false", True, True),
                 (True, "6.8.3", "true", True, False), (False, "6.8.3", "false", True, False),
                 (True, "6.10.0", "false", True, False), (True, "6.8.3", "false", False, False)]
        for index, (apple, version, available, target, changed) in enumerate(cases):
            root = self.root / ("cmake-" + str(index))
            root.mkdir()
            links = "-framework OpenGL;-framework AGL;/System/Library/Frameworks/AGL.framework;-framework Metal;keep-other-library"
            expected = "-framework OpenGL;-framework Metal;keep-other-library" if changed else links
            source = 'cmake_minimum_required(VERSION 3.21)\nproject(AGLContract NONE)\n'
            source += 'set(APPLE {})\ninclude("{}")\n'.format("true" if apple else "false", module.as_posix())
            if target:
                source += ('add_library(WrapOpenGL::WrapOpenGL INTERFACE IMPORTED)\n'
                           'set_property(TARGET WrapOpenGL::WrapOpenGL PROPERTY INTERFACE_LINK_LIBRARIES "{}")\n').format(links)
            source += 'lc_spike_legacy_qt_agl("{}" "{}" modified)\n'.format(version, available)
            source += 'if(NOT modified STREQUAL "{}")\nmessage(FATAL_ERROR "Wrong compatibility decision")\nendif()\n'.format("true" if changed else "false")
            if target:
                source += ('get_target_property(actual WrapOpenGL::WrapOpenGL INTERFACE_LINK_LIBRARIES)\n'
                           'if(NOT actual STREQUAL "{}")\nmessage(FATAL_ERROR "Dependencies changed incorrectly")\nendif()\n').format(expected)
            (root / "CMakeLists.txt").write_text(source, encoding="utf-8")
            result = subprocess.run(["cmake", "-S", str(root), "-B", str(root / "build")],
                                    capture_output=True, text=True, timeout=30)
            with self.subTest(index=index):
                self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_schema_and_record_kinds_fail_closed(self):
        for key, value in (("schema_version", 1), ("schema_version", 2), ("schema_version", 4), ("schema_version", True), ("evidence_kind", "package"),
                           ("build", []), ("outputs", [])):
            original = self.report[key]
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.report[key] = value
                try:
                    self.validate()
                finally:
                    self.report[key] = original

    def test_duplicate_json_fields_rejected(self):
        self.path.write_text('{"schema_version":1,"schema_version":1}')
        with self.assertRaisesRegex(ValueError, "duplicate"):
            inventory.read_json(self.path)

    def test_nan_rejected(self):
        self.path.write_text('{"dpr":NaN}')
        with self.assertRaisesRegex(ValueError, "nonfinite"):
            inventory.read_json(self.path)

    def test_missing_zero_and_bool_frame_counts_rejected(self):
        for value in (0, True, None):
            with self.subTest(value=value), self.assertRaises(ValueError):
                self.report["runtime"]["views"][0]["rendered_frames"] = value
                self.validate()

    def test_incomplete_readback_rejected(self):
        self.report["runtime"]["views"][0]["readback_frame"] = 2
        with self.assertRaises(ValueError):
            self.validate()

    def test_hash_mismatch_rejected(self):
        self.report["outputs"][0]["image_sha256"] = "0" * 64
        with self.assertRaisesRegex(ValueError, "hash"):
            self.validate()

    def test_png_raw_mismatch_rejected(self):
        self.rgba[0] ^= 255
        (self.root / "frame.png.rgba").write_bytes(self.rgba)
        self.report["outputs"][0]["rgba_sha256"] = inventory.sha256(self.rgba)
        with self.assertRaisesRegex(ValueError, "content mismatch"):
            self.validate()

    def test_blank_and_each_missing_landmark_rejected(self):
        for point in self.fixture["landmarks"]:
            with self.subTest(point=point["id"]), self.assertRaisesRegex(ValueError, "landmark"):
                rgba = self.rgba.copy()
                offset = (point["y"] * 256 + point["x"]) * 4
                rgba[offset:offset + 4] = bytes((127, 127, 127, 255))
                self.report["outputs"] = [self.write_image("bad.png", bytes(rgba), "view-0")]
                self.validate()
        self.report["outputs"] = [self.write_image("blank.png", bytes(256 * 192 * 4), "view-0")]
        with self.assertRaisesRegex(ValueError, "landmark|nonopaque"):
            self.validate()

    def make_transparent_report(self):
        self.report["runtime"].update(background="transparent", alpha="premultiplied-blending-straight-readback")
        self.rgba = bytearray(256 * 192 * 4)
        for point in self.fixture["landmarks"]:
            offset = (point["y"] * 256 + point["x"]) * 4
            self.rgba[offset:offset + 4] = bytes(point.get("transparent_rgba", point["rgba"]))
        self.report["outputs"] = [self.write_image(output["image"], bytes(self.rgba), output["id"], "straight")
                                  if output["id"] != "composition" else output for output in self.report["outputs"]]

    def test_transparent_readback_is_straight_alpha(self):
        self.make_transparent_report()
        self.assertEqual(len(self.fixture["landmarks"]), len(self.validate(background="transparent")))
        with self.assertRaisesRegex(ValueError, "background"):
            self.validate()

    def test_premultiplied_or_double_unpremultiplied_png_fails(self):
        self.make_transparent_report()
        for ident, bad in (("half-alpha", (128, 0, 0, 128)), ("stacked-alpha", (113, 0, 226, 192))):
            point = next(point for point in self.fixture["landmarks"] if point["id"] == ident)
            rgba = self.rgba.copy()
            offset = (point["y"] * 256 + point["x"]) * 4
            rgba[offset:offset + 4] = bytes(bad)
            self.report["outputs"] = [self.write_image("alpha-bad.png", bytes(rgba), "view-0", "straight")]
            with self.subTest(ident=ident), self.assertRaisesRegex(ValueError, "landmark"):
                self.validate(background="transparent")

    def test_opaque_output_checks_all_alpha_not_just_landmarks(self):
        self.rgba[3] = 128
        self.report["outputs"] = [self.write_image("opaque-bad.png", bytes(self.rgba), "view-0")]
        with self.assertRaisesRegex(ValueError, "nonopaque"):
            self.validate()

    def make_owned_lifecycle_report(self):
        runtime = self.report["runtime"]
        runtime["scenario"] = "lifecycle"
        base = runtime["views"][0]
        runtime["views"] = []
        self.report["outputs"] = []
        generations = []
        for index in range(3):
            view = copy.deepcopy(base)
            view.update(id="view-" + str(index), owner_generation=index + 1)
            runtime["views"].append(view)
            self.report["outputs"].append(self.write_image("owned-{}.png".format(index), bytes(self.rgba), view["id"]))
            generations.append({"generation": index + 1, "actual_api": "metal", "device_name": "Test device",
                                "device_class": "hardware", "readback_completed": True, "owner_destroyed": True})
        runtime["lifecycle"] = {"scope": "owned-QRhi-teardown-recreate", "owned_rhi_creations": 3,
                                "owned_rhi_destructions": 3, "generations": generations}

    def test_each_owned_recreated_device_has_pixels_and_teardown(self):
        self.make_owned_lifecycle_report()
        self.assertEqual(3 * len(self.fixture["landmarks"]) + 2, len(self.validate(scenario="lifecycle")))

    def test_owned_lifecycle_incomplete_generation_fails(self):
        self.make_owned_lifecycle_report()
        original = copy.deepcopy(self.report)
        mutations = [lambda: self.report["runtime"]["lifecycle"].update(owned_rhi_destructions=2),
                     lambda: self.report["runtime"]["lifecycle"]["generations"][1].update(readback_completed=False),
                     lambda: self.report["runtime"]["lifecycle"]["generations"][0].update(generation=True),
                     lambda: self.report["runtime"]["lifecycle"]["generations"][2].update(owner_destroyed=False),
                     lambda: self.report["runtime"]["lifecycle"]["generations"][2].update(actual_api="opengl"),
                     lambda: self.report["runtime"]["views"][1].update(owner_generation=1)]
        for mutation in mutations:
            self.report = copy.deepcopy(original)
            mutation()
            with self.assertRaises(ValueError):
                self.validate(scenario="lifecycle")

    def test_scenario_request_cannot_be_ignored(self):
        with self.assertRaisesRegex(ValueError, "scenario"):
            self.validate(scenario="lifecycle")

    def make_recovery_report(self):
        runtime = self.report["runtime"]
        runtime["scenario"] = "recovery"
        base = runtime["views"][0]
        runtime["views"], self.report["outputs"], faults = [], [], []
        for index, point in enumerate(("device-create", "texture-create", "begin-frame", "submit")):
            ident = "view-" + str(index)
            view = copy.deepcopy(base)
            view.update(id=ident, owner_generation=index * 2 + 2)
            runtime["views"].append(view)
            self.report["outputs"].append(self.write_image("recovered-{}.png".format(index), bytes(self.rgba), ident))
            submitted, created = point == "submit", point != "device-create"
            failed = {"owner_generation": index * 2 + 1, "owned_rhi_creations": int(created),
                "owned_rhi_destructions": int(created), "actual_api": "metal" if created else "",
                "device_name": "Test device" if created else "", "device_class": "hardware" if created else "",
                "frame_started": submitted, "readback_completed": submitted, "image_accepted": False,
                "actual_submit_result": 0 if submitted else -1, "effective_submit_result": 3 if submitted else -1}
            recovered = {"owner_generation": index * 2 + 2, "owned_rhi_creations": 1, "owned_rhi_destructions": 1,
                "actual_api": "metal", "device_name": "Test device", "device_class": "hardware",
                "frame_started": True, "readback_completed": True, "image_accepted": True,
                "actual_submit_result": 0, "effective_submit_result": 0}
            faults.append({"fault": point, "handled": True, "failed_attempt": failed,
                           "recovered_attempt": recovered, "output_id": ident})
        runtime["recovery"] = {"scope": "injected-host-failures-and-owned-device-recreation",
            "injection_method": "host-result-substitution", "physical_device_loss_observed": False,
            "owned_rhi_creations": 7, "owned_rhi_destructions": 7, "faults": faults}

    def test_injected_failures_require_new_owned_devices_and_pixels(self):
        self.make_recovery_report()
        self.assertEqual(4 * len(self.fixture["landmarks"]) + 6, len(self.validate(scenario="recovery")))

    def test_recovery_cannot_claim_physical_loss_or_leak_devices(self):
        self.make_recovery_report()
        recovery = self.report["runtime"]["recovery"]
        for key, value in (("physical_device_loss_observed", True), ("owned_rhi_creations", 8),
                           ("owned_rhi_destructions", 6), ("injection_method", "driver-reset"), ("faults", [])):
            original = recovery[key]
            recovery[key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.validate(scenario="recovery")
            recovery[key] = original

    def test_recovery_requires_each_failed_attempt_to_reject_pixels_and_clean_up(self):
        self.make_recovery_report()
        original = copy.deepcopy(self.report)
        for key, value in (("image_accepted", True), ("owned_rhi_destructions", 0), ("owner_generation", True),
                           ("frame_started", True), ("readback_completed", True), ("actual_api", "opengl")):
            self.report = copy.deepcopy(original)
            self.report["runtime"]["recovery"]["faults"][1]["failed_attempt"][key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.validate(scenario="recovery")

    def test_submit_injection_must_follow_real_completed_submission(self):
        self.make_recovery_report()
        original = copy.deepcopy(self.report)
        for key, value in (("actual_submit_result", 3), ("effective_submit_result", 0),
                           ("readback_completed", False), ("frame_started", False)):
            self.report = copy.deepcopy(original)
            self.report["runtime"]["recovery"]["faults"][3]["failed_attempt"][key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.validate(scenario="recovery")

    def test_every_recovery_device_must_match_the_required_class(self):
        self.make_recovery_report()
        original = copy.deepcopy(self.report)
        for index, attempt, classification in ((0, "failed_attempt", "hardware"),
                (1, "failed_attempt", "software"), (0, "recovered_attempt", "software"),
                (2, "recovered_attempt", "unknown")):
            self.report = copy.deepcopy(original)
            self.report["runtime"]["recovery"]["faults"][index][attempt]["device_class"] = classification
            with self.subTest(index=index, attempt=attempt), self.assertRaises(ValueError):
                self.validate(scenario="recovery")

    def test_recovery_output_must_belong_to_the_successful_generation(self):
        self.make_recovery_report()
        original = copy.deepcopy(self.report)
        mutations = [lambda: self.report["runtime"]["views"][2].update(owner_generation=5),
            lambda: self.report["runtime"]["recovery"]["faults"][2]["recovered_attempt"].update(readback_completed=False),
            lambda: self.report["runtime"]["recovery"]["faults"][2].update(output_id="view-0"),
            lambda: self.report["runtime"]["recovery"]["faults"][2].update(fault="texture-create"),
            lambda: self.report["runtime"]["recovery"]["faults"][2].update(handled=False)]
        for mutation in mutations:
            self.report = copy.deepcopy(original)
            mutation()
            with self.assertRaises(ValueError):
                self.validate(scenario="recovery")

    def test_mac_widget_session_guard_precedes_window_creation(self):
        widget = (inventory.SPIKE_ROOT / "widget.cpp").read_text()
        self.assertLess(widget.index("if (const auto issue = spikeGuiSessionIssue())"), widget.index("QWidget window;"))
        guard = (inventory.SPIKE_ROOT / "macossession.cpp").read_text()
        self.assertIn("CGSessionCopyCurrentDictionary", guard)
        self.assertIn("CGSSessionScreenIsLocked", guard)
        self.assertNotIn("activateIgnoringOtherApps", guard)

    def test_artifact_escape_rejected(self):
        for name in ("../outside.png", "/tmp/outside.png", "..\\outside.png"):
            with self.subTest(name=name), self.assertRaises(ValueError):
                self.report["outputs"][0]["image"] = name
                self.validate()

    def test_crc_and_truncated_png_rejected(self):
        image = (self.root / "frame.png").read_bytes()
        for bad in (image[:-1], image[:45] + b"garbage" + image[52:]):
            with self.assertRaises(ValueError):
                inventory.decode_png(bad)

    def make_widget_report(self):
        runtime = self.report["runtime"]
        runtime["mode"], runtime["resized"] = "widget", True
        runtime["views"][0]["exposed"] = True
        runtime["views"][0]["exposed_rendered_frames"] = 1
        view = copy.deepcopy(runtime["views"][0])
        view["id"] = "view-1"
        runtime["views"].append(view)
        composition = bytes((192, 32, 64, 255)) * (256 * 192)
        self.report["outputs"].extend([
            self.write_image("second.png", bytes(self.rgba), "view-1"),
            self.write_image("composition.png", composition, "composition")])

    def test_two_exposed_views_and_composition(self):
        self.make_widget_report()
        self.assertEqual(2 * len(self.fixture["landmarks"]) + 1, len(self.validate(mode="widget")))

    def make_reparent_report(self):
        self.make_widget_report()
        runtime = self.report["runtime"]
        runtime["scenario"] = "reparent"
        base = copy.deepcopy(runtime["views"][0])
        info = {key: copy.deepcopy(runtime[key]) for key in
                ("actual_api", "qt_version", "architecture", "qpa", "device_name", "device_class", "capabilities")}
        composition = self.report["outputs"][-1]
        runtime["views"], self.report["outputs"], stages = [], [], []
        for index, name in enumerate(("initial", "resumed", "detached", "returned")):
            ids = []
            for role in range(2):
                ident = "view-" + str(index * 2 + role)
                generation = max(index, 1) if role == 0 else 1
                view = copy.deepcopy(base)
                view.update(id=ident, rendered_frames=index + 1, submitted_frames=index + 1,
                    last_submitted_frame=index + 1, readback_frame=index + 1, device_generation=generation,
                    resource_releases=generation - 1, readback_device_generation=generation,
                    capture_generation=index + 1, device_info=copy.deepcopy(info))
                runtime["views"].append(view)
                ids.append(ident)
                self.report["outputs"].append(self.write_image(ident + ".png", bytes(self.rgba), ident))
            stages.append({"name": name, "view_ids": ids, "shared_device": name != "detached",
                "moving_in_primary_window": name != "detached", "survivor_device_unchanged": True})
        self.report["outputs"].append(composition)
        runtime["reparent"] = {"scope": "Qt-borrowed-device-cross-window-reparent", "stages": stages,
            "physical_device_loss_observed": False, "shutdown_failed": False, "recapture_delivery_drops": 1,
            "recapture_completed_before_invalidation": True,
            "suspensions": [{"kind": "hidden", "observed": True, "visible": False, "logical_width": 300,
                "logical_height": 250, "observed_ms": 250, "rendered_delta": 0},
                {"kind": "zero-size", "observed": True, "visible": True, "logical_width": 0,
                "logical_height": 0, "observed_ms": 250, "rendered_delta": 0}]}

    def test_reparent_proves_all_stage_pixels_and_borrowed_device_transitions(self):
        self.make_reparent_report()
        self.assertEqual(8 * len(self.fixture["landmarks"]) + 8, len(self.validate(mode="widget", scenario="reparent")))

    def test_reparent_requires_each_observed_window_device_transition(self):
        self.make_reparent_report()
        original = copy.deepcopy(self.report)
        for index, field, value in ((2, "shared_device", True), (3, "shared_device", False),
                (1, "survivor_device_unchanged", False), (2, "moving_in_primary_window", True),
                (3, "name", "detached"), (2, "view_ids", ["view-0", "view-1"])):
            self.report = copy.deepcopy(original)
            self.report["runtime"]["reparent"]["stages"][index][field] = value
            with self.subTest(index=index, field=field), self.assertRaises(ValueError):
                self.validate(mode="widget", scenario="reparent")

    def test_reparent_rejects_stale_pixels_and_missing_resource_release(self):
        self.make_reparent_report()
        original = copy.deepcopy(self.report)
        for index, field, value in ((4, "resource_releases", 0), (4, "readback_device_generation", 1),
                (6, "capture_generation", 3), (7, "readback_frame", 3), (4, "owner_generation", 2),
                (2, "device_generation", True)):
            self.report = copy.deepcopy(original)
            self.report["runtime"]["views"][index][field] = value
            with self.subTest(index=index, field=field), self.assertRaises(ValueError):
                self.validate(mode="widget", scenario="reparent")

    def test_reparent_cannot_recreate_survivor_or_reuse_the_moving_generation(self):
        self.make_reparent_report()
        original = copy.deepcopy(self.report)
        for index in (5, 6):
            self.report = copy.deepcopy(original)
            self.report["runtime"]["views"][index].update(device_generation=2, resource_releases=1,
                                                         readback_device_generation=2)
            with self.subTest(index=index), self.assertRaises(ValueError):
                self.validate(mode="widget", scenario="reparent")

    def test_suspend_requires_real_hidden_and_visible_zero_size_intervals(self):
        self.make_reparent_report()
        original = copy.deepcopy(self.report)
        for index, field, value in ((0, "visible", True), (1, "visible", False), (1, "logical_width", 1),
                (0, "logical_height", 0), (0, "observed_ms", 199), (1, "rendered_delta", 1), (1, "observed", False)):
            self.report = copy.deepcopy(original)
            self.report["runtime"]["reparent"]["suspensions"][index][field] = value
            with self.subTest(index=index, field=field), self.assertRaises(ValueError):
                self.validate(mode="widget", scenario="reparent")

    def test_every_reparent_capture_must_use_the_required_device_and_provider(self):
        self.make_reparent_report()
        original = copy.deepcopy(self.report)
        for field, value in (("actual_api", "opengl"), ("device_class", "software"),
                             ("qt_version", "6.9.0"), ("qpa", "offscreen"), ("architecture", "x86_64")):
            self.report = copy.deepcopy(original)
            self.report["runtime"]["views"][3]["device_info"][field] = value
            with self.subTest(field=field), self.assertRaises(ValueError):
                self.validate(mode="widget", scenario="reparent")

    def test_reparent_rejects_missing_stages_shutdown_failure_and_loss_overclaim(self):
        self.make_reparent_report()
        reparent = self.report["runtime"]["reparent"]
        for field, value in (("stages", []), ("suspensions", []), ("shutdown_failed", True),
                             ("physical_device_loss_observed", True), ("recapture_delivery_drops", 0),
                             ("recapture_delivery_drops", True), ("recapture_completed_before_invalidation", False)):
            original = reparent[field]
            reparent[field] = value
            with self.subTest(field=field), self.assertRaises(ValueError):
                self.validate(mode="widget", scenario="reparent")
            reparent[field] = original

    def make_widget_lifecycle_report(self):
        self.make_widget_report()
        runtime = self.report["runtime"]
        runtime["scenario"] = "lifecycle"
        runtime["views"][0]["owner_generation"] = 2
        runtime["lifecycle"] = {"scope": "widget-consumer-invalidation-and-surface-replacement",
            "consumer_invalidations": 1, "stale_deliveries_dropped": 1, "surface_replacements": 1,
            "gpu_completed_after_invalidation": 1, "gpu_pending_at_invalidation": True,
            "gpu_pending_at_destruction": False, "shutdown_failed": False, "device_loss_injected": False,
            "close_request_stage": "after-readback-recording-before-submission"}
        runtime["interactions"] = {"dispatch": "synthetic-Qt-events-with-observed-focus", "focused_view": "view-0",
            "keyboard_recipient": "view-0", "focus_observed": True,
            "target": {"mouse_presses": 1, "space_keys": 1, "context_requests": 1, "menu_actions": 1, "popup_observed": True},
            "other_view": {"mouse_presses": 0, "space_keys": 0, "context_requests": 0, "menu_actions": 0, "popup_observed": False}}

    def test_widget_lifecycle_proves_pending_invalidation_not_physical_loss(self):
        self.make_widget_lifecycle_report()
        self.assertEqual(2 * len(self.fixture["landmarks"]) + 4, len(self.validate(mode="widget", scenario="lifecycle")))

    def test_widget_lifecycle_rejects_missing_pending_dropped_or_recreated_state(self):
        self.make_widget_lifecycle_report()
        original = copy.deepcopy(self.report)
        for key, value in (("consumer_invalidations", 0), ("stale_deliveries_dropped", 0),
                           ("surface_replacements", 0), ("gpu_completed_after_invalidation", 0),
                           ("gpu_pending_at_invalidation", False), ("gpu_pending_at_destruction", 1),
                           ("shutdown_failed", True), ("device_loss_injected", True)):
            self.report = copy.deepcopy(original)
            self.report["runtime"]["lifecycle"][key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.validate(mode="widget", scenario="lifecycle")

    def test_widget_lifecycle_requires_observed_focus_visible_popup_and_routing(self):
        self.make_widget_lifecycle_report()
        original = copy.deepcopy(self.report)
        mutations = [lambda: self.report["runtime"]["interactions"].update(focus_observed=False),
                     lambda: self.report["runtime"]["interactions"].update(keyboard_recipient="view-1"),
                     lambda: self.report["runtime"]["interactions"]["target"].update(popup_observed=False),
                     lambda: self.report["runtime"]["interactions"]["other_view"].update(space_keys=1),
                     lambda: self.report["runtime"]["views"][0].update(owner_generation=1)]
        for mutation in mutations:
            self.report = copy.deepcopy(original)
            mutation()
            with self.assertRaises(ValueError):
                self.validate(mode="widget", scenario="lifecycle")

    def test_wayland_popup_requires_native_compositor_input_not_only_qt_events(self):
        self.make_widget_lifecycle_report()
        self.report["runtime"]["qpa"] = "wayland"
        with self.assertRaisesRegex(ValueError, "native compositor input"):
            self.validate(mode="widget", scenario="lifecycle")
        self.report["runtime"]["interactions"]["target"]["native_popup_seed"] = True
        self.report["runtime"]["interactions"]["popup_dispatch"] = "native-F12-key-handler"
        self.validate(mode="widget", scenario="lifecycle")

    def test_unexposed_missing_view_and_offscreen_qpa_fail_widget(self):
        self.make_widget_report()
        original = copy.deepcopy(self.report)
        mutations = [lambda: self.report["runtime"]["views"][0].update(exposed=False),
                     lambda: self.report["runtime"].update(qpa="offscreen"),
                     lambda: self.report["runtime"].update(qpa="minimal"),
                     lambda: self.report["runtime"].update(resized=False),
                     lambda: self.report["runtime"]["views"].pop()]
        for mutation in mutations:
            self.report = copy.deepcopy(original)
            mutation()
            with self.assertRaises(ValueError):
                self.validate(mode="widget")

    def make_failure_report(self, point):
        runtime = self.report["runtime"]
        runtime.update(mode="widget")
        runtime.pop("views", None)
        frames = 0 if point == "render" else 1
        capture = int(point != "render")
        view = {"id": "fault-view", "exposed": True, "dpr": 1, "owner_generation": 1,
                "device_generation": 1, "capture_generation": capture, "readback_device_generation": capture,
                "rendered_frames": frames, "exposed_rendered_frames": frames, "readback_frame": capture,
                "submitted_frames": int(point == "shutdown"), "last_submitted_frame": int(point == "shutdown"),
                "device_info": copy.deepcopy(runtime)}
        shutdown = point == "shutdown"
        self.report.update(evidence_kind="r0-spike-failure", outputs=[], errors=["Injected widget failure: " + point],
            failure={"requested": point, "reached": point, "injection_method": "host-terminal-result-substitution",
                     "physical_device_loss_observed": False, "view": view, "pending_at_injection": point == "readback",
                     "actual_finish_result": 0 if shutdown else -1, "effective_finish_result": 1 if shutdown else -1,
                     "readback_completed_before_drain": shutdown, "readback_completed_after_drain": shutdown,
                     "capture_completed_at_release": point != "render",
                     "retained_results": int(shutdown), "released_results": int(shutdown), "all_retired_expired": True,
                     "retirement_release_boundary": "after-QApplication-destruction", "windows_destroyed": True,
                     "qt_shutdown_complete": True, "success_report_blocked": True, "success_image_blocked": True,
                     "zero_exit_requested_after_failure": True, "terminal_latch_survived": True})

    def validate_failure(self, point="shutdown", **kwargs):
        self.path.write_text(json.dumps(self.report), encoding="utf-8")
        return inventory.validate_failure(self.path, "metal", "hardware", point, **kwargs)

    def test_expected_failures_require_real_identity_without_invented_success_views(self):
        for point in ("render", "readback", "shutdown"):
            self.make_failure_report(point)
            self.assertEqual(["expected-failure:" + point, "terminal:success-blocked", "terminal:cleanup-complete"],
                             self.validate_failure(point))
            with self.assertRaisesRegex(ValueError, "evidence kind"):
                self.validate(mode="widget")

    def test_expected_failure_rejects_unrelated_errors_or_accepted_pixels(self):
        self.make_failure_report("shutdown")
        original = copy.deepcopy(self.report)
        for key, value in (("errors", []), ("errors", ["No Vulkan driver"]),
                           ("errors", ["Injected widget failure: shutdown", "Unexpected drain failure"]),
                           ("outputs", [self.output]), ("evidence_kind", "r0-spike")):
            self.report = copy.deepcopy(original)
            self.report[key] = value
            with self.subTest(key=key, value=value), self.assertRaises(ValueError):
                self.validate_failure()

    def test_expected_failure_rejects_unreached_fault_and_unsafe_shutdown(self):
        self.make_failure_report("shutdown")
        original = copy.deepcopy(self.report)
        changes = [("reached", "render"), ("requested", "readback"), ("physical_device_loss_observed", True),
                   ("retained_results", 0), ("retained_results", True), ("released_results", 0),
                   ("retirement_release_boundary", "QRhi-cleanup-callback"), ("actual_finish_result", 1),
                   ("effective_finish_result", 0), ("pending_at_injection", True),
                   ("readback_completed_before_drain", False), ("readback_completed_after_drain", False)]
        changes.append(("capture_completed_at_release", False))
        changes += [(key, False) for key in ("windows_destroyed", "qt_shutdown_complete", "all_retired_expired",
                    "success_report_blocked", "success_image_blocked", "zero_exit_requested_after_failure", "terminal_latch_survived")]
        for key, value in changes:
            self.report = copy.deepcopy(original)
            self.report["failure"][key] = value
            with self.subTest(key=key, value=value), self.assertRaises(ValueError):
                self.validate_failure()

    def test_expected_failure_rejects_wrong_device_session_source_and_dpr(self):
        self.make_failure_report("shutdown")
        original = copy.deepcopy(self.report)
        for key, value in (("actual_api", "vulkan"), ("device_class", "software"), ("architecture", "x86_64"),
                           ("qpa", "offscreen"), ("qt_version", "6.8.2")):
            self.report = copy.deepcopy(original)
            self.report["runtime"][key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.validate_failure()
        self.report = copy.deepcopy(original)
        for required in ({"expected_revision": "b" * 40}, {"expected_arch": "x86_64"},
                         {"expected_dpr": 1.5}, {"expected_qpa": "wayland"}):
            with self.subTest(required=required), self.assertRaises(ValueError):
                self.validate_failure(**required)
        self.report["build"]["source_inputs"] += "a" * 64 + "  missing.cpp\n"
        with self.assertRaises(ValueError):
            self.validate_failure()

    def test_expected_failure_cannot_claim_unexposed_or_missing_capture(self):
        self.make_failure_report("readback")
        original = copy.deepcopy(self.report)
        for key, value in (("exposed", False), ("capture_generation", 0), ("device_generation", 0),
                           ("readback_device_generation", 0), ("readback_frame", 0), ("rendered_frames", 0)):
            self.report = copy.deepcopy(original)
            self.report["failure"]["view"][key] = value
            with self.subTest(key=key), self.assertRaises(ValueError):
                self.validate_failure("readback")


class VulkanValidationTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.directory = Path(self.temp.name)
        self.library = self.directory / 'libVkLayer_khronos_validation.so'
        self.library.write_bytes(b'CPU fixture library; never loaded')
        self.manifest = self.directory / 'VkLayer_khronos_validation.json'
        self.manifest.write_text(json.dumps({'layer': {'name': 'VK_LAYER_KHRONOS_validation',
            'type': 'GLOBAL', 'library_path': self.library.name, 'api_version': '1.4.341'}}))
        self.args = argparse.Namespace(backend='vulkan', vulkan_validation_manifest=self.manifest,
                                       vulkan_validation_library=self.library)
        with patch.object(runner.sys, 'platform', 'linux'):
            self.profile = runner.validation_inputs(self.args)
        runner.validation_environment(self.profile, self.directory, {})
        self.stdout = ('Validation Information: [ WARNING-CreateInstance-status-message ]\n'
            'vkCreateInstance(): Khronos Validation Layer Active:\n'
            '    Current Enables: VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT.\n'
            '    Current Disables: None.\n')
        self.stderr = ('123: calling init: {0}\n'
            '[Vulkan Loader] INFO | LAYER: Insert instance layer "VK_LAYER_KHRONOS_validation" ({0})\n'
            '[Vulkan Loader] INFO | LAYER: Inserted device layer "VK_LAYER_KHRONOS_validation" ({0})\n').format(self.library)

    def logs(self, stdout=None, stderr=None):
        (self.directory / 'stdout.log').write_text(self.stdout if stdout is None else stdout)
        (self.directory / 'stderr.log').write_text(self.stderr if stderr is None else stderr)

    def test_profile_requires_linux_vulkan_and_both_exact_inputs(self):
        for key, value in (('backend', 'metal'), ('vulkan_validation_manifest', None),
                           ('vulkan_validation_library', None)):
            saved = getattr(self.args, key)
            setattr(self.args, key, value)
            with self.subTest(key=key), patch.object(runner.sys, 'platform', 'linux'), self.assertRaises(ValueError):
                runner.validation_inputs(self.args)
            setattr(self.args, key, saved)
        with patch.object(runner.sys, 'platform', 'darwin'), self.assertRaises(ValueError):
            runner.validation_inputs(self.args)

    def test_foreign_manifest_or_library_cannot_select_another_layer(self):
        original = self.manifest.read_bytes()
        for key, value in (('name', 'VK_LAYER_OTHER'), ('type', 'INSTANCE'),
                           ('library_path', '/foreign/libVkLayer_khronos_validation.so')):
            data = json.loads(original)
            data['layer'][key] = value
            self.manifest.write_text(json.dumps(data))
            with self.subTest(key=key), patch.object(runner.sys, 'platform', 'linux'), self.assertRaises(ValueError):
                runner.validation_inputs(self.args)

    def test_environment_cannot_inherit_filters_disables_redirection_or_other_layers(self):
        base = {'VK_LAYER_MESSAGE_ID_FILTER': 'VUID-hide', 'VK_LAYER_DISABLES': 'ALL',
            'VK_LAYER_KHRONOS_VALIDATION_VALIDATE_CORE': 'false', 'VK_INSTANCE_LAYERS': 'OTHER',
            'VK_LOADER_LAYERS_DISABLE': '*', 'VK_DRIVER_FILES': '/forced/icd.json',
            'LD_DEBUG_OUTPUT': '/discard', 'LD_PRELOAD': '/foreign.so', 'LD_LIBRARY_PATH': '/qt',
            'QT_QPA_PLATFORM': 'wayland'}
        env = runner.validation_environment(self.profile, self.directory, base)
        self.assertEqual('VUID-hide', base['VK_LAYER_MESSAGE_ID_FILTER'])
        for key in ('VK_LAYER_MESSAGE_ID_FILTER', 'VK_LAYER_DISABLES',
                    'VK_LAYER_KHRONOS_VALIDATION_VALIDATE_CORE', 'LD_DEBUG_OUTPUT', 'LD_PRELOAD'):
            self.assertNotIn(key, env)
        self.assertEqual('VK_LAYER_KHRONOS_validation', env['VK_INSTANCE_LAYERS'])
        self.assertEqual('~implicit~', env['VK_LOADER_LAYERS_DISABLE'])
        self.assertEqual('/forced/icd.json', env['VK_DRIVER_FILES'])
        self.assertEqual('wayland', env['QT_QPA_PLATFORM'])
        self.assertEqual(str(self.library.parent.resolve()) + os.pathsep + '/qt', env['LD_LIBRARY_PATH'])

    def test_clean_profile_binds_actual_library_settings_and_both_log_streams(self):
        self.logs()
        record = runner.validate_vulkan_validation(self.profile, self.directory)
        self.assertEqual('linux-core-sync-v1', record['profile'])
        self.assertEqual((1, 1, 0), (record['instances'], record['devices'], record['errors']))
        for name, digest in record['log_sha256'].items():
            self.assertEqual(inventory.sha256((self.directory / name).read_bytes()), digest)
        self.assertEqual(inventory.sha256(runner.VALIDATION_SETTINGS.encode()), record['settings_sha256'])

    def test_layer_discovery_alone_missing_device_or_disabled_checks_never_pass(self):
        for stdout, stderr in (('', self.stderr), (self.stdout, ''),
                (self.stdout, self.stderr.replace('calling init:', 'trying file=')),
                (self.stdout, self.stderr.replace('Inserted device layer', 'Found device layer')),
                (self.stdout.replace('None.', 'VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT.'), self.stderr),
                (self.stdout.replace('SYNCHRONIZATION_VALIDATION', 'BEST_PRACTICES'), self.stderr),
                (self.stdout + self.stdout, self.stderr)):
            self.logs(stdout, stderr)
            with self.subTest(stdout=stdout, stderr=stderr), self.assertRaisesRegex(ValueError, 'Missing/foreign'):
                runner.validate_vulkan_validation(self.profile, self.directory)

    def test_each_instance_and_device_must_use_the_declared_library(self):
        for stderr in (self.stderr.replace('({})'.format(self.library), '(/foreign/layer.so)'),
                       self.stderr + self.stderr.replace('({})'.format(self.library), '(/foreign/layer.so)')):
            self.logs(self.stdout + self.stdout, stderr)
            with self.subTest(stderr=stderr), self.assertRaises((ValueError, OSError)):
                runner.validate_vulkan_validation(self.profile, self.directory)

    def test_vuid_sync_and_unassigned_errors_in_either_stream_fail(self):
        for error in ('Validation Error: [ UNASSIGNED-test ]', 'VUID-vkDestroySurfaceKHR-surface-01266',
                      'SYNC-HAZARD-READ-AFTER-WRITE', '[Vulkan Loader] ERROR | failed'):
            for stream in ('stdout', 'stderr'):
                self.logs(self.stdout + (error if stream == 'stdout' else ''),
                          self.stderr + (error if stream == 'stderr' else ''))
                with self.subTest(error=error, stream=stream), self.assertRaisesRegex(ValueError, 'validation error'):
                    runner.validate_vulkan_validation(self.profile, self.directory)

    def test_input_or_settings_changes_fail(self):
        self.logs()
        for path in (self.library, self.manifest, self.directory / 'vk_layer_settings.txt'):
            original = path.read_bytes()
            path.write_bytes(original + b'changed')
            with self.subTest(path=path), self.assertRaisesRegex(ValueError, 'changed'):
                runner.validate_vulkan_validation(self.profile, self.directory)
            path.write_bytes(original)

    def test_zero_exit_and_expected_terminal_failure_cannot_override_validation_error(self):
        self.args = argparse.Namespace(**vars(self.args), app=self.library, device='hardware', mode='widget',
            report_dir=self.directory / 'reports', timeout=1, expected_revision=None, expected_arch='arm64',
            expected_dpr=None, expected_qpa='xcb', background='opaque', scenario='standard', icd_manifest=[])
        def execute(command, **kwargs):
            report = Path(next(arg.split('=', 1)[1] for arg in command if arg.startswith('--renderer-report=')))
            self.assertEqual('VK_LAYER_KHRONOS_validation', kwargs['env']['VK_INSTANCE_LAYERS'])
            self.assertEqual(runner.VALIDATION_SETTINGS, (report.parent / 'vk_layer_settings.txt').read_text())
            return subprocess.CompletedProcess(command, 1 if self.args.expect_failure else 0,
                b'Validation Error: [ VUID-fixture-only ]', b'')
        for failure in (None, 'shutdown'):
            self.args.expect_failure = failure
            with self.subTest(failure=failure), patch.object(runner.sys, 'platform', 'linux'), \
                 patch.object(runner.subprocess, 'run', side_effect=execute), \
                 patch.object(runner, 'validate_report') as valid, self.assertRaisesRegex(ValueError, 'validation error'):
                runner.run(self.args)
            valid.assert_not_called()
        self.assertFalse(list(self.args.report_dir.rglob('checks.json')))

    def test_wayland_profile_is_applied_only_to_app_and_retained_with_success(self):
        self.args = argparse.Namespace(**vars(self.args), app=self.library, device='hardware', mode='offscreen',
            report_dir=self.directory / 'reports', timeout=1, expected_revision=None, expected_arch='arm64',
            expected_dpr=None, expected_qpa='wayland', background='opaque', scenario='standard', icd_manifest=[],
            weston=self.library, weston_prefix=None, weston_backend='x11', weston_input_tool=None)
        session = Mock()
        session.__enter__, session.__exit__ = Mock(return_value=session), Mock(return_value=False)
        session.client_env = {'WAYLAND_DISPLAY': 'private-seat', 'VK_LAYER_DISABLES': 'ALL'}
        session.env, session.files, session.record = {'VK_LAYER_DISABLES': 'ALL'}, set(), {}
        def execute(command, timeout):
            self.assertEqual('ALL', session.env['VK_LAYER_DISABLES'])
            self.assertNotIn('VK_LAYER_DISABLES', session.client_env)
            self.assertEqual('private-seat', session.client_env['WAYLAND_DISPLAY'])
            report = Path(next(arg.split('=', 1)[1] for arg in command if arg.startswith('--renderer-report=')))
            report.write_text('{}')
            return subprocess.CompletedProcess(command, 0, self.stdout.encode(), self.stderr.encode())
        session.execute.side_effect = execute
        with patch.object(runner.sys, 'platform', 'linux'), patch.object(runner, 'WestonSession', return_value=session), \
             patch.object(runner, 'validate_report', return_value=['fixture:only']):
            directory = runner.run(self.args)
        record = inventory.read_json(directory / 'checks.json')
        self.assertEqual(runner.validate_vulkan_validation(self.profile, directory), record['vulkan_validation'])


class VulkanValidationCIResultTests(unittest.TestCase):
    """Run the actual inline aggregate on CPU-only synthetic layer/log receipts."""

    def execute(self, change=None):
        workflow = (inventory.REPO_ROOT / '.github/workflows/renderer.yml').read_text()
        block = workflow.split('- name: Require all 60 clean Vulkan validation receipts', 1)[1].split('- name:', 1)[0]
        script = textwrap.dedent(block.split("python3 -B - <<'PY'\n", 1)[1].rsplit('\n          PY', 1)[0])
        fixture = VulkanValidationTests()
        fixture.setUp()
        self.addCleanup(fixture.doCleanups)
        root = fixture.directory
        (root / 'scripts').symlink_to(inventory.REPO_ROOT / 'scripts', target_is_directory=True)
        directories = []
        for qpa in ('xcb', 'wayland'):
            cases = []
            for build, mode, scenarios in (('build-rhi', 'offscreen', ('standard', 'lifecycle', 'recovery')),
                    ('build-rhi', 'widget', ('standard', 'lifecycle', 'reparent')),
                    ('build-rhi-noui', 'offscreen', ('standard', 'lifecycle', 'recovery'))):
                for bg in ('opaque', 'transparent'):
                    for scenario in scenarios:
                        cases.append((build, mode, bg, scenario, 1, None))
            for dpr in (1.25, 1.5, 2):
                for scenario in ('lifecycle', 'reparent'):
                    cases.append(('build-rhi', 'widget', 'transparent', scenario, dpr, None))
            for bg in ('opaque', 'transparent'):
                for point in ('render', 'readback', 'shutdown'):
                    cases.append(('build-rhi', 'widget', bg, 'standard', 1, point))
            self.assertEqual(30, len(cases))
            for index, (build, mode, bg, scenario, dpr, point) in enumerate(cases):
                directory = root / build / ('evidence-{}-{}'.format(qpa, index)) / 'run-fixture'
                directory.mkdir(parents=True)
                app = root / build / ('lcrhispike_' + mode)
                app.write_bytes((build + '/' + mode + ': CPU fixture only; never executed').encode())
                runner.validation_environment(fixture.profile, directory, {})
                (directory / 'stdout.log').write_text(fixture.stdout)
                (directory / 'stderr.log').write_text(fixture.stderr)
                report = {'evidence_kind': 'r0-spike-failure' if point else 'r0-spike',
                          'runtime': {'qpa': qpa, 'mode': mode, 'background': bg, 'scenario': scenario},
                          'build': {'options': {'widget': build == 'build-rhi'}}}
                if point:
                    report['failure'] = {'requested': point, 'view': {'dpr': dpr}}
                else:
                    report['runtime']['views'] = [{'dpr': dpr}]
                native = directory / ('failure.json' if point else 'app.json')
                native.write_text(json.dumps(report))
                (directory / 'checks.json').write_text(json.dumps({'runner_sha256': runner.helper_digests()['runner_sha256'],
                    'app_report': report, 'app_report_sha256': runner.sha256(native.read_bytes()),
                    'app_sha256': runner.sha256(app.read_bytes()), 'command': [str(app)],
                    'evidence_kind': 'r0-spike-expected-failure' if point else 'r0-spike-runner',
                    'vulkan_validation': runner.validate_vulkan_validation(fixture.profile, directory)}))
                directories.append(directory)
        if change:
            change(root, directories)
        env = {**os.environ, 'R0_VULKAN_VALIDATION_MANIFEST': str(fixture.manifest),
               'R0_VULKAN_VALIDATION_LIBRARY': str(fixture.library)}
        # sys.platform override permits testing the Linux-only validator's pure
        # file/log checks on macOS; no Vulkan library/executable is ever loaded.
        result = subprocess.run([sys.executable, '-B', '-c', "import sys;sys.platform='linux'\n" + script],
            cwd=root, env=env, capture_output=True, text=True)
        return result, root

    def test_exact_two_qpa_counts_pass_and_emit_all_validation_identities(self):
        result, root = self.execute()
        self.assertEqual(0, result.returncode, result.stderr)
        records = inventory.read_json(root / 'build-rhi/vulkan-validation.json')
        self.assertEqual(60, len(records))
        self.assertEqual(60, len({tuple(record['case']) for record in records}))
        for qpa in ('xcb', 'wayland'):
            cases = [record['case'] for record in records if record['case'][0] == qpa]
            self.assertEqual(24, sum(case[-1] is None for case in cases))
            self.assertEqual(6, sum(case[-1] is not None for case in cases))
            self.assertEqual(6, sum(case[1] == 'build-rhi-noui' for case in cases))
            self.assertEqual({1, 1.25, 1.5, 2}, {case[-2] for case in cases})

    def test_missing_receipt_or_wrong_expected_failure_counts_fail(self):
        for mutation in (lambda root, dirs: (dirs[0] / 'checks.json').unlink(),
                         lambda root, dirs: MacModuleCIResultTests.change_json(root,
                             str((dirs[24] / 'checks.json').relative_to(root)),
                             lambda data: data.update(evidence_kind='r0-spike-runner'))):
            result, root = self.execute(mutation)
            with self.subTest(mutation=mutation):
                self.assertNotEqual(0, result.returncode)
                self.assertFalse((root / 'build-rhi/vulkan-validation.json').exists())

    def test_duplicate_native_case_cannot_replace_required_case_with_same_counts(self):
        for index, native in ((0, 'app.json'), (24, 'failure.json')):
            def duplicate(root, directories):
                source, target = directories[index:index + 2]
                for name in ('checks.json', native):
                    (target / name).write_bytes((source / name).read_bytes())
            result, root = self.execute(duplicate)
            with self.subTest(index=index):
                self.assertNotEqual(0, result.returncode, 'Duplicate native case passed the all-60 CI aggregate')
                self.assertIn('duplicate native case', result.stderr)
                self.assertFalse((root / 'build-rhi/vulkan-validation.json').exists())

    @staticmethod
    def change_native(directory, change):
        path = directory / 'checks.json'
        evidence = json.loads(path.read_text())
        change(evidence['app_report'])
        native = directory / ('failure.json' if evidence['evidence_kind'] == 'r0-spike-expected-failure' else 'app.json')
        native.write_text(json.dumps(evidence['app_report']))
        evidence['app_report_sha256'] = runner.sha256(native.read_bytes())
        path.write_text(json.dumps(evidence))

    def test_missing_mixed_boolean_nonfinite_or_unknown_dpr_fail(self):
        for value in ([], [{'dpr': True}], [{'dpr': float('nan')}], [{'dpr': 1}, {'dpr': 1.5}], [{'dpr': 1.75}]):
            result, root = self.execute(lambda root, dirs: self.change_native(dirs[0],
                lambda report: report['runtime'].update(views=value)))
            with self.subTest(value=value):
                self.assertNotEqual(0, result.returncode)
                self.assertIn('native DPR', result.stderr)
                self.assertFalse((root / 'build-rhi/vulkan-validation.json').exists())

    def test_unexpected_scenario_background_failure_or_mixed_build_fail(self):
        changes = [(0, lambda report: report['runtime'].update(scenario='reparent')),
                   (0, lambda report: report['runtime'].update(background='unknown')),
                   (24, lambda report: report['failure'].update(requested='device-create')),
                   (0, lambda report: report['build']['options'].update(widget=False)),
                   (0, lambda report: report['build']['options'].update(widget=1)),
                   (12, lambda report: report['build']['options'].update(widget=True))]
        for index, change in changes:
            result, root = self.execute(lambda root, dirs: self.change_native(dirs[index], change))
            with self.subTest(index=index, change=change):
                self.assertNotEqual(0, result.returncode)
                self.assertFalse((root / 'build-rhi/vulkan-validation.json').exists())

    def test_changed_native_report_binary_or_foreign_command_fail(self):
        changes = [lambda root, dirs: (dirs[0] / 'app.json').write_text('{}'),
                   lambda root, dirs: (root / 'build-rhi/lcrhispike_offscreen').write_bytes(b'changed binary'),
                   lambda root, dirs: MacModuleCIResultTests.change_json(root,
                       str((dirs[0] / 'checks.json').relative_to(root)),
                       lambda evidence: evidence.update(command=[str(root / 'build-rhi/lcrhispike_widget')]))]
        for change in changes:
            result, root = self.execute(change)
            with self.subTest(change=change):
                self.assertNotEqual(0, result.returncode)
                self.assertIn('misbound native report or build', result.stderr)
                self.assertFalse((root / 'build-rhi/vulkan-validation.json').exists())

    def test_dpr_normalization_matches_existing_native_validator_tolerance(self):
        result, root = self.execute(lambda root, dirs: self.change_native(dirs[18],
            lambda report: report['runtime'].update(views=[{'dpr': 1.2505}, {'dpr': 1.25}])))
        self.assertEqual(0, result.returncode, result.stderr)
        records = inventory.read_json(root / 'build-rhi/vulkan-validation.json')
        self.assertEqual(60, len({tuple(record['case']) for record in records}))

    def test_failure_without_fault_cannot_impersonate_a_positive_tuple(self):
        def mutate(root, directories):
            path = directories[0] / 'checks.json'
            evidence = json.loads(path.read_text())
            evidence['evidence_kind'] = 'r0-spike-expected-failure'
            report = evidence['app_report']
            report.update(evidence_kind='r0-spike-failure', failure={'requested': None, 'view': {'dpr': 1}})
            native = directories[0] / 'failure.json'
            native.write_text(json.dumps(report))
            evidence['app_report_sha256'] = runner.sha256(native.read_bytes())
            path.write_text(json.dumps(evidence))
        result, root = self.execute(mutate)
        self.assertNotEqual(0, result.returncode)
        self.assertIn('Unqualified native failure point', result.stderr)
        self.assertFalse((root / 'build-rhi/vulkan-validation.json').exists())

    def test_changed_logs_or_helper_and_missing_profile_fail(self):
        changes = [lambda root, dirs: (dirs[0] / 'stdout.log').write_text('Validation Error: [ VUID-fixture ]')]
        for field, value in (('runner_sha256', '0' * 64), ('vulkan_validation', None)):
            changes.append(lambda root, dirs, field=field, value=value: MacModuleCIResultTests.change_json(root,
                str((dirs[0] / 'checks.json').relative_to(root)), lambda data: data.update({field: value})))
        for mutation in changes:
            result, root = self.execute(mutation)
            with self.subTest(mutation=mutation):
                self.assertNotEqual(0, result.returncode)
                self.assertFalse((root / 'build-rhi/vulkan-validation.json').exists())

    def test_every_linux_smoke_call_requires_the_pinned_layer_and_receipt_gate(self):
        workflow = (inventory.REPO_ROOT / '.github/workflows/renderer.yml').read_text()
        linux = workflow.split('  renderer-linux:', 1)[1].split('  minimum-qt-provider-build:', 1)[0]
        calls = linux.split('scripts/run_renderer_checks.py --spike --xvfb-run')[1:]
        self.assertEqual(7, len(calls))
        for call in calls:
            flags = call.split('--app', 1)[0]
            self.assertIn('--vulkan-validation-manifest "$R0_VULKAN_VALIDATION_MANIFEST"', flags)
            self.assertIn('--vulkan-validation-library "$R0_VULKAN_VALIDATION_LIBRARY"', flags)
        self.assertIn('vulkan-validationlayers=1.4.341.0-1', linux)
        self.assertIn('build-rhi/vulkan-validation.json', linux)
        self.assertNotIn('continue-on-error', linux)


class RendererRunnerTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        root = Path(self.temp.name)
        app = root / "spike"
        app.write_bytes(b"test executable bytes")
        self.args = argparse.Namespace(app=app, backend="metal", device="hardware", mode="offscreen",
            report_dir=root / "reports", timeout=1, expected_revision=None, expected_arch="arm64",
            expected_dpr=None, expected_qpa=None, background="opaque", scenario="standard", icd_manifest=[])

    def test_timeout_fails_and_keeps_logs(self):
        with patch.object(runner.subprocess, "run", side_effect=subprocess.TimeoutExpired("spike", 1, b"partial", b"error")):
            with self.assertRaisesRegex(ValueError, "timed out"):
                runner.run(self.args)
        self.assertEqual(b"partial", next(self.args.report_dir.rglob("stdout.log")).read_bytes())
        self.assertFalse(list(self.args.report_dir.rglob("checks.json")))

    def test_recovery_cannot_execute_the_borrowed_widget_host(self):
        self.args.mode, self.args.scenario = "widget", "recovery"
        with patch.object(runner.subprocess, "run") as execute, self.assertRaisesRegex(ValueError, "owned offscreen"):
            runner.run(self.args)
        execute.assert_not_called()

    def test_reparent_cannot_execute_the_owned_offscreen_host(self):
        self.args.mode, self.args.scenario = "offscreen", "reparent"
        with patch.object(runner.subprocess, "run") as execute, self.assertRaisesRegex(ValueError, "borrowed widget"):
            runner.run(self.args)
        execute.assert_not_called()

    def test_nonzero_exit_never_validates(self):
        result = subprocess.CompletedProcess([], 1, b"", b"driver error")
        with patch.object(runner.subprocess, "run", return_value=result), patch.object(runner, "validate_report") as validate:
            with self.assertRaisesRegex(ValueError, "driver error"):
                runner.run(self.args)
            validate.assert_not_called()

    def test_missing_report_is_not_success(self):
        result = subprocess.CompletedProcess([], 0, b"", b"")
        with patch.object(runner.subprocess, "run", return_value=result):
            with self.assertRaises(OSError):
                runner.run(self.args)

    def test_weston_requires_forced_vulkan_and_declared_wayland(self):
        self.args.weston = self.args.app
        with patch.object(runner.subprocess, "run") as execute, self.assertRaisesRegex(ValueError, "forced Vulkan"):
            runner.run(self.args)
        execute.assert_not_called()

    def test_wayland_lifecycle_cannot_silently_use_an_inputless_compositor(self):
        self.args.weston = self.args.app
        self.args.backend, self.args.expected_qpa = "vulkan", "wayland"
        self.args.mode, self.args.scenario, self.args.weston_input_tool = "widget", "lifecycle", None
        with patch.object(runner, "WestonSession") as prepare, self.assertRaisesRegex(ValueError, "input seat"):
            runner.run(self.args)
        prepare.assert_not_called()

    def test_weston_cannot_mask_a_failing_app_with_successful_server_exit(self):
        self.args.weston = self.args.app
        self.args.backend, self.args.expected_qpa = "vulkan", "wayland"
        self.args.weston_prefix, self.args.weston_backend, self.args.weston_input_tool = None, "headless", None
        session = Mock()
        session.__enter__ = Mock(return_value=session)
        session.__exit__ = Mock(return_value=False)
        session.execute.return_value = subprocess.CompletedProcess([], 7, b"", b"app failed")
        with patch.object(runner, "WestonSession", return_value=session), \
             patch.object(runner, "validate_report") as validate, self.assertRaisesRegex(ValueError, "app failed"):
            runner.run(self.args)
        validate.assert_not_called()

    def failure_execute(self, command, **kwargs):
        report = Path(next(arg.split("=", 1)[1] for arg in command if arg.startswith("--renderer-report=")))
        (report.parent / "failure.json").write_text('{}')
        return subprocess.CompletedProcess(command, 1, b"", b"injected terminal error")

    def test_expected_failure_has_separate_ids_and_no_rendered_success_claim(self):
        self.args.mode, self.args.expect_failure = "widget", "shutdown"
        with patch.object(runner.subprocess, "run", side_effect=self.failure_execute), \
             patch.object(runner, "validate_failure", return_value=["expected-failure:shutdown"]):
            directory = runner.run(self.args)
        evidence = inventory.read_json(directory / "checks.json")
        self.assertEqual("r0-spike-expected-failure", evidence["evidence_kind"])
        self.assertEqual([], evidence["checks"]["executed_smoke_ids"])
        self.assertEqual(["expected-failure:shutdown"], evidence["checks"]["expected_failure_ids"])
        self.assertIsNone(evidence["checks"]["gtest"])
        self.assertIn("--inject-failure=shutdown", evidence["command"])

    def test_expected_failure_rejects_wrong_host_scenario_and_exit(self):
        self.args.expect_failure = "render"
        with patch.object(runner.subprocess, "run") as execute, self.assertRaisesRegex(ValueError, "standard widget"):
            runner.run(self.args)
        execute.assert_not_called()
        self.args.mode, self.args.scenario = "widget", "lifecycle"
        with self.assertRaisesRegex(ValueError, "standard widget"):
            runner.run(self.args)
        self.args.scenario = "standard"
        for status in (0, 2, -11):
            with patch.object(runner.subprocess, "run", return_value=subprocess.CompletedProcess([], status, b"", b"wrong failure")), \
                 patch.object(runner, "validate_failure") as validate:
                with self.subTest(status=status), self.assertRaisesRegex(ValueError, "renderer exited"):
                    runner.run(self.args)
                validate.assert_not_called()

    def test_expected_failure_requires_diagnostic_and_rejects_success_files(self):
        self.args.mode, self.args.expect_failure = "widget", "readback"
        with patch.object(runner.subprocess, "run", return_value=subprocess.CompletedProcess([], 1, b"", b"No driver")):
            with self.assertRaises(OSError):
                runner.run(self.args)
        for name in ("app.json", "frame.png", "frame.png.rgba"):
            def execute(command, **kwargs):
                result = self.failure_execute(command)
                report = Path(next(arg.split("=", 1)[1] for arg in command if arg.startswith("--renderer-report=")))
                (report.parent / name).write_bytes(b"unexpected success")
                return result
            with patch.object(runner.subprocess, "run", side_effect=execute), \
                 patch.object(runner, "validate_failure", return_value=["expected-failure:readback"]):
                with self.subTest(name=name), self.assertRaisesRegex(ValueError, "unreported output"):
                    runner.run(self.args)

    def test_expected_failure_binds_helper_and_binary_hashes(self):
        self.args.mode, self.args.expect_failure = "widget", "render"
        with patch.object(runner.subprocess, "run", side_effect=self.failure_execute), \
             patch.object(runner, "validate_failure", return_value=["expected-failure:render"]), \
             patch.object(runner, "helper_digests", side_effect=[{"runner_sha256": "a" * 64}, {"runner_sha256": "b" * 64}]):
            with self.assertRaisesRegex(ValueError, "helper source changed"):
                runner.run(self.args)
        def mutate(command, **kwargs):
            self.args.app.write_bytes(b"changed executable")
            return self.failure_execute(command)
        with patch.object(runner.subprocess, "run", side_effect=mutate), self.assertRaisesRegex(ValueError, "executable changed"):
            runner.run(self.args)

    def test_success_binds_binary_report_and_no_gtest_claim(self):
        def execute(command, **kwargs):
            report = Path(next(arg.split("=", 1)[1] for arg in command if arg.startswith("--renderer-report=")))
            report.write_text('{}')
            return subprocess.CompletedProcess(command, 0, b"", b"")
        with patch.object(runner.subprocess, "run", side_effect=execute), \
             patch.object(runner, "validate_report", return_value=["view-0:landmark"]):
            directory = runner.run(self.args)
        report = inventory.read_json(directory / "checks.json")
        self.assertEqual(inventory.sha256(self.args.app.read_bytes()), report["app_sha256"])
        self.assertIsNone(report["checks"]["gtest"])
        self.assertEqual("r0-spike-runner", report["evidence_kind"])
        self.assertEqual(3, report["schema_version"])

    def test_unreported_failed_frame_pixels_are_not_accepted(self):
        def execute(command, **kwargs):
            report = Path(next(arg.split("=", 1)[1] for arg in command if arg.startswith("--renderer-report=")))
            report.write_text('{}')
            (report.parent / "failed-frame.png").write_bytes(b"unreported output")
            return subprocess.CompletedProcess(command, 0, b"", b"")
        with patch.object(runner.subprocess, "run", side_effect=execute), \
             patch.object(runner, "validate_report", return_value=["recovery:device-create"]):
            with self.assertRaisesRegex(ValueError, "unreported output"):
                runner.run(self.args)
        self.assertFalse(list(self.args.report_dir.rglob("checks.json")))

    def test_helper_edit_during_execution_is_rejected(self):
        def execute(command, **kwargs):
            report = Path(next(arg.split("=", 1)[1] for arg in command if arg.startswith("--renderer-report=")))
            report.write_text('{}')
            return subprocess.CompletedProcess(command, 0, b"", b"")
        with patch.object(runner.subprocess, "run", side_effect=execute), \
             patch.object(runner, "validate_report", return_value=["view-0:landmark"]), \
             patch.object(runner, "helper_digests", side_effect=[{"runner_sha256": "a" * 64}, {"runner_sha256": "b" * 64}]):
            with self.assertRaisesRegex(ValueError, "helper source changed"):
                runner.run(self.args)
        self.assertFalse(list(self.args.report_dir.rglob("checks.json")))

    def test_software_report_without_exact_forced_icd_is_rejected(self):
        def execute(command, **kwargs):
            report = Path(next(arg.split("=", 1)[1] for arg in command if arg.startswith("--renderer-report=")))
            report.write_text('{}')
            return subprocess.CompletedProcess(command, 0, b"", b"")
        self.args.device = "software"
        with patch.object(runner.subprocess, "run", side_effect=execute), \
             patch.object(runner, "validate_report", return_value=["view-0:landmark"]), \
             patch.dict(runner.os.environ, {"VK_DRIVER_FILES": "", "VK_ICD_FILENAMES": ""}):
            with self.assertRaisesRegex(ValueError, "exactly the declared ICD"):
                runner.run(self.args)

    def test_workflow_actions_are_immutable_and_scope_is_explicit(self):
        workflow = (inventory.SPIKE_ROOT.parents[2] / ".github/workflows/renderer.yml").read_text()
        uses = re.findall(r"uses:\s*([^\s]+)", workflow)
        self.assertTrue(uses)
        for action in uses:
            if action == './.github/workflows/renderer-source.yml':
                # Same-checkout workflow code is bound to the caller commit.
                # Allow only this reviewed path, not arbitrary local/remote refs.
                target = inventory.REPO_ROOT / action[2:]
                self.assertTrue(target.is_file())
                self.assertFalse(target.is_symlink())
                for remote in re.findall(r"uses:\s*([^\s]+)", target.read_text()):
                    self.assertRegex(remote, r"@[0-9a-f]{40}$")
            else:
                self.assertRegex(action, r"@[0-9a-f]{40}$")
        self.assertIn("R0 scaffolding (not migration acceptance)", workflow)
        self.assertIn("if: always()\n    needs:", workflow)
        self.assertNotIn("assert results", workflow)
        self.assertIn("raise SystemExit", workflow)


if __name__ == "__main__":
    unittest.main()
