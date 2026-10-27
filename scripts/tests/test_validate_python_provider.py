import tempfile
import unittest
from pathlib import Path

from scripts.validate_python_provider import config_flag, validate_provider


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
        errors = self.check(
            implementation="PyPy", debug=True, gil_disabled=True)
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


if __name__ == "__main__":
    unittest.main()
