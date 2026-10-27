#!/usr/bin/env python3
"""Validate that a CMake Python runtime inventory belongs to one provider."""

from __future__ import annotations

import argparse
import ntpath
import os
import re
import sys
from pathlib import Path, PureWindowsPath


_REQUIRED_FIELDS = (
    "provider_implementation",
    "provider_version",
    "provider_architecture",
    "provider_pointer_bits",
    "provider_debug",
    "provider_gil_disabled",
    "executable",
    "include_dirs",
    "link_library",
    "prefix",
    "stdlib",
    "stdarch",
    "dynamic_modules",
)
_PATH_FIELDS = (
    "executable",
    "include_dirs",
    "link_library",
    "prefix",
    "stdlib",
    "stdarch",
    "dynamic_modules",
)


def parse_inventory(text: str) -> tuple[dict[str, str], list[str]]:
    fields: dict[str, str] = {}
    errors = []
    for line_number, line in enumerate(text.splitlines(), start=1):
        if not line.strip():
            continue
        key, separator, value = line.partition("=")
        if not separator or not key.strip():
            errors.append("line {} is not a key=value entry".format(line_number))
            continue
        key = key.strip()
        if key in fields:
            errors.append("duplicate inventory field: {}".format(key))
            continue
        fields[key] = value.strip()
    return fields, errors


def _path_module(expected_root: str):
    return ntpath if PureWindowsPath(expected_root).drive else os.path


def _is_within(path: str, root: str, path_module) -> bool:
    try:
        if not path_module.isabs(path) or not path_module.isabs(root):
            return False
        normalized_path = path_module.normcase(path_module.abspath(path))
        normalized_root = path_module.normcase(path_module.abspath(root))
        return path_module.commonpath((normalized_path, normalized_root)) == normalized_root
    except ValueError:
        return False


def validate_inventory(
    text: str,
    expected_root: str,
    expected_version: str = "3.14",
    expected_architecture: str = "x86_64",
    expected_pointer_bits: int = 64,
) -> list[str]:
    fields, errors = parse_inventory(text)
    missing = [field for field in _REQUIRED_FIELDS if field not in fields]
    if missing:
        errors.append("inventory is missing required fields: {}".format(", ".join(missing)))
        return errors

    if fields["provider_implementation"] != "CPython":
        errors.append("provider_implementation must be CPython")
    if not re.fullmatch(
        re.escape(expected_version) + r"\.\d+", fields["provider_version"]
    ):
        errors.append(
            "provider_version must be {}.x, got {}".format(
                expected_version, fields["provider_version"]
            )
        )
    architecture_aliases = {
        "amd64": "x86_64",
        "x64": "x86_64",
        "x86_64": "x86_64",
        "arm64": "arm64",
        "aarch64": "arm64",
    }
    actual_architecture = architecture_aliases.get(
        fields["provider_architecture"].strip().lower(),
        fields["provider_architecture"].strip().lower(),
    )
    if actual_architecture != expected_architecture.lower():
        errors.append(
            "provider_architecture must be {}, got {}".format(
                expected_architecture, fields["provider_architecture"]
            )
        )
    if fields["provider_pointer_bits"] != str(expected_pointer_bits):
        errors.append(
            "provider_pointer_bits must be {}, got {}".format(
                expected_pointer_bits, fields["provider_pointer_bits"]
            )
        )
    if fields["provider_debug"].lower() != "false":
        errors.append("debug CPython providers are not supported")
    if fields["provider_gil_disabled"].lower() != "false":
        errors.append("free-threaded CPython providers are not supported")

    path_module = _path_module(expected_root)
    if not expected_root.strip():
        errors.append("expected provider root is empty")
    else:
        for field in _PATH_FIELDS:
            raw_paths = fields[field].split(";") if field == "include_dirs" else [fields[field]]
            if not raw_paths or any(not path.strip() for path in raw_paths):
                errors.append("{} contains an empty path".format(field))
                continue
            for raw_path in raw_paths:
                path = raw_path.strip()
                if not _is_within(path, expected_root, path_module):
                    errors.append(
                        "{} path is outside the selected provider root: {}".format(
                            field, path
                        )
                    )
    return errors


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--inventory", required=True, help="CMake provider inventory")
    parser.add_argument("--expected-root", required=True, help="selected Python root")
    parser.add_argument("--expected-version", default="3.14")
    parser.add_argument("--expected-architecture", default="x86_64")
    parser.add_argument("--expected-pointer-bits", type=int, default=64)
    args = parser.parse_args(argv)

    try:
        text = Path(args.inventory).read_text(encoding="utf-8")
    except OSError as error:
        print("ERROR: unable to read Python inventory: {}".format(error), file=sys.stderr)
        return 2

    errors = validate_inventory(
        text,
        args.expected_root,
        args.expected_version,
        args.expected_architecture,
        args.expected_pointer_bits,
    )
    if errors:
        for error in errors:
            print("ERROR: {}".format(error), file=sys.stderr)
        return 1

    fields, _ = parse_inventory(text)
    print(
        "Validated {} CPython {} / {}-bit / {} under {}".format(
            fields["provider_architecture"],
            fields["provider_version"],
            fields["provider_pointer_bits"],
            "GIL-enabled release ABI",
            args.expected_root,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
