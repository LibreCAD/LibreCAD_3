"""Original/final native byte coverage, independent of SDKs and native loaders.

This is evidence consistency, not build attestation or legal qualification.
Stagers supply original paths BEFORE modifying copies; never infer an original
from a rewritten file, its basename, or a matching license identifier.
"""

import hashlib
import os
import re
import stat
from pathlib import Path

try:
    from ._providernotices import (audit_source_provider_notices, canonical_json,
                                  notice_path, path_key, read_notice, unique_json)
except ImportError:  # Packaging imports the module without starting lcUIPy.
    from _providernotices import (audit_source_provider_notices, canonical_json,
                                 notice_path, path_key, read_notice, unique_json)

PLATFORMS = {"appimage-x86_64": ("ELF", "x86_64"), "snap-amd64": ("ELF", "x86_64"),
             "windows-x64": ("PE", "x86_64"), "macos-arm64": ("Mach-O", "arm64"),
             "macos-x86_64": ("Mach-O", "x86_64")}
MAX_BINARIES = 10000
MAX_BINARY_BYTES = 2 * 1024 * 1024 * 1024
IDENTITY_KEYS = ("name", "version", "revision", "architecture")


def native_header(data):
    """Identify loadable formats/architectures, not dependency or import closure."""
    def cpu(number, table):
        if number not in table:
            raise ValueError("unsupported native architecture: " + hex(number))
        return table[number]

    if data.startswith(b"\x7fELF"):
        if len(data) < 64 or data[4:7] != b"\x02\x01\x01" or \
                int.from_bytes(data[16:18], "little") not in (2, 3):
            raise ValueError("unsupported or truncated loadable ELF header")
        return "ELF", [cpu(int.from_bytes(data[18:20], "little"), {62: "x86_64", 183: "arm64"})]
    magic = data[:4]
    if magic in (b"\xcf\xfa\xed\xfe", b"\xfe\xed\xfa\xcf"):
        if len(data) < 32:
            raise ValueError("truncated Mach-O header")
        endian = "little" if magic[0] == 0xcf else "big"
        if int.from_bytes(data[12:16], endian) not in (2, 6, 8):
            raise ValueError("Mach-O is not an executable, dylib or bundle")
        return "Mach-O", [cpu(int.from_bytes(data[4:8], endian),
                              {0x1000007: "x86_64", 0x100000c: "arm64"})]
    if magic in (b"\xca\xfe\xba\xbe", b"\xca\xfe\xba\xbf"):
        if len(data) < 8:
            raise ValueError("truncated fat Mach-O header")
        count, size = int.from_bytes(data[4:8], "big"), 32 if magic[-1] == 0xbf else 20
        if not 0 < count <= 16 or len(data) < 8 + count * size:
            raise ValueError("invalid fat Mach-O architecture table")
        architectures = [cpu(int.from_bytes(data[8 + index * size:12 + index * size], "big"),
                             {0x1000007: "x86_64", 0x100000c: "arm64"}) for index in range(count)]
        if len(set(architectures)) != count:
            raise ValueError("duplicate fat Mach-O architecture")
        return "Mach-O", sorted(architectures)
    if magic in (b"\xce\xfa\xed\xfe", b"\xfe\xed\xfa\xce", b"\xbe\xba\xfe\xca", b"\xbf\xba\xfe\xca"):
        raise ValueError("unsupported Mach-O encoding/architecture")
    if data.startswith(b"MZ"):
        if len(data) < 64:
            raise ValueError("truncated PE header")
        offset = int.from_bytes(data[60:64], "little")
        if offset < 64 or offset + 26 > len(data) or data[offset:offset + 4] != b"PE\0\0" or \
                data[offset + 24:offset + 26] != b"\x0b\x02":
            raise ValueError("invalid or unsupported PE header")
        return "PE", [cpu(int.from_bytes(data[offset + 4:offset + 6], "little"), {0x8664: "x86_64"})]
    return None


def binary_fingerprint(path):
    """Stream stable original/final bytes; no subprocesses or whole-file reads."""
    path = Path(path)
    before = path.lstat()
    if not stat.S_ISREG(before.st_mode) or not 0 < before.st_size <= MAX_BINARY_BYTES:
        raise ValueError("native input is linked, empty, special or too large: " + str(path))
    with path.open("rb") as stream:
        header = stream.read(65536)
        native = native_header(header)
        if native is None:
            raise ValueError("native input has no supported header: " + str(path))
        digest = hashlib.sha256(header)
        size = len(header)
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
            size += len(chunk)
        opened = os.fstat(stream.fileno())
    after = path.lstat()
    key = lambda item: (item.st_dev, item.st_ino, item.st_size, item.st_mtime_ns, item.st_ctime_ns)
    if key(before) != key(opened) or key(before) != key(after) or size != before.st_size:
        raise ValueError("native input changed while hashing: " + str(path))
    return dict(sha256=digest.hexdigest(), size_bytes=size, binary_format=native[0], architectures=native[1])


def native_payload(root, include_aliases=False):
    """Inventory actual native files once, accepting only contained aliases.

    This filesystem inventory is deliberately not a second dependency walker.
    Internal framework/SONAME aliases resolve to their one real payload file.
    """
    root = Path(root)
    if root.is_symlink() or not root.is_dir() or root.resolve() == Path(root.anchor):
        raise ValueError("native payload requires a real, non-filesystem-root directory")
    root = root.resolve(strict=True)
    device = root.stat().st_dev
    result, names, aliases = {}, {}, {}
    def fail(error):
        raise error
    for directory, directories, filenames in os.walk(root, followlinks=False, onerror=fail):
        # Match the existing ELF audit's payload boundary: Snap content mounts
        # supply dependencies but are not files redistributed in our snap.
        directories[:] = [name for name in directories if (Path(directory) / name).lstat().st_dev == device]
        for name in directories + filenames:
            path = Path(directory) / name
            if path.lstat().st_dev != device:
                continue
            relative = path.relative_to(root).as_posix()
            notice_path(relative)
            key = path_key(relative)
            if key in names and names[key] != relative:
                raise ValueError("native payload has aliased paths: " + relative)
            names[key] = relative
            if path.is_symlink():
                target = path.resolve(strict=True)
                if not target.is_relative_to(root) or target == root:
                    raise ValueError("native payload contains an escaped alias: " + relative)
                aliases[relative] = target.relative_to(root).as_posix()
                if len(aliases) > MAX_BINARIES:
                    raise ValueError("native payload exceeds alias limit")
                continue
            if path.is_dir():
                continue
            if not path.is_file():
                raise ValueError("native payload contains a special file: " + relative)
            with path.open("rb") as stream:
                header = stream.read(65536)
            if native_header(header) is not None:
                result[relative] = path
                if len(result) > MAX_BINARIES:
                    raise ValueError("native payload exceeds inventory limit")
    if not result:
        raise ValueError("native payload is empty")
    return (result, aliases) if include_aliases else result


def provider_key(provider):
    if not isinstance(provider, dict) or set(provider) != set(IDENTITY_KEYS) or not all(
            isinstance(provider.get(key), str) and provider[key].strip() and
            len(provider[key]) <= 1024 and provider[key] != "None" and
            not any(ord(char) < 32 for char in provider[key])
            for key in IDENTITY_KEYS):
        raise ValueError("native provider identity is incomplete")
    return hashlib.sha256(canonical_json(provider)).hexdigest()


class OriginLedger:
    def __init__(self, root, platform):
        if platform not in PLATFORMS:
            raise ValueError("unknown packaged native platform: " + str(platform))
        root = Path(root)
        if root.is_symlink() or not root.is_dir() or root.resolve() == Path(root.anchor):
            raise ValueError("native origin ledger requires a real, non-filesystem-root directory")
        self.root, self.platform = root.resolve(strict=True), platform
        self.providers, self.files = {}, {}
        self.required = None

    def capture(self, destination, source, provider, source_root):
        """Call before rewriting a staged copy; source must be the selected original."""
        source, source_root = Path(source).resolve(strict=True), Path(source_root).resolve(strict=True)
        relative = Path(destination).resolve().relative_to(self.root).as_posix()
        notice_path(relative)
        source_path = source.relative_to(source_root).as_posix()
        notice_path(source_path)
        key, original = provider_key(provider), binary_fingerprint(source)
        expected_format, architecture = PLATFORMS[self.platform]
        if original["binary_format"] != expected_format or architecture not in original["architectures"] or \
                provider["architecture"] != architecture:
            raise ValueError("selected original/provider does not match native platform: " + relative)
        entry = dict(path=relative, provider=key, source_path=source_path, original=original)
        if relative in self.files and self.files[relative] != entry:
            raise ValueError("conflicting original native origin: " + relative)
        self.providers[key], self.files[relative] = dict(provider), entry
        return entry

    def capture_state(self):
        """Original-only staging checkpoint, explicitly not a final-byte seal."""
        return dict(format=1, stage="original-capture", platform=self.platform,
                    providers=self.providers, files=[self.files[key] for key in sorted(self.files)],
                    required=sorted(self.files if self.required is None else self.required))

    def seal(self):
        payload, aliases = native_payload(self.root, include_aliases=True)
        if set(payload) != set(self.files):
            raise ValueError("native original coverage differs; unknown={}, missing={}".format(
                sorted(set(payload) - set(self.files)), sorted(set(self.files) - set(payload))))
        files = []
        for relative, binary in sorted(payload.items()):
            entry, final = self.files[relative], binary_fingerprint(binary)
            if any(final[key] != entry["original"][key] for key in ("binary_format", "architectures")):
                raise ValueError("staged native format/architecture changed: " + relative)
            files.append(dict(entry, final=final))
        return dict(format=1, platform=self.platform, providers=self.providers, files=files, aliases=aliases)


def _read_record(path):
    raw = read_notice(path, 16 * 1024 * 1024)
    manifest = unique_json(raw)
    if not isinstance(manifest, dict) or manifest.get("format") != 1 or raw != canonical_json(manifest):
        raise ValueError("native origin manifest is unsupported or noncanonical")
    return manifest


def _ledger_from_record(root, manifest):
    ledger = OriginLedger(root, manifest.get("platform"))
    providers, files = manifest.get("providers"), manifest.get("files")
    if not isinstance(providers, dict) or not providers or len(providers) > 128 or \
            not isinstance(files, list) or not 0 < len(files) <= MAX_BINARIES:
        raise ValueError("native origin inventory is empty or exceeds limits")
    aliases, used = set(), set()
    for key, provider in providers.items():
        if key != provider_key(provider):
            raise ValueError("native provider content identity differs")
    for entry in files:
        if not isinstance(entry, dict):
            raise ValueError("native origin entry is malformed")
        relative = str(notice_path(entry.get("path")))
        if path_key(relative) in aliases or entry.get("provider") not in providers:
            raise ValueError("native origin is duplicated or provider is unknown")
        notice_path(entry.get("source_path"))
        original = entry.get("original")
        if not isinstance(original, dict) or set(original) != {"sha256", "size_bytes", "binary_format", "architectures"} or \
                not re.fullmatch(r"[0-9a-f]{64}", str(original.get("sha256", ""))) or \
                type(original.get("size_bytes")) is not int or not 0 < original["size_bytes"] <= MAX_BINARY_BYTES:
            raise ValueError("original native byte identity is incomplete")
        expected_format, architecture = PLATFORMS[ledger.platform]
        if original.get("binary_format") != expected_format or original.get("architectures") not in \
                ([architecture], ["arm64", "x86_64"] if expected_format == "Mach-O" else [] ) or \
                providers[entry["provider"]]["architecture"] != architecture:
            raise ValueError("original native platform/architecture differs")
        aliases.add(path_key(relative))
        used.add(entry["provider"])
        ledger.files[relative] = {key: entry[key] for key in ("path", "provider", "source_path", "original")}
    ledger.providers = providers
    if used != set(providers):
        raise ValueError("final native bytes/inventory differ from original ledger")
    return ledger


def read_origin_capture(root, path):
    state = _read_record(path)
    ledger = _ledger_from_record(root, state)
    required = state.get("required")
    if not isinstance(required, list) or any(not isinstance(path, str) for path in required) or \
            required != sorted(set(required)) or not set(required).issubset(ledger.files):
        raise ValueError("native original capture has invalid required paths")
    ledger.required = set(required)
    if ledger.capture_state() != state:
        raise ValueError("native original capture is malformed or is a final-byte report")
    return ledger


def audit_native_origins(root, manifest_file, notice_directory=None):
    """Verify complete final-byte coverage, optionally exact external notice joins.

    Without notice_directory this checks origins/bytes ONLY. Supplying a notice
    directory requires EVERY provider to match its exact four-field identity;
    there is no project/system/unknown-provider license exemption here.
    """
    manifest = _read_record(manifest_file)
    ledger = _ledger_from_record(root, manifest)
    providers, files = manifest["providers"], manifest["files"]
    if ledger.seal() != manifest:
        raise ValueError("final native bytes/inventory differ from original ledger")
    if notice_directory is not None:
        notices = Path(notice_directory)
        if notices.is_symlink() or not notices.is_dir():
            raise ValueError("native provider notice directory is missing or linked")
        if not notices.resolve(strict=True).is_relative_to(ledger.root):
            raise ValueError("native provider notices escape the packaged payload")
        identities = set()
        collections = list(notices.iterdir())
        if not 0 < len(collections) <= 128:
            raise ValueError("native provider notice collections are empty or exceed limits")
        for collection in collections:
            audit_source_provider_notices(collection)
            identity = unique_json(read_notice(collection / "manifest.json"))["provider"]
            identity = tuple(identity[key] for key in IDENTITY_KEYS)
            if identity in identities:
                raise ValueError("native provider notice identity is duplicated")
            identities.add(identity)
        missing = [provider for provider in providers.values()
                   if tuple(provider[key] for key in IDENTITY_KEYS) not in identities]
        if missing:
            raise ValueError("native providers lack exact original notices: " + str(missing))
    return len(files), len(providers)


def main(argv=None):
    import argparse
    import sys
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("payload", type=Path)
    parser.add_argument("manifest", type=Path)
    parser.add_argument("--notices", type=Path)
    arguments = parser.parse_args(argv)
    try:
        count = audit_native_origins(arguments.payload, arguments.manifest, arguments.notices)
    except (OSError, ValueError) as error:
        print("Native origin audit failed: " + str(error), file=sys.stderr)
        return 1
    print("SDK-free native origin audit: {} files, {} providers".format(*count))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
