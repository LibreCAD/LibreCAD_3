import sys
import tempfile
import unittest
from pathlib import Path

from scripts.validate_python_provider import (
    config_flag,
    read_inventory,
    validate_inventory,
    validate_provider,
)


class ValidatePythonProviderTest(unittest.TestCase):
    def setUp(self):
        self.tempdir = tempfile.TemporaryDirectory()
        self.root = Path(self.tempdir.name)
        self.prefix = self.root / "python"
        self.include = self.prefix / "include" / "python3.14"
        self.library_dir = self.prefix / "lib"
        self.include.mkdir(parents=True)
        self.library_dir.mkdir()
        self.library = self.library_dir / "libpython3.14.so"
        self.library.touch()
        self.provider = {
            "implementation": "CPython",
            "version": "3.14.4",
            "pointer_bits": 64,
            "architecture": "x86_64",
            "gil_disabled": False,
            "debug": False,
            "include_dirs": [str(self.include)],
            "prefixes": [str(self.prefix)],
            "library_names": ["libpython3.14.so"],
            "platform": "linux-x86_64",
            "soabi": "cpython-314-x86_64-linux-gnu",
            "extension_suffix": ".cpython-314-x86_64-linux-gnu.so",
            "library": "libpython3.14.so",
            "libdir": str(self.library_dir),
            "dynamic_modules": str(self.prefix / "lib/python3.14/lib-dynload"),
        }

    def tearDown(self):
        self.tempdir.cleanup()

    def check(self, provider=None, **overrides):
        provider = dict(provider or self.provider)
        provider.update(overrides)
        return validate_provider(
            provider,
            "3.14.4",
            64,
            "AMD64",
            [str(self.include)],
            str(self.library),
        )

    def test_matching_provider_passes_with_architecture_alias(self):
        self.assertEqual(self.check(), [])

    def test_config_flags_accept_string_zero_without_truthiness_bug(self):
        self.assertFalse(config_flag("0"))
        self.assertFalse(config_flag(None))
        self.assertTrue(config_flag("1"))

    def test_rejects_version_mismatch(self):
        errors = self.check(version="3.13.9")
        self.assertTrue(any("version" in error for error in errors))

    def test_rejects_pointer_width_mismatch(self):
        errors = validate_provider(
            self.provider, "3.14.4", 32, "AMD64", [str(self.include)], str(self.library))
        self.assertTrue(any("pointer width" in error for error in errors))

    def test_rejects_architecture_mismatch(self):
        errors = self.check(architecture="aarch64")
        self.assertTrue(any("architecture" in error for error in errors))

    def test_rejects_non_cpython_and_unsupported_abis(self):
        errors = self.check(implementation="PyPy", debug=True, gil_disabled=True)
        self.assertTrue(any("only CPython" in error for error in errors))
        self.assertTrue(any("debug CPython" in error for error in errors))
        self.assertTrue(any("free-threaded" in error for error in errors))

    def test_rejects_header_mismatch(self):
        errors = self.check(include_dirs=[str(self.root / "other-headers")])
        self.assertTrue(any("headers do not match" in error for error in errors))

    def test_rejects_library_outside_interpreter_prefix(self):
        other_library = self.root / "other-python" / "libpython.so"
        other_library.parent.mkdir()
        other_library.touch()
        errors = validate_provider(
            self.provider, "3.14.4", 64, "AMD64", [str(self.include)], str(other_library))
        self.assertTrue(any("outside the selected interpreter prefix" in error
                            for error in errors))

    def test_rejects_library_with_another_python_abi_version(self):
        other_version_library = self.library_dir / "libpython3.13.so"
        other_version_library.touch()
        errors = validate_provider(
            self.provider, "3.14.4", 64, "AMD64", [str(self.include)],
            str(other_version_library))
        self.assertTrue(any("does not match CPython 3.14" in error for error in errors))

    def test_inventory_parser_preserves_path_values(self):
        path = self.root / "inventory.txt"
        path.write_text(
            "version=3.14.4\n"
            "target_pointer_bits=64\n"
            "target_architecture=AMD64\n"
            "executable=C:/Program Files/Python/python.exe\n"
            "include_dirs=C:/Python/include;C:/Python/platinclude\n"
            "link_library=C:/Python/libs/python314.lib\n",
            encoding="utf-8")
        inventory = read_inventory(path)
        self.assertEqual("C:/Program Files/Python/python.exe", inventory["executable"])
        self.assertEqual(["C:/Python/include", "C:/Python/platinclude"],
                         inventory["include_dirs"])

    def test_inventory_parser_rejects_missing_required_fields(self):
        path = self.root / "incomplete-inventory.txt"
        path.write_text("version=3.14.4\n", encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "missing required fields"):
            read_inventory(path)

    def test_inventory_parser_rejects_duplicate_fields(self):
        path = self.root / "duplicate-inventory.txt"
        path.write_text(
            "version=3.14.4\nversion=3.14.5\n",
            encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "duplicate inventory field"):
            read_inventory(path)

    def test_rejects_validator_under_a_different_python_executable(self):
        inventory = {
            "executable": str(self.root / "other-python"),
            "version": "3.14.4",
            "target_pointer_bits": 64,
            "target_architecture": "AMD64",
            "include_dirs": [str(self.include)],
            "link_library": str(self.library),
        }
        errors = validate_inventory(self.provider, inventory, running_executable=sys.executable)
        self.assertTrue(any("not running under CMake's selected" in error for error in errors))

    def test_rejects_cmake_provider_metadata_drift(self):
        inventory = {
            "executable": sys.executable,
            "version": "3.14.4",
            "target_pointer_bits": 64,
            "target_architecture": "AMD64",
            "include_dirs": [str(self.include)],
            "link_library": str(self.library),
            "provider_soabi": "wrong-soabi",
        }
        errors = validate_inventory(self.provider, inventory)
        self.assertTrue(any("provider_soabi" in error for error in errors))


if __name__ == "__main__":
    unittest.main()
