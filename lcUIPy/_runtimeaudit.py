"""Internal, SDK-free dependency checks for installed native artifacts."""

from pathlib import Path
import ctypes
import hashlib
import json
import os
import subprocess
import time


def audit_conan_notices(directory):
    """Verify selected Conan notice inventory/bytes, not legal compliance."""
    if Path(directory).is_symlink():
        raise RuntimeError("Conan notice inventory root is a symlink")
    root = Path(directory).resolve(strict=True)
    manifest_file = root / "manifest.json"
    if manifest_file.is_symlink():
        raise RuntimeError("Conan notice manifest is a symlink")
    manifest = json.loads(manifest_file.read_text(encoding="utf-8"))
    if not isinstance(manifest, dict) or manifest.get("format") != 1 or \
            not isinstance(manifest.get("providers"), list) or not manifest["providers"]:
        raise RuntimeError("Conan provider notice manifest is empty or unsupported")
    expected = {"manifest.json"}
    for provider in manifest["providers"]:
        if not isinstance(provider, dict) or not all(
                   isinstance(provider.get(key), str) and provider[key] and provider[key] != "None"
                   for key in ("reference", "recipe_revision", "package_id", "package_revision")):
            raise RuntimeError("Conan notice provider identity is incomplete")
        if "system_os" in provider or "system_libraries" in provider:
            system_libs = {"opengl/system": ["opengl32"], "glu/system": ["glu32"]}.get(provider["reference"])
            if not system_libs or provider.get("system_os") != "Windows" or \
                    provider.get("system_libraries") != system_libs or provider.get("files") != []:
                raise RuntimeError("Conan system dependency exemption is unsupported")
            continue
        if not isinstance(provider.get("files"), list) or not provider["files"]:
            raise RuntimeError("Conan provider has zero notice files")
        for entry in provider["files"]:
            if not isinstance(entry, dict) or "sha256" not in entry:
                raise RuntimeError("Conan notice file entry is incomplete")
            relative = entry.get("path")
            if not isinstance(relative, str) or not relative or "\\" in relative or ":" in relative or \
                    Path(relative).is_absolute() or ".." in Path(relative).parts or \
                    relative.casefold() in {path.casefold() for path in expected}:
                raise RuntimeError("Conan notice path is unsafe or duplicated: {}".format(relative))
            file = root / relative
            if file.is_symlink() or not file.resolve(strict=True).is_relative_to(root) or not file.is_file():
                raise RuntimeError("Conan notice file escapes its inventory: {}".format(relative))
            data = file.read_bytes()
            if not data.strip() or hashlib.sha256(data).hexdigest() != entry["sha256"]:
                raise RuntimeError("Conan notice content hash differs: {}".format(relative))
            expected.add(relative)
    actual = set()
    for path in root.rglob("*"):
        if path.is_symlink():
            raise RuntimeError("Conan notice inventory contains a symlink: {}".format(path))
        if path.is_file():
            actual.add(path.relative_to(root).as_posix())
    if actual != expected or len(expected) == 1:
        raise RuntimeError("Conan notice inventory differs from installed files")
    return len(manifest["providers"]), len(expected) - 1


def audit_windows_extensions(python_root):
    """Load each staged .pyd's normal imports, not PyInit or delay-load paths.

    Search only the extension's directory, the application directory and
    System32; neither PATH, CWD nor user-added SDK directories may rescue it.
    The installed CI smoke has an outer timeout for native loader calls.
    """
    root = Path(python_root).resolve(strict=True)
    checked = set()
    for candidate in sorted(root.rglob("*")):
        if candidate.suffix.lower() != ".pyd":
            continue
        binary = candidate.resolve(strict=True)
        if not binary.is_file() or not binary.is_relative_to(root):
            raise RuntimeError("Python extension symlink escapes runtime or is not a file: {}".format(candidate))
        if binary in checked:
            continue
        try:
            # LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | APPLICATION_DIR | SYSTEM32.
            ctypes.WinDLL(str(binary), winmode=0x100 | 0x200 | 0x800)
        except (OSError, AttributeError) as error:
            raise RuntimeError("unresolved Windows extension {}: {}".format(
                binary.relative_to(root), error)) from error
        checked.add(binary)
    if not checked:
        raise RuntimeError("Windows closure audit found zero Python extension files")
    return len(checked)


def _glibc_loader():
    loaders = set()
    for line in Path("/proc/self/maps").read_text().splitlines():
        fields = line.split(maxsplit=5)
        if len(fields) == 6 and fields[5].startswith("/"):
            path = Path(fields[5])
            if path.name.startswith("ld-linux-"):
                loaders.add(path.resolve())
    if len(loaders) != 1:
        raise RuntimeError("expected exactly one mapped glibc loader, found {}".format(loaders))
    return loaders.pop()


def _payload_files(root):
    device = root.stat().st_dev

    def fail(error):
        raise error

    for directory, directories, filenames in os.walk(root, onerror=fail):
        directory = Path(directory)
        # Snap content mounts supply dependencies, not files shipped by us.
        # lstat preserves symlinks for the later escape/deduplication checks.
        directories[:] = sorted(name for name in directories
                                if (directory / name).lstat().st_dev == device)
        for name in sorted(filenames):
            candidate = directory / name
            if candidate.lstat().st_dev == device:
                yield candidate


def audit_elf_closure(prefix, timeout_seconds=120):
    """Inspect every distinct loadable ELF with the running provider's loader.

    Inherit the real launcher's search paths and namespace (Snap includes its
    base/content providers); inspect the artifact's own filesystem, not
    external content mounts. This checks DT_NEEDED closure, not arbitrary
    dlopen names, module functionality, licenses or older-system compatibility.
    """
    if timeout_seconds <= 0:
        raise ValueError("ELF audit timeout must be positive")
    root = Path(prefix).resolve(strict=True)
    loader = _glibc_loader()
    deadline = time.monotonic() + timeout_seconds
    checked = set()
    for candidate in _payload_files(root):
        if not candidate.is_file():
            continue
        with candidate.open("rb") as stream:
            header = stream.read(20)
        if header[:4] != b"\x7fELF":
            continue
        if len(header) < 20 or header[4] not in (1, 2) or header[5] not in (1, 2):
            raise RuntimeError("invalid ELF header: {}".format(candidate))
        kind = int.from_bytes(header[16:18], "little" if header[5] == 1 else "big")
        if kind == 1:  # ET_REL (e.g. CPython config/python.o) is not loadable.
            continue
        if kind not in (2, 3):  # ET_EXEC / ET_DYN
            raise RuntimeError("unexpected payload ELF type {}: {}".format(kind, candidate))
        binary = candidate.resolve()
        if not binary.is_relative_to(root):
            raise RuntimeError("payload ELF symlink escapes prefix: {}".format(candidate))
        if binary in checked:
            continue
        remaining = deadline - time.monotonic()
        if remaining <= 0:
            raise RuntimeError("ELF closure audit exceeded its total time limit")
        try:
            result = subprocess.run(
                [str(loader), "--list", str(binary)],
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                text=True, errors="replace", check=False,
                timeout=min(15, remaining),
            )
        except (OSError, subprocess.TimeoutExpired) as error:
            raise RuntimeError("ELF closure inspection failed for {}: {}".format(binary, error)) from error
        if result.returncode or "=> not found" in result.stdout:
            raise RuntimeError("unresolved ELF closure for {} (exit {}):\n{}".format(
                binary.relative_to(root), result.returncode, result.stdout))
        checked.add(binary)
    if not checked:
        raise RuntimeError("ELF closure audit found zero payload ELF files")
    return len(checked)
