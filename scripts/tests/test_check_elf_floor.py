"""The AppImage ELF floor: host-provided symbol versions and x86 ISA levels."""

import os
import struct
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "scripts/check_elf_floor.py"
sys.path.insert(0, str(SCRIPT.parent))
import check_elf_floor  # noqa: E402

ET_REL, ET_EXEC, ET_DYN = 1, 2, 3
EM_386, EM_X86_64, EM_S390 = 3, 62, 22
V2, V3 = 0b11, 0b111
BASE = 0x10000


def build_elf(needs=(), isa=None, e_type=ET_DYN, machine=EM_X86_64, is64=True, little=True,
              note_align=8, notes_in="PT_NOTE", leading_note=True, feature_first=False,
              split_load=False, strtab_in_memsz_tail=False, understate_counts=False,
              duplicate_strtab=False, drop_null=False, second_dynamic=False):
    """A minimal ELF image as a loader sees it: PT_LOAD, PT_DYNAMIC for the
    version needs and, for isa, a GNU property note.  No section headers.

    needs: [(library, [(version, flags), ...]), ...]
    split_load: put the string table, needs and dynamic section in a second
        PT_LOAD whose file offset is not its address minus the base, the way
        patchelf moves .dynstr and .gnu.version_r.
    strtab_in_memsz_tail: point DT_STRTAB past the end of the file-backed part
        of a segment, into its zero-filled tail.
    feature_first: put GNU_PROPERTY_X86_FEATURE_1_AND before the ISA level,
        as compilers do.
    understate_counts: write DT_VERNEEDNUM and every vn_cnt as 1 while the
        chains carry everything.
    duplicate_strtab, drop_null, second_dynamic: dynamic sections glibc would
        read differently from a first-wins reader.
    """
    endian = "<" if little else ">"
    word = 8 if is64 else 4
    ehsize, phentsize = (64, 56) if is64 else (52, 32)
    names = [b""]
    offsets = {}

    def string(text):
        if text not in offsets:
            offsets[text] = sum(len(name) + 1 for name in names)
            names.append(text.encode())
        return offsets[text]

    verneed = bytearray()
    for index, (library, versions) in enumerate(needs):
        file_name = string(library)
        entry = struct.pack(endian + "HHIII", 1, 1 if understate_counts else len(versions), file_name,
                            16, 0 if index == len(needs) - 1 else 16 + 16 * len(versions))
        aux = bytearray()
        for aux_index, (version, flags) in enumerate(versions):
            aux += struct.pack(endian + "IHHII", 0, flags, 0, string(version),
                               0 if aux_index == len(versions) - 1 else 16)
        verneed += entry + aux
    strtab = b"\0".join(names) + b"\0"

    note = bytearray()
    if isa is not None:
        pad = (lambda size: (size + note_align - 1) // note_align * note_align)
        if leading_note:
            # A build-id note first: the GNU property note must be found
            # after skipping a differently-typed note with the same owner.
            build_id = b"\x11" * 20
            note += struct.pack(endian + "III", 4, len(build_id), 3) + b"GNU\0"
            note += build_id + b"\0" * (pad(16 + len(build_id)) - 16 - len(build_id))
        prop = bytearray()
        properties = [(check_elf_floor.GNU_PROPERTY_X86_ISA_1_NEEDED, isa)]
        if feature_first:
            properties.insert(0, (0xC0000002, 0x3))  # GNU_PROPERTY_X86_FEATURE_1_AND: IBT, SHSTK
        for pr_type, value in properties:
            prop += struct.pack(endian + "III", pr_type, 4, value)
            prop += b"\0" * ((-len(prop)) % word)
        note += struct.pack(endian + "III", 4, len(prop), 5) + b"GNU\0" + prop
        note += b"\0" * (pad(len(note)) - len(note))

    has_note = isa is not None
    segments = 2 + (1 if split_load else 0) + (1 if second_dynamic else 0) \
        + (1 if has_note and notes_in in ("PT_NOTE", "both") else 0) \
        + (1 if has_note and notes_in in ("PT_GNU_PROPERTY", "both") else 0)
    cursor = ehsize + segments * phentsize
    layout = {}
    for name, blob, alignment in (("note", note, 8), ("strtab", strtab, 1), ("verneed", verneed, 4),
                                  ("dynamic", b"", word)):
        cursor += (-cursor) % alignment
        layout[name] = cursor
        cursor += len(blob)
    second = layout["strtab"]
    moved = 0x400000 if split_load else 0

    def address(offset):
        return BASE + offset + (moved if offset >= second else 0)

    strtab_address = address(layout["strtab"])
    dynamic_entries = [(5, strtab_address), (10, len(strtab))]
    if needs:
        dynamic_entries += [(0x6FFFFFFE, address(layout["verneed"])),
                            (0x6FFFFFFF, 1 if understate_counts else len(needs))]
    if duplicate_strtab:
        dynamic_entries.append((5, strtab_address))
    if not drop_null:
        dynamic_entries.append((0, 0))
    dynamic = b"".join(struct.pack(endian + ("qQ" if is64 else "iI"), tag, value)
                       for tag, value in dynamic_entries)
    total = layout["dynamic"] + len(dynamic)
    if strtab_in_memsz_tail:
        # Re-point DT_STRTAB past the file-backed end of the last segment.
        tail = address(total) + 0x40
        dynamic = dynamic.replace(struct.pack(endian + ("qQ" if is64 else "iI"), 5, strtab_address),
                                  struct.pack(endian + ("qQ" if is64 else "iI"), 5, tail), 1)

    def phdr(p_type, offset, size, align, memsz=None):
        vaddr, memsz = address(offset), size if memsz is None else memsz
        if is64:
            return struct.pack(endian + "IIQQQQQQ", p_type, 4, offset, vaddr, vaddr, size, memsz, align)
        return struct.pack(endian + "IIIIIIII", p_type, offset, vaddr, vaddr, size, memsz, 4, align)

    tail_memsz = 0x1000 if strtab_in_memsz_tail else 0
    if split_load:
        headers = phdr(1, 0, second, 0x1000) + phdr(1, second, total - second, 0x1000,
                                                      total - second + tail_memsz)
    else:
        headers = phdr(1, 0, total, 0x1000, total + tail_memsz)
    headers += phdr(2, layout["dynamic"], len(dynamic), word)
    if second_dynamic:
        headers += phdr(2, layout["dynamic"], len(dynamic), word)
    if has_note and notes_in in ("PT_NOTE", "both"):
        headers += phdr(4, layout["note"], len(note), note_align)
    if has_note and notes_in in ("PT_GNU_PROPERTY", "both"):
        headers += phdr(0x6474E553, layout["note"], len(note), note_align)

    ident = b"\x7fELF" + bytes([2 if is64 else 1, 1 if little else 2, 1]) + b"\0" * 9
    if is64:
        header = ident + struct.pack(endian + "HHIQQQIHHHHHH", e_type, machine, 1, 0, ehsize, 0, 0,
                                     ehsize, phentsize, segments, 64, 0, 0)
    else:
        header = ident + struct.pack(endian + "HHIIIIIHHHHHH", e_type, machine, 1, 0, ehsize, 0, 0,
                                     ehsize, phentsize, segments, 40, 0, 0)
    image = bytearray(total)
    image[:len(header)] = header
    image[ehsize:ehsize + len(headers)] = headers
    for name, blob in (("strtab", strtab), ("verneed", verneed), ("note", note), ("dynamic", dynamic)):
        image[layout[name]:layout[name] + len(blob)] = blob
    return bytes(image)


GLIBC_OK = ("libc.so.6", [("GLIBC_2.2.5", 0), ("GLIBC_2.34", 0)])
GLIBCXX_OK = ("libstdc++.so.6", [("GLIBCXX_3.4.30", 0), ("CXXABI_1.3.13", 0)])


class ReadElfTests(unittest.TestCase):
    def test_version_needs_are_read_through_the_dynamic_segment(self):
        facts = check_elf_floor.read_elf(build_elf([GLIBC_OK, GLIBCXX_OK]))
        self.assertTrue(facts.loadable)
        self.assertEqual({("libc.so.6", "GLIBC_2.2.5"), ("libc.so.6", "GLIBC_2.34"),
                          ("libstdc++.so.6", "GLIBCXX_3.4.30"), ("libstdc++.so.6", "CXXABI_1.3.13")},
                         {(need.library, need.version) for need in facts.needs})

    def test_isa_level_is_read_from_an_8_aligned_property_note(self):
        # "GNU\0" is not padded to 8: the header plus name is.  Reading it as
        # padded put the descriptor four bytes late and failed every file.
        for notes_in in ("PT_NOTE", "PT_GNU_PROPERTY", "both"):
            with self.subTest(notes_in=notes_in):
                facts = check_elf_floor.read_elf(build_elf([GLIBC_OK], isa=V3, notes_in=notes_in))
                self.assertEqual(2, check_elf_floor.isa_level(facts.isa_needed))

    def test_the_isa_level_is_the_highest_bit_set(self):
        # A note may name only its level, without the levels below it.
        for bits, level in ((0b100, 2), (0b101, 2), (0b1000, 3), (0b1, 0)):
            with self.subTest(bits=bin(bits)):
                facts = check_elf_floor.read_elf(build_elf([GLIBC_OK], isa=bits))
                self.assertEqual(level, check_elf_floor.isa_level(facts.isa_needed))

    def test_isa_level_is_read_from_a_4_aligned_note(self):
        facts = check_elf_floor.read_elf(build_elf([GLIBC_OK], isa=V2, note_align=4))
        self.assertEqual(1, check_elf_floor.isa_level(facts.isa_needed))

    def test_32_bit_and_big_endian_images(self):
        facts = check_elf_floor.read_elf(build_elf([GLIBC_OK], isa=V3, is64=False, machine=EM_386))
        self.assertEqual(2, check_elf_floor.isa_level(facts.isa_needed))
        self.assertIn("GLIBC_2.34", {need.version for need in facts.needs})
        facts = check_elf_floor.read_elf(build_elf([("libc.so.6", [("GLIBC_2.43", 0)])],
                                                   little=False, machine=EM_S390))
        self.assertEqual(["GLIBC_2.43"], [need.version for need in facts.needs])

    def test_isa_note_is_ignored_on_other_machines(self):
        facts = check_elf_floor.read_elf(build_elf([GLIBC_OK], isa=V3, machine=EM_S390, little=False))
        self.assertEqual(0, facts.isa_needed)

    def test_relocatable_objects_are_not_loadable(self):
        self.assertFalse(check_elf_floor.read_elf(build_elf([GLIBC_OK], e_type=ET_REL)).loadable)

    def test_needs_in_a_segment_patchelf_moved_are_found(self):
        # patchelf moves .dynstr and .gnu.version_r into a new PT_LOAD whose
        # file offset is not its address minus the base.
        facts = check_elf_floor.read_elf(build_elf([GLIBC_OK, GLIBCXX_OK], split_load=True))
        self.assertIn("GLIBCXX_3.4.30", {need.version for need in facts.needs})

    def test_a_string_table_in_a_zero_filled_tail_is_an_error(self):
        with self.assertRaisesRegex(check_elf_floor.ElfError, "not backed by a PT_LOAD"):
            check_elf_floor.read_elf(build_elf([GLIBC_OK], strtab_in_memsz_tail=True))

    def test_the_isa_level_is_found_after_another_property(self):
        for is64 in (True, False):
            with self.subTest(is64=is64):
                facts = check_elf_floor.read_elf(build_elf(
                    [GLIBC_OK], isa=V3, feature_first=True, is64=is64,
                    machine=EM_X86_64 if is64 else EM_386, note_align=8 if is64 else 4))
                self.assertEqual(2, check_elf_floor.isa_level(facts.isa_needed))

    def test_chains_are_followed_past_counts_that_understate_them(self):
        # glibc follows vn_next and vna_next, not DT_VERNEEDNUM or vn_cnt.
        facts = check_elf_floor.read_elf(build_elf(
            [("libc.so.6", [("GLIBC_2.2.5", 0), ("GLIBC_2.43", 0)]),
             ("libstdc++.so.6", [("GLIBCXX_3.4.30", 0)])], understate_counts=True))
        self.assertEqual({"GLIBC_2.2.5", "GLIBC_2.43", "GLIBCXX_3.4.30"},
                         {need.version for need in facts.needs})

    def test_a_dynamic_section_the_loader_could_read_differently_is_an_error(self):
        # glibc takes the last of each tag and the last PT_DYNAMIC; refuse
        # both rather than check a chain the loader would not use.
        for options, message in (({"duplicate_strtab": True}, "appears more than once"),
                                 ({"second_dynamic": True}, "more than one PT_DYNAMIC"),
                                 ({"drop_null": True}, "no DT_NULL")):
            with self.subTest(**options):
                with self.assertRaisesRegex(check_elf_floor.ElfError, message):
                    check_elf_floor.read_elf(build_elf([GLIBC_OK], **options))

    def test_truncated_and_inconsistent_images_are_errors(self):
        image = build_elf([GLIBC_OK], isa=V3)
        for broken in (image[:40], image[:len(image) - 8], b"\x7fELF\x03" + image[5:]):
            with self.subTest(length=len(broken)):
                with self.assertRaises(check_elf_floor.ElfError):
                    check_elf_floor.read_elf(broken)


class FloorTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name).resolve() / "AppDir Ω"
        (self.root / "usr/lib").mkdir(parents=True)
        (self.root / "usr/bin").mkdir(parents=True)

    def put(self, relative, image):
        path = self.root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(image)
        return path

    def run_gate(self, *extra, paths=None):
        return subprocess.run(
            [sys.executable, "-B", str(SCRIPT), *(paths or [str(self.root)]), *extra],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    FLOOR = ("--max", "GLIBC_2.39", "--max", "GLIBCXX_3.4.33", "--max-isa", "x86-64-baseline")

    def test_a_tree_within_the_floor_passes_and_reports_its_highest_needs(self):
        self.put("usr/bin/librecad", build_elf([GLIBC_OK, GLIBCXX_OK]))
        self.put("usr/lib/libQt6Core.so.6", build_elf([("libc.so.6", [("GLIBC_2.39", 0)])]))
        self.put("usr/share/notes.txt", b"not an ELF")
        result = self.run_gate(*self.FLOOR)
        self.assertEqual(0, result.returncode, result.stdout + result.stderr)
        self.assertIn("ELF floor: PASS", result.stdout)
        self.assertIn("Loadable ELF objects: 2", result.stdout)
        self.assertRegex(result.stdout, r"GLIBC_2\.39\s+usr/lib/libQt6Core\.so\.6")
        self.assertRegex(result.stdout, r"CXXABI_1\.3\.13\s+usr/bin/librecad")
        self.assertIn("no object declares an x86 ISA level", result.stdout)

    def test_a_glibc_need_above_the_floor_fails_and_names_the_file(self):
        self.put("usr/bin/librecad", build_elf([GLIBC_OK]))
        self.put("usr/lib/liblckernel.so", build_elf([("libm.so.6", [("GLIBC_2.43", 0)])]))
        result = self.run_gate(*self.FLOOR)
        self.assertEqual(1, result.returncode)
        self.assertIn("ELF floor: FAIL", result.stdout)
        self.assertIn("usr/lib/liblckernel.so: needs GLIBC_2.43 from libm.so.6, newer than the "
                      "GLIBC_2.39 floor", result.stdout)

    def test_a_libstdcxx_need_above_the_floor_fails(self):
        self.put("usr/lib/libdxfrw.so.2", build_elf([("libstdc++.so.6", [("GLIBCXX_3.4.34", 0)])]))
        result = self.run_gate(*self.FLOOR)
        self.assertEqual(1, result.returncode)
        self.assertIn("needs GLIBCXX_3.4.34 from libstdc++.so.6", result.stdout)

    def test_an_undeclared_family_is_reported_but_not_held(self):
        self.put("usr/lib/libdxfrw.so.2", build_elf([("libstdc++.so.6", [("GLIBCXX_3.4.34", 0)])]))
        result = self.run_gate("--max", "GLIBC_2.39")
        self.assertEqual(0, result.returncode, result.stdout)
        self.assertRegex(result.stdout, r"GLIBCXX_3\.4\.34\s+usr/lib/libdxfrw\.so\.2")

    def test_an_x86_64_v3_note_fails_a_baseline_floor(self):
        self.put("usr/bin/librecad", build_elf([GLIBC_OK], isa=V3))
        result = self.run_gate(*self.FLOOR)
        self.assertEqual(1, result.returncode)
        self.assertIn("usr/bin/librecad: declares x86-64-v3 (GNU_PROPERTY_X86_ISA_1_NEEDED), above "
                      "the x86-64-baseline floor", result.stdout)

    def test_an_isa_note_within_the_floor_passes(self):
        self.put("usr/bin/librecad", build_elf([GLIBC_OK], isa=V2))
        self.assertEqual(0, self.run_gate("--max-isa", "x86-64-v2").returncode)
        self.assertEqual(1, self.run_gate("--max-isa", "x86-64-baseline").returncode)

    def test_weak_needs_do_not_fail(self):
        self.put("usr/lib/libweak.so", build_elf([("libc.so.6", [("GLIBC_2.43", 0x2)])]))
        result = self.run_gate(*self.FLOOR)
        self.assertEqual(0, result.returncode, result.stdout)
        self.assertIn("weak version needs ignored: 1", result.stdout)

    def test_glibc_names_without_a_release(self):
        self.put("usr/lib/librelr.so", build_elf([("libc.so.6", [("GLIBC_ABI_DT_RELR", 0)])]))
        self.assertEqual(0, self.run_gate("--max", "GLIBC_2.39").returncode)
        self.assertEqual(1, self.run_gate("--max", "GLIBC_2.35").returncode)
        self.put("usr/lib/libtls.so", build_elf([("libc.so.6", [("GLIBC_ABI_GNU2_TLS", 0)])]))
        result = self.run_gate("--max", "GLIBC_2.39")
        self.assertEqual(1, result.returncode)
        self.assertIn("needs GLIBC_ABI_GNU2_TLS from libc.so.6, newer than the GLIBC_2.39 floor",
                      result.stdout)
        self.assertEqual(0, self.run_gate("--max", "GLIBC_2.43").returncode)
        self.put("usr/lib/libprivate.so", build_elf([("ld-linux-x86-64.so.2", [("GLIBC_PRIVATE", 0)])]))
        result = self.run_gate("--max", "GLIBC_2.39")
        self.assertEqual(1, result.returncode)
        self.assertIn("needs GLIBC_PRIVATE from ld-linux-x86-64.so.2, which names no release",
                      result.stdout)

    def test_cxxabi_and_libgcc_floors(self):
        self.put("usr/lib/libQt6Core.so.6", build_elf([
            ("libstdc++.so.6", [("CXXABI_1.3.13", 0), ("CXXABI_TM_1", 0)]),
            ("libgcc_s.so.1", [("GCC_4.0.0", 0)])]))
        self.assertEqual(0, self.run_gate("--max", "CXXABI_1.3.15", "--max", "GCC_14.0.0").returncode)
        # CXXABI_TM_1 is held to the release it was read from, 1.3.15.
        result = self.run_gate("--max", "CXXABI_1.3.14")
        self.assertEqual(1, result.returncode)
        self.assertIn("needs CXXABI_TM_1 from libstdc++.so.6, newer than the CXXABI_1.3.14 floor",
                      result.stdout)
        self.assertEqual(1, self.run_gate("--max", "GCC_3.4").returncode)

    def test_unrelated_version_families_are_ignored(self):
        self.put("usr/lib/libssl.so.3", build_elf([("libcrypto.so.3", [("OPENSSL_3.4.0", 0)]),
                                                   ("libQt6Core.so.6", [("Qt_6", 0)])]))
        result = self.run_gate(*self.FLOOR)
        self.assertEqual(0, result.returncode, result.stdout)
        self.assertNotIn("OPENSSL", result.stdout)

    def test_relocatable_objects_are_skipped_but_cannot_satisfy_the_gate_alone(self):
        self.put("usr/lib/python3/config/python.o",
                 build_elf([("libc.so.6", [("GLIBC_2.43", 0)])], e_type=ET_REL))
        result = self.run_gate(*self.FLOOR)
        self.assertEqual(1, result.returncode)
        self.assertIn("other ELF files skipped: 1", result.stdout)
        self.assertIn("no loadable ELF object was found to check", result.stdout)
        self.put("usr/bin/librecad", build_elf([GLIBC_OK]))
        self.assertEqual(0, self.run_gate(*self.FLOOR).returncode)

    def test_an_empty_tree_fails(self):
        result = self.run_gate(*self.FLOOR)
        self.assertEqual(1, result.returncode)
        self.assertIn("no loadable ELF object was found to check", result.stdout)

    def test_a_malformed_elf_fails_closed(self):
        self.put("usr/bin/librecad", build_elf([GLIBC_OK]))
        self.put("usr/lib/libbroken.so", build_elf([GLIBC_OK])[:100])
        result = self.run_gate(*self.FLOOR)
        self.assertEqual(1, result.returncode)
        self.assertIn("usr/lib/libbroken.so: cannot be checked:", result.stdout)

    def test_symbolic_links_are_not_followed(self):
        outside = self.root.parent / "outside.so"
        outside.write_bytes(build_elf([("libc.so.6", [("GLIBC_2.43", 0)])]))
        self.put("usr/lib/libQt6Core.so.6.10.2", build_elf([GLIBC_OK]))
        os.symlink(outside, self.root / "usr/lib/libescape.so")
        os.symlink("libQt6Core.so.6.10.2", self.root / "usr/lib/libQt6Core.so.6")
        os.symlink(outside.parent, self.root / "usr/lib/escape-dir")
        result = self.run_gate(*self.FLOOR)
        self.assertEqual(0, result.returncode, result.stdout)
        self.assertIn("Loadable ELF objects: 1", result.stdout)

    def test_a_symbolic_link_named_on_the_command_line_is_followed(self):
        self.put("usr/lib/liblckernel.so", build_elf([("libm.so.6", [("GLIBC_2.43", 0)])]))
        link = self.root.parent / "AppDir link"
        os.symlink(self.root, link)
        result = self.run_gate(*self.FLOOR, paths=[str(link)])
        self.assertEqual(1, result.returncode)
        self.assertIn("usr/lib/liblckernel.so: needs GLIBC_2.43", result.stdout)

    @unittest.skipIf(hasattr(os, "geteuid") and os.geteuid() == 0, "root can list any directory")
    def test_a_directory_that_cannot_be_listed_fails(self):
        self.put("usr/bin/librecad", build_elf([GLIBC_OK]))
        hidden = self.root / "usr/lib/hidden"
        self.put("usr/lib/hidden/libnew.so", build_elf([("libc.so.6", [("GLIBC_2.43", 0)])]))
        hidden.chmod(0)
        self.addCleanup(hidden.chmod, 0o755)
        result = self.run_gate(*self.FLOOR)
        self.assertEqual(1, result.returncode)
        self.assertRegex(result.stdout, r"hidden: cannot be listed: ")

    def test_files_and_directories_can_be_checked_together(self):
        self.put("usr/bin/librecad", build_elf([GLIBC_OK]))
        runtime = self.root.parent / "LibreCAD3-x86_64.AppImage"
        runtime.write_bytes(build_elf([], e_type=ET_DYN) + b"hsqs" + b"\0" * 64)
        result = self.run_gate(*self.FLOOR, paths=[str(self.root), str(runtime)])
        self.assertEqual(0, result.returncode, result.stdout)
        self.assertIn("Loadable ELF objects: 2", result.stdout)

    def test_the_report_file_matches_standard_output(self):
        self.put("usr/bin/librecad", build_elf([GLIBC_OK], isa=V3))
        report = self.root.parent / "appimage-elf-floor.txt"
        result = self.run_gate(*self.FLOOR, "--report", str(report))
        self.assertEqual(1, result.returncode)
        self.assertEqual(result.stdout, report.read_text(encoding="utf-8"))

    def test_usage_errors(self):
        self.put("usr/bin/librecad", build_elf([GLIBC_OK]))
        for arguments in ((), ("--max", "GLIBC"), ("--max", "glibc_2.39"),
                          ("--max", "GLIBC_2.39", "--max", "GLIBC_2.35"),
                          ("--max-isa", "x86-64-v5")):
            with self.subTest(arguments=arguments):
                self.assertEqual(2, self.run_gate(*arguments).returncode)
        missing = self.run_gate("--max", "GLIBC_2.39", paths=[str(self.root / "missing")])
        self.assertEqual(2, missing.returncode)


if __name__ == "__main__":
    unittest.main()
