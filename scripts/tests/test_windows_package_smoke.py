"""Guard Windows scripting providers and installed Python packaging."""

import ast
import shutil
import subprocess
import tempfile
import unittest
import zipfile
from pathlib import Path
from types import SimpleNamespace


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
PACKAGE_SCRIPT = REPOSITORY_ROOT / "scripts/windows-install/buildLibrecadAndCreatePackage.bat"
CONAN_SCRIPT = REPOSITORY_ROOT / "scripts/windows-install/createConanDirAndInstallDependencies.bat"
CONANFILE = REPOSITORY_ROOT / "conanfile.py"
WORKFLOW = REPOSITORY_ROOT / ".github/workflows/main.yml"
UI_CMAKE = REPOSITORY_ROOT / "lcUI/CMakeLists.txt"
ROOT_CMAKE = REPOSITORY_ROOT / "CMakeLists.txt"
PYTHON_INVENTORY_VALIDATOR = REPOSITORY_ROOT / "scripts/validate_python_inventory.py"


class WindowsPackageSmokeContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.package_script = PACKAGE_SCRIPT.read_text(encoding="utf-8")
        cls.conan_script = CONAN_SCRIPT.read_text(encoding="utf-8")
        cls.conanfile = CONANFILE.read_text(encoding="utf-8")
        cls.workflow = WORKFLOW.read_text(encoding="utf-8")
        cls.ui_cmake = UI_CMAKE.read_text(encoding="utf-8")
        cls.root_cmake = ROOT_CMAKE.read_text(encoding="utf-8")
        cls.python_inventory_validator = PYTHON_INVENTORY_VALIDATOR.read_text(
            encoding="utf-8"
        )

    def assert_singleton_owned_by_library(self, stem, class_name):
        header = (REPOSITORY_ROOT / (stem + ".h")).read_text(encoding="utf-8")
        implementation = (REPOSITORY_ROOT / (stem + ".cpp")).read_text(encoding="utf-8")
        self.assertIn(f"static {class_name}& getInstance();", header)
        self.assertNotIn(f"static {class_name} _instance;", header)
        self.assertIn(f"{class_name}& {class_name}::getInstance()", implementation)
        self.assertIn(f"static {class_name} _instance;", implementation)

    def test_document_registry_singleton_is_owned_by_kernel_dll(self):
        self.assert_singleton_owned_by_library(
            "lckernel/cad/storage/documentlist", "DocumentList"
        )

    def test_custom_entity_manager_singleton_is_owned_by_lua_dll(self):
        self.assert_singleton_owned_by_library(
            "lcadluascript/managers/luacustomentitymanager", "LuaCustomEntityManager"
        )

    def test_windows_selects_one_shared_prebuilt_lua_provider(self):
        recipe = next(node for node in ast.parse(self.conanfile).body
                      if isinstance(node, ast.ClassDef) and node.name == "LibreCAD3")
        configure = next((node for node in recipe.body
                          if isinstance(node, ast.FunctionDef)
                          and node.name == "configure"), None)
        self.assertIsNotNone(configure, "Windows must select shared Lua across DLLs")
        namespace = {}
        module = ast.Module(body=[configure], type_ignores=[])
        exec(compile(ast.fix_missing_locations(module), str(CONANFILE), "exec"),
             namespace)
        for operating_system in ("Windows", "Linux", "Macos"):
            for initially_shared in (False, True):
                with self.subTest(os=operating_system, initially_shared=initially_shared):
                    lua_options = SimpleNamespace(shared=initially_shared)
                    consumer = SimpleNamespace(
                        settings=SimpleNamespace(os=operating_system),
                        options={"lua": lua_options})
                    namespace["configure"](consumer)
                    self.assertEqual(True if operating_system == "Windows"
                                     else initially_shared, lua_options.shared)
        self.assertIn("--build=never", self.conan_script)
        self.assertIn('"VirtualRunEnv"', self.conanfile)

    @unittest.skipUnless(shutil.which("cmake"), "CMake dependency-directory fixture")
    def test_runtime_install_searches_selected_conan_dll_directories(self):
        # Execute the actual MSVC install block, intercepting only install()
        # so this runs without pretending to resolve PE binaries on macOS.
        section = self.root_cmake.split("\n# The installed executable", 1)[0]
        block = "if(MSVC)\n" + section.rsplit("if(MSVC)\n", 1)[1]
        with tempfile.TemporaryDirectory() as temporary:
            script = Path(temporary) / "runtime-dirs.cmake"
            script.write_text(
                'cmake_minimum_required(VERSION 3.28)\n'
                'set(MSVC TRUE)\nset(WITH_PYTHONSCRIPT ON)\n'
                'set(CONAN_RUNTIME_LIB_DIRS "C:/Conan lua Ω/bin;C:/other/bin")\n'
                'set(Python_EXECUTABLE "C:/Selected Python/python.exe")\n'
                'set(ENV{QT_ROOT_DIR} "C:/Selected Qt")\n'
                'set(CMAKE_INSTALL_BINDIR bin)\n'
                'function(install)\n'
                '  foreach(expected "C:/Conan lua Ω/bin" "C:/other/bin" '
                '"C:/Selected Python" "C:/Selected Qt/bin")\n'
                '    if(NOT expected IN_LIST ARGV)\n'
                '      message(FATAL_ERROR "Missing runtime directory: ${expected}")\n'
                '    endif()\n'
                '  endforeach()\n'
                'endfunction()\n' + block, encoding="utf-8")
            result = subprocess.run(["cmake", "-P", str(script)],
                                    text=True, capture_output=True, timeout=30)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_windows_feature_matrix_uses_fresh_trees_and_existing_providers(self):
        step = self.workflow.split(
            "- name: Verify Windows Python feature configurations", 1)[1].split(
            "\n      - name:", 1)[0]
        self.assertIn("shell: cmd", step)
        self.assertIn('call "${{ github.workspace }}\\conan\\conanrun.bat"', step)
        disabled, no_persistence = step.split("REM Binding-only", 1)
        for required in (
            "build-python-off", "-DWITH_PYTHONSCRIPT=OFF",
            "-DCMAKE_DISABLE_FIND_PACKAGE_Python=TRUE", "-DWITH_UNITTESTS=ON",
            "--target librecad lcunittest", "scripts\\run_required_gtests.py",
            '--filter="LuaRuntimeTest.*"', '--require-suite "LuaRuntimeTest."',
        ):
            self.assertIn(required, disabled)
        for required in (
            "build-python-no-persistence", "-DWITH_PYTHONSCRIPT=ON",
            "-DWITH_PERSISTENCE=OFF", "-DWITH_QT_UI=OFF", "-DWITH_UNITTESTS=OFF",
            '-DPython_ROOT_DIR="%Python_ROOT_DIR%"',
            '-DPython_EXECUTABLE="%Python_EXECUTABLE%"', "--target lcpythonscript",
        ):
            self.assertIn(required, no_persistence)
        self.assertEqual(6, step.count("if errorlevel 1 exit /b 1"))
        self.assertNotIn("conan install", step)
        self.assertNotIn("pip install", step)

    @unittest.skipUnless(shutil.which("cmake") and shutil.which("cpack"),
                         "real nested CPack binary-install fixture")
    def test_nested_cpack_cannot_replace_the_binary_install_with_build_outputs(self):
        # Use the production settings/order and the vendor's CPack invocation.
        # A real ZIP consumes the same binary config as NSIS, without claiming
        # that this host can execute a Windows installer.
        start = self.root_cmake.lower().index('    set(cpack_binary_nsis')
        end = self.root_cmake.index('    include(CPack)', start) + len('    include(CPack)\n')
        settings = self.root_cmake[start:end]
        vendor = (REPOSITORY_ROOT / "lckernel/tinyspline/src/CMakeLists.txt").read_text()
        vendor = vendor.split("# CPack\n", 1)[1].split("# Dub\n", 1)[0]
        # CPack writes beneath build/bin in production. Exercise the real Qt
        # DLL install rule too: otherwise an out-of-tree archive fixture misses
        # the installer recursively copying its own staging directory.
        qt_install = "if(MSVC)\n" + self.ui_cmake.split(
            "# WinDeployQt runs after the build", 1)[1].split("if(MSVC)\n", 1)[1]
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary).resolve()
            source = root / "source"
            build = source / "build"
            source.mkdir()
            nested = source / "nested"
            nested.mkdir()
            (source / "LICENSE").write_text("project license")
            (source / "runtime.txt").write_text("installed provider data")
            (nested / "CMakeLists.txt").write_text(
                'set(TINYSPLINE_VERSION "0.6.0")\n' + vendor)
            packaging_first = start < self.root_cmake.index('add_subdirectory("lckernel")')
            (source / "CMakeLists.txt").write_text(
                'cmake_minimum_required(VERSION 3.28)\nproject(LibreCAD3 LANGUAGES NONE)\n'
                'set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")\n'
                'set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/unused-prefix")\n'
                'set(FINAL_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")\n'
                'file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin")\n'
                'file(WRITE "${CMAKE_BINARY_DIR}/bin/librecad.exe" "application")\n'
                'file(WRITE "${CMAKE_BINARY_DIR}/bin/build-only.txt" "not installed")\n'
                'file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin/platforms" '
                '"${CMAKE_BINARY_DIR}/bin/_CPack_Packages/stale/bin")\n'
                'file(WRITE "${CMAKE_BINARY_DIR}/bin/Qt6Core.dll" "Qt runtime")\n'
                'file(WRITE "${CMAKE_BINARY_DIR}/bin/platforms/qoffscreen.dll" "Qt plugin")\n'
                'file(WRITE "${CMAKE_BINARY_DIR}/bin/_CPack_Packages/stale/bin/stale.dll" '
                '"old installer payload, not runtime")\n'
                'file(WRITE "${CMAKE_BINARY_DIR}/bin/qt.conf" "[Paths]\\nPlugins=.\\n")\n'
                'install(FILES "${CMAKE_BINARY_DIR}/bin/librecad.exe" DESTINATION bin)\n'
                'install(FILES runtime.txt DESTINATION share/librecad/python)\n'
                'set(MSVC TRUE)\nset(CMAKE_INSTALL_BINDIR bin)\n' + qt_install +
                (settings if packaging_first else "") + 'add_subdirectory(nested)\n' +
                ("" if packaging_first else settings))
            commands = (["cmake", "-S", str(source), "-B", str(build)],
                        ["cmake", "--install", str(build), "--prefix", str(root / "install Ω")])
            for command in commands:
                result = subprocess.run(command, capture_output=True, text=True, timeout=30)
                self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            # Fail the old implementation before asking it to recurse into an
            # active CPack staging tree. No timeout/path-length failure needed.
            self.assertFalse((root / "install Ω/bin/_CPack_Packages").exists(),
                             "Qt runtime install copied CPack's staging tree")
            package_command = ["cpack", "--config", str(build / "CPackConfig.cmake"),
                               "-G", "ZIP", "-C", "Release", "-B", str(build / "bin"),
                               "-D", "CPACK_INCLUDE_TOPLEVEL_DIRECTORY=OFF"]
            # The second pack must exclude the previous pack's staging files
            # just as the first excludes its own live staging directory.
            for _ in range(2):
                result = subprocess.run(package_command, capture_output=True,
                                        text=True, timeout=30)
                self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            archives = list((build / "bin").glob("*.zip"))
            self.assertEqual(1, len(archives))
            with zipfile.ZipFile(archives[0]) as archive:
                files = {name for name in archive.namelist() if not name.endswith("/")}
                self.assertEqual({"bin/librecad.exe", "bin/Qt6Core.dll",
                                  "bin/platforms/qoffscreen.dll", "bin/qt.conf",
                                  "share/librecad/python/runtime.txt"}, files)
                self.assertEqual(b"installed provider data",
                                 archive.read("share/librecad/python/runtime.txt"))
            self.assertNotEqual((build / "CPackConfig.cmake").read_bytes(),
                                (build / "CPackSourceConfig.cmake").read_bytes())

    @unittest.skipUnless(shutil.which("cmake"), "CMake install-layout fixture")
    def test_msvc_python_package_installs_adjacent_to_runtime_and_license(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary).resolve()
            source, build, prefix = root / "source", root / "build", root / "install Ω"
            source.mkdir()
            shutil.copytree(REPOSITORY_ROOT / "lcUIPy", source / "lcUIPy",
                            ignore=shutil.ignore_patterns("__pycache__"))
            (source / "CMakeLists.txt").write_text(
                'cmake_minimum_required(VERSION 3.28)\n'
                'project(PythonInstallLayout LANGUAGES NONE)\n'
                'set(MSVC TRUE)\nset(WITH_PYTHONSCRIPT ON)\n'
                'set(CMAKE_INSTALL_DATADIR share)\n'
                'set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")\n'
                'add_subdirectory(lcUIPy)\n', encoding="utf-8")
            for command in (
                ["cmake", "-S", str(source), "-B", str(build)],
                ["cmake", "--install", str(build), "--prefix", str(prefix)],
            ):
                result = subprocess.run(command, text=True, capture_output=True,
                                        timeout=30, check=False)
                self.assertEqual(0, result.returncode, result.stdout + result.stderr)
            package = prefix / "share/librecad/lcUIPy"
            self.assertTrue((package / "__init__.py").is_file())
            self.assertTrue((package / "_runtimeaudit.py").is_file())
            self.assertFalse((prefix / "lcUIPy").exists())
            python_cmake = (REPOSITORY_ROOT / "lcadpythonscript/CMakeLists.txt").read_text()
            self.assertIn('${CMAKE_INSTALL_DATADIR}/librecad/python', python_cmake)
            self.assertIn('${CMAKE_INSTALL_DATADIR}/librecad/licenses/python', python_cmake)
            smoke = (REPOSITORY_ROOT / "lcUI/python/pyruntimesmoke.cpp").read_text()
            self.assertIn('packageDirectory.filePath("python")', smoke)
            self.assertIn('"licenses/python/LICENSE.txt"', smoke)

    def test_installed_smoke_runs_between_deployment_and_nsis(self):
        script = self.package_script.lower()
        deployment = script.index("windeployqt --include-plugins qoffscreen librecad.exe")
        install = script.index("cmake --install . --config release --prefix")
        smoke = script.index("librecad.exe\" --python-smoke")
        package = script.index("cpack --verbose -g nsis")

        self.assertLess(deployment, install)
        self.assertLess(install, smoke)
        self.assertLess(smoke, package)
        self.assertIn("%temp%\\librecad python smoke", script)

    def test_smoke_is_isolated_and_propagates_failure(self):
        script = self.package_script.lower()
        for required in (
            'set "pythonhome=%temp%\\librecad-poison-python-home"',
            'set "pythonpath=%temp%\\librecad-poison-python-path"',
            'set "qt_qpa_platform=offscreen"',
            'set "qt_plugin_path="',
            'set "qt_qpa_platform_plugin_path="',
            'set "path=%lc_python_smoke_prefix%\\bin;%systemroot%\\system32;%systemroot%"',
            'set "lc_python_smoke_exit=%errorlevel%"',
            'if not "%lc_python_smoke_exit%"=="0" exit /b 1',
        ):
            with self.subTest(required=required):
                self.assertIn(required, script)

    def test_installed_smoke_uses_staged_offscreen_plugin(self):
        workflow = self.workflow.lower()
        smoke_start = workflow.index(
            "- name: install and smoke the isolated python runtime"
        )
        smoke_end = workflow.index("- name: uploading exe", smoke_start)
        smoke_step = workflow[smoke_start:smoke_end]

        self.assertIn("$env:qt_qpa_platform = 'offscreen'", smoke_step)
        self.assertIn("$env:librecad_require_bundled_python = '1'", smoke_step)
        self.assertIn("& $app --python-smoke", smoke_step)
        self.assertLess(
            smoke_step.index("$env:qt_qpa_platform = 'offscreen'"),
            smoke_step.index("& $app --python-smoke"),
        )

        package_script = self.package_script.lower()
        self.assertIn(
            "windeployqt --include-plugins qoffscreen librecad.exe",
            package_script,
        )
        ui_cmake = self.ui_cmake.lower()
        self.assertIn(
            'install(directory "${cmake_runtime_output_directory}/"',
            ui_cmake,
        )
        self.assertIn('files_matching pattern "*.dll"', ui_cmake)
        self.assertIn('install(files "${cmake_runtime_output_directory}/qt.conf"', ui_cmake)

    def test_workflow_invokes_batch_with_cmd(self):
        workflow = self.workflow.lower()
        self.assertIn("name: build and package librecad on windows", workflow)
        self.assertIn("shell: cmd", workflow)
        self.assertIn(
            'run: call "${{ github.workspace }}\\scripts\\windows-install\\buildlibrecadandcreatepackage.bat"',
            workflow,
        )

    def test_windows_inventory_is_validated_before_required_upload(self):
        workflow = self.workflow.lower()
        windows_job_start = workflow.index("\n  windows:")
        windows_job_end = workflow.index("\n  dwg-sanitizers:", windows_job_start)
        windows_job = workflow[windows_job_start:windows_job_end]
        verify_step_start = windows_job.index(
            "- name: verify selected python runtime inventory"
        )
        upload_step_start = windows_job.index("- name: uploading exe")
        verify_step = windows_job[verify_step_start:upload_step_start]

        self.assertLess(verify_step_start, upload_step_start)
        for required in (
            "& $env:python_executable scripts/validate_python_inventory.py",
            "--inventory build/python-runtime-inventory.txt",
            '--expected-root "$env:pythonlocation"',
            "if ($lastexitcode -ne 0)",
        ):
            with self.subTest(required=required):
                self.assertIn(required, verify_step)
        self.assertIn("if-no-files-found: error", windows_job[upload_step_start:])

    def test_windows_inventory_paths_must_resolve_inside_setup_python_root(self):
        workflow = self.workflow.lower()
        windows_job_start = workflow.index("\n  windows:")
        windows_job_end = workflow.index("\n  dwg-sanitizers:", windows_job_start)
        windows_job = workflow[windows_job_start:windows_job_end]
        verify_start = windows_job.index(
            "- name: verify selected python runtime inventory"
        )
        upload_start = windows_job.index("- name: uploading exe", verify_start)
        verify_step = windows_job[verify_start:upload_start]

        self.assertIn('--expected-root "$env:pythonlocation"', verify_step)
        self.assertIn("def validate_inventory(", self.python_inventory_validator)
        self.assertIn("path_module.commonpath", self.python_inventory_validator)
        for field in (
            '"executable"',
            '"include_dirs"',
            '"link_library"',
            '"prefix"',
            '"stdlib"',
            '"stdarch"',
            '"dynamic_modules"',
        ):
            with self.subTest(field=field):
                self.assertIn(field, self.python_inventory_validator)

    def test_linux_inventory_is_verified_before_required_artifact_upload(self):
        workflow = self.workflow.lower()
        linux_job_start = workflow.index("\n  linux:")
        linux_job_end = workflow.index("\n  linux-arm64:", linux_job_start)
        linux_job = workflow[linux_job_start:linux_job_end]
        verify_step_start = linux_job.index(
            "- name: verify selected linux python runtime inventory"
        )
        upload_step_start = linux_job.index("- name: uploading appimage")
        verify_step = linux_job[verify_step_start:upload_step_start]

        self.assertLess(verify_step_start, upload_step_start)
        self.assertIn("test -s build/python-runtime-inventory.txt", verify_step)
        self.assertIn("cat build/python-runtime-inventory.txt", verify_step)
        self.assertIn("if-no-files-found: error", linux_job[upload_step_start:])

    def test_windows_jobs_use_prebuilt_msvc_2022_dependencies_only(self):
        workflow = self.workflow.lower()
        windows_start = workflow.index("\n  windows:")
        persistence_start = workflow.index("\n  windows-persistence:")
        update_assets_start = workflow.index("\n  updateassets:", persistence_start)
        windows_job = workflow[windows_start:persistence_start]
        persistence_job = workflow[persistence_start:update_assets_start]
        self.assertIn("runs-on: windows-2022", windows_job)
        self.assertIn("runs-on: windows-2022", persistence_job)
        self.assertNotIn("runs-on: windows-latest", workflow)

        package_script = self.package_script.lower()
        self.assertIn("both windows jobs pin windows-2022", package_script)
        self.assertNotIn("compiler.version=195", package_script)
        self.assertNotIn("toolset v145", package_script)
        self.assertIn('self.requires("libcurl/8.15.0")', self.conanfile)
        self.assertNotIn('self.requires("libcurl/8.2.1")', self.conanfile)

        conan = self.conan_script.lower()
        self.assertIn(
            "conan install .. -s build_type=release -s compiler.cppstd=17 --build=never",
            conan,
        )
        self.assertNotIn("--build missing", conan)
        self.assertNotIn("cmake_policy_version_minimum", conan)
        self.assertIn("qt 6.8.3 win64_msvc2022_64", windows_job)

    def test_conan_generators_have_one_explicit_location(self):
        self.assertIn('--output-folder "%LIBRECAD_ROOT%\\conan"', self.conan_script)
        self.assertIn('if not exist conan\\conan_toolchain.cmake', self.conan_script)
        self.assertIn('if not exist conan\\conanrun.bat', self.conan_script)
        self.assertIn('-DCMAKE_TOOLCHAIN_FILE="%cd%\\conan\\conan_toolchain.cmake"', self.package_script)
        self.assertIn('-DCMAKE_TOOLCHAIN_FILE="%CD%\\conan\\conan_toolchain.cmake"', self.workflow)

    def test_ui_data_exports_cover_qt_metaobjects_and_window_registry(self):
        for relative, name in (
            ("dialogs/addlayerdialog.h", "AddLayerDialog"),
            ("widgets/layermodel.h", "LayerModel"),
            ("widgets/linepatternpathpart.h", "LinePatternPathPart"),
            ("cadmdichild.h", "CadMdiChild"),
            ("lcadpaperviewer.h", "LCADPaperViewer"),
            ("windowmanager.h", "WindowManager"),
        ):
            with self.subTest(name=name):
                header = (UI_CMAKE.parent / relative).read_text(encoding="utf-8")
                self.assertIn('#include "lcui_api.h"', header)
                self.assertIn("class LC_UI_API " + name, header)
        self.assertIn("target_compile_definitions(lcui PUBLIC LC_UI_SHARED)", self.ui_cmake)

    @unittest.skipUnless(shutil.which("clang++"), "Clang is needed for the export regression")
    def test_ui_api_import_export_and_direct_executable_modes(self):
        header = UI_CMAKE.parent / "lcui_api.h"
        for defines, expected in (
            ([], ""),
            (["_WIN32"], ""),
            (["_WIN32", "LC_UI_SHARED"], "__declspec(dllimport)"),
            (["_WIN32", "LC_UI_SHARED", "lcui_EXPORTS"], "__declspec(dllexport)"),
            (["LC_UI_SHARED", "lcui_EXPORTS"], ""),
        ):
            for target in ([], ["--target=x86_64-pc-windows-msvc"]):
                with self.subTest(defines=defines, target=target):
                    result = subprocess.run(
                        ["clang++"] + target
                        # These are simulated modes, independent of the
                        # compiler host/target's predefined _WIN32 macro.
                        + ["-E", "-P", "-x", "c++", "-U_WIN32", "-include", str(header)]
                        + ["-D" + define for define in defines] + ["-"],
                        input="LC_UI_API int exported_data;\n", capture_output=True,
                        text=True, check=False,
                    )
                    self.assertEqual(0, result.returncode, result.stderr)
                    self.assertEqual((expected + " int exported_data;").strip(), result.stdout.strip())

    @unittest.skipUnless(shutil.which("cmake"), "CMake is needed for the syntax regression")
    def test_runtime_dependency_install_configures_with_and_without_search_paths(self):
        rule = "    install(RUNTIME_DEPENDENCY_SET " + self.root_cmake.split(
            "    install(RUNTIME_DEPENDENCY_SET ", 1
        )[1].split("\nendif()", 1)[0]
        for directories in ("", 'DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}"'):
            with self.subTest(directories=directories), tempfile.TemporaryDirectory() as directory:
                source = Path(directory)
                (source / "CMakeLists.txt").write_text(
                    'cmake_minimum_required(VERSION 3.28)\nproject(RuntimeInstall NONE)\n'
                    'set(CMAKE_INSTALL_BINDIR bin)\n'
                    f'set(_lc_runtime_dependency_directory_args {directories})\n{rule}\n',
                    encoding="utf-8",
                )
                result = subprocess.run(
                    ["cmake", "-S", str(source), "-B", str(source / "build")],
                    capture_output=True, text=True, check=False,
                )
                self.assertEqual(0, result.returncode, result.stdout + result.stderr)


if __name__ == "__main__":
    unittest.main()
