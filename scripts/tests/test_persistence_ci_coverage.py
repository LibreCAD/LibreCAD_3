"""Keep the complete persistence/settings contract selected on every source OS."""

import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
REQUIRED = {"LuaFixture", "UiSettingsTest", "DxfRoundTripTest", "DxfExportTest",
            "FormatTest", "DocumentSourceTest", "MTextCodecTest"}


class PersistenceCiCoverageTests(unittest.TestCase):
    def test_mac_and_windows_require_complete_contract_not_only_smokes(self):
        for path in (".github/workflows/python-binding-macos.yml",
                     "scripts/windows-install/buildLibrecadAndCreatePackage.bat"):
            with self.subTest(path=path):
                source = (ROOT / path).read_text(encoding="utf-8")
                filters = re.findall(r"--filter=['\"]([^'\"]+)['\"]", source)
                self.assertTrue(any(REQUIRED <= set(re.findall(r"(\w+)\.\*", selected))
                                    for selected in filters))
                self.assertIn("run_required_gtests.py", source)

    def test_linux_requires_the_same_contract(self):
        source = (ROOT / "scripts/ubuntu-install/installDependenciesAndBuildRepo.sh").read_text()
        assignments = " ".join(re.findall(r'^CORE_SUITES="(.*)"$', source, re.MULTILINE))
        self.assertLessEqual(REQUIRED, set(re.findall(r"(\w+)\.\*", assignments)))

    def test_core_and_qt_runtime_tests_remain_separately_compiled(self):
        cmake = (ROOT / "unittest/CMakeLists.txt").read_text()
        runtime = (ROOT / "unittest/python/python_runtime_test.cpp").read_text()
        self.assertIn("LC_UNITTEST_WITH_QT", cmake)
        self.assertIn("#ifdef LC_UNITTEST_WITH_QT\n#include <python_runtime_config.h>", runtime)
        self.assertIn("ui/testuisettings.cpp", cmake)
