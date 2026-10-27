#!/usr/bin/env python3
"""Carry original identities through Mac deployment; seal outside signed code.

This is an inventory/copy-path adapter, not another dependency walker. The
existing stager resolves dependencies and captures any further external copies.
"""

import argparse
import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "lcUIPy"))
from _nativeorigins import (MAX_BINARIES, OriginLedger, audit_native_origins, canonical_json, native_header,
                            read_notice, read_origin_capture, unique_json)
from stagePythonDependencies import SYSTEM_PREFIXES, RuntimeStager, homebrew_origin


def write_external(root, output, record):
    output = Path(output)
    if output.resolve().is_relative_to(Path(root).resolve()) or output.is_symlink() or output.exists():
        raise ValueError("native origin output must be fresh and outside signed resources")
    with output.open("xb") as stream:
        stream.write(canonical_json(record))


def bundle_path(relative):
    if relative == "bin/librecad":
        return "MacOS/librecad"
    for source, destination in (("lib/", "Frameworks/"), ("share/librecad/", "Resources/librecad/")):
        if relative.startswith(source):
            return destination + relative[len(source):]
    raise ValueError("unmapped install-native path: " + relative)


def prepare(prefix, contents, qt_libraries, qt_plugins, output, otool="otool"):
    prefix, contents = Path(prefix), Path(contents)
    source_manifest = prefix / "share/librecad/licenses/providers/native-origins.json"
    audit_native_origins(prefix, source_manifest)
    source = unique_json(read_notice(source_manifest, 16 * 1024 * 1024))
    ledger = OriginLedger(contents, source["platform"])
    if not ledger.platform.startswith("macos-"):
        raise ValueError("bundle capture requires a Mac prefix")
    ledger.providers = dict(source["providers"])
    for entry in source["files"]:
        entry = {key: entry[key] for key in ("path", "provider", "source_path", "original")}
        entry["path"] = bundle_path(entry["path"])
        if entry["path"] in ledger.files:
            raise ValueError("conflicting carried original native path")
        ledger.files[entry["path"]] = entry
    # Unlike optional SDK candidates, every carried prefix file is required.
    # Missing extension/native files must not disappear during candidate pruning.
    ledger.required = set(ledger.files)

    architecture = ledger.platform.removeprefix("macos-")
    qt_libraries, qt_plugins = Path(qt_libraries), Path(qt_plugins)
    if not qt_libraries.is_dir() or not qt_plugins.is_dir():
        raise ValueError("selected Qt library/plugin roots are missing")

    originals = set()
    def capture(destination, source):
        source = source.resolve(strict=True)
        with source.open("rb") as stream:
            if native_header(stream.read(65536)) is None:
                return
        provider, keg = homebrew_origin(source, architecture)
        ledger.capture(contents / destination, source, provider, keg)
        originals.add(source)
        if len(ledger.files) > MAX_BINARIES or len(ledger.providers) > 128:
            raise ValueError("native deployment candidates exceed inventory limits")

    # Query roots are those of the actual macdeployqt installation. Record
    # physical framework files, not duplicate Versions/Current aliases. Optional
    # candidates are pruned against actual deployed files by the same stager.
    for framework in sorted(qt_libraries.glob("Qt*.framework")):
        root = framework.resolve(strict=True)
        for directory, _, files in os.walk(root, followlinks=False):
            for name in files:
                path = Path(directory) / name
                if path.is_file() and not path.is_symlink():
                    capture(Path("Frameworks") / framework.name / path.relative_to(root), path)
    # Qt plugins use category/file layout; category and file links are SDK
    # selections, resolved to the exact provider keg before fingerprinting.
    for category in sorted(qt_plugins.iterdir()):
        if category.is_dir():
            for path in sorted(category.iterdir()):
                if path.is_file():
                    capture(Path("PlugIns") / category.name / path.name, path)
    if len(ledger.files) == len(source["files"]):
        raise ValueError("selected Qt SDK has no native deployment candidates")
    # macdeployqt also copies the dependencies of these selected SDK objects.
    # Use the SAME load/RPATH resolver, read-only; record each load's explicit
    # copy destination before it is rewritten, not a post-hoc basename guess.
    resolver = RuntimeStager(contents, contents, contents / "Frameworks", otool,
                             "install_name_tool", "codesign", None, False, [], False,
                             search_directories=[qt_libraries])
    queue, seen = sorted(originals), set()
    while queue:
        binary = queue.pop(0)
        if binary in seen:
            continue
        seen.add(binary)
        for load in resolver._loads(binary):
            if load.startswith(SYSTEM_PREFIXES):
                continue
            dependency = resolver._resolve_load(binary, load)
            if dependency is None:
                raise ValueError("unresolved original SDK dependency: {} from {}".format(load, binary))
            original = dependency.resolve(strict=True)
            framework = next((parent for parent in original.parents if parent.name.endswith(".framework")), None)
            destination = Path("Frameworks") / dependency.name
            if framework is not None:
                destination = Path("Frameworks") / framework.name / original.relative_to(framework)
            capture(destination, original)
            if original not in seen:
                queue.append(original)
    write_external(contents, output, ledger.capture_state())
    print("Captured pre-deployment originals: {} candidates, {} providers".format(
        len(ledger.files), len(ledger.providers)))


def finalize(contents, capture, output):
    ledger = read_origin_capture(contents, capture)
    manifest = ledger.seal()
    write_external(contents, output, manifest)
    print("Sealed final signed-app native origins: {} files, {} providers, {} aliases".format(
        len(manifest["files"]), len(manifest["providers"]), len(manifest["aliases"])))
    print("Verified SDK-free signed-app native origins:", audit_native_origins(contents, output))


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    before = commands.add_parser("prepare")
    for name in ("prefix", "contents", "qt_libraries", "qt_plugins", "output"):
        before.add_argument(name, type=Path)
    before.add_argument("--otool", default="otool")
    after = commands.add_parser("finalize")
    for name in ("contents", "capture", "output"):
        after.add_argument(name, type=Path)
    arguments = vars(parser.parse_args(argv))
    command = arguments.pop("command")
    try:
        (prepare if command == "prepare" else finalize)(**arguments)
    except (OSError, RuntimeError, ValueError) as error:
        print("Mac native-origin capture failed: " + str(error), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
