"""Contract checks for the isolated installed-runtime smoke path."""

import ast
import ctypes
import pathlib
import re
import textwrap
import unittest
from pathlib import Path
from types import SimpleNamespace


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]


class RuntimeSmokeContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        source = (REPOSITORY_ROOT / "lcUI/python/pyruntimesmoke.cpp").read_text(
            encoding="utf-8"
        )
        cls.smoke_source = source
        cls.cmake = (REPOSITORY_ROOT / "CMakeLists.txt").read_text(
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
        cls.workflow = (REPOSITORY_ROOT / ".github/workflows/main.yml").read_text(
            encoding="utf-8"
        )
        cls.macos_workflow = (
            REPOSITORY_ROOT / ".github/workflows/python-binding-macos.yml"
        ).read_text(encoding="utf-8")

    def test_installed_smoke_uses_python_inits_selected_staged_root(self):
        self.assertIn(
            "PythonInit::configuredStagedRuntimeHome()", self.smoke_source
        )
        self.assertIn(
            'ns.set("expected_staged_python_root",\n               stagedPythonRoot);',
            self.smoke_source,
        )
        self.assertNotIn('packageDirectory.filePath("python")', self.smoke_source)

    def test_linux_package_option_stages_the_selected_shared_python_runtime(self):
        self.assertRegex(
            self.cmake,
            r'(?m)^option\(WITH_BUNDLED_PYTHON_RUNTIME\s+.* OFF\)$',
        )
        self.assertIn(
            "WITH_BUNDLED_PYTHON_RUNTIME requires WITH_PYTHONSCRIPT=ON",
            self.cmake,
        )
        self.assertIn(
            "WITH_BUNDLED_PYTHON_RUNTIME is currently supported only on Linux",
            self.cmake,
        )
        self.assertIn(
            'if(WITH_BUNDLED_PYTHON_RUNTIME)',
            (REPOSITORY_ROOT / "lcadpythonscript/CMakeLists.txt").read_text(
                encoding="utf-8"
            ),
        )
        python_cmake = (
            REPOSITORY_ROOT / "lcadpythonscript/CMakeLists.txt"
        ).read_text(encoding="utf-8")
        self.assertIn('get_filename_component(LC_PYTHON_STAGED_RUNTIME_LIBRARY', python_cmake)
        self.assertIn('"${LC_PYTHON_RUNTIME_LIBRARY}" REALPATH', python_cmake)
        self.assertIn(
            'install(FILES "${LC_PYTHON_STAGED_RUNTIME_LIBRARY}"', python_cmake
        )
        self.assertIn('"${CMAKE_INSTALL_LIBDIR}"', python_cmake)
        self.assertIn('"staged_shared_runtime_rel=${LC_PYTHON_STAGED_RUNTIME_REL}', python_cmake)
        self.assertIn('INSTALL_RPATH "$ORIGIN"', self.cmake)
        linux_build = (
            REPOSITORY_ROOT / "scripts/ubuntu-install/installDependenciesAndBuildRepo.sh"
        ).read_text(encoding="utf-8")
        self.assertIn("-DWITH_BUNDLED_PYTHON_RUNTIME=ON", linux_build)
        linux_smoke = linux_build.split('"$installed_app" --python-smoke', 1)[0]
        self.assertIn("LIBRECAD_REQUIRE_BUNDLED_PYTHON=1", linux_smoke)
        self.assertIn("-DWITH_BUNDLED_PYTHON_RUNTIME=ON", (
            REPOSITORY_ROOT / "snapcraft.yaml"
        ).read_text(encoding="utf-8"))

    def test_cmake_installs_verbatim_vendored_license_notices(self):
        license_files = {
            "lckernel/nano-signal-slot/LICENSE": "nano-signal-slot-LICENSE",
            "lckernel/tinyspline/LICENSE": "tinyspline-LICENSE",
            "third_party/kaguya/LICENSE_1_0.txt": "kaguya-LICENSE_1_0.txt",
            "third_party/pybind11/LICENSE": "pybind11-LICENSE",
        }
        self.assertIn(
            'set(LC_LICENSE_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/librecad/licenses")',
            self.cmake,
        )
        self.assertIn(
            'DESTINATION "${LC_LICENSE_INSTALL_DIR}/third-party"', self.cmake
        )
        for source_name, installed_name in license_files.items():
            with self.subTest(source=source_name):
                self.assertTrue((REPOSITORY_ROOT / source_name).is_file())
                self.assertIn(source_name, self.cmake)
                self.assertIn(installed_name, self.cmake)
                call_pattern = (
                    r'lc_install_third_party_license_notice\(\s*"'
                    + re.escape(source_name)
                    + r'"\s*"'
                    + re.escape(installed_name)
                    + r'"\s*\)'
                )
                self.assertRegex(self.cmake, call_pattern)
                self.assertIn(f'QStringLiteral("{installed_name}")', self.smoke_source)
        self.assertIn("if(WITH_PYTHONSCRIPT)", self.cmake)
        self.assertIn("if(WITH_LIBOPENCAD)", self.cmake)
        self.assertIn(
            'install(FILES "${CMAKE_SOURCE_DIR}/LICENSE"', self.cmake
        )

    def test_all_platform_workflows_run_runtime_smoke_contracts(self):
        test_command = "scripts.tests.test_runtime_smoke_contract"
        self.assertGreaterEqual(self.workflow.count(test_command), 3)
        self.assertIn(test_command, self.macos_workflow)

    def test_macos_bundle_job_runs_missing_python_runtime_negative_smoke(self):
        marker = "      - name: Reject a missing bundled Python runtime"
        start = self.macos_workflow.index(marker)
        end = self.macos_workflow.find("\n      - name:", start + len(marker))
        step = self.macos_workflow[start : end if end >= 0 else len(self.macos_workflow)]
        self.assertIn(
            'scripts/macos-install/testMissingPythonRuntime.sh "$RUNNER_TEMP/LibreCAD.app"',
            step,
        )

        helper = (
            REPOSITORY_ROOT / "scripts/macos-install/testMissingPythonRuntime.sh"
        ).read_text(encoding="utf-8")
        self.assertIn('Contents/Resources/librecad/python', helper)
        self.assertIn('PYTHONHOME="$poison"', helper)
        self.assertIn('DYLD_LIBRARY_PATH="$poison"', helper)
        self.assertIn("staged CPython runtime is missing beside the application", helper)
        self.assertIn("[[ $status -ne 1 ]]", helper)

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
        self._run_windows_check(
            "/private/tmp/librecad-smoke-test/bin/python314.dll"
        )

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

    def test_windows_build_runs_scripting_ui_and_custom_entity_suites(self):
        build_script = (
            REPOSITORY_ROOT
            / "scripts/windows-install/buildLibrecadAndCreatePackage.bat"
        ).read_text(encoding="utf-8")
        suites = (
            "EventBus.",
            "ScriptValueRoundTrip.",
            "ScriptCallbackInvoke.",
            "ScriptCallbackEquality.",
            "ScriptCallbackLifecycle.",
            "CustomEntityDxfRoundTrip.",
            "ScriptCustomEntity.",
            "ScriptCustomEntityBuilder.",
            "ScriptCustomEntityLuaRegistration.",
            "MWindowTest.",
            "PyGuiBridgeTest.",
            "ScriptDockTest.",
        )
        loop_start = build_script.index("for %%S in (")
        loop_end = build_script.index("\n)", loop_start) + len("\n)")
        discovery_loop = build_script[loop_start:loop_end]
        self.assertIn('findstr /C:"%%S"', discovery_loop)
        self.assertIn("if errorlevel 1", discovery_loop)
        for suite in suites:
            with self.subTest(suite=suite):
                self.assertIn(suite, discovery_loop)

        self.assertIn(
            '"%Python_EXECUTABLE%" ..\\..\\scripts\\run_required_gtests.py '
            '--binary lcunittest.exe --filter="'
            "EventBus.*:ScriptValueRoundTrip.*:ScriptCallbackInvoke.*:"
            "ScriptCallbackEquality.*:ScriptCallbackLifecycle.*:"
            "CustomEntityDxfRoundTrip.*:ScriptCustomEntity.*:"
            "ScriptCustomEntityBuilder.*:ScriptCustomEntityLuaRegistration.*:"
            "MWindowTest.*:PyGuiBridgeTest.*:ScriptDockTest.Python*"
            '"',
            build_script,
        )
        self.assertIn('findstr /C:"PythonCreatesLine"', build_script)
        self.assertIn(
            'findstr /C:"PythonEventDrivenOnEventReachesMainWindow"',
            build_script,
        )

    def test_windows_required_filters_use_shared_skip_aware_runner(self):
        build_script = (
            REPOSITORY_ROOT
            / "scripts/windows-install/buildLibrecadAndCreatePackage.bat"
        ).read_text(encoding="utf-8")
        required_filters = (
            "PythonRuntime.*:PythonFixture.*",
            "PersistenceTest.*",
            "LuaRuntimeTest.*:ParityTest.*",
            "EventBus.*:ScriptValueRoundTrip.*:ScriptCallbackInvoke.*:"
            "ScriptCallbackEquality.*:ScriptCallbackLifecycle.*:"
            "CustomEntityDxfRoundTrip.*:ScriptCustomEntity.*:"
            "ScriptCustomEntityBuilder.*:ScriptCustomEntityLuaRegistration.*:"
            "MWindowTest.*:PyGuiBridgeTest.*:ScriptDockTest.Python*",
        )
        for test_filter in required_filters:
            with self.subTest(test_filter=test_filter):
                self.assertIn(
                    '"%Python_EXECUTABLE%" ..\\..\\scripts\\run_required_gtests.py '
                    f'--binary lcunittest.exe --filter="{test_filter}"',
                    build_script,
                )
        self.assertNotIn("lcunittest.exe --gtest_filter=", build_script)

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

    def test_windows_native_extension_origins_remain_confined_to_provider(self):
        self.assertIn("import _ctypes", self.python_smoke)
        self.assertIn("import _sqlite3", self.python_smoke)
        self.assertIn("import _ssl", self.python_smoke)
        self.assertIn("_origin.is_relative_to(_prefix)", self.python_smoke)


if __name__ == "__main__":
    unittest.main()
