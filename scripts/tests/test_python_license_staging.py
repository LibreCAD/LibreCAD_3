"""Keep the selected CPython license in every staged runtime layout."""

import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
PYTHON_CMAKE = REPOSITORY_ROOT / "lcadpythonscript/CMakeLists.txt"
RUNTIME_SMOKE = REPOSITORY_ROOT / "lcUI/python/pyruntimesmoke.cpp"
MACOS_BUNDLE = REPOSITORY_ROOT / "scripts/macos-install/createBundle.sh"
ROOT_CMAKE = REPOSITORY_ROOT / "CMakeLists.txt"


class PythonLicenseStagingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.python_cmake = PYTHON_CMAKE.read_text(encoding="utf-8")
        cls.runtime_smoke = RUNTIME_SMOKE.read_text(encoding="utf-8")
        cls.macos_bundle = MACOS_BUNDLE.read_text(encoding="utf-8")

    def test_cmake_uses_only_the_selected_provider_license_locations(self):
        for candidate in (
            "${Python_STDLIB}/LICENSE.txt",
            "${Python_STDLIB}/LICENSE",
            "${_LC_PYTHON_EXECUTABLE_DIR}/LICENSE.txt",
            "${_LC_PYTHON_EXECUTABLE_DIR}/LICENSE",
            "${Python_ROOT_DIR}/LICENSE.txt",
            "${Python_ROOT_DIR}/LICENSE",
        ):
            with self.subTest(candidate=candidate):
                self.assertIn(candidate, self.python_cmake)
        self.assertIn("Could not find the selected CPython provider's", self.python_cmake)
        self.assertIn(
            'DESTINATION "${CMAKE_INSTALL_DATADIR}/librecad/licenses/python"',
            self.python_cmake,
        )
        self.assertIn('RENAME "LICENSE.txt"', self.python_cmake)

    def test_staged_runtime_smoke_requires_adjacent_provider_license(self):
        self.assertIn('"licenses/python/LICENSE.txt"', self.runtime_smoke)
        self.assertIn("expected_staged_python_license", self.runtime_smoke)
        self.assertIn("selected CPython license is missing from the staged package", self.runtime_smoke)

    def test_app_bundle_copies_the_installed_license_with_python_assets(self):
        self.assertIn(
            'ditto "$prefix/share/librecad" "$resources/librecad"',
            self.macos_bundle,
        )


@unittest.skipUnless(shutil.which("cmake"), "CMake source-license install fixture")
class SourceLicenseStagingTests(unittest.TestCase):
    ALWAYS = ("LICENSE", "lckernel/LICENSE", "lcadluascript/LICENSE",
              "lcviewernoqt/LICENSE", "third_party/kaguya/LICENSE_1_0.txt",
              "lckernel/nano-signal-slot/LICENSE", "lckernel/tinyspline/LICENSE")
    OPTIONAL = {"WITH_PERSISTENCE": ("persistence/LICENSE", "third_party/libdxfrw/COPYING"),
                "WITH_LUACMDINTERFACE": ("luacmdinterface/LICENSE",),
                "WITH_PYTHONSCRIPT": ("third_party/pybind11/LICENSE",)}

    def install(self, enabled, missing=None):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        source = Path(temporary.name) / "source"
        prefix = Path(temporary.name) / "installed source notices Ω"
        source.mkdir()
        for relative in self.ALWAYS + tuple(path for files in self.OPTIONAL.values()
                                           for path in files):
            if relative != missing:
                destination = source / relative
                destination.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(REPOSITORY_ROOT / relative, destination)
        block = ROOT_CMAKE.read_text(encoding="utf-8").split(
            "# BEGIN source license installation\n", 1)[1].split(
            "# END source license installation", 1)[0]
        switches = "".join("set({} {})\n".format(name, "ON" if value else "OFF")
                           for name, value in enabled.items())
        (source / "CMakeLists.txt").write_text(
            'cmake_minimum_required(VERSION 3.28)\n'
            'project(SourceLicenses LANGUAGES NONE)\n'
            'set(CMAKE_INSTALL_DATADIR share)\n' + switches + block,
            encoding="utf-8")
        build = source / "build"
        configure = subprocess.run(["cmake", "-S", str(source), "-B", str(build)],
                                   capture_output=True, text=True, timeout=30)
        self.assertEqual(0, configure.returncode, configure.stdout + configure.stderr)
        result = subprocess.run(["cmake", "--install", str(build), "--prefix", str(prefix)],
                                capture_output=True, text=True, timeout=30)
        return prefix / "share/librecad/licenses/source", result

    def test_installs_original_notices_for_enabled_source_dependencies(self):
        configurations = [dict.fromkeys(self.OPTIONAL, value) for value in (True, False)]
        configurations += [{name: name == selected for name in self.OPTIONAL}
                           for selected in self.OPTIONAL]
        for enabled in configurations:
            with self.subTest(enabled=enabled):
                installed, result = self.install(enabled)
                self.assertEqual(0, result.returncode, result.stdout + result.stderr)
                expected = self.ALWAYS + tuple(path for name, files in self.OPTIONAL.items()
                                              if enabled[name] for path in files)
                actual = sorted(str(path.relative_to(installed))
                                for path in installed.rglob("*") if path.is_file())
                self.assertEqual(sorted(expected), actual)
                for relative in expected:
                    self.assertEqual((REPOSITORY_ROOT / relative).read_bytes(),
                                     (installed / relative).read_bytes(), relative)

    def test_missing_source_notice_fails_install_instead_of_omitting_it(self):
        _, result = self.install(dict.fromkeys(self.OPTIONAL, True),
                                 missing="third_party/pybind11/LICENSE")
        self.assertNotEqual(0, result.returncode)
        self.assertIn("pybind11/LICENSE", result.stderr.replace("\\", "/"))


if __name__ == "__main__":
    unittest.main()
