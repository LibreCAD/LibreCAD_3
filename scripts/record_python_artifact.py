#!/usr/bin/env python3
"""Record/verify exact package, selected-runtime inventory and evidence bytes.

This binds files retained by a build; it does not itself prove artifact/runtime
behavior, dependency closure, licensing, signing, or the provenance of a binary.
Call only after the workflow's required artifact qualification checks.
"""

import argparse
import hashlib
import json
import re
import sys
from pathlib import Path

from validate_python_inventory import _REQUIRED_FIELDS, parse_inventory


PLATFORMS = {"appimage-x86_64": "x86_64", "snap-amd64": "x86_64",
             "windows-x64": "x86_64", "macos-arm64": "arm64", "macos-x86_64": "x86_64"}
ARCHITECTURES = {"amd64": "x86_64", "x64": "x86_64", "x86_64": "x86_64",
                 "aarch64": "arm64", "arm64": "arm64"}


def fingerprint(path):
    path = Path(path)
    if path.is_symlink() or not path.is_file():
        raise ValueError("identity input is missing, linked or not a regular file: " + str(path))
    before = path.stat()
    if not before.st_size:
        raise ValueError("identity input is empty: " + str(path))
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    after = path.stat()
    if path.is_symlink() or (before.st_dev, before.st_ino, before.st_size, before.st_mtime_ns) != \
            (after.st_dev, after.st_ino, after.st_size, after.st_mtime_ns):
        raise ValueError("identity input changed while hashing: " + str(path))
    return dict(name=path.name, size_bytes=before.st_size, sha256=digest.hexdigest())


def identity(artifact, inventory, evidence, platform, revision):
    if not re.fullmatch(r"[0-9a-f]{40}", revision):
        raise ValueError("identity requires the full tested Git revision")
    if platform not in PLATFORMS or not evidence:
        raise ValueError("identity requires a supported platform and qualification evidence")
    files = [Path(artifact), Path(inventory), *sorted(map(Path, evidence), key=lambda path: path.name)]
    names, inodes, records = set(), set(), []
    for path in files:
        records.append(fingerprint(path))
        stat = path.stat()
        inode = stat.st_dev, stat.st_ino
        if path.name.casefold() in names or inode in inodes:
            raise ValueError("identity input names/files are ambiguous or duplicated")
        names.add(path.name.casefold())
        inodes.add(inode)
    fields, errors = parse_inventory(Path(inventory).read_text(encoding="utf-8"))
    if errors or any(not fields.get(key) for key in (*_REQUIRED_FIELDS, "version")):
        raise ValueError("selected Python inventory is incomplete or malformed: " + "; ".join(errors))
    if fields["provider_implementation"] != "CPython" or fields["provider_pointer_bits"] != "64" or \
            fields["provider_debug"].lower() != "false" or fields["provider_gil_disabled"].lower() != "false":
        raise ValueError("selected Python inventory has an unsupported ABI")
    if ARCHITECTURES.get(fields["provider_architecture"].lower()) != PLATFORMS[platform]:
        raise ValueError("selected Python inventory does not match the artifact architecture")
    version = fields["provider_version"]
    if not re.fullmatch(r"3\.\d+\.\d+", version) or int(version.split(".")[1]) < 9 or \
            version != fields["version"]:
        raise ValueError("selected Python inventory versions differ or are unsupported")
    return dict(format=1, revision=revision, platform=platform, artifact=records[0],
                inventory=records[1], selected_python=fields, evidence=records[2:])


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--artifact", type=Path, required=True)
    parser.add_argument("--inventory", type=Path, required=True)
    parser.add_argument("--evidence", type=Path, action="append", required=True)
    parser.add_argument("--platform", choices=PLATFORMS, required=True)
    parser.add_argument("--revision", required=True)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--output", type=Path)
    mode.add_argument("--verify", type=Path)
    args = parser.parse_args(argv)
    try:
        record = identity(args.artifact, args.inventory, args.evidence, args.platform, args.revision)
        if args.verify:
            fingerprint(args.verify)
            if json.loads(args.verify.read_text(encoding="utf-8")) != record:
                raise ValueError("artifact/runtime/evidence identity differs from the receipt")
        else:
            if args.output.is_symlink() or any(args.output.resolve() == path.resolve() or
                    (args.output.exists() and args.output.samefile(path))
                    for path in (args.artifact, args.inventory, *args.evidence)):
                raise ValueError("identity output cannot overwrite or alias an input")
            args.output.write_text(json.dumps(record, indent=2, sort_keys=True, ensure_ascii=False) + "\n",
                                   encoding="utf-8")
    except (OSError, ValueError) as error:
        print("ERROR: " + str(error), file=sys.stderr)
        return 1
    print("Artifact identity {} ({} evidence files)".format(
        "verified" if args.verify else "recorded", len(args.evidence)))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
