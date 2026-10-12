"""Contract checks for the isolated installed-runtime smoke path."""

import ast
import ctypes
import importlib.machinery
import os
import pathlib
import subprocess
import sys
import tempfile
import textwrap
import unittest
from pathlib import Path
from pathlib import PurePosixPath
from types import SimpleNamespace
from unittest.mock import patch

from lcUIPy import _runtimeaudit as runtimeaudit
from lcUIPy._runtimeaudit import _glibc_loader, audit_elf_closure


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]


class RuntimeSmokeContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        source = (REPOSITORY_ROOT / "lcUI/python/pyruntimesmoke.cpp").read_text(
            encoding="utf-8"
        )
        start_marker = 'const std::string importsAndProviderCheck = R"py('
        start = source.index(start_marker) + len(start_marker)
        end = source.index('\n)py";', start)
        cls.python_smoke = source[start:end]
        strict_check = cls.python_smoke.split(
            'if os.environ.get("LIBRECAD_REQUIRE_BUNDLED_PYTHON") == "1":', 1
        )[1]
        windows_body = strict_check.split(
            'elif sys.platform == "win32":', 1
        )[1].split("    else:\n        raise AssertionError", 1)[0]
        cls.windows_check = textwrap.dedent(windows_body)
        linux_body = strict_check.split(
            'if sys.platform.startswith("linux"):', 1
        )[1].split('    elif sys.platform == "win32":', 1)[0]
        cls.linux_check = "if sys.platform.startswith('linux'):\n" + textwrap.indent(
            textwrap.dedent(linux_body), "    "
        )
        cls.workflow = (REPOSITORY_ROOT / ".github/workflows/main.yml").read_text(
            encoding="utf-8"
        )
        cls.linux_build_script = (
            REPOSITORY_ROOT
            / "scripts/ubuntu-install/installDependenciesAndBuildRepo.sh"
        ).read_text(encoding="utf-8")
        cls.appimage_script = (
            REPOSITORY_ROOT / "scripts/ubuntu-install/createAppImage.sh"
        ).read_text(encoding="utf-8")
        cls.windows_package_script = (
            REPOSITORY_ROOT
            / "scripts/windows-install/buildLibrecadAndCreatePackage.bat"
        ).read_text(encoding="utf-8")
        cls.macos_binding_workflow = (
            REPOSITORY_ROOT / ".github/workflows/python-binding-macos.yml"
        ).read_text(encoding="utf-8")

    def test_embedded_runtime_check_is_valid_python(self):
        ast.parse(self.python_smoke)

    def test_actual_smoke_runs_native_module_audits_and_propagates_failures(self):
        code = self.python_smoke.split("# BEGIN native Python modules\n", 1)[1].split(
            "# END native Python modules", 1)[0]
        namespace = dict(pathlib=pathlib, expected_staged_python_root="/runtime Ω")
        with patch.object(runtimeaudit, "audit_native_stdlib", create=True, return_value="offline probes") as core, \
                patch.object(runtimeaudit, "audit_python_extensions", create=True, return_value=("alpha",)) as extensions:
            exec(code, namespace)
            core.assert_called_once_with()
            extensions.assert_called_once_with(Path("/runtime Ω").resolve())
            self.assertIn("1 distinct files", namespace["_native_diagnostics"])
            self.assertIn("offline probes", namespace["_native_diagnostics"])
            extensions.side_effect = RuntimeError("failed module initialization")
            with self.assertRaisesRegex(RuntimeError, "failed module initialization"):
                exec(code, namespace)
            extensions.reset_mock(side_effect=True)
            core.side_effect = RuntimeError("failed native behavior")
            with self.assertRaisesRegex(RuntimeError, "failed native behavior"):
                exec(code, namespace)

    def test_windows_runtime_check_resolves_loaded_cpython_dll(self):
        strict_check = self.python_smoke.split(
            'if os.environ.get("LIBRECAD_REQUIRE_BUNDLED_PYTHON") == "1":', 1
        )[1]
        self.assertIn('elif sys.platform == "win32":', strict_check)
        self.assertIn("GetModuleHandleW", strict_check)
        self.assertIn("GetModuleFileNameW", strict_check)
        self.assertIn(
            "python{sys.version_info.major}{sys.version_info.minor}.dll",
            strict_check,
        )
        self.assertIn("is_relative_to(_expected_runtime_dir)", strict_check)

    def _run_windows_check(self, loaded_dll_path, require_extension_closure=False):
        requested_modules = []

        def get_module_handle(name):
            requested_modules.append(name)
            return 1

        def get_module_filename(_handle, output_buffer, _capacity):
            output_buffer.value = loaded_dll_path
            return len(loaded_dll_path)

        kernel32 = SimpleNamespace(
            GetModuleHandleW=get_module_handle,
            GetModuleFileNameW=get_module_filename,
        )
        fake_ctypes = SimpleNamespace(
            WinDLL=lambda _name, use_last_error: kernel32,
            POINTER=ctypes.POINTER,
            c_wchar=ctypes.c_wchar,
            c_wchar_p=ctypes.c_wchar_p,
            c_void_p=ctypes.c_void_p,
            c_uint32=ctypes.c_uint32,
            create_unicode_buffer=ctypes.create_unicode_buffer,
            get_last_error=lambda: 0,
        )
        namespace = {
            "ctypes": fake_ctypes,
            "os": SimpleNamespace(environ={
                "LIBRECAD_REQUIRE_PYTHON_EXTENSION_CLOSURE":
                    "1" if require_extension_closure else "0",
            }),
            "pathlib": pathlib,
            "sys": SimpleNamespace(
                platform="win32", version_info=SimpleNamespace(major=3, minor=14)
            ),
            "expected_staged_python_root": str(
                Path("/private/tmp/librecad-smoke-test/share/librecad/python")
            ),
        }
        exec(self.windows_check, namespace)
        self.assertEqual(["python314.dll"], requested_modules)
        return namespace

    def test_windows_runtime_check_accepts_installed_cpython_dll(self):
        self._run_windows_check("/private/tmp/librecad-smoke-test/bin/python314.dll")

    def test_windows_runtime_check_rejects_external_cpython_dll(self):
        with self.assertRaisesRegex(AssertionError, "CPython loaded from"):
            self._run_windows_check("/external/python314.dll")

    def test_windows_strict_gate_runs_extension_audit_and_propagates_failure(self):
        with patch.object(runtimeaudit, "audit_windows_extensions", create=True,
                          return_value=7) as audit:
            namespace = self._run_windows_check(
                "/private/tmp/librecad-smoke-test/bin/python314.dll", True)
            audit.assert_called_once_with(Path(
                "/private/tmp/librecad-smoke-test/share/librecad/python").resolve())
            self.assertIn("7 distinct files", namespace["_windows_diagnostics"])
            audit.side_effect = RuntimeError("missing native dependency")
            with self.assertRaisesRegex(RuntimeError, "missing native dependency"):
                self._run_windows_check(
                    "/private/tmp/librecad-smoke-test/bin/python314.dll", True)

    def test_windows_nsis_smoke_enables_strict_runtime_check(self):
        marker = "      - name: Install and smoke the isolated Python runtime"
        start = self.workflow.index(marker)
        end = self.workflow.find("\n      - name:", start + len(marker))
        step = self.workflow[start : end if end >= 0 else len(self.workflow)]
        self.assertIn("$env:LIBRECAD_REQUIRE_BUNDLED_PYTHON = '1'", step)
        self.assertIn("$env:LIBRECAD_REQUIRE_PYTHON_EXTENSION_CLOSURE = '1'", step)
        self.assertIn("timeout-minutes: 5", step)
        self.assertIn(
            '$env:PATH = "$env:SystemRoot\\System32;$env:SystemRoot"', step
        )
        self.assertIn("& $app --python-smoke", step)

    def test_final_appimage_smoke_enables_strict_runtime_check(self):
        marker = "      - name: Smoke final AppImage Python runtime"
        start = self.workflow.index(marker)
        end = self.workflow.find("\n      - name:", start + len(marker))
        step = self.workflow[start : end if end >= 0 else len(self.workflow)]
        self.assertIn("LIBRECAD_REQUIRE_BUNDLED_PYTHON=1", step)
        self.assertIn("LIBRECAD_REQUIRE_PYTHON_EXTENSION_CLOSURE=1", step)
        self.assertIn('"$appimage" --python-smoke', step)
        self.assertIn("PYTHONHOME=/__librecad_python_poison__", step)
        self.assertIn("PYTHONPATH=/__librecad_python_poison__", step)

    def test_pre_linuxdeploy_install_smoke_allows_selected_system_libpython(self):
        start = self.linux_build_script.index("(cd /tmp && xvfb-run")
        end = self.linux_build_script.index("\npopd", start)
        smoke = self.linux_build_script[start:end]
        self.assertNotIn("LIBRECAD_REQUIRE_BUNDLED_PYTHON=1", smoke)
        self.assertIn('"$installed_app" --python-smoke', smoke)
        self.assertIn("PYTHONHOME=/__librecad_python_poison__", smoke)
        self.assertIn("PYTHONPATH=/__librecad_python_poison__", smoke)

    def test_post_linuxdeploy_appdir_smoke_requires_bundled_libpython(self):
        packager = self.appimage_script.index("--output appimage")
        smoke_start = self.appimage_script.index('appdir_root="$(realpath AppDir)"')
        smoke = self.appimage_script[smoke_start:]
        self.assertLess(packager, smoke_start)
        self.assertIn('"$appdir_root/AppRun"', self.appimage_script)
        self.assertIn("xvfb-run -a env", smoke)
        self.assertIn("LIBRECAD_REQUIRE_BUNDLED_PYTHON=1", smoke)
        self.assertIn("PYTHONHOME=/__librecad_python_poison__", smoke)
        self.assertIn("PYTHONPATH=/__librecad_python_poison__", smoke)
        self.assertIn('"$appdir_launcher" --python-smoke', smoke)
        self.assertIn("-u QT_PLUGIN_PATH", smoke)
        self.assertIn("-u QT_QPA_PLATFORM_PLUGIN_PATH", smoke)

    def test_appimage_bundles_python_extension_libraries_and_uses_private_runpath(self):
        for library in (
            "libssl.so.3",
            "libcrypto.so.3",
            "libsqlite3.so.0",
            "libffi.so.8",
            "libz.so.1",
        ):
            with self.subTest(library=library):
                self.assertIn(
                    f"--library=/usr/lib/x86_64-linux-gnu/{library}",
                    self.appimage_script,
                )
        self.assertIn(
            "--custom-apprun ../scripts/ubuntu-install/AppRun",
            self.appimage_script,
        )
        app_run = (
            REPOSITORY_ROOT / "scripts/ubuntu-install/AppRun"
        ).read_text(encoding="utf-8")
        self.assertIn(
            "$appdir/usr/lib:$appdir/usr/lib/x86_64-linux-gnu:$appdir/usr/lib64",
            app_run,
        )
        self.assertIn(
            'export LD_LIBRARY_PATH="$private_library_path:$LD_LIBRARY_PATH"',
            app_run,
        )
        self.assertIn('exec "$appdir/usr/bin/librecad" "$@"', app_run)

    def test_appimage_bundles_harfbuzz_instead_of_weakening_clean_runtime(self):
        for library in ("libharfbuzz.so.0", "libgraphite2.so.3"):
            with self.subTest(library=library):
                self.assertIn(f"--library=/usr/lib/x86_64-linux-gnu/{library}",
                              self.appimage_script)
        runtime_image = (REPOSITORY_ROOT / "scripts/ubuntu-install/appimage-runtime.Dockerfile").read_text()
        self.assertNotIn("libharfbuzz", runtime_image)

    def test_linuxdeploy_qt_plugin_has_patchelf_dependency(self):
        install_start = self.linux_build_script.index("sudo apt install")
        install_end = self.linux_build_script.index("\n\n", install_start)
        install_command = self.linux_build_script[install_start:install_end]
        self.assertNotIn("#", install_command)
        self.assertIn("python3-dev python3 xvfb patchelf", install_command)
        self.assertIn("--plugin qt", self.appimage_script)

    def test_windows_native_extension_origins_remain_confined_to_provider(self):
        self.assertIn("import _ctypes", self.python_smoke)
        self.assertIn("import _sqlite3", self.python_smoke)
        self.assertIn("import _ssl", self.python_smoke)
        self.assertIn("_origin.is_relative_to(_prefix)", self.python_smoke)

    def test_first_open_dispatch_runs_with_its_window_owner_scope(self):
        window_manager = (
            REPOSITORY_ROOT / "lcUI/windowmanager.cpp"
        ).read_text(encoding="utf-8")
        start = window_manager.index("void WindowManager::openFile()")
        end = window_manager.index("\n}", start)
        open_file_body = window_manager[start:end]

        scope = open_file_body.index(
            "lc::ui::python::MainWindowExecutionScope ownerScope(window.get());"
        )
        dispatch = open_file_body.index("if (!window->cadMdiChild()->openFile()) return;")
        registration = open_file_body.index("mainWindows.push_back(window.release());")
        self.assertIn("std::unique_ptr<MainWindow>", open_file_body[:scope])
        self.assertLess(scope, dispatch)
        self.assertLess(dispatch, registration)
        self.assertIn("#ifdef LC_WITH_PYTHONSCRIPT", open_file_body[:scope])
        self.assertIn('#include "python/pyeventhooks.h"', window_manager)

    def test_linux_blocking_core_tests_use_required_runner_and_fail_if_missing(self):
        start = self.linux_build_script.index('echo "running lcunittest (core suites')
        end = self.linux_build_script.index(
            'echo "running lcunittest (remaining suites', start
        )
        blocking = self.linux_build_script[start:end]
        self.assertIn("../scripts/run_required_gtests.py", blocking)
        self.assertIn('--binary ./bin/lcunittest', blocking)
        self.assertIn('--filter="${CORE_SUITES}"', blocking)
        self.assertIn("--command-prefix=xvfb-run", blocking)
        self.assertIn("--command-prefix=-a", blocking)
        core_filter = self.linux_build_script.split('CORE_SUITES="', 1)[1].split('"', 1)[0]
        self.assertNotIn("ScriptCallback.*", core_filter)
        for suite in (
            "ScriptCallbackInvoke.*",
            "ScriptCallbackEquality.*",
            "ScriptCallbackLifecycle.*",
        ):
            with self.subTest(suite=suite):
                self.assertIn(suite, core_filter)
        self.assertNotIn("--gtest_list_tests", self.linux_build_script)
        self.assertIn("ERROR: bin/lcunittest not built; required tests cannot run", self.linux_build_script)
        self.assertNotIn("skipping test run", self.linux_build_script)

    @unittest.skipUnless(os.name == "posix", "Linux CI shell contract")
    def test_linux_informational_tests_are_bounded_and_preserve_exit_diagnostics(self):
        start = self.linux_build_script.index(
            'echo "running lcunittest (remaining suites'
        )
        end = self.linux_build_script.index(
            '\nelse\n    echo "ERROR: bin/lcunittest', start
        )
        informational = self.linux_build_script[start:end]
        for status in (0, 7, 124, 137):
            with self.subTest(status=status):
                script = """set -e
CORE_SUITES='Core.*:Other.*'
timeout() { printf 'bounded: <%%s>\\n' "$@"; return %d; }
xvfb-run() { echo 'unbounded invocation'; return %d; }
%s
echo 'continued to install'
""" % (status, status, informational)
                result = subprocess.run(
                    ["bash"], input=script, text=True, stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT, timeout=10, check=False,
                )
                self.assertEqual(0, result.returncode, result.stdout)
                for argument in (
                    "--kill-after=10s", "180s", "xvfb-run", "-a",
                    "./bin/lcunittest", "--gtest_filter=-Core.*:Other.*",
                ):
                    self.assertIn("bounded: <{}>".format(argument), result.stdout)
                self.assertNotIn("unbounded invocation", result.stdout)
                self.assertIn("continued to install", result.stdout)
                if status:
                    self.assertIn("non-core tests exited {}".format(status), result.stdout)
                    self.assertIn("180s limit, 10s kill grace", result.stdout)
                else:
                    self.assertNotIn("WARNING:", result.stdout)

    def test_windows_required_filters_use_bounded_required_runner(self):
        required_filters = (
            "PythonRuntime.*:PythonFixture.*",
            "LuaRuntimeTest.*:ParityTest.*",
            "MWindowTest.PyGuiBridgeRejectsWorkerThreadAccess",
            "PythonFixture.*:PythonRuntime.*:ParityTest.*:ScriptValueRoundTrip.*:",
        )
        for test_filter in required_filters:
            with self.subTest(test_filter=test_filter):
                self.assertIn("scripts\\run_required_gtests.py", self.windows_package_script)
                self.assertIn(test_filter, self.windows_package_script)
        self.assertNotIn("lcunittest.exe --gtest_filter=", self.windows_package_script)
        self.assertNotIn("--gtest_list_tests", self.windows_package_script)
        self.assertIn("--require-suite \"MenuEntriesTest.\"", self.windows_package_script)

    def test_macos_archives_verified_bundle_and_durable_runtime_evidence(self):
        workflow = self.macos_binding_workflow
        smoke = workflow.index('"$relocated_app/Contents/MacOS/librecad" --python-smoke')
        archive = workflow.index('tar -czf "$archive"')
        upload = workflow.index("- name: Upload the verified app")
        self.assertLess(smoke, archive)
        self.assertLess(archive, upload)
        for required in (
            'test -s build/python-runtime-inventory.txt',
            'codesign --verify --deep --strict "$app"',
            "Print :LSMinimumSystemVersion",
            'shasum -a 256 "$archive"',
            'tar -xzf "$archive" -C "$extracted"',
            'codesign --verify --deep --strict "$extracted/LibreCAD.app"',
            '"$extracted/LibreCAD.app/Contents/MacOS/librecad" --python-smoke',
            '2>&1 | tee build/macos-archive-smoke.log',
            'revision=$(git rev-parse HEAD)',
            'name: LibreCAD-macos-${{ matrix.arch }}',
            'name: Python-macos-evidence-${{ matrix.arch }}',
            'build/macos-artifact-identity.txt',
            '2>&1 | tee build/python-required-tests.log',
            '2>&1 | tee build/macos-relocated-smoke.log',
        ):
            with self.subTest(required=required):
                self.assertIn(required, workflow)
        self.assertEqual(2, workflow.count("if-no-files-found: error"))
        self.assertIn("if: always()", workflow[upload:])

    def test_macos_required_filters_use_bounded_required_runner(self):
        self.assertIn("scripts/run_required_gtests.py", self.macos_binding_workflow)
        self.assertIn("--binary \"$test_binary\"", self.macos_binding_workflow)
        self.assertIn("--filter='LuaRuntimeTest.*'", self.macos_binding_workflow)
        self.assertIn("--filter='PythonFixture.*:PythonRuntime.*:ParityTest.*:", self.macos_binding_workflow)
        self.assertNotIn('QT_QPA_PLATFORM=offscreen "$test_binary" \\', self.macos_binding_workflow)
        self.assertNotIn("--gtest_list_tests", self.macos_binding_workflow)
        self.assertIn('required_suite_args+=(--require-suite "$suite")', self.macos_binding_workflow)
        self.assertIn("--require-suite 'LuaRuntimeTest.'", self.macos_binding_workflow)

    def _run_linux_check(
        self, mapped_libpython, mapped_python_dependencies=(), require_extension_closure=False
    ):
        class FakePath:
            def __init__(self, value):
                self.value = PurePosixPath(str(value))

            @property
            def name(self):
                return self.value.name

            def resolve(self):
                return self

            @property
            def parents(self):
                return self.value.parents

            def is_file(self):
                return str(self.value) == "/proc/self/maps"

            def read_text(self):
                return self.maps_text

            def is_relative_to(self, other):
                return self.value.is_relative_to(other)

            def __hash__(self):
                return hash(self.value)

            def __eq__(self, other):
                return isinstance(other, FakePath) and self.value == other.value

            def __repr__(self):
                return repr(self.value)

        mapped_paths = list(mapped_python_dependencies)
        if mapped_libpython:
            mapped_paths.append(mapped_libpython)
        if not mapped_paths:
            mapped_paths.append("/usr/lib/libQt6Core.so")
        FakePath.maps_text = "".join(
            "1000-2000 r-xp 00000000 00:01 1 {}\n".format(path)
            for path in mapped_paths
        )

        package_root = "/Users/Example/AppDir/usr"
        namespace = {
            "os": os,
            "pathlib": SimpleNamespace(Path=FakePath),
            "sys": SimpleNamespace(platform="linux"),
            "expected_staged_python_root": (
                package_root + "/share/librecad/python"
            ),
        }
        environment = {
            "LIBRECAD_REQUIRE_PYTHON_EXTENSION_CLOSURE": (
                "1" if require_extension_closure else "0"
            )
        }
        with patch.dict(os.environ, environment):
            exec(self.linux_check, namespace)

    def test_linux_runtime_check_accepts_libpython_under_packaged_usr(self):
        self._run_linux_check(
            "/Users/Example/AppDir/usr/lib/libpython3.14.so"
        )

    def test_linux_runtime_check_rejects_external_libpython(self):
        with self.assertRaisesRegex(AssertionError, "outside the packaged usr tree"):
            self._run_linux_check("/usr/lib/libpython3.14.so")

    def test_linux_runtime_check_fails_when_no_loaded_libpython_is_found(self):
        with self.assertRaisesRegex(AssertionError, "no dynamically loaded libpython"):
            self._run_linux_check(None)

    def test_linux_python_extension_dependencies_must_be_inside_package(self):
        self._run_linux_check(
            "/Users/Example/AppDir/usr/lib/libpython3.14.so",
            (
                "/Users/Example/AppDir/usr/lib/libssl.so.3",
                "/Users/Example/AppDir/usr/lib/libcrypto.so.3",
                "/Users/Example/AppDir/usr/lib/libsqlite3.so.0",
                "/Users/Example/AppDir/usr/lib/libffi.so.8",
                "/Users/Example/AppDir/usr/lib/libz.so.1",
            ),
            require_extension_closure=True,
        )

    def test_linux_python_extension_dependency_outside_package_is_rejected(self):
        with self.assertRaisesRegex(
            AssertionError,
            "Python extension dependency libssl.so loaded outside the packaged usr tree",
        ):
            self._run_linux_check(
                "/Users/Example/AppDir/usr/lib/libpython3.14.so",
                (
                    "/usr/lib/libssl.so.3",
                    "/Users/Example/AppDir/usr/lib/libcrypto.so.3",
                    "/Users/Example/AppDir/usr/lib/libsqlite3.so.0",
                    "/Users/Example/AppDir/usr/lib/libffi.so.8",
                    "/Users/Example/AppDir/usr/lib/libz.so.1",
                ),
                require_extension_closure=True,
            )

    def test_linux_python_extension_dependency_must_be_loaded(self):
        with self.assertRaisesRegex(
            AssertionError, "Python extension dependency libssl.so was not loaded"
        ):
            self._run_linux_check(
                "/Users/Example/AppDir/usr/lib/libpython3.14.so",
                (
                    "/Users/Example/AppDir/usr/lib/libcrypto.so.3",
                    "/Users/Example/AppDir/usr/lib/libsqlite3.so.0",
                    "/Users/Example/AppDir/usr/lib/libffi.so.8",
                    "/Users/Example/AppDir/usr/lib/libz.so.1",
                ),
                require_extension_closure=True,
            )


class WindowsExtensionClosureTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name).resolve() / "Python runtime Ω"
        self.root.mkdir()
        loader_patch = patch.object(ctypes, "WinDLL", create=True)
        self.load = loader_patch.start()
        self.addCleanup(loader_patch.stop)

    def extension(self, name):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(b"MZ fixture: native loader is mocked")
        return path

    def test_every_extension_is_loaded_without_path_cwd_or_user_dll_dirs(self):
        first = self.extension("DLLs/_ssl.pyd")
        second = self.extension("DLLs/nested/other.PYD")
        self.extension("DLLs/not-an-extension.dll")
        self.assertEqual(2, runtimeaudit.audit_windows_extensions(self.root))
        self.assertEqual({str(first), str(second)}, {
            call.args[0] for call in self.load.call_args_list})
        for call in self.load.call_args_list:
            self.assertEqual({"winmode": 0x100 | 0x200 | 0x800}, call.kwargs)

    def test_loader_failure_names_the_extension_and_propagates_os_diagnostic(self):
        self.extension("DLLs/_ssl.pyd")
        self.load.side_effect = OSError(126, "dependent DLL not found")
        with self.assertRaisesRegex(RuntimeError, "_ssl.pyd.*dependent DLL not found"):
            runtimeaudit.audit_windows_extensions(self.root)

    def test_empty_extension_inventory_fails_closed(self):
        with self.assertRaisesRegex(RuntimeError, "zero.*extension"):
            runtimeaudit.audit_windows_extensions(self.root)
        self.load.assert_not_called()

    def test_extension_path_that_is_not_a_file_cannot_be_silently_skipped(self):
        (self.root / "invalid.pyd").mkdir()
        with self.assertRaisesRegex(RuntimeError, "not a file"):
            runtimeaudit.audit_windows_extensions(self.root)
        self.load.assert_not_called()

    @unittest.skipUnless(os.name == "posix", "POSIX fixture symlinks")
    def test_aliases_are_deduplicated_and_escaped_extensions_fail(self):
        original = self.extension("DLLs/_ssl.pyd")
        (original.parent / "alias.pyd").symlink_to(original.name)
        self.assertEqual(1, runtimeaudit.audit_windows_extensions(self.root))
        outside = self.root.parent / "outside.pyd"
        outside.write_bytes(original.read_bytes())
        (original.parent / "escaped.pyd").symlink_to(outside)
        with self.assertRaisesRegex(RuntimeError, "extension symlink escapes"):
            runtimeaudit.audit_windows_extensions(self.root)
        self.assertNotIn(str(outside), {call.args[0] for call in self.load.call_args_list})


class FullElfClosureTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name).resolve() / "runtime Ω" / "usr"
        self.root.mkdir(parents=True)
        self.loader = Path(temporary.name) / "ld-linux-fixture.so"
        loader_patch = patch("lcUIPy._runtimeaudit._glibc_loader", return_value=self.loader)
        loader_patch.start()
        self.addCleanup(loader_patch.stop)
        run_patch = patch("lcUIPy._runtimeaudit.subprocess.run", return_value=SimpleNamespace(
            returncode=0, stdout="libc.so.6 => /usr/lib/libc.so.6\n"))
        self.run = run_patch.start()
        self.addCleanup(run_patch.stop)

    def elf(self, name="extension.so"):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(b"\x7fELF\x02\x01\x01" + b"\0" * 9 + b"\x03\0\x3e\0" + b"\0" * 44)
        return path

    def run_package_gate(self, staged_python):
        source = (REPOSITORY_ROOT / "lcUI/python/pyruntimesmoke.cpp").read_text()
        body = source.split(
            'if os.environ.get("LIBRECAD_REQUIRE_FULL_ELF_CLOSURE") == "1":', 1
        )[1].split("\n# BEGIN native Python modules", 1)[0]
        namespace = {
            "os": SimpleNamespace(environ={"LIBRECAD_REQUIRE_BUNDLED_PYTHON": "1"}),
            "sys": SimpleNamespace(platform="linux"), "pathlib": pathlib,
            "expected_staged_python_root": str(staged_python),
        }
        exec(textwrap.dedent(body), namespace)
        return namespace

    def test_full_gate_scans_artifact_root_including_non_usr_payload_elf(self):
        staged = self.root / "share/librecad/python"
        staged.mkdir(parents=True)
        self.elf("lib/inside-usr.so")
        self.elf("../lib/outside-usr.so")
        for marker in ("AppRun", "meta/snap.yaml"):
            with self.subTest(marker=marker):
                metadata = self.root.parent / marker
                metadata.parent.mkdir(parents=True, exist_ok=True)
                metadata.write_text("artifact marker", encoding="utf-8")
                self.run.reset_mock()
                namespace = self.run_package_gate(staged)
                self.assertEqual(self.root.parent, namespace["_elf_root"])
                self.assertEqual(2, namespace["_elf_count"])
                self.assertEqual(2, self.run.call_count)
                metadata.unlink()

    def test_full_gate_rejects_unknown_layout_and_filesystem_root(self):
        staged = self.root / "share/librecad/python"
        staged.mkdir(parents=True)
        self.elf()
        with patch("lcUIPy._runtimeaudit.audit_elf_closure") as audit:
            for prefix in (staged, Path("/usr/share/librecad/python")):
                with self.subTest(prefix=prefix):
                    with self.assertRaisesRegex(AssertionError, "recognized AppImage or Snap root"):
                        self.run_package_gate(prefix)
            audit.assert_not_called()
        self.run.assert_not_called()

    def test_every_elf_is_checked_by_header_with_direct_loader_arguments(self):
        first = self.elf("lib/python/extension.so")
        second = self.elf("bin/executable Ω")
        (self.root / "not-an-elf.so").write_text("ordinary data", encoding="utf-8")
        self.assertEqual(2, audit_elf_closure(self.root))
        self.assertEqual(2, self.run.call_count)
        self.assertEqual({str(first), str(second)}, {
            call.args[0][2] for call in self.run.call_args_list})
        for call in self.run.call_args_list:
            self.assertEqual([str(self.loader), "--list"], call.args[0][:2])
            self.assertLessEqual(call.kwargs["timeout"], 15)
            self.assertFalse(call.kwargs.get("shell", False))
            self.assertNotIn("env", call.kwargs)  # Preserve launcher/base/content paths.

    def test_external_content_mount_is_provider_not_artifact_payload(self):
        own = self.elf("lib/owned.so")
        self.elf("gpu-2604/lib/provider.so")
        content_mount = self.root / "gpu-2604"
        original_lstat = Path.lstat

        def lstat(path):
            if path == content_mount:
                return SimpleNamespace(st_dev=self.root.stat().st_dev + 1)
            return original_lstat(path)

        with patch.object(Path, "lstat", autospec=True, side_effect=lstat):
            self.assertEqual(1, audit_elf_closure(self.root))
        self.assertEqual([str(self.loader), "--list", str(own)], self.run.call_args.args[0])

    def test_missing_dependency_or_inspector_failure_rejects_the_payload(self):
        self.elf()
        for status, output in ((0, "libmissing.so => not found"), (127, "loader failure")):
            with self.subTest(status=status):
                self.run.return_value = SimpleNamespace(returncode=status, stdout=output)
                with self.assertRaisesRegex(RuntimeError, "unresolved ELF closure for extension.so"):
                    audit_elf_closure(self.root)

    def test_empty_payload_cannot_pass(self):
        with self.assertRaisesRegex(RuntimeError, "zero payload ELF"):
            audit_elf_closure(self.root)
        self.run.assert_not_called()

    def test_relocatable_objects_are_not_loaded_and_malformed_elf_fails(self):
        self.elf()
        obj = self.elf("config/python.o")
        header = bytearray(obj.read_bytes())
        header[16:18] = b"\x01\0"  # ET_REL, not a runtime executable/library.
        obj.write_bytes(header)
        self.assertEqual(1, audit_elf_closure(self.root))
        (self.root / "malformed.so").write_bytes(b"\x7fELF")
        with self.assertRaisesRegex(RuntimeError, "invalid ELF header"):
            audit_elf_closure(self.root)

    def test_inspector_timeout_and_missing_loader_fail_with_binary_diagnostic(self):
        self.elf()
        for error in (subprocess.TimeoutExpired("loader", 15), FileNotFoundError("loader")):
            with self.subTest(error=error):
                self.run.side_effect = error
                with self.assertRaisesRegex(RuntimeError, "inspection failed for .*extension.so"):
                    audit_elf_closure(self.root)

    def test_total_deadline_and_invalid_budget_fail_closed(self):
        self.elf()
        with patch("lcUIPy._runtimeaudit.time.monotonic", side_effect=(0, 121)):
            with self.assertRaisesRegex(RuntimeError, "total time limit"):
                audit_elf_closure(self.root)
        with self.assertRaisesRegex(ValueError, "timeout must be positive"):
            audit_elf_closure(self.root, timeout_seconds=0)
        self.run.assert_not_called()

    @unittest.skipUnless(os.name == "posix", "POSIX payload symlinks")
    def test_internal_alias_is_deduplicated_and_escaped_elf_is_rejected(self):
        target = self.elf()
        (self.root / "alias.so").symlink_to(target.name)
        self.assertEqual(1, audit_elf_closure(self.root))
        outside = self.root.parent / "outside.so"
        outside.write_bytes(target.read_bytes())
        (self.root / "escaped.so").symlink_to(outside)
        with self.assertRaisesRegex(RuntimeError, "ELF symlink escapes prefix"):
            audit_elf_closure(self.root)

    def test_mapped_loader_discovery_requires_one_distinct_glibc_provider(self):
        mapping = "100-200 r-xp 0000 00:00 1 /runtime/ld-linux-x86-64.so.2\n"
        with patch.object(Path, "read_text", return_value=mapping * 2):
            self.assertEqual(Path("/runtime/ld-linux-x86-64.so.2").resolve(), _glibc_loader())
        for maps in ("", mapping + mapping.replace("/runtime/", "/other/")):
            with patch.object(Path, "read_text", return_value=maps):
                with self.assertRaisesRegex(RuntimeError, "exactly one mapped glibc loader"):
                    _glibc_loader()

    def test_actual_package_smokes_require_full_closure_and_prebuilt_dependencies(self):
        runtime = (REPOSITORY_ROOT / "lcUI/python/pyruntimesmoke.cpp").read_text()
        clean = (REPOSITORY_ROOT / "scripts/ubuntu-install/smokeAppImageRuntime.sh").read_text()
        workflow = (REPOSITORY_ROOT / ".github/workflows/main.yml").read_text()
        snap = workflow.split("- name: Smoke installed snap Python runtime", 1)[1].split("# if: always()", 1)[0]
        self.assertIn("from lcUIPy._runtimeaudit import audit_elf_closure", runtime)
        self.assertIn("LIBRECAD_REQUIRE_FULL_ELF_CLOSURE=1", clean)
        self.assertIn("LIBRECAD_REQUIRE_FULL_ELF_CLOSURE=1", snap)
        self.assertIn("timeout --kill-after=10s 180s", snap)
        self.assertIn("snap-elf-runtime-smoke.log", snap)
        appimage = (REPOSITORY_ROOT / "scripts/ubuntu-install/createAppImage.sh").read_text()
        for library in ("libreadline.so.8", "libgdbm.so.6", "libtcl8.6.so", "libtk8.6.so", "libXft.so.2", "libXss.so.1"):
            self.assertIn("--library=/usr/lib/x86_64-linux-gnu/" + library, appimage)
        recipe = (REPOSITORY_ROOT / "snapcraft.yaml").read_text()
        stage = recipe.split("    stage-packages:", 1)[1].split("    cmake-parameters:", 1)[0]
        for package in ("libreadline8t64", "libtcl8.6", "libtk8.6", "libxft2", "libxss1"):
            self.assertIn("      - " + package + "\n", stage)


class NativePythonModuleTests(unittest.TestCase):
    def fixture(self, names=("alpha", "nested.beta")):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        root = Path(temporary.name) / "runtime Ω"
        library = root / "Lib"
        library.mkdir(parents=True)
        suffix = importlib.machinery.EXTENSION_SUFFIXES[0]
        binaries = {}
        for name in names:
            path = library.joinpath(*name.split(".")).with_suffix(suffix)
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(b"fixture, never passed to the native loader")
            binaries[name] = path
        return root, library, binaries

    def run_catalog(self, root, paths, load, **options):
        with patch.object(sys, "path", list(map(str, paths))), \
                patch("importlib.import_module", side_effect=load):
            return runtimeaudit.audit_python_extensions(root, **options)

    def test_initializes_the_whole_catalog_with_exact_origins(self):
        root, library, binaries = self.fixture()
        loaded = []
        def load(name):
            loaded.append(name)
            return SimpleNamespace(__file__=str(binaries[name]))
        self.assertEqual(("alpha", "nested.beta"), self.run_catalog(root, [library], load))
        self.assertEqual(["alpha", "nested.beta"], loaded)

    def test_missing_unreachable_escaped_and_ambiguous_catalogs_fail(self):
        for kind in ("empty", "unreachable", "escaped", "directory-link", "ambiguous", "invalid"):
            with self.subTest(kind=kind):
                root, library, binaries = self.fixture(() if kind == "empty" else ("alpha",))
                paths = [library]
                binary = binaries.get("alpha")
                if kind == "unreachable":
                    paths = []
                elif kind == "escaped":
                    outside = root.parent / binary.name
                    outside.write_bytes(binary.read_bytes())
                    binary.unlink()
                    binary.symlink_to(outside)
                elif kind == "ambiguous":
                    other = root / "DLLs"
                    other.mkdir()
                    (other / binary.name).write_bytes(b"different native file")
                    paths.append(other)
                elif kind == "directory-link":
                    (library / "alias").symlink_to(root.parent, target_is_directory=True)
                elif kind == "invalid":
                    binary.rename(binary.with_name("not-valid" + importlib.machinery.EXTENSION_SUFFIXES[0]))
                with self.assertRaisesRegex(RuntimeError, "catalog|escape|ambiguous|name|path"):
                    self.run_catalog(root, paths, lambda name: SimpleNamespace(__file__=str(binary)))

    def test_build_config_libpython_link_is_not_a_python_extension(self):
        root, library, binaries = self.fixture(("alpha",))
        config = library / "config-{}.{}/".format(*sys.version_info[:2])
        config.mkdir()
        (config / "libpython{}.{}.so".format(*sys.version_info[:2])).symlink_to("missing-interpreter-library")
        self.assertEqual(("alpha",), self.run_catalog(
            root, [library], lambda name: SimpleNamespace(__file__=str(binaries[name]))))

    def test_missing_root_and_unreadable_walk_fail_instead_of_omitting_files(self):
        root, library, binaries = self.fixture()
        with self.assertRaises(FileNotFoundError):
            self.run_catalog(root / "missing", [library], lambda name: None)
        def unreadable(root, onerror):
            onerror(PermissionError("catalog unreadable"))
            return iter(())
        with patch.object(runtimeaudit.os, "walk", side_effect=unreadable), \
                self.assertRaisesRegex(PermissionError, "catalog unreadable"):
            self.run_catalog(root, [library], lambda name: None)

    def test_import_failures_wrong_origins_and_total_timeout_fail(self):
        root, library, binaries = self.fixture(("alpha",))
        def broken(name):
            raise ImportError("missing transitive dependency")
        with self.assertRaisesRegex(RuntimeError, "alpha.*missing transitive dependency"):
            self.run_catalog(root, [library], broken)
        for origin in (None, str(root / "wrong.so")):
            with self.subTest(origin=origin), self.assertRaisesRegex(RuntimeError, "origin|selected file"):
                self.run_catalog(root, [library], lambda name: SimpleNamespace(__file__=origin))
        with self.assertRaises(ValueError):
            self.run_catalog(root, [library], broken, timeout_seconds=0)
        with patch.object(runtimeaudit.time, "monotonic", side_effect=[0, 0, 61]), \
                self.assertRaisesRegex(RuntimeError, "time limit"):
            self.run_catalog(root, [library], lambda name: SimpleNamespace(__file__=str(binaries[name])))

    def test_core_native_behavior_is_real_and_offline(self):
        with patch("socket.socket", side_effect=AssertionError("network is forbidden")):
            diagnostics = runtimeaudit.audit_native_stdlib()
        for name in ("gzip/zlib", "SHA-256", "SQLite", "TLS client hello", "ffi callback"):
            self.assertIn(name, diagnostics)

    def test_core_native_behavior_rejects_corrupt_results(self):
        cases = (("gzip.decompress", b"bad"), ("zlib.decompress", b"bad"),
                 ("hashlib.sha256", SimpleNamespace(hexdigest=lambda: "bad")),
                 ("ctypes.CFUNCTYPE", lambda callback: lambda value: 0))
        for target, value in cases:
            with self.subTest(target=target), patch(target, return_value=value), \
                    self.assertRaises(RuntimeError):
                runtimeaudit.audit_native_stdlib()

    def test_sqlite_result_and_tls_output_are_required(self):
        closed = []
        connection = SimpleNamespace(commit=lambda: None, close=lambda: closed.append(True))
        connection.execute = lambda *args: SimpleNamespace(fetchone=lambda: ("wrong", b"wrong"))
        with patch("sqlite3.connect", return_value=connection), self.assertRaisesRegex(RuntimeError, "SQLite"):
            runtimeaudit.audit_native_stdlib()
        self.assertEqual([True], closed)
        import ssl
        def need_input():
            raise ssl.SSLWantReadError()
        context = SimpleNamespace(wrap_bio=lambda *args, **kwargs: SimpleNamespace(do_handshake=need_input))
        with patch("ssl.SSLContext", return_value=context), self.assertRaisesRegex(RuntimeError, "TLS"):
            runtimeaudit.audit_native_stdlib()
