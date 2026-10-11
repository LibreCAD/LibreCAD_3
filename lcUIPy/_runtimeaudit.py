"""Internal, SDK-free dependency check for installed glibc/Linux artifacts."""

from pathlib import Path
import os
import subprocess
import time


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
