"""Original source-provider notice bytes/provenance; not legal qualification."""

import hashlib
import json
import re
import unicodedata
from pathlib import Path, PurePosixPath
from urllib.parse import urlsplit


MAX_FILE_BYTES = 16 * 1024 * 1024
MAX_TOTAL_BYTES = 128 * 1024 * 1024
MAX_FILES = 2048


def notice_path(value):
    """Require one portable, canonical relative name before touching disk."""
    if not isinstance(value, str) or not value or len(value) > 1024 or \
            any(ord(char) < 32 or char in '\\:<>"|?*' for char in value):
        raise ValueError("unsafe notice path: {!r}".format(value))
    parts = value.split("/")
    if any(part in ("", ".", "..") or part.endswith((".", " ")) or
           re.fullmatch(r"(?:con|prn|aux|nul|com[1-9]|lpt[1-9])(?:\..*)?", part, re.I)
           for part in parts):
        raise ValueError("unsafe notice path: " + value)
    return PurePosixPath(value)


def path_key(value):
    return unicodedata.normalize("NFC", value).casefold()


def canonical_json(value):
    return (json.dumps(value, sort_keys=True, ensure_ascii=False, indent=2) + "\n").encode("utf-8")


def unique_json(data, repeated=(), **kwargs):
    def pairs(items):
        result = {}
        for key, value in items:
            if key in result and key not in repeated:
                raise ValueError("duplicate notice metadata key: " + key)
            result[key] = value
        return result
    return json.loads(data, object_pairs_hook=pairs, **kwargs)


def source_identity(value):
    if isinstance(value, dict) and value.get("kind") == "dpkg-installed":
        fields = {"kind", "package", "version", "architecture", "source_package", "source_version", "sha256", "size_bytes"}
        if set(value) != fields or not re.fullmatch(r"[a-z0-9][a-z0-9+.-]*(?::(?:amd64|arm64))?", str(value.get("package"))) or \
                not re.fullmatch(r"[a-z0-9][a-z0-9+.-]*", str(value.get("source_package"))) or \
                value.get("architecture") not in ("x86_64", "arm64") or any(
                    not isinstance(value.get(key), str) or not re.fullmatch(r"[A-Za-z0-9.+:~_-]{1,256}", value[key])
                    for key in ("version", "source_version")) or \
                not re.fullmatch(r"[0-9a-f]{64}", str(value.get("sha256"))) or \
                type(value.get("size_bytes")) is not int or not 0 < value["size_bytes"] <= MAX_FILE_BYTES:
            raise ValueError("installed dpkg provenance is incomplete")
        return
    if not isinstance(value, dict) or not isinstance(value.get("url"), str):
        raise ValueError("source provenance is missing")
    url = urlsplit(value["url"])
    if any(ord(char) < 32 for char in value["url"]) or \
            url.scheme != "https" or not url.netloc or url.username or url.password or url.fragment or \
            not re.fullmatch(r"[0-9a-f]{64}", str(value.get("sha256", ""))) or \
            type(value.get("size_bytes")) is not int or value["size_bytes"] <= 0:
        raise ValueError("source URL/checksum/size provenance is incomplete")


def read_notice(path, limit=MAX_FILE_BYTES):
    path = Path(path)
    if path.is_symlink() or not path.is_file() or not 0 < path.stat().st_size <= limit:
        raise ValueError("notice input is missing, linked, empty or too large: " + str(path))
    before = path.stat()
    with path.open("rb") as stream:
        data = stream.read(limit + 1)
    after = path.stat()
    if len(data) > limit or not data.strip() or path.is_symlink() or \
            (before.st_dev, before.st_ino, before.st_size, before.st_mtime_ns, before.st_ctime_ns) != \
            (after.st_dev, after.st_ino, after.st_size, after.st_mtime_ns, after.st_ctime_ns):
        raise ValueError("notice input changed, is blank or too large: " + str(path))
    return data


def audit_source_provider_notices(directory):
    """Verify a content-addressed collection without its archive, SDK or network.

    This proves byte/provenance consistency, not native-provider coverage,
    upstream authenticity, license sufficiency or redistribution compliance.
    """
    directory = Path(directory)
    if directory.is_symlink():
        raise ValueError("provider notice collection is linked")
    root = directory.resolve(strict=True)
    raw = read_notice(root / "manifest.json", 4 * 1024 * 1024)
    manifest = unique_json(raw)
    if not isinstance(manifest, dict) or manifest.get("format") != 1 or \
            not isinstance(manifest.get("provider"), dict) or not all(
                isinstance(manifest["provider"].get(key), str) and
                manifest["provider"][key].strip() and manifest["provider"][key] != "None"
                for key in ("name", "version", "revision", "architecture")):
        raise ValueError("source notice provider identity is incomplete")
    if raw != canonical_json(manifest) or hashlib.sha256(raw).hexdigest() != root.name:
        raise ValueError("source notice manifest content identity differs")
    source_identity(manifest.get("source"))
    files = manifest.get("files")
    if not isinstance(files, list) or not files or len(files) > MAX_FILES or \
            not isinstance(manifest.get("patches"), list) or \
            not isinstance(manifest.get("metadata"), list):
        raise ValueError("source provider notice inventory is empty or unsupported")
    if sum(len(manifest[key]) for key in ("files", "patches", "metadata")) > MAX_FILES:
        raise ValueError("source provider notice inventory exceeds file limit")
    expected, aliases, parents, total = {"manifest.json"}, {"manifest.json"}, {}, 0
    for category in ("files", "patches", "metadata"):
        for entry in manifest[category]:
            if not isinstance(entry, dict):
                raise ValueError("source provider notice entry is malformed")
            relative = str(notice_path(entry.get("path")))
            prefix = "notices/" if category == "files" else "provenance/"
            if not relative.startswith(prefix) or path_key(relative) in aliases:
                raise ValueError("source provider notice path is duplicated or misclassified")
            parts = relative.split("/")
            for index in range(1, len(parts)):
                parent = "/".join(parts[:index])
                key = path_key(parent)
                if key in parents and parents[key] != parent:
                    raise ValueError("source provider notice parent is aliased")
                parents[key] = parent
            if category == "patches":
                source_identity(entry)
            file = root / relative
            if not file.resolve(strict=True).is_relative_to(root) or any(
                    parent.is_symlink() for parent in file.parents if parent != root and root in parent.parents):
                raise ValueError("source provider notice path is linked or escaped")
            data = read_notice(file)
            total += len(data)
            if total > MAX_TOTAL_BYTES or type(entry.get("size_bytes")) is not int or \
                    len(data) != entry.get("size_bytes") or \
                    hashlib.sha256(data).hexdigest() != entry.get("sha256"):
                raise ValueError("source provider notice content hash differs: " + relative)
            expected.add(relative)
            aliases.add(path_key(relative))
    if total > MAX_TOTAL_BYTES or len(expected) > MAX_FILES + 1:
        raise ValueError("source provider notice inventory exceeds limits")
    actual = set()
    for file in root.rglob("*"):
        if file.is_symlink() or not file.resolve(strict=True).is_relative_to(root):
            raise ValueError("source provider notice tree contains a linked/escaped path")
        if file.is_file():
            actual.add(file.relative_to(root).as_posix())
        elif not file.is_dir():
            raise ValueError("source provider notice tree contains a special file")
    if actual != expected:
        raise ValueError("source provider notice inventory differs from installed files")
    source = manifest["source"]
    if source.get("kind") == "dpkg-installed":
        metadata = read_notice(root / "provenance/dpkg-query.json")
        record = unique_json(metadata)
        selected = record.get("selected", {}) if isinstance(record, dict) else {}
        provider = dict(name=source["package"], version=source["version"], architecture=source["architecture"],
                        revision="dpkg:{}:{}".format(source["source_package"], source["source_version"]))
        if hashlib.sha256(metadata).hexdigest() != source["sha256"] or len(metadata) != source["size_bytes"] or \
                metadata != canonical_json(record) or manifest["provider"] != provider or not isinstance(selected, dict) or \
                any(selected.get(key) != source[key] for key in ("package", "version", "source_package", "source_version")) or \
                {"amd64": "x86_64", "arm64": "arm64"}.get(selected.get("architecture")) != source["architecture"] or \
                selected.get("status") not in ("install ok installed", "hold ok installed") or \
                not isinstance(record.get("inputs"), dict) or set(record["inputs"]) != {item["path"] for item in files} or \
                not isinstance(record.get("packages"), dict) or not 0 < len(record["packages"]) <= 128 or \
                record["packages"].get(source["package"]) != selected:
            raise ValueError("installed dpkg notice provenance/provider differs")
        owned = {}
        for name, package in record["packages"].items():
            if not isinstance(package, dict) or name != package.get("package") or \
                    package.get("status") not in ("install ok installed", "hold ok installed") or \
                    not isinstance(package.get("md5sums"), str) or package.get("show") != "\t".join(
                        str(package.get(key, "")) for key in ("package", "version", "architecture", "status", "source_package", "source_version")) + "\n":
                raise ValueError("installed dpkg query/control metadata differs")
            seen = set()
            for line in package["md5sums"].splitlines():
                match = re.fullmatch(r"([0-9a-f]{32})  (.+)", line)
                if match is None or match[2] in seen:
                    raise ValueError("installed dpkg control checksums are malformed or duplicated")
                notice_path(match[2])
                seen.add(match[2])
                owned.setdefault("notices/" + match[2], []).append(match[1])
        for name, checksum in record["inputs"].items():
            if owned.get(name) != [checksum] or not re.fullmatch(r"[0-9a-f]{32}", str(checksum)) or \
                    hashlib.md5(read_notice(root / name), usedforsecurity=False).hexdigest() != checksum:
                raise ValueError("original dpkg notice checksum differs: " + name)
    return manifest["provider"]["name"], len(files)


def audit_source_provider_tree(directory):
    """Audit installed collections; native file/provider coverage is separate."""
    directory = Path(directory)
    if directory.is_symlink() or not directory.is_dir():
        raise ValueError("source provider notice directory is missing or linked")
    collections = sorted(directory.iterdir())
    if not collections or len(collections) > 128:
        raise ValueError("source provider notice directory is empty or exceeds limits")
    identities, count = set(), 0
    for collection in collections:
        _, files = audit_source_provider_notices(collection)
        provider = unique_json(read_notice(collection / "manifest.json"))["provider"]
        identity = tuple(provider[key] for key in ("name", "version", "revision", "architecture"))
        if identity in identities:
            raise ValueError("source provider notice identity is duplicated")
        identities.add(identity)
        count += files
    return len(identities), count
