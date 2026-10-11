"""Guard the Windows package lane's installed Python smoke contract."""

import unittest
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
PACKAGE_SCRIPT = REPOSITORY_ROOT / "scripts/windows-install/buildLibrecadAndCreatePackage.bat"
CONAN_SCRIPT = REPOSITORY_ROOT / "scripts/windows-install/createConanDirAndInstallDependencies.bat"
CONANFILE = REPOSITORY_ROOT / "conanfile.py"
WORKFLOW = REPOSITORY_ROOT / ".github/workflows/main.yml"
UI_CMAKE = REPOSITORY_ROOT / "lcUI/CMakeLists.txt"
PYTHON_INVENTORY_VALIDATOR = REPOSITORY_ROOT / "scripts/validate_python_inventory.py"


class WindowsPackageSmokeContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.package_script = PACKAGE_SCRIPT.read_text(encoding="utf-8")
        cls.conan_script = CONAN_SCRIPT.read_text(encoding="utf-8")
        cls.conanfile = CONANFILE.read_text(encoding="utf-8")
        cls.workflow = WORKFLOW.read_text(encoding="utf-8")
        cls.ui_cmake = UI_CMAKE.read_text(encoding="utf-8")
        cls.python_inventory_validator = PYTHON_INVENTORY_VALIDATOR.read_text(
            encoding="utf-8"
        )

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
            "conan install .. -s build_type=release -s compiler.cppstd=14 --build=never",
            conan,
        )
        self.assertNotIn("--build missing", conan)
        self.assertNotIn("cmake_policy_version_minimum", conan)
        self.assertIn("qt 6.8.3 win64_msvc2022_64", windows_job)


if __name__ == "__main__":
    unittest.main()
