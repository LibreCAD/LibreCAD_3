"""Tests for macOS Mach-O dependency parsing."""

import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import call, patch


SCRIPT_DIRECTORY = Path(__file__).resolve().parents[1] / "macos-install"
sys.path.insert(0, str(SCRIPT_DIRECTORY))

from stagePythonDependencies import RuntimeStager  # noqa: E402


class MachODependencyParsingTests(unittest.TestCase):
    def make_stager(self):
        return RuntimeStager(
            Path("/bundle/Contents"),
            Path("/bundle/Contents/Resources/librecad/python"),
            Path("/bundle/Contents/Frameworks"),
            "otool",
            "install_name_tool",
            "codesign",
            None,
            False,
            [],
            False,
        )

    def test_dylib_install_id_is_not_a_dependency(self):
        binary = Path("/bundle/Contents/Frameworks/QtQmlMeta.framework/Versions/A/QtQmlMeta")
        install_id = "/opt/homebrew/lib/QtQmlMeta.framework/Versions/A/QtQmlMeta"
        dependency = "@rpath/QtCore.framework/Versions/A/QtCore"
        stager = self.make_stager()
        with patch.object(
            stager,
            "_run",
            side_effect=[
                "{}:\n\t{} (compatibility version 6.0.0, current version 6.11.2)\n"
                "\t{} (compatibility version 6.0.0, current version 6.11.2)\n".format(
                    binary, install_id, dependency
                ),
                "{}:\n{}\n".format(binary, install_id),
            ],
        ) as run:
            self.assertEqual([dependency], stager._loads(binary))

        self.assertEqual(
            [call(["otool", "-L", str(binary)]), call(["otool", "-D", str(binary)])],
            run.call_args_list,
        )

    def test_executable_dependencies_are_preserved_when_it_has_no_install_id(self):
        binary = Path("/bundle/Contents/MacOS/librecad")
        dependency = "@rpath/QtCore.framework/Versions/A/QtCore"
        stager = self.make_stager()
        with patch.object(
            stager,
            "_run",
            side_effect=[
                "{}:\n\t{} (compatibility version 6.0.0, current version 6.11.2)\n".format(
                    binary, dependency
                ),
                "{}:\n".format(binary),
            ],
        ):
            self.assertEqual([dependency], stager._loads(binary))

    def test_rpath_dependency_resolves_from_configured_provider_directory(self):
        with tempfile.TemporaryDirectory() as temporary_directory:
            dependency_directory = Path(temporary_directory)
            dependency = dependency_directory / "libdxfrw.1.dylib"
            dependency.touch()
            stager = RuntimeStager(
                Path("/bundle/Contents"),
                Path("/bundle/Contents/Resources/librecad/python"),
                Path("/bundle/Contents/Frameworks"),
                "otool",
                "install_name_tool",
                "codesign",
                None,
                False,
                [],
                False,
                [dependency_directory],
            )
            with patch.object(stager, "_rpaths", return_value=[]):
                resolved = stager._resolve_load(
                    Path("/bundle/Contents/MacOS/librecad"),
                    "@rpath/libdxfrw.1.dylib",
                )

        self.assertEqual(dependency.resolve(), resolved)


if __name__ == "__main__":
    unittest.main()
