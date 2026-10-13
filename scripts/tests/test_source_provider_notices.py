"""Original provider collection, archive safety and real common-install proof."""

import hashlib
import importlib.util
import io
import json
import os
import shutil
import stat
import subprocess
import sys
import tarfile
import tempfile
import unittest
import zipfile
from pathlib import Path
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("source_notice_collector", ROOT / "scripts/collect_provider_notices.py")
collector = importlib.util.module_from_spec(spec)
spec.loader.exec_module(collector)
import _providernotices as notices


class SourceProviderNoticeTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.provider = dict(name="qtbase", version="6.8.3", revision="selected-bottle-sha256",
                             architecture="arm64")

    def archive(self, files=None, kind="tar", special=None):
        files = files if files is not None else {"provider/LICENSE.txt": b"original terms\r\n"}
        path = self.root / ("source Ω." + kind)
        if kind == "zip":
            with zipfile.ZipFile(path, "w", compression=zipfile.ZIP_DEFLATED) as archive:
                for name, data in files.items():
                    archive.writestr(name, data)
                if special:
                    entry = zipfile.ZipInfo(special)
                    entry.create_system = 3
                    entry.external_attr = (stat.S_IFLNK | 0o777) << 16
                    archive.writestr(entry, "LICENSE.txt")
        else:
            mode = {"tar": "w", "tgz": "w:gz", "tbz2": "w:bz2", "txz": "w:xz"}[kind]
            with tarfile.open(path, mode) as archive:
                for name, data in files.items():
                    entry = tarfile.TarInfo(name)
                    entry.size = len(data)
                    archive.addfile(entry, io.BytesIO(data))
                if special:
                    entry = tarfile.TarInfo(special[0])
                    entry.type = special[1]
                    entry.linkname = "LICENSE.txt"
                    archive.addfile(entry)
        return path

    def collect(self, archive=None, **kwargs):
        archive = archive or self.archive()
        return collector.collect(archive, "https://provider.invalid/source.tar.xz",
                                 hashlib.sha256(archive.read_bytes()).hexdigest(),
                                 self.provider, self.root / "collected Ω", **kwargs)

    def clone(self, original, label="installed Ω"):
        target = self.root / label / original.name
        shutil.copytree(original, target)
        return target

    def reseal(self, directory, manifest):
        data = notices.canonical_json(manifest)
        (directory / "manifest.json").write_bytes(data)
        target = directory.with_name(hashlib.sha256(data).hexdigest())
        directory.rename(target)
        return target

    def test_all_archive_formats_preserve_original_notices_and_attribution_references(self):
        raw = b'{"Copyright":"first\nsecond", "Comment":"one", "Comment":"two", ' \
              b'"LicenseFile":"text/README", "LicenseFiles":["../COPYRIGHT", "text/CUSTOM"], ' \
              b'"CopyrightFile":"text/AUTHORS"}'
        files = {"provider/LICENSES/MIT.txt": b"original MIT terms\r\n\x00",
                 "provider/COPYRIGHT": b"copyright notice",
                 "provider/third-party/qt_attribution.json": raw,
                 "provider/third-party/text/README": b"nonstandard original license",
                 "provider/third-party/text/CUSTOM": b"other original license",
                 "provider/third-party/text/AUTHORS": b"original authors",
                 "provider/source.cpp": b"not a notice"}
        for kind in ("tar", "tgz", "tbz2", "txz", "zip"):
            with self.subTest(kind=kind):
                result = self.collect(self.archive(files, kind))
                self.assertEqual(("qtbase", 6), notices.audit_source_provider_notices(result))
                for name, data in files.items():
                    if not name.endswith("source.cpp"):
                        self.assertEqual(data, (result / "notices" / name).read_bytes())
                self.assertFalse((result / "notices/provider/source.cpp").exists())

    def test_verified_patches_and_selected_provider_metadata_are_retained(self):
        patch_file, metadata = self.root / "qt.patch", self.root / "INSTALL_RECEIPT.json"
        patch_file.write_bytes(b"original exact patch\r\n")
        metadata.write_bytes(b'{"selected":"receipt"}\n')
        result = self.collect(patches=[(patch_file, "https://provider.invalid/exact.patch",
                                        hashlib.sha256(patch_file.read_bytes()).hexdigest())],
                              metadata=[metadata])
        self.assertEqual(result, self.collect(patches=[(patch_file, "https://provider.invalid/exact.patch",
                                                       hashlib.sha256(patch_file.read_bytes()).hexdigest())],
                                             metadata=[metadata]))
        manifest = json.loads((result / "manifest.json").read_bytes())
        self.assertEqual(self.provider, manifest["provider"])
        self.assertEqual(patch_file.read_bytes(), (result / manifest["patches"][0]["path"]).read_bytes())
        self.assertEqual(metadata.read_bytes(), (result / manifest["metadata"][0]["path"]).read_bytes())

    def test_wrong_checksum_is_rejected_before_any_archive_parsing(self):
        archive = self.archive()
        with patch.object(collector, "original_notices") as parse:
            with self.assertRaisesRegex(ValueError, "checksum"):
                collector.collect(archive, "https://provider.invalid/source", "0" * 64,
                                  self.provider, self.root / "output")
            parse.assert_not_called()

    def test_wrong_patch_checksum_and_incomplete_provider_fail_closed(self):
        patch_file = self.root / "patch"
        patch_file.write_bytes(b"patch bytes")
        with self.assertRaisesRegex(ValueError, "patch checksum"):
            self.collect(patches=[(patch_file, "https://provider.invalid/patch", "0" * 64)])
        for key in self.provider:
            with self.subTest(key=key), patch.dict(self.provider, {key: "None"}):
                with self.assertRaisesRegex(ValueError, "identity"):
                    self.collect()

    def test_archives_reject_unsafe_member_paths_on_every_platform(self):
        for name in ("/LICENSE", "../LICENSE", "a/../LICENSE", "a/./LICENSE", "a//LICENSE",
                     "a\\LICENSE", "C:/LICENSE", "a/CON.txt", "a/LICENSE.", "a/LICENSE\n"):
            with self.subTest(name=name):
                with self.assertRaisesRegex(ValueError, "unsafe"):
                    self.collect(self.archive({name: b"license"}))

    def test_archive_links_and_special_members_are_never_followed(self):
        for kind in (tarfile.SYMTYPE, tarfile.LNKTYPE, tarfile.FIFOTYPE, tarfile.CHRTYPE):
            with self.subTest(kind=kind):
                with self.assertRaisesRegex(ValueError, "linked|special"):
                    self.collect(self.archive(special=("provider/link", kind)))
        with self.assertRaisesRegex(ValueError, "linked"):
            self.collect(self.archive(kind="zip", special="provider/link"))

    def test_duplicate_case_unicode_parent_and_file_directory_aliases_fail(self):
        for names in (("p/LICENSE", "p/license"), ("p/LICENSE", "P/NOTICE"),
                      ("p/COPYRIGHT-é", "p/COPYRIGHT-e\u0301"),
                      ("p/LICENSE", "p/LICENSE/NOTICE")):
            with self.subTest(names=names):
                with self.assertRaisesRegex(ValueError, "aliased|conflicting"):
                    self.collect(self.archive(dict.fromkeys(names, b"original")))
        archive = self.archive()
        with tarfile.open(archive, "a") as stream:
            entry = tarfile.TarInfo("provider/LICENSE.txt")
            entry.size = 9
            stream.addfile(entry, io.BytesIO(b"duplicate"))
        with self.assertRaisesRegex(ValueError, "duplicate"):
            self.collect(archive)

    def test_missing_empty_and_escaping_attribution_references_fail(self):
        for value in ("missing.txt", "../../outside", "/outside", "..\\outside"):
            with self.subTest(value=value):
                with self.assertRaisesRegex(ValueError, "missing|unsafe|escapes"):
                    self.collect(self.archive({"p/sub/qt_attribution.json":
                                               json.dumps({"LicenseFile": value}).encode()}))
        for data in (b"", b" \n\t"):
            with self.assertRaisesRegex(ValueError, "empty"):
                self.collect(self.archive({"p/LICENSE": data}))
        with self.assertRaisesRegex(ValueError, "no original"):
            self.collect(self.archive({"p/qt_attribution.json": b'{"LicenseId":"MIT"}'}))
        with self.assertRaisesRegex(ValueError, "duplicate"):
            self.collect(self.archive({"p/qt_attribution.json":
                                       b'{"LicenseFile":"LICENSE", "LicenseFile":"OTHER"}'}))

    def test_explicit_nonstandard_notice_is_preserved_and_required(self):
        result = self.collect(self.archive({"p/README.legal": b"exact terms"}), extra_notices=["p/README.legal"])
        self.assertEqual(b"exact terms", (result / "notices/p/README.legal").read_bytes())
        with self.assertRaisesRegex(ValueError, "missing"):
            self.collect(extra_notices=["p/missing"])

    def test_archive_member_expansion_and_collection_limits(self):
        for limit in ("MAX_ARCHIVE_BYTES", "MAX_EXPANDED_BYTES", "MAX_MEMBERS",
                      "MAX_FILE_BYTES", "MAX_FILES", "MAX_TOTAL_BYTES"):
            with self.subTest(limit=limit), patch.object(collector, limit, 1):
                with self.assertRaisesRegex(ValueError, "limit|large"):
                    self.collect(self.archive({"p/LICENSE": b"terms", "p/NOTICE": b"notice"}, "txz"))

    def test_linked_archive_output_and_provenance_inputs_fail(self):
        archive = self.archive()
        linked = self.root / "linked.tar"
        linked.symlink_to(archive)
        with self.assertRaisesRegex(ValueError, "linked"):
            self.collect(linked)
        (self.root / "collected Ω").symlink_to(self.root / "other", target_is_directory=True)
        with self.assertRaisesRegex(ValueError, "linked"):
            self.collect(archive)

    def test_archive_changed_during_collection_is_rejected(self):
        archive = self.archive()
        original = collector.original_notices
        def change(stream, extra):
            result = original(stream, extra)
            archive.write_bytes(archive.read_bytes() + b"changed")
            return result
        with patch.object(collector, "original_notices", side_effect=change):
            with self.assertRaisesRegex(ValueError, "changed"):
                self.collect(archive)

    def test_installed_audit_rejects_missing_tampered_unlisted_linked_and_changed_identity(self):
        original = self.collect()
        for kind in ("missing", "tampered", "unlisted", "linked", "manifest", "identity"):
            with self.subTest(kind=kind):
                target = self.clone(original, kind)
                file = target / "notices/provider/LICENSE.txt"
                if kind == "missing": file.unlink()
                elif kind == "tampered": file.write_bytes(b"changed terms")
                elif kind == "unlisted": (target / "extra").write_bytes(b"extra")
                elif kind == "linked":
                    file.unlink()
                    file.symlink_to(original / "notices/provider/LICENSE.txt")
                elif kind == "manifest": (target / "manifest.json").unlink()
                else: (target / "manifest.json").write_text("{}")
                with self.assertRaises((ValueError, OSError)):
                    notices.audit_source_provider_notices(target)

    def test_resealed_invalid_manifest_paths_duplicates_and_provenance_fail(self):
        original = self.collect()
        for kind in ("absolute", "parent", "duplicate", "identity", "source", "empty", "case", "parent-case"):
            with self.subTest(kind=kind):
                target = self.clone(original, kind)
                manifest = json.loads((target / "manifest.json").read_bytes())
                if kind == "absolute": manifest["files"][0]["path"] = "/outside"
                elif kind == "parent": manifest["files"][0]["path"] = "../outside"
                elif kind == "duplicate": manifest["files"].append(dict(manifest["files"][0]))
                elif kind == "identity": manifest["provider"]["revision"] = "None"
                elif kind == "source": manifest["source"]["url"] = "http://unverified.invalid/source"
                elif kind == "empty": manifest["files"] = []
                elif kind == "case":
                    entry = dict(manifest["files"][0]); entry["path"] = entry["path"].lower()
                    manifest["files"].append(entry)
                else:
                    entry = dict(manifest["files"][0]); entry["path"] = "notices/PROVIDER/NOTICE"
                    manifest["files"].append(entry)
                target = self.reseal(target, manifest)
                with self.assertRaises((ValueError, OSError)):
                    notices.audit_source_provider_notices(target)

    def test_installed_tree_requires_nonempty_unique_provider_collections(self):
        original = self.collect()
        self.assertEqual((1, 1), notices.audit_source_provider_tree(original.parent))
        changed = self.clone(original, "other")
        manifest = json.loads((changed / "manifest.json").read_bytes())
        manifest["source"]["url"] = "https://provider.invalid/another-source"
        changed = self.reseal(changed, manifest)
        shutil.copytree(changed, original.parent / changed.name)
        with self.assertRaisesRegex(ValueError, "duplicated"):
            notices.audit_source_provider_tree(original.parent)
        with self.assertRaises((ValueError, OSError)):
            notices.audit_source_provider_tree(self.root / "absent")

    @unittest.skipUnless(shutil.which("cmake"), "real CMake source-provider install fixture")
    def test_real_common_cmake_install_retains_collections_and_requires_manifest(self):
        collection = self.collect()
        source, build, prefix = self.root / "cmake source", self.root / "build", self.root / "installed Ω"
        source.mkdir()
        block = (ROOT / "CMakeLists.txt").read_text().split("# BEGIN source provider notices\n", 1)[1].split(
            "# END source provider notices", 1)[0]
        (source / "CMakeLists.txt").write_text('cmake_minimum_required(VERSION 3.28)\n'
                                             'project(Notices LANGUAGES NONE)\n'
                                             'set(CMAKE_INSTALL_DATADIR share)\n' + block)
        configure = ["cmake", "-S", str(source), "-B", str(build),
                     "-DLIBRECAD_EXTERNAL_PROVIDER_LICENSE_DIRS=" + str(collection)]
        for command in (configure, ["cmake", "--install", str(build), "--prefix", str(prefix)]):
            result = subprocess.run(command, capture_output=True, text=True, timeout=30)
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
        root = prefix / "share/librecad/licenses/providers/external"
        self.assertEqual((1, 1), notices.audit_source_provider_tree(root))
        (collection / "manifest.json").unlink()
        result = subprocess.run(configure, capture_output=True, text=True, timeout=30)
        self.assertNotEqual(0, result.returncode)
        self.assertIn("manifest is missing", result.stderr)

    def test_actual_installed_smoke_block_requires_and_audits_collections(self):
        original = self.collect()
        package = self.root / "app share/librecad/lcUIPy"
        package.mkdir(parents=True)
        source_root = package.parent / "licenses/providers/external"
        source_root.mkdir(parents=True)
        shutil.copytree(original, source_root / original.name)
        block = (ROOT / "lcUI/python/pyruntimesmoke.cpp").read_text().split(
            "# BEGIN original source-provider notices\n", 1)[1].split(
            "# END original source-provider notices", 1)[0]
        with patch.dict(sys.modules, {"lcUIPy._providernotices": notices}):
            namespace = dict(_package_root=package.resolve(), os=os)
            exec(block, namespace)
            self.assertIn("1 providers, 1 files", namespace["_source_notice_diagnostics"])
            (source_root / original.name / "notices/provider/LICENSE.txt").write_bytes(b"changed")
            with self.assertRaisesRegex(ValueError, "hash"):
                exec(block, namespace)
            shutil.rmtree(source_root)
            with patch.dict(os.environ, {"LIBRECAD_REQUIRE_PROVIDER_NOTICES": "1"}):
                with self.assertRaisesRegex(ValueError, "missing"):
                    exec(block, namespace)


if __name__ == "__main__":
    unittest.main()
