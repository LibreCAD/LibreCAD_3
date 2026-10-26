#!/usr/bin/env python3
"""Fail when a packaged ELF needs a newer host than the declared floor.

An AppImage bundles most of its libraries, but not glibc, libstdc++ or
libgcc_s: those come from the machine it runs on.  Every ELF object records
the newest symbol version it needs from them (GLIBC_2.39, GLIBCXX_3.4.33, ...)
and the dynamic loader refuses to start it on a host that lacks one.  An x86
object can also declare the micro-architecture level it was built for
(GNU_PROPERTY_X86_ISA_1_NEEDED, "x86-64-v3" and so on), and glibc 2.33 and
later refuse to load it on a CPU below that level.

Both are read the way the loader reads them -- from the program headers, the
dynamic segment and the GNU property notes, not from section headers -- and
compared with the floor given on the command line.  Only loadable objects
(ET_EXEC and ET_DYN) are checked: a relocatable object such as CPython's
config/python.o is never loaded at run time.  A weak version need
(VER_FLG_WEAK) is counted but does not fail: the loader only warns about one
it cannot satisfy.  Anything that cannot be read -- a malformed ELF, a
directory that cannot be listed, a dynamic section the loader might read
differently -- fails the check rather than being skipped.
"""

from __future__ import annotations

import argparse
import os
import re
import struct
import sys
from pathlib import Path
from typing import Dict, Iterator, List, NamedTuple, Optional, Sequence, Tuple


ELF_MAGIC = b"\x7fELF"
ET_EXEC, ET_DYN = 2, 3
EM_386, EM_X86_64 = 3, 62
PT_LOAD, PT_DYNAMIC, PT_NOTE = 1, 2, 4
PT_GNU_PROPERTY = 0x6474E553
DT_NULL, DT_STRTAB, DT_STRSZ = 0, 5, 10
DT_VERNEED = 0x6FFFFFFE
VER_FLG_WEAK = 0x2
NT_GNU_PROPERTY_TYPE_0 = 5
GNU_PROPERTY_X86_ISA_1_NEEDED = 0xC0008002

# GNU_PROPERTY_X86_ISA_1_NEEDED is a bit set; its highest bit is the level.
ISA_LEVELS = ("x86-64-baseline", "x86-64-v2", "x86-64-v3", "x86-64-v4")

# The version families an AppImage leaves to the host, always reported.
HOST_FAMILIES = ("GLIBC", "GLIBCXX", "CXXABI", "GCC")

# Version names that stand for a release rather than a symbol set, held to
# that release.  GLIBC_ABI_DT_RELR marks relative relocations packed with
# -z pack-relative-relocs, which the loader understands from glibc 2.36.  The
# three newer GLIBC_ABI tags are first in every build from 2.43: stable
# branches gained them later, but distribution 2.39 builds such as Ubuntu
# 24.04's do not have them, so an earlier release would let one through.  The
# two CXXABI tags are defined by the libstdc++ of GCC 14 (CXXABI_1.3.15), as
# read from ubuntu:24.04.  Each of these may overstate how new a tag is, never
# understate it.  Any other such name in a family with a floor fails, since it
# cannot be placed.
RELEASE_MARKERS = {
    "GLIBC_ABI_DT_RELR": (2, 36),
    "GLIBC_ABI_GNU2_TLS": (2, 43),
    "GLIBC_ABI_GNU_TLS": (2, 43),
    "GLIBC_ABI_DT_X86_64_PLT": (2, 43),
    "CXXABI_TM_1": (1, 3, 15),
    "CXXABI_FLOAT128": (1, 3, 15),
}

# Bounds the chain walks, so a corrupt file cannot make one run forever.
MAX_CHAIN = 4096

VERSION_PATTERN = re.compile(r"^([A-Z][A-Z0-9]*)_([0-9]+(?:\.[0-9]+)*)$")


class ElfError(Exception):
    """The file is an ELF object that cannot be read safely."""


class VersionNeed(NamedTuple):
    library: str
    version: str
    weak: bool


class ElfFacts(NamedTuple):
    loadable: bool
    machine: int
    needs: Tuple[VersionNeed, ...]
    isa_needed: int


class Floor(NamedTuple):
    versions: Dict[str, Tuple[int, ...]]
    isa: Optional[int]


class Report(NamedTuple):
    checked: int
    skipped: int
    weak: int
    highest: Dict[str, Tuple[Tuple[int, ...], List[str]]]
    highest_isa: Tuple[Optional[int], List[str]]
    violations: List[str]


def parse_version(text: str) -> Tuple[str, Tuple[int, ...]]:
    """Split "GLIBC_2.39" into ("GLIBC", (2, 39)), or raise ValueError."""
    match = VERSION_PATTERN.match(text)
    if match is None:
        raise ValueError("not a FAMILY_N[.N...] version: {}".format(text))
    return match.group(1), tuple(int(part) for part in match.group(2).split("."))


def format_version(family: str, version: Tuple[int, ...]) -> str:
    return "{}_{}".format(family, ".".join(str(part) for part in version))


def isa_level(bits: int) -> Optional[int]:
    """The level a GNU_PROPERTY_X86_ISA_1_NEEDED bit set names, or None."""
    if bits == 0:
        return None
    return bits.bit_length() - 1


def isa_name(level: int) -> str:
    if level < len(ISA_LEVELS):
        return ISA_LEVELS[level]
    return "x86-64 ISA bit {}".format(level)


def _align(value: int, alignment: int) -> int:
    return (value + alignment - 1) & ~(alignment - 1)


class _Elf:
    """Bounds-checked access to one ELF image, seen through its segments."""

    def __init__(self, data: bytes):
        if len(data) < 16 or data[:4] != ELF_MAGIC:
            raise ElfError("not an ELF file")
        if data[4] not in (1, 2):
            raise ElfError("unknown ELF class {}".format(data[4]))
        if data[5] not in (1, 2):
            raise ElfError("unknown ELF data encoding {}".format(data[5]))
        self.data = data
        self.is64 = data[4] == 2
        self.endian = "<" if data[5] == 1 else ">"
        self.word = 8 if self.is64 else 4
        self.type, self.machine = self.unpack("HH", 16)
        if self.is64:
            (phoff,) = self.unpack("Q", 32)
            phentsize, phnum = self.unpack("HH", 54)
            layout, minimum = "IIQQQQQQ", 56
        else:
            (phoff,) = self.unpack("I", 28)
            phentsize, phnum = self.unpack("HH", 42)
            layout, minimum = "IIIIIIII", 32
        if phnum and phentsize < minimum:
            raise ElfError("program header entry size {} is too small".format(phentsize))
        # (type, file offset, virtual address, file size, alignment)
        self.segments: List[Tuple[int, int, int, int, int]] = []
        for index in range(phnum):
            fields = self.unpack(layout, phoff + index * phentsize)
            if self.is64:
                p_type, _flags, offset, vaddr, _paddr, filesz, _memsz, align = fields
            else:
                p_type, offset, vaddr, _paddr, filesz, _memsz, _flags, align = fields
            self.segments.append((p_type, offset, vaddr, filesz, align))

    def unpack(self, fmt: str, offset: int) -> Tuple[int, ...]:
        fmt = self.endian + fmt
        size = struct.calcsize(fmt)
        if offset < 0 or offset + size > len(self.data):
            raise ElfError("structure at offset {:#x} runs past the end of the file".format(offset))
        return struct.unpack_from(fmt, self.data, offset)

    def string(self, offset: int, limit: int) -> str:
        if offset < 0 or offset >= limit:
            raise ElfError("string offset {:#x} is outside its table".format(offset))
        end = self.data.find(b"\0", offset, limit)
        if end < 0:
            raise ElfError("string at offset {:#x} is not terminated".format(offset))
        return self.data[offset:end].decode("utf-8", "replace")

    def file_offset(self, address: int) -> int:
        for p_type, offset, vaddr, filesz, _align in self.segments:
            if p_type == PT_LOAD and vaddr <= address < vaddr + filesz:
                return offset + (address - vaddr)
        raise ElfError("address {:#x} is not backed by a PT_LOAD segment".format(address))

    def dynamic(self) -> Dict[int, int]:
        # glibc takes the last PT_DYNAMIC and the last of each tag; a gate that
        # took the first could check a chain the loader never uses.  Neither
        # occurs in linker output, so refuse rather than pick.
        segments = [segment for segment in self.segments if segment[0] == PT_DYNAMIC]
        if not segments:
            return {}
        if len(segments) > 1:
            raise ElfError("more than one PT_DYNAMIC segment")
        _type, offset, _vaddr, filesz, _align = segments[0]
        entry = "qQ" if self.is64 else "iI"
        size = 2 * self.word
        values: Dict[int, int] = {}
        for index in range(filesz // size):
            tag, value = self.unpack(entry, offset + index * size)
            if tag == DT_NULL:
                return values
            if tag in values and tag in (DT_STRTAB, DT_STRSZ, DT_VERNEED):
                raise ElfError("dynamic tag {:#x} appears more than once".format(tag))
            values[tag] = value
        raise ElfError("the dynamic section has no DT_NULL")

    def version_needs(self) -> List[VersionNeed]:
        dynamic = self.dynamic()
        if DT_VERNEED not in dynamic:
            return []
        if DT_STRTAB not in dynamic:
            raise ElfError("DT_VERNEED without DT_STRTAB")
        strtab = self.file_offset(dynamic[DT_STRTAB])
        limit = min(len(self.data), strtab + dynamic.get(DT_STRSZ, len(self.data)))
        entry = self.file_offset(dynamic[DT_VERNEED])
        # Walked as glibc's _dl_check_map_versions walks them: along vn_next
        # and vna_next until each is 0.  DT_VERNEEDNUM and vn_cnt are not
        # consulted, so a file whose counts understate its chain cannot hide a
        # need the loader would still enforce.
        needs = []
        for _ in range(MAX_CHAIN):
            _version, _count, file_name, aux, next_entry = self.unpack("HHIII", entry)
            library = self.string(strtab + file_name, limit)
            position = entry + aux
            for _ in range(MAX_CHAIN):
                _hash, flags, _other, name, next_aux = self.unpack("IHHII", position)
                needs.append(VersionNeed(library, self.string(strtab + name, limit),
                                         bool(flags & VER_FLG_WEAK)))
                if next_aux == 0:
                    break
                position += next_aux
            else:
                raise ElfError("the version needs of {} do not end".format(library))
            if next_entry == 0:
                return needs
            entry += next_entry
        raise ElfError("the version-need chain does not end")

    def isa_needed(self) -> int:
        # PT_GNU_PROPERTY points into the note PT_NOTE also covers; reading
        # both ORs the same bits twice and catches a file that has either.
        bits = 0
        for p_type, offset, _vaddr, filesz, p_align in self.segments:
            if p_type not in (PT_NOTE, PT_GNU_PROPERTY):
                continue
            # As glibc's ELF_NOTE_DESC_OFFSET/ELF_NOTE_NEXT_OFFSET: what is
            # aligned is the header plus name, then the descriptor's end --
            # in an 8-aligned GNU property note "GNU\0" is not padded to 8.
            alignment = 8 if p_align == 8 else 4
            position, end = offset, offset + filesz
            while position + 12 <= end:
                namesz, descsz, note_type = self.unpack("III", position)
                name_start = position + 12
                desc_start = position + _align(12 + namesz, alignment)
                next_note = position + _align(desc_start - position + descsz, alignment)
                if desc_start + descsz > end:
                    raise ElfError("note at offset {:#x} runs past its segment".format(position))
                position = next_note
                if note_type != NT_GNU_PROPERTY_TYPE_0 or self.data[name_start:name_start + namesz] != b"GNU\0":
                    continue
                cursor, desc_end = desc_start, desc_start + descsz
                while cursor + 8 <= desc_end:
                    pr_type, pr_datasz = self.unpack("II", cursor)
                    data_start = cursor + 8
                    if data_start + pr_datasz > desc_end:
                        raise ElfError("GNU property at offset {:#x} runs past its note".format(cursor))
                    if pr_type == GNU_PROPERTY_X86_ISA_1_NEEDED and pr_datasz == 4:
                        bits |= self.unpack("I", data_start)[0]
                    cursor = data_start + _align(pr_datasz, self.word)
        return bits


def read_elf(data: bytes) -> ElfFacts:
    """What the loader would require of a host to load this ELF image."""
    elf = _Elf(data)
    if elf.type not in (ET_EXEC, ET_DYN):
        return ElfFacts(False, elf.machine, (), 0)
    isa = elf.isa_needed() if elf.machine in (EM_386, EM_X86_64) else 0
    return ElfFacts(True, elf.machine, tuple(elf.version_needs()), isa)


def iter_files(paths: Sequence[Path], errors: List[str]) -> Iterator[Tuple[Path, str]]:
    """Regular files under each path.

    A path named here is followed even if it is a symbolic link; links found
    inside a directory are not, so nothing outside it is checked or counted.
    A directory that cannot be listed is reported in errors.
    """
    def unlisted(error: OSError) -> None:
        errors.append("{}: cannot be listed: {}".format(error.filename, error.strerror))

    for path in paths:
        path = path.resolve()
        if path.is_file():
            yield path, path.name
            continue
        for directory, subdirectories, files in os.walk(path, onerror=unlisted):
            subdirectories.sort()
            for name in sorted(files):
                candidate = Path(directory, name)
                if candidate.is_symlink() or not candidate.is_file():
                    continue
                yield candidate, candidate.relative_to(path).as_posix()


def _starts_like_elf(path: Path) -> bool:
    with path.open("rb") as handle:
        return handle.read(4) == ELF_MAGIC


def _need_version(need: VersionNeed, floor: Floor) -> Tuple[Optional[str], Optional[Tuple[int, ...]]]:
    """The family and release a need maps to; (family, None) when it has none."""
    try:
        return parse_version(need.version)
    except ValueError:
        family = need.version.split("_", 1)[0]
        if family not in floor.versions and family not in HOST_FAMILIES:
            return None, None
        return family, RELEASE_MARKERS.get(need.version)


def check(paths: Sequence[Path], floor: Floor) -> Report:
    checked = skipped = weak = 0
    highest: Dict[str, Tuple[Tuple[int, ...], List[str]]] = {}
    highest_isa: Tuple[Optional[int], List[str]] = (None, [])
    violations: List[str] = []
    unlisted: List[str] = []

    for path, label in iter_files(paths, unlisted):
        try:
            if not _starts_like_elf(path):
                continue
            facts = read_elf(path.read_bytes())
        except (OSError, ElfError) as error:
            violations.append("{}: cannot be checked: {}".format(label, error))
            continue
        if not facts.loadable:
            skipped += 1
            continue
        checked += 1
        for need in facts.needs:
            family, version = _need_version(need, floor)
            if family is None or (family not in floor.versions and family not in HOST_FAMILIES):
                continue
            if need.weak:
                weak += 1
                continue
            limit = floor.versions.get(family)
            if version is None:
                if limit is not None:
                    violations.append("{}: needs {} from {}, which names no release and so "
                                      "cannot be held to the {} floor".format(
                                          label, need.version, need.library,
                                          format_version(family, limit)))
                continue
            current = highest.get(family)
            if current is None or version > current[0]:
                highest[family] = (version, [label])
            elif version == current[0] and label not in current[1]:
                current[1].append(label)
            if limit is not None and version > limit:
                violations.append("{}: needs {} from {}, newer than the {} floor".format(
                    label, need.version, need.library, format_version(family, limit)))
        level = isa_level(facts.isa_needed)
        if level is not None:
            if highest_isa[0] is None or level > highest_isa[0]:
                highest_isa = (level, [label])
            elif level == highest_isa[0]:
                highest_isa[1].append(label)
            if floor.isa is not None and level > floor.isa:
                violations.append("{}: declares {} (GNU_PROPERTY_X86_ISA_1_NEEDED), above the {} floor"
                                  .format(label, isa_name(level), isa_name(floor.isa)))
    violations.extend(unlisted)
    if checked == 0:
        violations.append("no loadable ELF object was found to check")
    return Report(checked, skipped, weak, highest, highest_isa, violations)


def _examples(labels: List[str]) -> str:
    shown = ", ".join(labels[:3])
    if len(labels) > 3:
        shown += " (+{} more)".format(len(labels) - 3)
    return shown


def render(report: Report, paths: Sequence[Path], floor: Floor) -> str:
    declared = [format_version(family, version) for family, version in sorted(floor.versions.items())]
    if floor.isa is not None:
        declared.append(isa_name(floor.isa))
    lines = [
        "ELF floor: {}".format("FAIL" if report.violations else "PASS"),
        "Checked: {}".format(", ".join(str(path) for path in paths)),
        "Loadable ELF objects: {} (other ELF files skipped: {}, weak version needs ignored: {})"
        .format(report.checked, report.skipped, report.weak),
        "Declared floor: {}".format(", ".join(declared) or "none"),
        "Highest requirements found:",
    ]
    for family in sorted(report.highest):
        version, labels = report.highest[family]
        lines.append("  {:<18} {}".format(format_version(family, version), _examples(labels)))
    level, labels = report.highest_isa
    if level is None:
        lines.append("  {:<18} no object declares an x86 ISA level".format("x86 ISA"))
    else:
        lines.append("  {:<18} {}".format(isa_name(level), _examples(labels)))
    if report.violations:
        lines.append("Violations:")
        lines.extend("  " + violation for violation in report.violations)
    return "\n".join(lines) + "\n"


def _floor_version(text: str) -> Tuple[str, Tuple[int, ...]]:
    try:
        return parse_version(text)
    except ValueError as error:
        raise argparse.ArgumentTypeError(str(error))


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n", 1)[0])
    parser.add_argument("paths", nargs="+", type=Path,
                        help="directories (walked without following symlinks) or files to check")
    parser.add_argument("--max", dest="maxima", action="append", default=[], type=_floor_version,
                        metavar="FAMILY_VERSION",
                        help="newest version of a host-provided family allowed, e.g. GLIBC_2.39 "
                             "(repeatable, once per family)")
    parser.add_argument("--max-isa", choices=ISA_LEVELS,
                        help="highest GNU_PROPERTY_X86_ISA_1_NEEDED level allowed")
    parser.add_argument("--report", type=Path, help="also write the report to this file")
    args = parser.parse_args(argv)

    versions: Dict[str, Tuple[int, ...]] = {}
    for family, version in args.maxima:
        if family in versions:
            parser.error("--max given twice for {}".format(family))
        versions[family] = version
    if not versions and args.max_isa is None:
        parser.error("nothing to check: give --max and/or --max-isa")
    for path in args.paths:
        if not path.exists():
            parser.error("no such file or directory: {}".format(path))
    floor = Floor(versions, ISA_LEVELS.index(args.max_isa) if args.max_isa else None)

    report = check(args.paths, floor)
    text = render(report, args.paths, floor)
    sys.stdout.write(text)
    if args.report is not None:
        args.report.write_text(text, encoding="utf-8")
    return 1 if report.violations else 0


if __name__ == "__main__":
    raise SystemExit(main())
