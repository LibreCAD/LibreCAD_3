#!/usr/bin/env python3
"""Validate that CMake and the selected CPython describe one native ABI."""

import argparse
import os
import platform
import struct
import sys
import sysconfig
from pathlib import Path


def normalize_architecture(value):
    """Normalize common CMake/Python names without guessing unknown CPUs."""
    arch = value.strip().lower().replace("-", "_")
    aliases = {
        "amd64": "x86_64",
        "x64": "x86_64",
        "x86_64": "x86_64",
        "i386": "x86",
        "i486": "x86",
        "i586": "x86",
        "i686": "x86",
        "x86": "x86",
        "arm64": "arm64",
        "aarch64": "arm64",
        "armv8": "arm64",
        "armv8_a": "arm64",
        "armv7l": "arm",
        "armv7": "arm",
    }
    return aliases.get(arch, arch)


def normalized_path(path):
    return os.path.normcase(os.path.realpath(os.path.expanduser(str(path))))


def config_flag(value):
    if isinstance(value, bool):
        return value
    if value is None:
        return False
    return str(value).strip().lower() not in {"", "0", "false", "no", "none"}


def is_within(path, root):
    try:
        candidate = normalized_path(path)
        normalized_root = normalized_path(root)
        return os.path.commonpath((candidate, normalized_root)) == normalized_root
    except ValueError:
        return False


def collect_provider():
    include_dirs = {
        sysconfig.get_path("include"),
        sysconfig.get_path("platinclude"),
    }
    include_dirs.discard(None)
    include_dirs.discard("")
    library_names = {
        str(sysconfig.get_config_var(key) or "")
        for key in ("LDLIBRARY", "LIBRARY", "PY3LIBRARY", "INSTSONAME")
    }
    library_names.discard("")
    return {
        "implementation": platform.python_implementation(),
        "version": ".".join(str(part) for part in sys.version_info[:3]),
        "pointer_bits": struct.calcsize("P") * 8,
        "architecture": platform.machine(),
        "gil_disabled": config_flag(sysconfig.get_config_var("Py_GIL_DISABLED")),
        "debug": config_flag(sysconfig.get_config_var("Py_DEBUG")),
        "include_dirs": sorted(include_dirs),
        "prefixes": sorted({sys.prefix, sys.base_prefix}),
        "library_names": sorted(library_names),
        "platform": sysconfig.get_platform(),
        "soabi": str(sysconfig.get_config_var("SOABI") or ""),
        "extension_suffix": str(sysconfig.get_config_var("EXT_SUFFIX") or ""),
        "library": str(sysconfig.get_config_var("LDLIBRARY")
                        or sysconfig.get_config_var("LIBRARY") or ""),
        "libdir": str(sysconfig.get_config_var("LIBDIR") or ""),
        "dynamic_modules": str(sysconfig.get_config_var("DESTSHARED") or ""),
    }


def read_inventory(path):
    """Read CMake's key=value inventory, rejecting ambiguous or missing fields."""
    fields = {}
    for line_number, line in enumerate(Path(path).read_text(encoding="utf-8").splitlines(), 1):
        if not line:
            continue
        key, separator, value = line.partition("=")
        if not separator or not key:
            raise ValueError("malformed inventory line {}".format(line_number))
        if key in fields:
            raise ValueError("duplicate inventory field: {}".format(key))
        fields[key] = value

    required = (
        "version", "target_pointer_bits", "target_architecture", "executable",
        "include_dirs", "link_library",
    )
    missing = [key for key in required if not fields.get(key)]
    if missing:
        raise ValueError("inventory is missing required fields: {}".format(", ".join(missing)))
    try:
        fields["target_pointer_bits"] = int(fields["target_pointer_bits"])
    except ValueError as error:
        raise ValueError("target_pointer_bits must be an integer") from error
    fields["include_dirs"] = [item for item in fields["include_dirs"].split(";") if item]
    return fields


def validate_provider(provider, expected_version, expected_pointer_bits,
                      expected_architecture, cmake_include_dirs,
                      cmake_library):
    errors = []
    if provider["implementation"] != "CPython":
        errors.append("only CPython is supported for embedded scripting")

    if provider["version"] != expected_version:
        errors.append(
            "interpreter version {} does not match FindPython version {}".format(
                provider["version"], expected_version))

    if provider["pointer_bits"] != expected_pointer_bits:
        errors.append(
            "interpreter pointer width {} does not match target width {}".format(
                provider["pointer_bits"], expected_pointer_bits))

    actual_arch = normalize_architecture(provider["architecture"])
    target_arch = normalize_architecture(expected_architecture)
    if actual_arch != target_arch:
        errors.append(
            "interpreter architecture {} does not match target architecture {}".format(
                provider["architecture"], expected_architecture))

    if provider["debug"]:
        errors.append("debug CPython providers are not supported")
    if provider["gil_disabled"]:
        errors.append("free-threaded CPython is not supported by the current GIL model")

    configured_includes = {normalized_path(path) for path in cmake_include_dirs}
    interpreter_includes = {normalized_path(path) for path in provider["include_dirs"]}
    if configured_includes != interpreter_includes:
        errors.append("FindPython headers do not match the selected interpreter")

    if not cmake_library:
        errors.append("FindPython did not select an embedded Python library")
    elif not any(is_within(cmake_library, prefix) for prefix in provider["prefixes"]):
        errors.append(
            "FindPython library is outside the selected interpreter prefix: {}".format(
                cmake_library))
    else:
        try:
            major, minor, _patch = (int(part) for part in provider["version"].split("."))
        except ValueError:
            errors.append("interpreter version is not a numeric CPython version")
        else:
            normalized_library = normalized_path(cmake_library).replace("\\", "/")
            library_name = os.path.basename(normalized_library)
            expected_names = {
                os.path.basename(name.replace("\\", "/")).lower()
                for name in provider["library_names"]
            }
            version_markers = (
                "python{}.{}".format(major, minor),
                "python{}{}".format(major, minor),
                "/versions/{}.{}/".format(major, minor),
            )
            if (library_name.lower() not in expected_names and
                    not any(marker in normalized_library.lower()
                            for marker in version_markers)):
                errors.append(
                    "FindPython library does not match CPython {}.{}: {}".format(
                        major, minor, cmake_library))

    return errors


def validate_inventory(provider, inventory, running_executable=None):
    errors = []
    running_executable = running_executable or sys.executable
    if normalized_path(running_executable) != normalized_path(inventory["executable"]):
        errors.append("validator is not running under CMake's selected Python executable")

    errors.extend(validate_provider(
        provider,
        inventory["version"],
        inventory["target_pointer_bits"],
        inventory["target_architecture"],
        inventory["include_dirs"],
        inventory["link_library"],
    ))

    inventory_provider_fields = (
        ("provider_implementation", "implementation"),
        ("provider_version", "version"),
        ("provider_platform", "platform"),
        ("provider_soabi", "soabi"),
        ("provider_extension_suffix", "extension_suffix"),
        ("provider_library", "library"),
        ("provider_libdir", "libdir"),
        ("provider_dynamic_modules", "dynamic_modules"),
    )
    for inventory_key, provider_key in inventory_provider_fields:
        if inventory_key in inventory and inventory[inventory_key] != provider[provider_key]:
            errors.append("{} does not match the selected interpreter".format(inventory_key))
    return errors


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--inventory", required=True, help="CMake python-runtime-inventory.txt")
    args = parser.parse_args(argv)

    try:
        inventory = read_inventory(args.inventory)
    except (OSError, UnicodeError, ValueError) as error:
        print("Python provider validation failed: {}".format(error), file=sys.stderr)
        return 1

    provider = collect_provider()
    errors = validate_inventory(provider, inventory)
    if errors:
        for error in errors:
            print("Python provider validation failed: {}".format(error), file=sys.stderr)
        return 1

    print(
        "Validated embedded provider: CPython {} / {} / {}-bit / GIL-enabled release ABI".format(
            provider["version"], provider["architecture"], provider["pointer_bits"]))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
