#!/usr/bin/env python3
"""Collect original notices from a checksummed selected-provider source archive.

No download, dependency build, arbitrary extraction or license-text synthesis.
Archive/provider pins must come from the selected binary provider. The resulting
collection is a primitive for packaging, not proof that every binary is covered.
"""

import argparse
import bz2
import gzip
import hashlib
import lzma
import os
import posixpath
import re
import stat
import sys
import tarfile
import tempfile
import zipfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "lcUIPy"))
from _providernotices import (MAX_FILE_BYTES, MAX_FILES, MAX_TOTAL_BYTES,
                             audit_source_provider_notices, canonical_json,
                             notice_path, path_key, read_notice, source_identity, unique_json)

MAX_ARCHIVE_BYTES = 512 * 1024 * 1024
MAX_EXPANDED_BYTES = 2 * 1024 * 1024 * 1024
MAX_MEMBERS = 100000
NOTICE_NAME = re.compile(
    r"^(?:licen[cs]es?|copying|copyright|notices?|attributions?)(?:[._-].*)?$|"
    r"[._-](?:licen[cs]es?|copyright|notices?)(?:[._-].*)?$|"
    r"^third[-_]?party[-_]?notices?(?:[._-].*)?$", re.I)


class BoundedReader:
    def __init__(self, stream):
        self.stream, self.total = stream, 0

    def read(self, size):
        if size < 0 or size > MAX_FILE_BYTES:
            raise ValueError("archive read exceeds member limit")
        data = self.stream.read(size)
        self.total += len(data)
        if self.total > MAX_EXPANDED_BYTES:
            raise ValueError("archive decompression exceeds expanded limit")
        return data


def archive_members(stream):
    """Yield names, sizes and bounded readers; never extract archive paths."""
    stream.seek(0)
    magic = stream.read(6)
    stream.seek(0)
    if magic.startswith(b"PK"):
        with zipfile.ZipFile(stream) as archive:
            if len(archive.infolist()) > MAX_MEMBERS:
                raise ValueError("archive exceeds member count limit")
            for member in archive.infolist():
                mode = stat.S_IFMT(member.external_attr >> 16)
                if mode not in (0, stat.S_IFREG, stat.S_IFDIR) or member.flag_bits & 1:
                    raise ValueError("archive contains a linked, special or encrypted member")
                with archive.open(member) as content:
                    yield member.filename.rstrip("/"), member.file_size, member.is_dir(), content
        return
    wrapper = gzip.GzipFile if magic.startswith(b"\x1f\x8b") else \
        bz2.BZ2File if magic.startswith(b"BZh") else \
        lzma.LZMAFile if magic.startswith(b"\xfd7zXZ\x00") else None
    decoded = gzip.GzipFile(fileobj=stream) if wrapper is gzip.GzipFile else \
        wrapper(stream) if wrapper else stream
    try:
        with tarfile.open(fileobj=BoundedReader(decoded), mode="r|") as archive:
            for member in archive:
                if not (member.isfile() or member.isdir()) or member.issparse():
                    raise ValueError("archive contains a linked, sparse or special member")
                yield (member.name.rstrip("/"), member.size, member.isdir(),
                       archive.extractfile(member) if member.isfile() else None)
    finally:
        if wrapper:
            decoded.close()


def original_notices(stream, extra_notices=()):
    selected, members, aliases, kinds, attributes = {}, {}, {}, {}, {}
    expanded = 0

    def retain(name, size, content):
        if size <= 0 or size > MAX_FILE_BYTES or len(selected) >= MAX_FILES:
            raise ValueError("notice is empty or exceeds collection limits: " + name)
        data = content.read(size + 1)
        if len(data) != size or not data.strip() or \
                sum(map(len, selected.values())) + len(data) > MAX_TOTAL_BYTES:
            raise ValueError("notice bytes are empty, truncated or exceed limits: " + name)
        selected[name] = data

    for name, size, directory, content in archive_members(stream):
        notice_path(name)
        expanded += size
        if name in members or len(members) >= MAX_MEMBERS or size < 0 or \
                size > 64 * 1024 * 1024 or expanded > MAX_EXPANDED_BYTES:
            raise ValueError("archive has duplicate members or exceeds size/count limits")
        members[name] = None if directory else size
        parts = name.split("/")
        for index in range(1, len(parts) + 1):
            prefix = "/".join(parts[:index])
            key = path_key(prefix)
            is_directory = index < len(parts) or directory
            if key in aliases and (aliases[key] != prefix or kinds[key] != is_directory):
                raise ValueError("archive contains aliased or conflicting paths: " + name)
            aliases[key], kinds[key] = prefix, is_directory
        if directory:
            continue
        attribution = parts[-1] == "qt_attribution.json"
        if attribution or NOTICE_NAME.search(parts[-1]) or \
                any(part.lower() in ("licenses", "licences") for part in parts[:-1]):
            retain(name, size, content)
        if attribution:
            # Some original Qt records contain literal newlines in descriptive
            # strings. Parse these without normalizing the retained raw bytes.
            # Some Qt records also repeat descriptive Comment/Version fields.
            # Neither selects notice files nor supplies our provider identity.
            # Never allow duplicate decision fields (LicenseFile etc.).
            entries = unique_json(selected[name], repeated=("Comment", "Version"), strict=False)
            entries = entries if isinstance(entries, list) else [entries]
            if not entries or not all(isinstance(entry, dict) for entry in entries):
                raise ValueError("Qt attribution metadata is malformed: " + name)
            attributes[name] = entries

    requested = set(str(notice_path(name)) for name in extra_notices)
    for name, entries in attributes.items():
        for entry in entries:
            for field in ("LicenseFile", "LicenseFiles", "CopyrightFile"):
                if field not in entry:
                    continue
                references = entry[field] if field == "LicenseFiles" else [entry[field]]
                if not isinstance(references, list) or not references:
                    raise ValueError("Qt attribution references are malformed: " + name)
                for reference in references:
                    if not isinstance(reference, str) or not reference or reference.startswith("/") or \
                            any(ord(char) < 32 or char in "\\:" for char in reference):
                        raise ValueError("unsafe Qt attribution reference: " + name)
                    relative = posixpath.normpath(posixpath.join(posixpath.dirname(name), reference))
                    notice_path(relative)
                    if "/" in name and relative.split("/")[0] != name.split("/")[0]:
                        raise ValueError("Qt attribution reference escapes its source tree: " + name)
                    requested.add(relative)
    if any(members.get(name) is None for name in requested):
        raise ValueError("referenced provider notice is missing or not a regular file")
    missing = requested - selected.keys()
    if missing:
        for name, size, directory, content in archive_members(stream):
            if name in missing:
                retain(name, size, content)
                missing.remove(name)
        if missing:
            raise ValueError("referenced provider notice disappeared")
    if not selected or not any(not name.endswith("/qt_attribution.json") for name in selected):
        raise ValueError("source archive provides no original notice files")
    return selected


def collect(archive, url, sha256, provider, output, patches=(), metadata=(), extra_notices=()):
    archive, output = Path(archive), Path(output)
    if not all(isinstance(provider.get(key), str) and provider[key].strip() and provider[key] != "None"
               for key in ("name", "version", "revision", "architecture")):
        raise ValueError("selected provider identity is incomplete")
    if archive.is_symlink() or not archive.is_file() or \
            not 0 < archive.stat().st_size <= MAX_ARCHIVE_BYTES:
        raise ValueError("source archive is missing, linked, empty or too large")
    source = dict(url=url, sha256=sha256, size_bytes=archive.stat().st_size)
    source_identity(source)
    before = archive.stat()
    with archive.open("rb") as stream:
        opened = os.fstat(stream.fileno())
        if (before.st_dev, before.st_ino) != (opened.st_dev, opened.st_ino):
            raise ValueError("source archive changed before verification")
        digest = hashlib.sha256()
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
        if digest.hexdigest() != sha256:
            raise ValueError("source archive checksum differs from the selected provider pin")
        notices = original_notices(stream, extra_notices)
    after = archive.stat()
    if archive.is_symlink() or \
            (before.st_dev, before.st_ino, before.st_size, before.st_mtime_ns, before.st_ctime_ns) != \
            (after.st_dev, after.st_ino, after.st_size, after.st_mtime_ns, after.st_ctime_ns):
        raise ValueError("source archive changed during collection")
    payload = {"notices/" + name: data for name, data in notices.items()}
    manifest = dict(format=1, provider=provider, source=source, files=[], patches=[], metadata=[])
    for category, inputs in (("patches", patches), ("metadata", metadata)):
        for index, item in enumerate(inputs):
            path = item[0] if category == "patches" else item
            data = read_notice(path)
            relative = "provenance/{}-{:03d}".format(category, index)
            record = dict(path=relative, sha256=hashlib.sha256(data).hexdigest(), size_bytes=len(data))
            if category == "patches":
                record["url"] = item[1]
                source_identity(record)
                if record["sha256"] != item[2]:
                    raise ValueError("provider patch checksum differs from its pin")
            else:
                record["name"] = Path(path).name
            payload[relative] = data
            manifest[category].append(record)
    if len(payload) > MAX_FILES or sum(map(len, payload.values())) > MAX_TOTAL_BYTES:
        raise ValueError("source provider collection exceeds limits")
    manifest["files"] = [dict(path="notices/" + name, sha256=hashlib.sha256(data).hexdigest(),
                              size_bytes=len(data)) for name, data in sorted(notices.items())]
    return publish_collection(output, manifest, payload)


def publish_collection(output, manifest, payload):
    """Publish one audited, content-addressed collection; no provenance inference."""
    output = Path(output)
    if not payload or len(payload) > MAX_FILES or any(not isinstance(data, bytes) or
            not 0 < len(data) <= MAX_FILE_BYTES for data in payload.values()) or \
            sum(map(len, payload.values())) > MAX_TOTAL_BYTES:
        raise ValueError("provider collection payload is empty or exceeds limits")
    for name in payload:
        notice_path(name)
        if not name.startswith(("notices/", "provenance/")):
            raise ValueError("provider collection payload is misclassified")
    raw = canonical_json(manifest)
    key = hashlib.sha256(raw).hexdigest()
    if output.is_symlink():
        raise ValueError("provider notice output root is linked")
    output.mkdir(parents=True, exist_ok=True)
    destination = output / key
    if destination.exists() or destination.is_symlink():
        audit_source_provider_notices(destination)
        if read_notice(destination / "manifest.json") != raw:
            raise ValueError("existing provider collection identity differs")
        return destination
    with tempfile.TemporaryDirectory(prefix=".collect-", dir=output) as temporary:
        stage = Path(temporary) / key
        stage.mkdir()
        for name, data in payload.items():
            file = stage / name
            file.parent.mkdir(parents=True, exist_ok=True)
            file.write_bytes(data)
        (stage / "manifest.json").write_bytes(raw)
        audit_source_provider_notices(stage)
        stage.rename(destination)
    return destination


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("archive", "url", "sha256", "name", "version", "revision", "architecture", "output"):
        parser.add_argument("--" + name, required=True)
    parser.add_argument("--patch", action="append", nargs=3, default=[], metavar=("PATH", "URL", "SHA256"))
    parser.add_argument("--metadata", action="append", default=[])
    parser.add_argument("--extra-notice", action="append", default=[])
    args = parser.parse_args(argv)
    try:
        result = collect(args.archive, args.url, args.sha256,
                         {key: getattr(args, key) for key in ("name", "version", "revision", "architecture")},
                         args.output, args.patch, args.metadata, args.extra_notice)
    except (OSError, ValueError, tarfile.TarError, zipfile.BadZipFile, EOFError, lzma.LZMAError) as error:
        parser.exit(1, "ERROR: {}\n".format(error))
    print(result)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
