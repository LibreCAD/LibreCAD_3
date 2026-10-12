"""SDK-free Mach-O runtime-origin control flow; real API runs in app smoke."""

import importlib.util
import os
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import Mock, patch

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("macos_origin_audit", ROOT / "lcUIPy/_runtimeaudit.py")
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)


class MacOSRuntimeOriginTests(unittest.TestCase):
    def test_resolves_actual_c_api_address_and_rejects_external_origin(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary).resolve()
            packaged = root / "Contents/Frameworks"
            packaged.mkdir(parents=True)
            image = packaged / "libpython3.14.dylib"
            image.write_bytes(b"fixture image, not loaded")
            def resolve(address, output):
                self.assertIsNotNone(address.value)
                output._obj.dli_fname = os.fsencode(image)
                return 1
            function = Mock(side_effect=resolve)
            with patch("sys.platform", "darwin"), \
                    patch.object(audit.ctypes, "CDLL", return_value=SimpleNamespace(dladdr=function)):
                self.assertEqual(image, audit.audit_macos_runtime(packaged))
                external = root / "external"
                external.mkdir()
                with self.assertRaisesRegex(RuntimeError, "outside"):
                    audit.audit_macos_runtime(external)
            self.assertEqual(audit.ctypes.c_int, function.restype)
            self.assertEqual(audit.ctypes.c_void_p, function.argtypes[0])

    def test_missing_image_information_or_failed_lookup_rejects_qualification(self):
        with tempfile.TemporaryDirectory() as temporary:
            for result in (0, 1):
                with self.subTest(result=result), patch("sys.platform", "darwin"), \
                        patch.object(audit.ctypes, "CDLL", return_value=SimpleNamespace(
                            dladdr=Mock(return_value=result))):
                    with self.assertRaisesRegex(RuntimeError, "could not resolve"):
                        audit.audit_macos_runtime(temporary)

    def test_platform_and_packaged_directory_are_required(self):
        with patch("sys.platform", "win32"):
            with self.assertRaisesRegex(RuntimeError, "requires macOS"):
                audit.audit_macos_runtime("/not-used")
        with patch("sys.platform", "darwin"), tempfile.TemporaryDirectory() as temporary:
            file = Path(temporary) / "not-a-directory"
            file.write_bytes(b"file")
            with self.assertRaisesRegex(RuntimeError, "directory is missing"):
                audit.audit_macos_runtime(file)

    def test_both_macos_lanes_require_loaded_origin_for_every_installed_artifact(self):
        workflow = (ROOT / ".github/workflows/python-binding-macos.yml").read_text()
        for start, end in (("Install LibreCAD", "Create and smoke"),
                           ("Create and smoke", "Archive the verified"),
                           ("Archive the verified", "Upload the verified")):
            block = workflow.split("- name: " + start, 1)[1].split("- name: " + end, 1)[0]
            self.assertEqual(block.count("--python-smoke"), block.count("LIBRECAD_REQUIRE_BUNDLED_PYTHON=1"))
            self.assertGreater(block.count("--python-smoke"), 0)
        for evidence in ("macos-installed-smoke.log", "macos-smoke.log", "macos-relocated-smoke.log"):
            self.assertIn("--evidence build/" + evidence, workflow)


if __name__ == "__main__":
    unittest.main()
