"""Exercise the production staging guard without a platform-specific Python SDK."""

import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path


PYTHON_CMAKE = Path(__file__).resolve().parents[2] / "lcadpythonscript/CMakeLists.txt"
LAYOUT_VARIABLES = (
    "LC_PYTHON_STAGED_STDLIB_REL",
    "LC_PYTHON_STAGED_STDARCH_REL",
    "LC_PYTHON_STAGED_DYNLIBS_REL",
)


@unittest.skipUnless(shutil.which("cmake"), "Python staging guard requires CMake")
class PythonRuntimeLayoutTests(unittest.TestCase):
    def configure_layout(self, paths):
        source = PYTHON_CMAKE.read_text(encoding="utf-8")
        guard = source.split("# BEGIN Python staged layout validation\n", 1)[1].split(
            "# END Python staged layout validation", 1)[0]
        with tempfile.TemporaryDirectory() as temporary:
            script = Path(temporary) / "layout.cmake"
            script.write_text(
                'cmake_minimum_required(VERSION 3.28)\n'
                'set(LC_PYTHON_RUNTIME_HOME "selected-provider")\n'
                + "".join('set({} [[{}]])\n'.format(name, value)
                          for name, value in zip(LAYOUT_VARIABLES, paths))
                + guard,
                encoding="utf-8")
            return subprocess.run(["cmake", "-P", str(script)], capture_output=True,
                                  text=True, timeout=30)

    def test_contained_unix_and_windows_layouts_are_accepted(self):
        for paths in (("lib/python3.14", "lib/python3.14", "lib/python3.14/lib-dynload"),
                      ("Lib", "Lib", "DLLs"), (".", ".", "DLLs")):
            with self.subTest(paths=paths):
                result = self.configure_layout(paths)
                self.assertEqual(0, result.returncode, result.stderr)

    def test_every_runtime_tree_rejects_parent_directory_escapes(self):
        for index, name in enumerate(LAYOUT_VARIABLES):
            for escape in ("..", "../base/lib/python3.14", "../../../../opt/python/lib"):
                with self.subTest(variable=name, escape=escape):
                    paths = ["Lib", "Lib", "DLLs"]
                    paths[index] = escape
                    result = self.configure_layout(paths)
                    self.assertNotEqual(0, result.returncode)
                    self.assertIn("escapes its staging directory", result.stderr)
                    self.assertIn(name, result.stderr)
                    self.assertIn("-DPython_EXECUTABLE", result.stderr)

    def test_absolute_runtime_tree_is_rejected(self):
        for index, name in enumerate(LAYOUT_VARIABLES):
            with self.subTest(variable=name):
                paths = ["Lib", "Lib", "DLLs"]
                paths[index] = Path(tempfile.gettempdir()).as_posix()
                result = self.configure_layout(paths)
                self.assertNotEqual(0, result.returncode)
                self.assertIn("escapes its staging directory", result.stderr)
                self.assertIn(name, result.stderr)


if __name__ == "__main__":
    unittest.main()
