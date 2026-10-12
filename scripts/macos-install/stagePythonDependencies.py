#!/usr/bin/env python3
"""Stage non-system dylib dependencies of the installed CPython runtime."""

from __future__ import annotations

import argparse
import hashlib
import os
import plistlib
import re
import shutil
import subprocess
import sys
from itertools import chain, islice
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Sequence, Set, Tuple

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "lcUIPy"))
from _nativeorigins import (OriginLedger, binary_fingerprint, canonical_json, native_payload,
                            read_notice, read_origin_capture, unique_json)
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from record_python_artifact import fingerprint


SYSTEM_PREFIXES = ("/System/Library/", "/usr/lib/")
LOAD_RE = re.compile(r"^\s*(.*?) \(compatibility version ")
RPATH_RE = re.compile(r"^\s*path (.*?) \(offset ")


def bottle_runtime_dependencies(keg: Path, entries, architecture: str, installed=False):
    """Validate build/current tables separately; only current entries name kegs."""
    required = {"full_name", "version", "revision", "pkg_version", "declared_directly"}
    if not isinstance(entries, list) or len(entries) > 256:
        raise ValueError("malformed Homebrew runtime dependency table")
    result = {}
    for entry in entries:
        if not isinstance(entry, dict) or not required <= entry.keys() or \
                entry.keys() - required - {"bottle_rebuild", "compatibility_version"}:
            raise ValueError("unknown or incomplete Homebrew runtime dependency fields")
        name, version, revision = (entry[key] for key in ("full_name", "version", "revision"))
        if not isinstance(name, str) or not re.fullmatch(r"[a-z0-9+_.@-]+", name) or name in (".", "..") or \
                name in result or not isinstance(version, str) or not re.fullmatch(r"[A-Za-z0-9.+_-]{1,128}", version) or \
                type(revision) is not int or revision < 0 or \
                entry["pkg_version"] != version + ("_" + str(revision) if revision else "") or \
                type(entry["declared_directly"]) is not bool or any(
                    type(entry[key]) is not int or entry[key] < 0
                    for key in ("bottle_rebuild", "compatibility_version") if key in entry):
            raise ValueError("inconsistent Homebrew runtime dependency identity")
        result[name] = entry
        if installed:
            selected = keg.parent.parent / name / entry["pkg_version"]
            if selected.is_symlink() or selected.resolve(strict=True) != selected:
                raise ValueError("selected Homebrew dependency keg is linked or escaped: " + str(selected))
            receipt = unique_json(read_notice(selected / "INSTALL_RECEIPT.json"))
            source = receipt.get("source", {}) if isinstance(receipt, dict) else {}
            if not isinstance(receipt, dict) or not isinstance(source, dict) or \
                    not isinstance(source.get("versions"), dict) or \
                    receipt.get("built_as_bottle") is not True or receipt.get("poured_from_bottle") is not True or \
                    receipt.get("arch") != architecture or source.get("tap") != "homebrew/core" or \
                    source.get("versions", {}).get("stable") != version or \
                    receipt.get("compatibility_version") != entry.get("compatibility_version"):
                raise ValueError("selected Homebrew dependency receipt differs: " + str(selected))
    return result


def cached_bottle_digest(keg: Path, receipt: dict, architecture: str) -> str:
    """Bind source-only SBOMs to retained inputs, never the current formula/API.

    Homebrew can retain a source-only SBOM after pouring a bottle. The original
    cache location, OCI build tab, and actual archive must agree. This proves
    metadata/input consistency, not that post-pour native bytes equal tar bytes.
    """
    fields = ("homebrew_version", "changed_files", "source_modified_time", "compiler",
              "runtime_dependencies", "arch", "built_on")
    source = receipt["source"]
    api = Path(source.get("path", ""))
    if source.get("tap") != "homebrew/core" or not api.is_absolute() or ".." in api.parts or \
            api.parent.name != "internal" or api.parent.parent.name != "api" or \
            not re.fullmatch(r"packages\.[a-z0-9_]+\.jws\.json", api.name) or \
            not all(key in receipt for key in fields) or \
            not re.fullmatch(r"[a-z0-9+_.@-]+", keg.parent.name + keg.name):
        raise RuntimeError("selected Homebrew receipt has no exact cached bottle context: " + str(keg))
    downloads = api.parents[2] / "downloads"
    if downloads.is_symlink() or not downloads.is_dir():
        raise RuntimeError("selected Homebrew original download cache is absent: " + str(keg))

    def cached_file(suffix):
        matches = list(islice(downloads.glob("*--" + suffix), 2))
        if len(matches) != 1:
            state, count = ("missing", "0") if not matches else ("ambiguous", "2+")
            raise RuntimeError("selected Homebrew cached input is {}: {} (matches={}, cache={})".format(
                state, suffix, count, downloads))
        if not re.fullmatch(r"[0-9a-f]{64}--" + re.escape(suffix), matches[0].name):
            raise RuntimeError("selected Homebrew cached input filename is invalid: " + str(matches[0]))
        return matches[0]

    # Homebrew's index uses version-R, its descriptor version.tag.R, and its
    # archive version.tag.bottle.R.tar.gz. Check all retained rebuilds, never
    # silently prefer the base bottle or guess a tag from the current host.
    suffix = "{}-{}.bottle_manifest.json".format(keg.parent.name, keg.name)
    candidates = list(islice(chain(downloads.glob("*--" + suffix), downloads.glob(
        "*--{}-{}-[0-9]*.bottle_manifest.json".format(keg.parent.name, keg.name))), 257))
    if not candidates:
        cached_file(suffix)  # Retain precise absence diagnostics.
    if len(candidates) > 256:
        raise RuntimeError("selected Homebrew cached input is ambiguous: too many bottle manifests")
    selected, differences, seen = [], set(), set()
    optional = {"built_prefix", "padded_prefix", "linkage_files", "binary_relocation_files", "stdlib", "source"}
    for manifest in candidates:
        match = re.fullmatch(r"[0-9a-f]{64}--" + re.escape(keg.parent.name + "-" + keg.name) +
                             r"(-[1-9][0-9]*)?\.bottle_manifest\.json", manifest.name)
        if match is None:
            raise RuntimeError("selected Homebrew cached input filename is invalid: " + str(manifest))
        rebuild = match[1] or ""
        if rebuild in seen:
            raise RuntimeError("selected Homebrew cached input is ambiguous: {} (matches=2+, cache={})".format(
                manifest.name[66:], downloads))
        seen.add(rebuild)
        raw = read_notice(manifest, 4 * 1024 * 1024)
        index = unique_json(raw)
        annotations = index.get("annotations", {}) if isinstance(index, dict) else {}
        expected = {"com.github.package.type": "homebrew_bottle", "org.opencontainers.image.vendor": "homebrew",
                    "org.opencontainers.image.title": keg.parent.name, "org.opencontainers.image.version": keg.name,
                    "org.opencontainers.image.ref.name": keg.name + rebuild}
        # Older retained indices (e.g. libb2) spell this vendor "Homebrew".
        # Accept only the two observed literal values, not arbitrary case-folded
        # aliases; every other identity field and the raw OCI hash stay exact.
        mismatches = [key for key, value in expected.items() if isinstance(annotations, dict) and
                      annotations.get(key) != value and not (key == "org.opencontainers.image.vendor" and
                      annotations.get(key) == "Homebrew")]
        descriptors = index.get("manifests") if isinstance(index, dict) else None
        if not isinstance(index, dict) or index.get("schemaVersion") != 2 or not isinstance(annotations, dict) or \
                mismatches or \
                not isinstance(descriptors, list) or not 0 < len(descriptors) <= 256:
            detail = "; ".join(key + "=" + repr(annotations.get(key))[:200] for key in mismatches)
            raise RuntimeError("selected Homebrew cached OCI identity differs: {} ({})".format(
                keg, detail or "schemaVersion/annotations/manifests"))
        for descriptor in descriptors:
            if not isinstance(descriptor, dict) or not isinstance(descriptor.get("platform"), dict) or \
                    not isinstance(descriptor.get("annotations"), dict):
                raise ValueError("malformed cached bottle descriptor")
            platform, values = descriptor["platform"], descriptor["annotations"]
            if platform.get("os") != "darwin" or platform.get("architecture") != \
                    {"arm64": "arm64", "x86_64": "amd64"}.get(architecture):
                continue
            tab_text = values.get("sh.brew.tab", "null")
            if not isinstance(tab_text, str):
                raise ValueError("malformed cached bottle build tab")
            tab = unique_json(tab_text)
            if not isinstance(tab, dict) or not set(fields) <= tab.keys() or tab.keys() - set(fields) - optional:
                differences.add("incomplete/unknown build fields")
                continue
            changed = {key for key in tab if key != "runtime_dependencies" and canonical_json(tab[key]) !=
                       canonical_json({"scm_revision": source.get("scm_revision")} if key == "source" else receipt.get(key))}
            try:
                original_deps = bottle_runtime_dependencies(keg, tab["runtime_dependencies"], architecture)
                current_deps = bottle_runtime_dependencies(keg, receipt["runtime_dependencies"], architecture,
                    installed=canonical_json(tab["runtime_dependencies"]) != canonical_json(receipt["runtime_dependencies"]))
                if original_deps.keys() != current_deps.keys() or any(
                        dep["declared_directly"] != current_deps[name]["declared_directly"]
                        for name, dep in original_deps.items() if name in current_deps):
                    changed.add("runtime_dependencies names/declarations")
            except (ValueError, OSError) as error:
                changed.add("runtime_dependencies: " + str(error))
            if changed:
                differences.update(changed)
                continue
            selected.append((manifest, raw, values, rebuild, tab["runtime_dependencies"]))
    if len(selected) != 1:
        raise RuntimeError("selected Homebrew cached bottle build fields are missing or ambiguous: {} (matches={}, differences={})".format(
            keg, len(selected), "; ".join(sorted(differences)) or "none"))
    manifest, raw, values, rebuild, original_deps = selected[0]
    reference, digest, size = (values.get(key, "") for key in
                              ("org.opencontainers.image.ref.name", "sh.brew.bottle.digest", "sh.brew.bottle.size"))
    ref = re.fullmatch(re.escape(keg.name) + r"\.([a-z0-9_]+)" + re.escape(rebuild.replace("-", ".")), str(reference))
    if ref is None or \
            not re.fullmatch(r"[0-9a-f]{64}", str(digest)) or not re.fullmatch(r"[1-9][0-9]{0,9}", str(size)) or \
            not 0 < int(size) <= 2 * 1024 * 1024 * 1024:
        raise RuntimeError("selected Homebrew cached bottle input identity is malformed: " + str(keg))
    archive = cached_file("{}--{}.{}.bottle{}.tar.gz".format(keg.parent.name, keg.name, ref[1], rebuild.replace("-", ".")))
    if archive.stat().st_size != int(size) or fingerprint(archive)["sha256"] != digest:
        raise RuntimeError("selected Homebrew cached bottle bytes differ: " + str(keg))
    print("Verified cached Homebrew input: {} OCIsha256={} bottleSHA256={} bytes={} buildDependenciesSHA256={} currentDependenciesSHA256={}".format(
        manifest.name, hashlib.sha256(raw).hexdigest(), digest, size,
        hashlib.sha256(canonical_json(original_deps)).hexdigest(),
        hashlib.sha256(canonical_json(receipt["runtime_dependencies"])).hexdigest()))
    return digest


def homebrew_origin(source: Path, architecture: str):
    """Identify the actual selected keg, never another installation on PATH."""
    source = source.resolve(strict=True)
    keg = next((parent for parent in source.parents if parent.parent.parent.name == "Cellar"), None)
    if keg is None:
        raise RuntimeError("unknown original native provider (not a selected Homebrew keg): " + str(source))
    receipt = unique_json(read_notice(keg / "INSTALL_RECEIPT.json"))
    sbom = unique_json(read_notice(keg / "sbom.spdx.json"))
    source_info = receipt.get("source", {})
    version = source_info.get("versions", {}).get("stable")
    tap = source_info.get("tap")
    if not isinstance(version, str) or not version or not isinstance(tap, str) or not tap or \
            not re.fullmatch(re.escape(version) + r"(?:_\d+)?", keg.name):
        raise RuntimeError("selected Homebrew source/keg version differs: " + str(keg))
    bottles = [item for item in sbom.get("packages", [])
               if item.get("SPDXID", "").startswith("SPDXRef-Bottle-") and
               item.get("name") == keg.parent.name and item.get("versionInfo") in (keg.name, version)]
    if receipt.get("poured_from_bottle") is not True or \
            receipt.get("built_as_bottle") is not True or receipt.get("arch") != architecture:
        raise RuntimeError("selected Homebrew binary receipt/bottle/architecture differs: " + str(keg))
    if not any(item.get("SPDXID", "").startswith("SPDXRef-Bottle-") for item in sbom.get("packages", [])):
        digest = cached_bottle_digest(keg, receipt, architecture)
        return dict(name=keg.parent.name, version=version,
                    revision="{}:{}:{}".format(tap, keg.name, digest), architecture=architecture), keg
    if len(bottles) != 1:
        raise RuntimeError("selected Homebrew binary receipt/bottle/architecture differs: " + str(keg))
    checksums = [item.get("checksumValue") for item in bottles[0].get("checksums", [])
                 if item.get("algorithm") == "SHA256"]
    # Older Homebrew SBOMs (e.g. the selected zstd1.5.7_1 bottle) use the
    # upstream version, not pkg_version. Still bind the actual installed keg
    # revision AND exact bottle SHA; a different upstream version is rejected.
    if len(checksums) != 1 or not re.fullmatch(r"[0-9a-f]{64}", str(checksums[0])) or \
            not version or not tap or bottles[0].get("downloadLocation") != \
            "https://ghcr.io/v2/{}/blobs/sha256:{}".format(
                tap + "/" + keg.parent.name.replace("@", "/"), checksums[0]):
        raise RuntimeError("selected Homebrew binary version/revision is incomplete: " + str(keg))
    return dict(name=keg.parent.name, version=version,
                revision="{}:{}:{}".format(tap, keg.name, checksums[0]), architecture=architecture), keg


def minimum_macos(load_commands: str) -> Tuple[int, int, int]:
    """Return the highest macOS deployment target, including every fat slice."""
    slices = re.split(r"(?m)^.* \(architecture [^)]+\):\s*$", load_commands)
    if len(slices) > 1:
        return max(minimum_macos(part) for part in slices[1:])
    versions = []
    for command in re.split(r"(?m)^\s*Load command \d+\s*$", load_commands):
        if re.search(r"(?m)^\s*cmd LC_VERSION_MIN_(?!MACOSX\b)\w+\s*$", command):
            raise RuntimeError("non-macOS deployment target in staged Mach-O")
        if re.search(r"(?m)^\s*cmd LC_BUILD_VERSION\s*$", command):
            if not re.search(r"(?m)^\s*platform\s+(?:1|macos)\s*$", command, re.I):
                raise RuntimeError("non-macOS deployment target in staged Mach-O")
            field = "minos"
        elif re.search(r"(?m)^\s*cmd LC_VERSION_MIN_MACOSX\s*$", command):
            field = "version"
        else:
            continue
        match = re.search(r"(?m)^\s*" + field + r"\s+(\d+(?:\.\d+){0,2})\s*$", command)
        if match is None:
            raise RuntimeError("missing or malformed macOS deployment target")
        parts = tuple(int(value) for value in match[1].split("."))
        if not any(parts):
            raise RuntimeError("unspecified macOS deployment target")
        versions.append(parts + (0,) * (3 - len(parts)))
    if not versions:
        raise RuntimeError("no minimum macOS load command in staged Mach-O")
    return max(versions)


class RuntimeStager:
    def __init__(
        self,
        prefix: Path,
        python_root: Path,
        library_dir: Path,
        otool: str,
        install_name_tool: str,
        codesign: str,
        application_binary: Optional[Path],
        defer_frameworks: bool,
        scan_directories: Sequence[Path],
        scan_frameworks: bool,
        search_directories: Sequence[Path] = (),
        bundle_info_plist: Optional[Path] = None,
        origin_manifest: Optional[Path] = None,
        origin_platform: Optional[str] = None,
        source_files: Sequence[Sequence[Path]] = (),
        source_trees: Sequence[Sequence[Path]] = (),
        project_files: Sequence[Sequence[Path]] = (),
        project_root: Optional[Path] = None,
        project_revision: Optional[str] = None,
        project_version: Optional[str] = None,
        relocate_python_runtime: Optional[Sequence[str]] = None,
        origin_capture: Optional[Sequence[Path]] = None,
    ) -> None:
        self.prefix = prefix.resolve()
        self.python_root = python_root.resolve()
        self.library_dir = library_dir.resolve()
        self.application_binary = (
            application_binary.resolve() if application_binary is not None else None
        )
        self.executable_dir = (
            self.application_binary.parent
            if self.application_binary is not None
            else self.prefix / "bin"
        )
        self.otool = otool
        self.install_name_tool = install_name_tool
        self.codesign = codesign
        self.defer_frameworks = defer_frameworks
        self.scan_directories = [path.resolve() for path in scan_directories]
        self.scan_frameworks = scan_frameworks
        self.search_directories = [path.resolve() for path in search_directories]
        self.bundle_info_plist = bundle_info_plist
        self.changed: Set[Path] = set()
        self.staged: Set[Path] = set()
        self.source_digests_by_name: Dict[str, str] = {}
        self.staged_sources: Dict[Path, Path] = {}
        self.deferred_framework_loads: Set[str] = set()
        self.origin_manifest = origin_manifest
        self.origins = OriginLedger(self.prefix, origin_platform) if origin_manifest is not None else None
        self.origin_capture = origin_capture
        if origin_capture is not None:
            if origin_manifest is not None or len(origin_capture) != 2:
                raise RuntimeError("use either fresh-prefix origins or a bundle original-capture pair")
            source_capture, output_capture = map(Path, origin_capture)
            if self._inside_prefix(source_capture) or self._inside_prefix(output_capture) or \
                    output_capture.exists() or output_capture.is_symlink():
                raise RuntimeError("bundle original captures require fresh outputs outside signed resources")
            self.origins = read_origin_capture(self.prefix, source_capture)
            payload = native_payload(self.prefix)
            unknown = set(payload) - set(self.origins.files)
            if unknown:
                raise RuntimeError("unknown deployed native origin before rewriting: " + str(sorted(unknown)))
            missing = self.origins.required - set(payload)
            if missing:
                raise RuntimeError("required installed native copy is missing before rewriting: " + str(sorted(missing)))
            self.origins.files = {key: self.origins.files[key] for key in sorted(payload)}
            used = {entry["provider"] for entry in self.origins.files.values()}
            self.origins.providers = {key: self.origins.providers[key] for key in used}
        self.source_files = {Path(dst).resolve(): Path(src).resolve(strict=True) for dst, src in source_files}
        self.source_trees = [(Path(dst).resolve(), Path(src).resolve(strict=True)) for dst, src in source_trees]
        self.project_files = {Path(dst).resolve(): Path(src).resolve(strict=True) for dst, src in project_files}
        self.project_root = project_root
        self.project_revision, self.project_version = project_revision, project_version
        self.relocate_python_runtime = relocate_python_runtime
        if origin_manifest is not None:
            if not self._inside_prefix(origin_manifest) or origin_manifest.exists() or origin_manifest.is_symlink():
                raise RuntimeError("native origin capture requires a fresh in-prefix manifest; use a fresh install prefix")
            if not project_root or not project_revision or not project_version:
                raise RuntimeError("native origin capture requires explicit project source context")
            if not re.fullmatch(r"[0-9a-f]{40}(?:\+diff[0-9a-f]{64})?", project_revision):
                raise RuntimeError("native project context must be a full revision, optionally plus exact diff SHA-256")

    def _capture_origin(self, destination: Path, source: Optional[Path] = None) -> None:
        if self.origins is None:
            return
        destination = destination.resolve()
        relative = destination.relative_to(self.prefix).as_posix()
        if relative in self.origins.files:
            return
        original = source or self.source_files.get(destination) or self.project_files.get(destination)
        if original is None:
            for target, original_root in self.source_trees:
                if destination.is_relative_to(target):
                    candidate = original_root / destination.relative_to(target)
                    if candidate.is_file():
                        if original is not None and candidate.resolve() != original.resolve():
                            raise RuntimeError("ambiguous selected original native file: " + str(destination))
                        original = candidate.resolve(strict=True)
        if original is None:
            raise RuntimeError("unknown original native file before rewriting: " + str(destination))
        if original in self.project_files.values():
            architecture = self.origins.platform.removeprefix("macos-")
            provider = dict(name="LibreCAD", version=self.project_version,
                            revision=self.project_revision, architecture=architecture)
            source_root = self.project_root
        else:
            provider, source_root = homebrew_origin(original, self.origins.platform.removeprefix("macos-"))
        self.origins.capture(destination, original, provider, source_root)

    @staticmethod
    def _run(command: Sequence[str]) -> str:
        result = subprocess.run(
            command,
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        if result.returncode:
            raise RuntimeError(
                "command failed ({}): {}\n{}".format(
                    result.returncode, " ".join(command), result.stderr.strip()
                )
            )
        return result.stdout

    def _inside_prefix(self, path: Path) -> bool:
        try:
            path.resolve().relative_to(self.prefix)
            return True
        except ValueError:
            return False

    def _loads(self, binary: Path) -> List[str]:
        output = self._run([self.otool, "-L", str(binary)])
        install_id_output = self._run([self.otool, "-D", str(binary)])
        install_id_lines = [
            line.strip() for line in install_id_output.splitlines()[1:] if line.strip()
        ]
        install_id = install_id_lines[0] if install_id_lines else None
        loads = []
        for line in output.splitlines()[1:]:
            match = LOAD_RE.match(line)
            if match and match.group(1) != install_id:
                loads.append(match.group(1))
        return loads

    def _rpaths(self, binary: Path) -> List[str]:
        output = self._run([self.otool, "-l", str(binary)])
        lines = output.splitlines()
        rpaths = []
        for index, line in enumerate(lines):
            if line.strip() != "cmd LC_RPATH":
                continue
            for candidate in lines[index + 1 : index + 4]:
                match = RPATH_RE.match(candidate)
                if match:
                    rpaths.append(match.group(1))
                    break
        return rpaths

    def _expand_rpath(self, rpath: str, binary: Path) -> Optional[Path]:
        if rpath.startswith("@loader_path"):
            source_binary = self.staged_sources.get(binary, binary)
            return source_binary.parent / rpath[len("@loader_path") :].lstrip("/")
        if rpath.startswith("@executable_path"):
            return self.executable_dir / rpath[len("@executable_path") :].lstrip("/")
        if rpath.startswith("/"):
            return Path(rpath)
        return None

    def _resolve_load(self, binary: Path, load: str) -> Optional[Path]:
        if load.startswith("/"):
            return Path(load) if Path(load).exists() else None
        if load.startswith("@loader_path"):
            suffix = load[len("@loader_path") :].lstrip("/")
            candidates = [binary.parent / suffix]
            source_binary = self.staged_sources.get(binary, binary)
            if source_binary != binary:
                candidates.append(source_binary.parent / suffix)
            for path in candidates:
                if path.exists():
                    return path
            return None
        if load.startswith("@executable_path"):
            path = self.executable_dir / load[len("@executable_path") :].lstrip("/")
            return path if path.exists() else None
        if load.startswith("@rpath/"):
            suffix = load[len("@rpath/") :]
            local_candidate = self.library_dir / suffix
            if local_candidate.exists():
                return local_candidate
            for rpath in self._rpaths(binary):
                base = self._expand_rpath(rpath, binary)
                if base is None:
                    continue
                candidate = base / suffix
                if candidate.exists():
                    return candidate
            for directory in self.search_directories:
                candidate = directory / suffix
                if candidate.exists():
                    return candidate
        return None

    def _local_rpath_for(self, binary: Path) -> str:
        if binary.parent == self.library_dir:
            return "@loader_path"
        relative = os.path.relpath(self.library_dir, binary.parent)
        return "@loader_path/" + relative

    def _add_local_rpath(self, binary: Path) -> None:
        required = self._local_rpath_for(binary)
        if required in self._rpaths(binary):
            return
        self._run([self.install_name_tool, "-add_rpath", required, str(binary)])
        self.changed.add(binary)

    def _remove_external_rpaths(self, binary: Path) -> None:
        for rpath in self._rpaths(binary):
            if not rpath.startswith("/") or rpath.startswith(SYSTEM_PREFIXES):
                continue
            if self._inside_prefix(Path(rpath)):
                continue
            self._run([self.install_name_tool, "-delete_rpath", rpath, str(binary)])
            self.changed.add(binary)

    def _stage_external(self, source: Path) -> Path:
        source = source.resolve()
        if ".framework/" in str(source):
            raise RuntimeError(
                "non-system framework dependency is not bundled; bundle it with "
                "macdeployqt or the app-bundle step: {}".format(source)
            )
        if not source.is_file():
            raise RuntimeError("Python extension dependency is not a file: {}".format(source))

        digest_builder = hashlib.sha256()
        with source.open("rb") as source_stream:
            for chunk in iter(lambda: source_stream.read(1024 * 1024), b""):
                digest_builder.update(chunk)
        digest = digest_builder.hexdigest()
        previous_digest = self.source_digests_by_name.get(source.name)
        if previous_digest and previous_digest != digest:
            raise RuntimeError(
                "distinct Python dependencies share basename {}: {} has SHA-256 {}, "
                "but another source has SHA-256 {}".format(
                    source.name, source, digest, previous_digest
                )
            )
        self.source_digests_by_name[source.name] = digest

        suffix = source.suffix
        stem = source.name[: -len(suffix)] if suffix else source.name
        staged_name = "{}.{}{}".format(stem, digest, suffix)
        destination = self.library_dir / staged_name
        if destination.exists():
            staged_id = "@rpath/" + staged_name
            output = self._run([self.otool, "-D", str(destination)])
            ids = [line.strip() for line in output.splitlines()[1:] if line.strip()]
            if staged_id not in ids:
                raise RuntimeError(
                    "existing staged dependency has an unexpected install ID: {}; "
                    "expected {}".format(destination, staged_id)
                )
        else:
            self._capture_origin(destination, source)
            shutil.copy2(str(source), str(destination))
            if self.origins is not None and binary_fingerprint(destination) != \
                    self.origins.files[destination.relative_to(self.prefix).as_posix()]["original"]:
                raise RuntimeError("original native bytes changed during staging: " + str(source))
            self.staged.add(destination)
        self.staged_sources[destination.resolve()] = source
        return destination

    def _rewrite_load(self, binary: Path, old_load: str, name: str) -> None:
        self._run(
            [self.install_name_tool, "-change", old_load, "@rpath/" + name, str(binary)]
        )
        self.changed.add(binary)
        self._add_local_rpath(binary)

    def _prepare_staged_library(self, binary: Path) -> None:
        identifier = "@rpath/" + binary.name
        self._run([self.install_name_tool, "-id", identifier, str(binary)])
        self.changed.add(binary)
        self._add_local_rpath(binary)

    def _framework_binaries(self) -> List[Path]:
        binaries = []
        for framework in self.library_dir.rglob("*.framework"):
            if not framework.is_dir():
                continue
            info_candidates = (
                framework / "Resources" / "Info.plist",
                framework / "Versions" / "Current" / "Resources" / "Info.plist",
            )
            bundle_info = next((path for path in info_candidates if path.is_file()), None)
            if bundle_info is None:
                raise RuntimeError("framework has no Info.plist: {}".format(framework))
            try:
                with bundle_info.open("rb") as plist_stream:
                    executable = plistlib.load(plist_stream).get("CFBundleExecutable")
            except (OSError, plistlib.InvalidFileException) as error:
                raise RuntimeError(
                    "could not read framework metadata {}: {}".format(bundle_info, error)
                )
            if not executable:
                raise RuntimeError("framework has no CFBundleExecutable: {}".format(bundle_info))
            candidates = (
                framework / "Versions" / "Current" / executable,
                framework / "Versions" / "A" / executable,
                framework / executable,
            )
            binary = next((path for path in candidates if path.is_file()), None)
            if binary is None:
                raise RuntimeError(
                    "framework executable {} is missing under {}".format(executable, framework)
                )
            binaries.append(binary.resolve())
        return binaries

    def stage(self) -> None:
        if not self.python_root.is_dir():
            raise RuntimeError("staged Python runtime not found: {}".format(self.python_root))
        if not self.library_dir.is_dir():
            raise RuntimeError("LibreCAD library directory not found: {}".format(self.library_dir))

        queue = [
            path
            for path in self.python_root.rglob("*")
            if path.is_file() and path.suffix in (".so", ".dylib")
        ]
        queue.extend(
            path
            for path in self.library_dir.glob("libpython*.dylib")
            if path.is_file()
        )
        if self.application_binary is not None:
            if not self.application_binary.is_file():
                raise RuntimeError(
                    "installed application executable not found: {}".format(
                        self.application_binary
                    )
                )
            queue.append(self.application_binary)
            queue.extend(
                path
                for path in self.library_dir.glob("*.dylib")
                if path.is_file()
            )
        for directory in self.scan_directories:
            if not directory.is_dir():
                raise RuntimeError("Mach-O scan directory not found: {}".format(directory))
            queue.extend(
                path
                for path in directory.rglob("*")
                if path.is_file() and path.suffix in (".so", ".dylib")
            )
        if self.scan_frameworks:
            queue.extend(self._framework_binaries())
        # Capture selected originals for ALL initial copies before the first
        # runtime/consumer install-name or RPATH change. External dependencies
        # are captured in _stage_external before their copy is made.
        for binary in queue:
            self._capture_origin(binary)
        if self.relocate_python_runtime is not None:
            runtime, original_id = self.relocate_python_runtime
            runtime = Path(runtime).resolve(strict=True)
            if not self._inside_prefix(runtime) or runtime.parent != self.library_dir:
                raise RuntimeError("relocated CPython runtime is outside the staged library directory")
            self._prepare_staged_library(runtime)
            for consumer in queue:
                if original_id in self._loads(consumer):
                    self._rewrite_load(consumer, original_id, runtime.name)
        processed: Set[Path] = set()

        while queue:
            binary = queue.pop(0).resolve()
            if binary in processed:
                continue
            processed.add(binary)
            self._capture_origin(binary, self.staged_sources.get(binary))

            if binary.parent == self.library_dir and binary not in self.staged:
                self._add_local_rpath(binary)

            for load in self._loads(binary):
                if load.startswith(SYSTEM_PREFIXES):
                    continue
                dependency = self._resolve_load(binary, load)
                if dependency is None:
                    if self.defer_frameworks and ".framework/" in load:
                        self.deferred_framework_loads.add(load)
                        continue
                    raise RuntimeError(
                        "could not resolve non-system load {!r} from {}".format(load, binary)
                    )

                if self._inside_prefix(dependency):
                    if load.startswith("/"):
                        self._rewrite_load(binary, load, dependency.name)
                    elif load.startswith("@rpath/"):
                        self._add_local_rpath(binary)
                    continue

                if self.defer_frameworks and ".framework/" in str(dependency):
                    self.deferred_framework_loads.add(load)
                    continue

                staged_dependency = self._stage_external(dependency)
                self._prepare_staged_library(staged_dependency)
                self._rewrite_load(binary, load, staged_dependency.name)
                queue.append(staged_dependency)

            self._remove_external_rpaths(binary)

        self._record_bundle_minimum_os(processed | self.changed | self.staged)
        for binary in sorted(processed | self.changed | self.staged):
            if self.bundle_info_plist is not None and binary == self.application_binary:
                # Signing an executable inside .app can seal the enclosing
                # bundle and reject not-yet-signed plugins. createBundle.sh
                # signs the complete bundle after every nested object is ready.
                continue
            self._run([self.codesign, "--force", "--sign", "-", str(binary)])
            self._run([self.codesign, "--verify", "--strict", str(binary)])

        self._verify_no_external_loads(processed | self.staged)
        if self.origins is not None:
            manifest = self.origins.seal()
            output = self.origin_manifest
            if self.origin_capture is not None:
                output = Path(self.origin_capture[1])
                manifest = self.origins.capture_state()
            output.parent.mkdir(parents=True, exist_ok=True)
            # Exclusive creation: an old, unverified report must never silently
            # stand in for capture from the current selected originals.
            with output.open("xb") as stream:
                stream.write(canonical_json(manifest))
            label = "selected original capture" if self.origin_capture else "original/final native coverage"
            print("Recorded {}: {} files, {} providers".format(label,
                len(manifest["files"]), len(manifest["providers"])))
        print(
            "Staged {} non-system runtime dylib(s); verified {} Mach-O file(s)".format(
                len(self.staged), len(processed | self.staged)
            )
        )
        if self.deferred_framework_loads:
            print(
                "Deferred {} framework load(s) to macdeployqt".format(
                    len(self.deferred_framework_loads)
                )
            )

    def _record_bundle_minimum_os(self, binaries: Iterable[Path]) -> None:
        if self.bundle_info_plist is None:
            return
        if not self._inside_prefix(self.bundle_info_plist):
            raise RuntimeError("bundle Info.plist is outside the staged prefix")
        with self.bundle_info_plist.open("rb") as stream:
            info = plistlib.load(stream)
        if not isinstance(info, dict):
            raise RuntimeError("bundle Info.plist must contain a dictionary")
        versions = [minimum_macos(self._run([self.otool, "-l", str(binary)]))
                    for binary in sorted(binaries)]
        if not versions:
            raise RuntimeError("cannot record a bundle minimum OS without Mach-O files")
        if "LSMinimumSystemVersion" in info:
            existing = info["LSMinimumSystemVersion"]
            versions.append(minimum_macos(
                "cmd LC_VERSION_MIN_MACOSX\nversion {}\n".format(existing)))
        version = ".".join(str(part) for part in max(versions))
        info["LSMinimumSystemVersion"] = version
        with self.bundle_info_plist.open("wb") as stream:
            plistlib.dump(info, stream, sort_keys=False)
        print("Recorded bundle minimum macOS: {}".format(version))

    def _verify_no_external_loads(self, binaries: Iterable[Path]) -> None:
        for binary in binaries:
            for load in self._loads(binary):
                if load.startswith(SYSTEM_PREFIXES):
                    continue
                if self.defer_frameworks and ".framework/" in load:
                    continue
                if load.startswith("/"):
                    raise RuntimeError(
                        "non-relocatable absolute load remains in {}: {}".format(binary, load)
                    )
                resolved = self._resolve_load(binary, load)
                if resolved is None or not self._inside_prefix(resolved):
                    if self.defer_frameworks and resolved is not None and ".framework/" in str(resolved):
                        continue
                    raise RuntimeError(
                        "unbundled Python runtime load remains in {}: {}".format(binary, load)
                    )


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("prefix", type=Path)
    parser.add_argument("python_root", type=Path)
    parser.add_argument("library_dir", type=Path)
    parser.add_argument("otool")
    parser.add_argument("install_name_tool")
    parser.add_argument("codesign")
    parser.add_argument("--application-binary", type=Path)
    parser.add_argument("--defer-frameworks", action="store_true")
    parser.add_argument("--scan-directory", type=Path, action="append", default=[])
    parser.add_argument("--search-directory", type=Path, action="append", default=[])
    parser.add_argument("--scan-frameworks", action="store_true")
    parser.add_argument("--bundle-info-plist", type=Path)
    parser.add_argument("--origin-manifest", type=Path)
    parser.add_argument("--origin-platform", choices=("macos-arm64", "macos-x86_64"))
    parser.add_argument("--source-file", type=Path, nargs=2, action="append", default=[])
    parser.add_argument("--source-tree", type=Path, nargs=2, action="append", default=[])
    parser.add_argument("--project-file", type=Path, nargs=2, action="append", default=[])
    parser.add_argument("--project-root", type=Path)
    parser.add_argument("--project-revision")
    parser.add_argument("--project-version")
    parser.add_argument("--relocate-python-runtime", nargs=2, metavar=("STAGED_LIBRARY", "ORIGINAL_ID"))
    parser.add_argument("--origin-capture", type=Path, nargs=2, metavar=("BEFORE_DEPLOYMENT", "AFTER_STAGING"))
    arguments = parser.parse_args(argv)

    try:
        RuntimeStager(
            arguments.prefix,
            arguments.python_root,
            arguments.library_dir,
            arguments.otool,
            arguments.install_name_tool,
            arguments.codesign,
            arguments.application_binary,
            arguments.defer_frameworks,
            arguments.scan_directory,
            arguments.scan_frameworks,
            arguments.search_directory,
            arguments.bundle_info_plist,
            arguments.origin_manifest,
            arguments.origin_platform,
            arguments.source_file,
            arguments.source_tree,
            arguments.project_file,
            arguments.project_root,
            arguments.project_revision,
            arguments.project_version,
            arguments.relocate_python_runtime,
            arguments.origin_capture,
        ).stage()
    except (OSError, RuntimeError, ValueError) as error:
        print("macOS Python runtime staging failed: {}".format(error), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
