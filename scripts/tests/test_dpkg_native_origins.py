"""Exact installed dpkg inputs and SDK-free original notice joins."""

import hashlib
import json
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "lcUIPy"))
sys.path.insert(0, str(ROOT / "scripts/ubuntu-install"))
from _nativeorigins import OriginLedger, audit_native_origins, canonical_json
from _providernotices import audit_source_provider_notices
from dpkgNativeOrigins import DpkgOrigins


class DpkgOriginTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name).resolve()
        self.sdk = self.root / "SDK Ω"
        self.source = self.sdk / "usr/lib/libselected.so.1"
        self.source.parent.mkdir(parents=True)
        data = bytearray(128)
        data[:7] = b"\x7fELF\x02\x01\x01"
        data[16:18], data[18:20] = (3).to_bytes(2, "little"), (62).to_bytes(2, "little")
        self.source.write_bytes(data)
        self.copyright = self.sdk / "usr/share/doc/libselected/copyright"
        self.copyright.parent.mkdir(parents=True)
        self.copyright.write_bytes(b"exact original distro copyright\r\n/usr/share/common-licenses/GPL-2\n")
        common = self.sdk / "usr/share/common-licenses"
        common.mkdir(parents=True)
        self.license = common / "GPL-2"
        self.license.write_bytes(b"actual original common license\r\n")
        self.owners = {"/usr/lib/libselected.so.1": "libselected:amd64",
                       "/usr/share/doc/libselected/copyright": "libselected:amd64",
                       "/usr/share/common-licenses/GPL-2": "base-files"}
        self.rows = {"libselected:amd64": "libselected:amd64\t1:2.0-3ubuntu1\tamd64\tinstall ok installed\tselected-source\t1:2.0-3\n",
                     "base-files": "base-files\t14\tamd64\tinstall ok installed\tbase-files\t14\n"}
        self.sums = {}
        for path, owner in self.owners.items():
            self.sums.setdefault(owner, "")
            self.sums[owner] += hashlib.md5((self.sdk / path[1:]).read_bytes()).hexdigest() + "  " + path[1:] + "\n"
        self.adapter = DpkgOrigins(self.sdk)
        self.adapter.run = self.run_query

    def run_query(self, *args, missing=False):
        if args[0] == "--search":
            path = args[1]
            return ((self.owners[path] + ": " + path + "\n").encode() if path in self.owners else b"")
        if args[0] == "--show":
            return self.rows[args[-1]].encode()
        if args[0] == "--control-show":
            return self.sums[args[1]].encode()
        raise AssertionError(args)

    def test_capture_and_complete_original_notice_join_use_the_same_exact_provider(self):
        payload = self.root / "AppDir"
        payload.mkdir()
        destination = payload / "different-name.native"
        shutil.copy2(self.source, destination)
        ledger = OriginLedger(payload, "appimage-x86_64")
        provider = self.adapter.capture(ledger, destination, self.source)
        self.assertEqual(dict(name="libselected:amd64", version="1:2.0-3ubuntu1",
                              revision="dpkg:selected-source:1:2.0-3", architecture="x86_64"), provider)
        notices = payload / "notices"
        collection = self.adapter.collect_notices(provider, notices)
        self.assertEqual((provider["name"], 2), audit_source_provider_notices(collection))
        self.assertEqual(self.copyright.read_bytes(), (collection / "notices/usr/share/doc/libselected/copyright").read_bytes())
        self.assertEqual(self.license.read_bytes(), (collection / "notices/usr/share/common-licenses/GPL-2").read_bytes())
        destination.write_bytes(destination.read_bytes() + b"actual staged RPATH rewrite")
        manifest = payload / "origins.json"
        manifest.write_bytes(canonical_json(ledger.seal()))
        self.assertEqual((1, 1), audit_native_origins(payload, manifest, notices))
        self.assertEqual(collection, self.adapter.collect_notices(provider, notices))
        self.license.write_bytes(b"modified SDK license")
        self.assertEqual((1, 1), audit_native_origins(payload, manifest, notices))
        with self.assertRaises(ValueError):
            self.adapter.collect_notices(provider, notices)

    def test_usr_merge_alias_is_accepted_only_for_the_same_actual_file(self):
        (self.sdk / "lib").symlink_to("usr/lib", target_is_directory=True)
        self.owners["/lib/libselected.so.1"] = self.owners.pop("/usr/lib/libselected.so.1")
        self.sums["libselected:amd64"] = self.sums["libselected:amd64"].replace("usr/lib/libselected", "lib/libselected")
        self.assertEqual("libselected:amd64", self.adapter.identify(self.source, "x86_64")["name"])
        (self.sdk / "lib").unlink()
        (self.sdk / "lib").mkdir()
        shutil.copy2(self.source, self.sdk / "lib/libselected.so.1")
        with self.assertRaises(ValueError):
            self.adapter.identify(self.source, "x86_64")

    def test_shared_copyright_requires_the_same_exact_source_package_version(self):
        shared = self.sdk / "usr/share/doc/selected-common"
        self.copyright.parent.rename(shared)
        self.copyright.parent.symlink_to("selected-common", target_is_directory=True)
        old = "/usr/share/doc/libselected/copyright"
        new = "/usr/share/doc/selected-common/copyright"
        self.owners.pop(old)
        self.owners[new] = "selected-common"
        self.rows["selected-common"] = self.rows["libselected:amd64"].replace("libselected:amd64", "selected-common").replace("\tamd64\t", "\tall\t")
        self.sums["selected-common"] = hashlib.md5((shared / "copyright").read_bytes()).hexdigest() + "  " + new[1:] + "\n"
        self.sums["libselected:amd64"] = self.sums["libselected:amd64"].splitlines(keepends=True)[0]
        provider = self.adapter.identify(self.source, "x86_64")
        result = self.adapter.collect_notices(provider, self.root / "notices")
        self.assertEqual((provider["name"], 2), audit_source_provider_notices(result))
        self.rows["selected-common"] = self.rows["selected-common"].replace("1:2.0-3\n", "1:2.0-4\n")
        other = DpkgOrigins(self.sdk)
        other.run = self.run_query
        with self.assertRaisesRegex(ValueError, "different source version"):
            other.collect_notices(provider, self.root / "different-source")

    def test_unknown_ambiguous_diverted_changed_and_wrong_architecture_fail(self):
        for mutation in ("unknown", "ambiguous", "diversion", "changed", "architecture", "status",
                         "source-version", "duplicate-md5", "missing-md5", "outside", "non-native"):
            with self.subTest(mutation=mutation):
                self.setUp()
                source, architecture = self.source, "x86_64"
                if mutation == "unknown":
                    self.owners.clear()
                elif mutation == "ambiguous":
                    self.owners["/usr/lib/libselected.so.1"] += ", another:amd64"
                elif mutation == "diversion":
                    self.owners["/usr/lib/libselected.so.1"] = "diversion by libselected"
                elif mutation == "changed":
                    source.write_bytes(source.read_bytes() + b"changed")
                elif mutation == "architecture":
                    architecture = "arm64"
                elif mutation == "status":
                    self.rows["libselected:amd64"] = self.rows["libselected:amd64"].replace("install ok installed", "deinstall ok config-files")
                elif mutation == "source-version":
                    self.rows["libselected:amd64"] = self.rows["libselected:amd64"].replace("1:2.0-3\n", "\n")
                elif mutation == "duplicate-md5":
                    self.sums["libselected:amd64"] *= 2
                elif mutation == "missing-md5":
                    self.sums["libselected:amd64"] = ""
                elif mutation == "outside":
                    source = self.root / "outside"
                    shutil.copy2(self.source, source)
                else:
                    source.write_bytes(b"not native")
                with self.assertRaises((ValueError, OSError)):
                    self.adapter.identify(source, architecture)

    def test_notice_collection_rejects_missing_changed_or_escaped_originals(self):
        for mutation in ("copyright-missing", "copyright-changed", "copyright-escaped", "common-changed", "common-escaped"):
            with self.subTest(mutation=mutation):
                self.setUp()
                provider = self.adapter.identify(self.source, "x86_64")
                path = self.copyright if mutation.startswith("copyright") else self.license
                if mutation.endswith("missing"):
                    path.unlink()
                elif mutation.endswith("changed"):
                    path.write_bytes(b"not original")
                else:
                    outside = self.root / "outside notice"
                    path.rename(outside)
                    path.symlink_to(outside)
                with self.assertRaises((ValueError, OSError)):
                    self.adapter.collect_notices(provider, self.root / "notices")

    def test_queries_are_locale_fixed_timeout_bounded_and_explicit_rooted(self):
        self.adapter = DpkgOrigins(self.sdk)
        with patch("dpkgNativeOrigins.subprocess.run", return_value=subprocess.CompletedProcess([], 0, b"ok", b"")) as run:
            self.assertEqual(b"ok", self.adapter.run("--search", "/usr/lib/exact"))
        arguments, options = run.call_args
        self.assertIn("--root=" + str(self.sdk), arguments[0])
        self.assertIn("--admindir=" + str(self.sdk / "var/lib/dpkg"), arguments[0])
        self.assertEqual("C.UTF-8", options["env"]["LC_ALL"])
        self.assertEqual(15, options["timeout"])
        for result in (subprocess.CompletedProcess([], 2, b"", b"failure"),
                       subprocess.CompletedProcess([], 0, b"x" * (8 * 1024 * 1024 + 1), b"")):
            with patch("dpkgNativeOrigins.subprocess.run", return_value=result), self.assertRaises(ValueError):
                self.adapter.run("--show", "selected")

    def test_installed_dpkg_provenance_cannot_be_resealed_as_an_unrelated_provider(self):
        provider = self.adapter.identify(self.source, "x86_64")
        collection = self.adapter.collect_notices(provider, self.root / "notices")
        data = json.loads((collection / "manifest.json").read_bytes())
        for mutation in ("provider", "source", "control", "inputs", "selected"):
            with self.subTest(mutation=mutation):
                clone = self.root / mutation / collection.name
                shutil.copytree(collection, clone)
                manifest = json.loads(json.dumps(data))
                record_file = clone / "provenance/dpkg-query.json"
                record = json.loads(record_file.read_bytes())
                if mutation == "provider":
                    manifest["provider"]["version"] = "unrelated"
                elif mutation == "source":
                    manifest["source"]["unknown"] = "unverified"
                else:
                    if mutation == "control":
                        record["packages"]["base-files"]["md5sums"] = ""
                    elif mutation == "inputs":
                        record["inputs"] = {}
                    else:
                        record["selected"]["version"] = "unrelated"
                    raw = canonical_json(record)
                    record_file.write_bytes(raw)
                    manifest["source"].update(sha256=hashlib.sha256(raw).hexdigest(), size_bytes=len(raw))
                    manifest["metadata"][0].update(sha256=hashlib.sha256(raw).hexdigest(), size_bytes=len(raw))
                raw = canonical_json(manifest)
                (clone / "manifest.json").write_bytes(raw)
                target = clone.with_name(hashlib.sha256(raw).hexdigest())
                clone.rename(target)
                with self.assertRaises(ValueError):
                    audit_source_provider_notices(target)


if __name__ == "__main__":
    unittest.main()
