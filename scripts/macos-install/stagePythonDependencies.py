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
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Sequence, Set


SYSTEM_PREFIXES = ("/System/Library/", "/usr/lib/")
LOAD_RE = re.compile(r"^\s*(.*?) \(compatibility version ")
RPATH_RE = re.compile(r"^\s*path (.*?) \(offset ")


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
        self.changed: Set[Path] = set()
        self.staged: Set[Path] = set()
        self.source_digests_by_name: Dict[str, str] = {}
        self.staged_sources: Dict[Path, Path] = {}
        self.deferred_framework_loads: Set[str] = set()

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
            shutil.copy2(str(source), str(destination))
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
        processed: Set[Path] = set()

        while queue:
            binary = queue.pop(0).resolve()
            if binary in processed:
                continue
            processed.add(binary)

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

        for binary in processed | self.changed | self.staged:
            self._run([self.codesign, "--force", "--sign", "-", str(binary)])
            if self.application_binary is not None and binary == self.application_binary:
                # The enclosing .app is re-signed only after all bundle edits.
                # Verifying its executable here would also check macdeployqt's
                # now-stale bundle resource seal.
                continue
            self._run([self.codesign, "--verify", "--strict", str(binary)])

        self._verify_no_external_loads(processed | self.staged)
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
        ).stage()
    except (OSError, RuntimeError) as error:
        print("macOS Python runtime staging failed: {}".format(error), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
