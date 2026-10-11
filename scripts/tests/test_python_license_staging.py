"""Keep the selected CPython license in every staged runtime layout."""

import unittest
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
PYTHON_CMAKE = REPOSITORY_ROOT / "lcadpythonscript/CMakeLists.txt"
RUNTIME_SMOKE = REPOSITORY_ROOT / "lcUI/python/pyruntimesmoke.cpp"
MACOS_BUNDLE = REPOSITORY_ROOT / "scripts/macos-install/createBundle.sh"


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


if __name__ == "__main__":
    unittest.main()
