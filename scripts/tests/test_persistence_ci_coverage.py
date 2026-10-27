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
        # The fresh-process runtime checks drive the probes through embedded
        # CPython rather than Qt, so Qt-free test builds still run them.
        self.assertIn("#include <python_runtime_config.h>", runtime)
        self.assertNotRegex(runtime, r'#include\s*[<"]Q[A-Z]')
        self.assertIn("ui/testuisettings.cpp", cmake)

    def test_settings_signal_metaobject_is_exported_to_windows_consumers(self):
        header = (ROOT / "lcUI/widgets/customizeToolbar/customizetoolbar.h").read_text()
        self.assertIn('#include "lcui_api.h"', header)
        self.assertIn("class LC_UI_API CustomizeToolbar : public QDialog", header)
        self.assertIn("Q_OBJECT", header)
        cmake = (ROOT / "lcUI/CMakeLists.txt").read_text()
        self.assertIn("target_compile_definitions(lcui PUBLIC LC_UI_SHARED)", cmake)

    def test_fixture_readers_close_before_windows_cleanup(self):
        for relative, suite, name, stream, path in (
            ("dxfexporttest.cpp", "DxfExportTest", "Phase3TablesAndHeaderFidelity", "in", "dxfPath"),
            ("dxfroundtriptest.cpp", "DxfRoundTripTest", "EveryAdvertisedSaveTypeProducesAFile", "written", "path"),
            ("dxfroundtriptest.cpp", "DxfRoundTripTest", "AsciiRoundTripStaysAscii", "written", "resaved"),
            ("dxfroundtriptest.cpp", "DxfRoundTripTest", "AttributedInsertSurfacesItsValues", "written", "path"),
            ("dxfroundtriptest.cpp", "DxfRoundTripTest", "DrawingUnitsSurviveASave", "written", "resaved"),
        ):
            with self.subTest(name=name):
                source = (ROOT / "unittest/persistence" / relative).read_text()
                body = source.split(f"TEST({suite}, {name})", 1)[1].split("\nTEST(", 1)[0]
                self.assertLess(body.index(f"{stream}.close();"),
                                body.rindex(f"boost::filesystem::remove({path});"))

    def test_lua_missing_path_is_passed_as_a_value_not_source(self):
        source = (ROOT / "unittest/scripting/luapersistence_test.cpp").read_text()
        body = source.split("TEST_F(LuaFixture, ImportResultIsReadable)", 1)[1].split("\nTEST_F(", 1)[0]
        self.assertIn('state["missing_path"] = missing;', body)
        self.assertNotIn('"missing_path = \'"', body)
