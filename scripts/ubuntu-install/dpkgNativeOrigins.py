#!/usr/bin/env python3
"""Selected installed-dpkg originals/notices, not another dependency resolver.

Call capture with existing deployment's explicit original/destination paths
BEFORE rewriting copies. Installed control md5sums prove input consistency,
not cryptographic authenticity, build attestation or redistribution sufficiency.
Do not point this at the host database to identify a different Snap/package root.
"""

import hashlib
import os
import re
import stat
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "lcUIPy"))
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from _nativeorigins import binary_fingerprint, canonical_json
from _providernotices import MAX_FILES, MAX_TOTAL_BYTES, notice_path, read_notice, source_identity
from collect_provider_notices import publish_collection

PACKAGE = r"[a-z0-9][a-z0-9+.-]*(?::(?:amd64|arm64))?"
ARCHITECTURES = {"amd64": "x86_64", "arm64": "arm64"}
SHOW = "${binary:Package}\t${Version}\t${Architecture}\t${Status}\t${source:Package}\t${source:Version}\n"


class DpkgOrigins:
    def __init__(self, root=Path("/"), query="dpkg-query"):
        root = Path(root)
        if not root.is_absolute() or root.is_symlink() or not root.is_dir():
            raise ValueError("dpkg origins require the actual absolute installed-package root")
        self.root, self.query, self.records = root.resolve(strict=True), query, {}

    def run(self, *arguments, missing=False):
        result = subprocess.run([self.query, "--root=" + str(self.root),
            "--admindir=" + str(self.root / "var/lib/dpkg"), "--no-pager", *arguments],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=15,
            env=dict(os.environ, LC_ALL="C.UTF-8", LANGUAGE="C"), check=False)
        if len(result.stdout) > 8 * 1024 * 1024 or len(result.stderr) > 1024 * 1024 or \
                result.returncode != 0 and not (missing and result.returncode == 1 and not result.stdout):
            raise ValueError("dpkg query failed or exceeded limits: " + str(arguments[:1]))
        return result.stdout

    def package(self, name):
        if not re.fullmatch(PACKAGE, name):
            raise ValueError("ambiguous, diverted or invalid dpkg owner: " + name)
        show = self.run("--show", "--showformat=" + SHOW, "--", name).decode("utf-8")
        values = show.removesuffix("\n").split("\t")
        if len(values) != 6 or values[0] != name or values[3] not in ("install ok installed", "hold ok installed") or \
                values[2] not in ("amd64", "arm64", "all"):
            raise ValueError("dpkg selected package is not exactly installed: " + name)
        record = dict(zip(("package", "version", "architecture", "status", "source_package", "source_version"), values))
        if not re.fullmatch(r"[a-z0-9][a-z0-9+.-]*", record["source_package"]) or any(
                not re.fullmatch(r"[A-Za-z0-9.+:~_-]{1,256}", record[key]) for key in ("version", "source_version")):
            raise ValueError("dpkg binary/source version is incomplete: " + name)
        sums = self.run("--control-show", name, "md5sums").decode("utf-8")
        checksums = {}
        for line in sums.splitlines():
            match = re.fullmatch(r"([0-9a-f]{32})  (.+)", line)
            if match is None or match[2] in checksums:
                raise ValueError("dpkg control checksums are malformed or duplicated: " + name)
            notice_path(match[2])
            checksums[match[2]] = match[1]
        if not checksums:
            raise ValueError("dpkg selected package has no original control checksums: " + name)
        record.update(show=show, md5sums=sums)
        if name in self.records and self.records[name] != record:
            raise ValueError("dpkg selected package metadata changed during capture: " + name)
        self.records[name] = record
        return record, checksums

    def paths(self, source):
        source = Path(source)
        actual = source.resolve(strict=True)
        relative = actual.relative_to(self.root).as_posix()
        notice_path(relative)
        candidates = {relative}
        if source.is_absolute() and source.is_relative_to(self.root):
            candidates.add(source.relative_to(self.root).as_posix())
        parts = relative.split("/")
        if len(parts) > 2 and parts[0] == "usr" and parts[1] in ("lib", "lib64", "bin", "sbin"):
            alias = self.root / "/".join(parts[1:])
            if alias.exists() and alias.samefile(actual):
                candidates.add(alias.relative_to(self.root).as_posix())
        for candidate in candidates:
            notice_path(candidate)
            if "[" in candidate or "]" in candidate:
                raise ValueError("dpkg original path cannot be a query glob")
        return actual, sorted(candidates)

    def verify_file(self, source):
        actual, candidates = self.paths(source)
        owners = set()
        for candidate in candidates:
            path = "/" + candidate
            for line in self.run("--search", path, missing=True).decode("utf-8").splitlines():
                owner, separator, claimed = line.partition(": ")
                if separator != ": " or claimed != path or not re.fullmatch(PACKAGE, owner):
                    raise ValueError("ambiguous, diverted or mismatched dpkg file owner: " + path)
                owners.add(owner)
        if len(owners) != 1:
            raise ValueError("unknown or ambiguous original dpkg file: " + str(actual))
        record, checksums = self.package(owners.pop())
        expected = {checksums[path] for path in candidates if path in checksums}
        if len(expected) != 1:
            raise ValueError("dpkg original file has missing or ambiguous control checksum: " + str(actual))
        before = actual.stat()
        if not stat.S_ISREG(before.st_mode) or not 0 < before.st_size <= 2 * 1024 * 1024 * 1024:
            raise ValueError("dpkg original is special, empty or exceeds size limit")
        digest = hashlib.md5(usedforsecurity=False)
        with actual.open("rb") as stream:
            for chunk in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(chunk)
        after = actual.stat()
        key = lambda item: (item.st_dev, item.st_ino, item.st_size, item.st_mtime_ns, item.st_ctime_ns)
        if actual.is_symlink() or key(before) != key(after) or digest.hexdigest() not in expected:
            raise ValueError("selected dpkg original bytes differ from control checksum: " + str(actual))
        return actual, record, digest.hexdigest()

    def identify(self, source, architecture):
        actual, record, _ = self.verify_file(source)
        native = binary_fingerprint(actual)
        if native["binary_format"] != "ELF" or native["architectures"] != [architecture] or \
                ARCHITECTURES.get(record["architecture"]) != architecture:
            raise ValueError("dpkg original/provider architecture differs")
        return dict(name=record["package"], version=record["version"], architecture=architecture,
                    revision="dpkg:{}:{}".format(record["source_package"], record["source_version"]))

    def capture(self, ledger, destination, source):
        architecture = {"appimage-x86_64": "x86_64", "snap-amd64": "x86_64"}.get(ledger.platform)
        if architecture is None:
            raise ValueError("dpkg capture requires an existing supported Linux artifact identity")
        original = binary_fingerprint(Path(source).resolve(strict=True))
        provider = self.identify(source, architecture)
        ledger.capture(destination, source, provider, self.root)
        relative = Path(destination).resolve().relative_to(ledger.root).as_posix()
        if ledger.files[relative]["original"] != original:
            raise ValueError("dpkg original changed while capturing SHA-256")
        return provider

    def collect_notices(self, provider, output):
        selected, _ = self.package(provider["name"])
        expected = dict(name=selected["package"], version=selected["version"],
                        revision="dpkg:{}:{}".format(selected["source_package"], selected["source_version"]),
                        architecture=ARCHITECTURES.get(selected["architecture"]))
        if provider != expected:
            raise ValueError("selected dpkg notice provider differs")
        copyright = self.root / "usr/share/doc" / provider["name"].partition(":")[0] / "copyright"
        common = self.root / "usr/share/common-licenses"
        if common.is_symlink() or not common.is_dir():
            raise ValueError("selected dpkg common-license directory is absent or linked")
        paths = [copyright]
        for path in common.iterdir():
            if len(paths) >= MAX_FILES:
                raise ValueError("dpkg common-license inventory exceeds limits")
            paths.append(path)
        payload, inputs, records = {}, {}, {selected["package"]: selected}
        for path in paths:
            actual, owner, checksum = self.verify_file(path)
            if path == copyright and (owner["source_package"], owner["source_version"]) != \
                    (selected["source_package"], selected["source_version"]):
                raise ValueError("shared dpkg copyright comes from a different source version")
            relative = "notices/" + actual.relative_to(self.root).as_posix()
            payload[relative] = read_notice(actual)
            if hashlib.md5(payload[relative], usedforsecurity=False).hexdigest() != checksum:
                raise ValueError("dpkg notice changed during collection")
            inputs[relative], records[owner["package"]] = checksum, owner
            if sum(map(len, payload.values())) > MAX_TOTAL_BYTES:
                raise ValueError("dpkg notices exceed total size limit")
        if len(payload) < 2:
            raise ValueError("dpkg copyright/common-license set is incomplete")
        metadata = canonical_json(dict(selected=selected, packages=records, inputs=inputs))
        source = dict(kind="dpkg-installed", package=selected["package"], version=selected["version"],
                      architecture=provider["architecture"], source_package=selected["source_package"],
                      source_version=selected["source_version"], sha256=hashlib.sha256(metadata).hexdigest(), size_bytes=len(metadata))
        source_identity(source)
        files = [dict(path=name, sha256=hashlib.sha256(data).hexdigest(), size_bytes=len(data)) for name, data in sorted(payload.items())]
        payload["provenance/dpkg-query.json"] = metadata
        manifest = dict(format=1, provider=provider, source=source, files=files, patches=[], metadata=[
            dict(path="provenance/dpkg-query.json", name="dpkg-query.json", sha256=source["sha256"], size_bytes=len(metadata))])
        return publish_collection(output, manifest, payload)
