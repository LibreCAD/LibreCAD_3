#!/usr/bin/env python3
"""Fail fast when CMake's embedded-Python provider is internally inconsistent."""

import argparse
import os
import platform
import struct
import sys
import sysconfig
from pathlib import Path


def normalize_architecture(value):
    """Normalize common CMake/Python spellings without guessing unknown CPUs."""
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
        normalized_candidate = normalized_path(path)
        normalized_root = normalized_path(root)
        return os.path.commonpath((normalized_candidate, normalized_root)) == normalized_root
    except ValueError:
        return False


def collect_provider():
    include_dirs = {
        sysconfig.get_path("include"),
        sysconfig.get_path("platinclude"),
    }
    include_dirs.discard(None)
    include_dirs.discard("")
    return {
        "implementation": platform.python_implementation(),
        "version": ".".join(str(part) for part in sys.version_info[:3]),
        "pointer_bits": struct.calcsize("P") * 8,
        "architecture": platform.machine(),
        "gil_disabled": config_flag(sysconfig.get_config_var("Py_GIL_DISABLED")),
        "debug": config_flag(sysconfig.get_config_var("Py_DEBUG")),
        "include_dirs": sorted(include_dirs),
        "prefixes": sorted({sys.prefix, sys.base_prefix}),
        "library_names": sorted({
            str(sysconfig.get_config_var(key) or "")
            for key in ("LDLIBRARY", "LIBRARY", "PY3LIBRARY", "INSTSONAME")
            if sysconfig.get_config_var(key)
        }),
    }


def validate_provider(provider, expected_version, expected_pointer_bits,
                      expected_architecture, cmake_include_dirs,
                      cmake_library):
    errors = []
    if provider["implementation"] != "CPython":
        errors.append("only CPython is supported for embedded scripting")

    if provider["version"] != expected_version:
        errors.append(
            f"interpreter version {provider['version']} does not match "
            f"FindPython version {expected_version}")

    if provider["pointer_bits"] != expected_pointer_bits:
        errors.append(
            f"interpreter pointer width {provider['pointer_bits']} does not "
            f"match target width {expected_pointer_bits}")

    actual_arch = normalize_architecture(provider["architecture"])
    target_arch = normalize_architecture(expected_architecture)
    if actual_arch != target_arch:
        errors.append(
            f"interpreter architecture {provider['architecture']} does not "
            f"match target architecture {expected_architecture}")

    if provider["debug"]:
        errors.append("debug CPython providers are not supported")
    if provider["gil_disabled"]:
        errors.append("free-threaded CPython is not supported by the current GIL model")

    configured_includes = {normalized_path(path) for path in cmake_include_dirs}
    for include_dir in provider["include_dirs"]:
        if normalized_path(include_dir) not in configured_includes:
            errors.append(
                f"FindPython headers do not match interpreter headers: {include_dir}")

    if not cmake_library:
        errors.append("FindPython did not select an embedded Python library")
    elif not any(is_within(cmake_library, prefix) for prefix in provider["prefixes"]):
        errors.append(
            f"FindPython library is outside the selected interpreter prefix: {cmake_library}")
    else:
        major, minor, _patch = (int(part) for part in provider["version"].split("."))
        normalized_library = normalized_path(cmake_library).replace("\\", "/")
        library_name = os.path.basename(normalized_library)
        expected_names = {
            os.path.basename(name.replace("\\", "/")).lower()
            for name in provider["library_names"]
        }
        version_markers = (
            f"python{major}.{minor}",
            f"python{major}{minor}",
            f"/versions/{major}.{minor}/",
        )
        if (library_name.lower() not in expected_names and
                not any(marker in normalized_library.lower() for marker in version_markers)):
            errors.append(
                f"FindPython library does not match CPython {major}.{minor}: {cmake_library}")

    return errors


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--expected-version", required=True)
    parser.add_argument("--expected-pointer-bits", required=True, type=int)
    parser.add_argument("--expected-architecture", required=True)
    parser.add_argument("--include-dir", action="append", default=[])
    parser.add_argument("--library", required=True)
    args = parser.parse_args(argv)

    provider = collect_provider()
    errors = validate_provider(
        provider,
        args.expected_version,
        args.expected_pointer_bits,
        args.expected_architecture,
        args.include_dir,
        args.library,
    )
    if errors:
        for error in errors:
            print(f"Python provider validation failed: {error}", file=sys.stderr)
        return 1

    print(
        "Validated embedded provider: "
        f"CPython {provider['version']} / {provider['architecture']} / "
        f"{provider['pointer_bits']}-bit / GIL-enabled release ABI")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
