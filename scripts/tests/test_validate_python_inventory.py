"""Regression tests for selected-provider CMake inventory validation."""

import unittest

from scripts.validate_python_inventory import validate_inventory


WINDOWS_ROOT = r"C:\hostedtoolcache\windows\Python\3.14.8\x64"


def windows_inventory(**overrides):
    fields = {
        "provider_implementation": "CPython",
        "provider_version": "3.14.8",
        "provider_architecture": "AMD64",
        "provider_pointer_bits": "64",
        "provider_debug": "false",
        "provider_gil_disabled": "false",
        "executable": WINDOWS_ROOT + r"\python.exe",
        "include_dirs": WINDOWS_ROOT + r"\include;" + WINDOWS_ROOT + r"\Include",
        "link_library": WINDOWS_ROOT + r"\libs\python314.lib",
        "prefix": WINDOWS_ROOT,
        "stdlib": WINDOWS_ROOT + r"\Lib",
        "stdarch": WINDOWS_ROOT + r"\DLLs",
        "dynamic_modules": WINDOWS_ROOT + r"\DLLs",
    }
    fields.update(overrides)
    return "".join("{}={}\n".format(key, value) for key, value in fields.items())


class ValidatePythonInventoryTests(unittest.TestCase):
    def test_accepts_selected_windows_cpython_paths_case_insensitively(self):
        inventory = windows_inventory(
            executable=r"c:\HOSTEDTOOLCACHE\WINDOWS\PYTHON\3.14.8\X64\python.exe"
        )

        self.assertEqual([], validate_inventory(inventory, WINDOWS_ROOT))

    def test_accepts_posix_provider_root(self):
        root = "/opt/cpython/3.14"
        fields = {
            "provider_implementation": "CPython",
            "provider_version": "3.14.8",
            "provider_architecture": "arm64",
            "provider_pointer_bits": "64",
            "provider_debug": "false",
            "provider_gil_disabled": "false",
            "executable": root + "/bin/python3.14",
            "include_dirs": root + "/include/python3.14",
            "link_library": root + "/lib/libpython3.14.so",
            "prefix": root,
            "stdlib": root + "/lib/python3.14",
            "stdarch": root + "/lib/python3.14/lib-dynload",
            "dynamic_modules": root + "/lib/python3.14/lib-dynload",
        }
        inventory = "".join("{}={}\n".format(key, value) for key, value in fields.items())

        self.assertEqual(
            [],
            validate_inventory(
                inventory,
                root,
                expected_architecture="arm64",
            ),
        )

    def test_rejects_a_sibling_directory_that_shares_the_root_prefix(self):
        inventory = windows_inventory(
            stdlib=WINDOWS_ROOT + r"-external\Lib"
        )

        errors = validate_inventory(inventory, WINDOWS_ROOT)

        self.assertTrue(any("stdlib path is outside" in error for error in errors))

    def test_rejects_parent_traversal_out_of_the_selected_root(self):
        inventory = windows_inventory(
            link_library=WINDOWS_ROOT + r"\..\External\python314.lib"
        )

        errors = validate_inventory(inventory, WINDOWS_ROOT)

        self.assertTrue(any("link_library path is outside" in error for error in errors))

    def test_rejects_relative_inventory_paths(self):
        inventory = windows_inventory(executable="python.exe")

        errors = validate_inventory(inventory, WINDOWS_ROOT)

        self.assertTrue(any("executable path is outside" in error for error in errors))

    def test_rejects_any_include_directory_outside_the_selected_root(self):
        inventory = windows_inventory(
            include_dirs=WINDOWS_ROOT + r"\include;C:\Python314\include"
        )

        errors = validate_inventory(inventory, WINDOWS_ROOT)

        self.assertTrue(any("include_dirs path is outside" in error for error in errors))

    def test_rejects_non_cpython_and_incompatible_abi_markers(self):
        inventory = windows_inventory(
            provider_implementation="PyPy",
            provider_version="3.13.2",
            provider_architecture="arm64",
            provider_pointer_bits="32",
            provider_debug="true",
            provider_gil_disabled="true",
        )

        errors = validate_inventory(inventory, WINDOWS_ROOT)

        self.assertEqual(6, len(errors))

    def test_rejects_missing_or_empty_path_fields(self):
        missing = windows_inventory().replace(
            "stdlib=" + WINDOWS_ROOT + r"\Lib" + "\n", ""
        )
        missing_errors = validate_inventory(missing, WINDOWS_ROOT)
        self.assertTrue(any("missing required fields" in error for error in missing_errors))

        empty = windows_inventory(stdarch="")
        empty_errors = validate_inventory(empty, WINDOWS_ROOT)
        self.assertTrue(any("stdarch contains an empty path" in error for error in empty_errors))

    def test_rejects_duplicate_and_malformed_inventory_entries(self):
        inventory = windows_inventory() + "provider_version=3.14.8\ninvalid entry\n"

        errors = validate_inventory(inventory, WINDOWS_ROOT)

        self.assertTrue(any("duplicate inventory field" in error for error in errors))
        self.assertTrue(any("is not a key=value entry" in error for error in errors))


if __name__ == "__main__":
    unittest.main()
