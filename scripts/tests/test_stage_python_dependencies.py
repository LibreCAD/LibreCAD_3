"""Tests for macOS Mach-O dependency parsing."""

import sys
import plistlib
import tempfile
import unittest
from pathlib import Path
from unittest.mock import call, patch


SCRIPT_DIRECTORY = Path(__file__).resolve().parents[1] / "macos-install"
sys.path.insert(0, str(SCRIPT_DIRECTORY))

from stagePythonDependencies import RuntimeStager, minimum_macos  # noqa: E402


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


class MacOSMinimumVersionTests(unittest.TestCase):
    def test_bundle_executable_is_sealed_only_by_final_bundle_signing(self):
        for is_bundle in (False, True):
            with self.subTest(is_bundle=is_bundle), tempfile.TemporaryDirectory() as directory:
                prefix = Path(directory).resolve()
                python_root = prefix / "python"
                library_dir = prefix / "Frameworks"
                plugins = prefix / "PlugIns"
                for path in (python_root, library_dir, plugins):
                    path.mkdir()
                application = prefix / "librecad"
                application.touch()
                plugin = plugins / "libqsvgicon.dylib"
                plugin.touch()
                stager = RuntimeStager(
                    prefix, python_root, library_dir, "otool", "install_name_tool",
                    "codesign", application, False, [plugins], False,
                    bundle_info_plist=prefix / "Info.plist" if is_bundle else None,
                )
                with patch.object(stager, "_loads", return_value=[]), \
                     patch.object(stager, "_remove_external_rpaths"), \
                     patch.object(stager, "_record_bundle_minimum_os"), \
                     patch.object(stager, "_run", return_value="") as run:
                    stager.stage()
                commands = [args[0] for args, _ in run.call_args_list]
                self.assertIn(["codesign", "--force", "--sign", "-", str(plugin)], commands)
                signs_app = ["codesign", "--force", "--sign", "-", str(application)] in commands
                self.assertEqual(not is_bundle, signs_app)
                verifies_app = ["codesign", "--verify", "--strict", str(application)] in commands
                self.assertEqual(not is_bundle, verifies_app)

    def test_modern_and_legacy_load_commands(self):
        for commands, expected in (
            ("cmd LC_BUILD_VERSION\n platform 1\n minos 15.2\n sdk 27.0\n", (15, 2, 0)),
            ("cmd LC_BUILD_VERSION\n platform MACOS\n minos 14.4.1\n", (14, 4, 1)),
            ("cmd LC_VERSION_MIN_MACOSX\n version 10.15\n sdk 15.0\n", (10, 15, 0)),
        ):
            with self.subTest(commands=commands):
                self.assertEqual(expected, minimum_macos(commands))

    def test_fat_binary_uses_numeric_maximum_not_sdk_version(self):
        self.assertEqual((10, 15, 1), minimum_macos(
            "Load command 1\ncmd LC_VERSION_MIN_MACOSX\nversion 10.9\nsdk 27.0\n"
            "Load command 2\ncmd LC_BUILD_VERSION\nplatform 1\nminos 10.15.1\nsdk 14.0\n"))

    def test_unknown_or_malformed_deployment_target_fails_closed(self):
        for commands in (
            "cmd LC_RPATH\npath @loader_path (offset 12)\n",
            "cmd LC_BUILD_VERSION\nplatform 2\nminos 15.0\n",
            "cmd LC_VERSION_MIN_IPHONEOS\nversion 15.0\n",
            "cmd LC_BUILD_VERSION\nplatform 1\nminos invalid\n",
            "cmd LC_VERSION_MIN_MACOSX\nversion 0.0\n",
        ):
            with self.subTest(commands=commands), self.assertRaises(RuntimeError):
                minimum_macos(commands)

    def test_each_architecture_slice_requires_a_macos_target(self):
        first = "app (architecture x86_64):\nLoad command 0\ncmd LC_VERSION_MIN_MACOSX\nversion 10.15\n"
        for second, expected in (
            ("cmd LC_BUILD_VERSION\nplatform 1\nminos 15.0\n", (15, 0, 0)),
            ("cmd LC_UUID\nuuid ABC\n", None),
            ("cmd LC_VERSION_MIN_IPHONEOS\nversion 15.0\n", None),
        ):
            with self.subTest(second=second):
                commands = first + "app (architecture arm64):\nLoad command 0\n" + second
                if expected is None:
                    with self.assertRaises(RuntimeError):
                        minimum_macos(commands)
                else:
                    self.assertEqual(expected, minimum_macos(commands))

    def test_bundle_floor_covers_all_dependencies_and_preserves_application_policy(self):
        for existing, expected in ((None, "15.3.0"), ("16.1", "16.1.0")):
            with self.subTest(existing=existing), tempfile.TemporaryDirectory() as directory:
                contents = Path(directory)
                info_path = contents / "Info.plist"
                info = {"CFBundleName": "LibreCAD"}
                if existing is not None:
                    info["LSMinimumSystemVersion"] = existing
                info_path.write_bytes(plistlib.dumps(info))
                stager = RuntimeStager(contents, contents, contents, "otool",
                                       "install_name_tool", "codesign", None,
                                       False, [], False, bundle_info_plist=info_path)
                binaries = [contents / name for name in ("app", "python", "QtCore")]
                with patch.object(stager, "_run", side_effect=[
                    "cmd LC_VERSION_MIN_MACOSX\nversion 10.15\n",
                    "cmd LC_BUILD_VERSION\nplatform 1\nminos 15.3\n",
                    "cmd LC_BUILD_VERSION\nplatform 1\nminos 14.0\n",
                ]) as run:
                    stager._record_bundle_minimum_os(binaries)
                written = plistlib.loads(info_path.read_bytes())
                self.assertEqual(expected, written["LSMinimumSystemVersion"])
                self.assertEqual("LibreCAD", written["CFBundleName"])
                self.assertEqual(3, run.call_count)

    def test_bundle_builder_records_floor_before_final_resource_seal(self):
        source = (SCRIPT_DIRECTORY / "createBundle.sh").read_text(encoding="utf-8")
        self.assertIn('--bundle-info-plist "$contents/Info.plist"', source)
        self.assertLess(source.index("--bundle-info-plist"),
                        source.index('"$codesign_bin" --force --deep'))


if __name__ == "__main__":
    unittest.main()
