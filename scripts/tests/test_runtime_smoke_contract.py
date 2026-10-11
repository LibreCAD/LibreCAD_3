"""Contract checks for the isolated installed-runtime smoke path."""

import ast
import ctypes
import os
import pathlib
import textwrap
import unittest
from pathlib import Path
from pathlib import PurePosixPath
from types import SimpleNamespace
from unittest.mock import patch


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

    def _run_windows_check(self, loaded_dll_path):
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

    def test_windows_runtime_check_accepts_installed_cpython_dll(self):
        self._run_windows_check("/private/tmp/librecad-smoke-test/bin/python314.dll")

    def test_windows_runtime_check_rejects_external_cpython_dll(self):
        with self.assertRaisesRegex(AssertionError, "CPython loaded from"):
            self._run_windows_check("/external/python314.dll")

    def test_windows_nsis_smoke_enables_strict_runtime_check(self):
        marker = "      - name: Install and smoke the isolated Python runtime"
        start = self.workflow.index(marker)
        end = self.workflow.find("\n      - name:", start + len(marker))
        step = self.workflow[start : end if end >= 0 else len(self.workflow)]
        self.assertIn("$env:LIBRECAD_REQUIRE_BUNDLED_PYTHON = '1'", step)
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
            "lc::ui::python::MainWindowExecutionScope ownerScope(window);"
        )
        dispatch = open_file_body.index("window->cadMdiChild()->openFile();")
        registration = open_file_body.index("mainWindows.push_back(window);")
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
