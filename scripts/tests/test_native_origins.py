"""Native original/final coverage and pre-rewrite Mac staging contracts."""

import hashlib
import io
import json
import shutil
import sys
import tarfile
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "lcUIPy"))
sys.path.insert(0, str(ROOT / "scripts"))
sys.path.insert(0, str(ROOT / "scripts/macos-install"))
import _nativeorigins as origins
import collect_provider_notices as collector
from stagePythonDependencies import RuntimeStager, homebrew_origin


def binary(kind="Mach-O", arch="arm64"):
    data = bytearray(128)
    if kind == "Mach-O":
        data[:4] = b"\xcf\xfa\xed\xfe"
        data[4:8] = (0x100000c if arch == "arm64" else 0x1000007).to_bytes(4, "little")
        data[12:16] = (6).to_bytes(4, "little")
    elif kind == "ELF":
        data[:7] = b"\x7fELF\x02\x01\x01"
        data[16:18] = (3).to_bytes(2, "little")
        data[18:20] = (183 if arch == "arm64" else 62).to_bytes(2, "little")
    else:
        data[:2] = b"MZ"
        data[60:64] = (64).to_bytes(4, "little")
        data[64:68] = b"PE\0\0"
        data[68:70] = (0x8664).to_bytes(2, "little")
        data[88:90] = b"\x0b\x02"
    return bytes(data)


class NativeOriginTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name).resolve()
        self.payload = self.root / "installed Ω"
        self.payload.mkdir()
        self.sdk = self.root / "selected SDK"
        self.sdk.mkdir()
        self.provider = dict(name="selected", version="1.2", revision="exact-prebuilt-revision", architecture="arm64")

    def capture(self, platform="macos-arm64"):
        kind, arch = origins.PLATFORMS[platform]
        source = self.sdk / "original"
        source.write_bytes(binary(kind, arch))
        destination = self.payload / "arbitrary filename Ω"
        shutil.copy2(source, destination)
        provider = dict(self.provider, architecture=arch)
        ledger = origins.OriginLedger(self.payload, platform)
        ledger.capture(destination, source, provider, self.sdk)
        destination.write_bytes(destination.read_bytes() + b"RPATH/signature rewrite")
        manifest = self.payload / "origins.json"
        manifest.write_bytes(origins.canonical_json(ledger.seal()))
        return ledger, manifest, source, destination

    def test_all_five_packaged_identities_use_the_same_original_final_contract(self):
        for platform in origins.PLATFORMS:
            with self.subTest(platform=platform):
                ledger, manifest, source, destination = self.capture(platform)
                self.assertEqual((1, 1), origins.audit_native_origins(self.payload, manifest))
                entry = json.loads(manifest.read_bytes())["files"][0]
                self.assertEqual(hashlib.sha256(source.read_bytes()).hexdigest(), entry["original"]["sha256"])
                self.assertEqual(hashlib.sha256(destination.read_bytes()).hexdigest(), entry["final"]["sha256"])
                self.assertNotEqual(entry["original"], entry["final"])
                self.assertEqual(source.name, entry["source_path"])
                self.assertFalse(Path(entry["path"]).is_absolute())

    def test_unknown_missing_and_tampered_native_payload_fail(self):
        _, manifest, _, destination = self.capture()
        destination.write_bytes(destination.read_bytes() + b"tampered")
        with self.assertRaisesRegex(ValueError, "final native bytes"):
            origins.audit_native_origins(self.payload, manifest)
        destination.unlink()
        with self.assertRaisesRegex(ValueError, "empty"):
            origins.audit_native_origins(self.payload, manifest)
        (self.payload / "unlisted native without extension").write_bytes(binary())
        with self.assertRaisesRegex(ValueError, "coverage differs"):
            origins.audit_native_origins(self.payload, manifest)

    def test_internal_native_aliases_are_deduplicated_but_escaping_aliases_fail(self):
        ledger, manifest, source, destination = self.capture()
        (self.payload / "alias.dylib").symlink_to(destination.name)
        with self.assertRaisesRegex(ValueError, "final native bytes/inventory"):
            origins.audit_native_origins(self.payload, manifest)
        manifest.write_bytes(origins.canonical_json(ledger.seal()))
        self.assertEqual((1, 1), origins.audit_native_origins(self.payload, manifest))
        (self.payload / "external.dylib").symlink_to(self.sdk / "original")
        with self.assertRaisesRegex(ValueError, "escaped alias"):
            origins.audit_native_origins(self.payload, manifest)

    def test_retargeting_contained_alias_is_detected_without_changing_native_file_bytes(self):
        ledger, manifest, source, destination = self.capture()
        other = self.payload / "other.native"
        shutil.copy2(source, other)
        ledger.capture(other, source, self.provider, self.sdk)
        alias = self.payload / "alias.dylib"
        alias.symlink_to(destination.name)
        manifest.write_bytes(origins.canonical_json(ledger.seal()))
        self.assertEqual((2, 1), origins.audit_native_origins(self.payload, manifest))
        alias.unlink()
        alias.symlink_to(other.name)
        with self.assertRaisesRegex(ValueError, "final native bytes/inventory"):
            origins.audit_native_origins(self.payload, manifest)

    def test_native_path_case_unicode_directory_aliases_and_walk_errors_fail(self):
        _, _, _, destination = self.capture()
        # APFS may not permit creating distinct case/normalization aliases;
        # feed the same hostile inventory on every host filesystem instead.
        for alias in (destination.name.upper(), destination.name.replace("Ω", "Ω")):
            if not (self.payload / alias).exists():
                (self.payload / alias).write_bytes(binary())
            with self.subTest(alias=alias), patch.object(origins.os, "walk", return_value=[
                    (str(self.payload), [], [destination.name, alias])]):
                with self.assertRaisesRegex(ValueError, "aliased paths"):
                    origins.native_payload(self.payload)
        def broken_walk(root, **kwargs):
            kwargs["onerror"](PermissionError("walk denied"))
        with patch.object(origins.os, "walk", broken_walk):
            with self.assertRaisesRegex(PermissionError, "walk denied"):
                origins.native_payload(self.payload)

    def test_original_provider_and_platform_must_be_explicit_and_matching(self):
        ledger, _, source, destination = self.capture()
        with self.assertRaisesRegex(ValueError, "identity"):
            ledger.capture(destination, source, dict(self.provider, revision="None"), self.sdk)
        with self.assertRaisesRegex(ValueError, "platform"):
            ledger.capture(destination, source, dict(self.provider, architecture="x86_64"), self.sdk)
        source.write_bytes(binary("PE", "x86_64"))
        with self.assertRaisesRegex(ValueError, "platform"):
            ledger.capture(destination, source, self.provider, self.sdk)
        with self.assertRaisesRegex(ValueError, "unknown packaged"):
            origins.OriginLedger(self.payload, "universal2")

    def test_changed_original_conflicting_capture_and_linked_input_are_rejected(self):
        ledger, _, source, destination = self.capture()
        source.write_bytes(source.read_bytes() + b"changed source")
        with self.assertRaisesRegex(ValueError, "conflicting original"):
            ledger.capture(destination, source, self.provider, self.sdk)
        alias = self.sdk / "linked"
        alias.symlink_to(source.name)
        with self.assertRaisesRegex(ValueError, "linked"):
            origins.binary_fingerprint(alias)
        original_header = origins.native_header
        def change_after_read(data):
            source.write_bytes(source.read_bytes() + b"raced")
            return original_header(data)
        with patch.object(origins, "native_header", change_after_read):
            with self.assertRaisesRegex(ValueError, "changed while hashing"):
                origins.binary_fingerprint(source)

    def test_malformed_duplicate_unsafe_or_unbound_manifest_entries_fail(self):
        _, manifest, _, _ = self.capture()
        original = json.loads(manifest.read_bytes())
        for change in (lambda value: value["files"].append(value["files"][0]),
                       lambda value: value["files"][0].update(path="../escape"),
                       lambda value: value["files"][0].update(provider="unknown"),
                       lambda value: value["files"][0]["original"].update(sha256="not a hash"),
                       lambda value: next(iter(value["providers"].values())).update(revision="changed")):
            value = json.loads(json.dumps(original))
            change(value)
            manifest.write_bytes(origins.canonical_json(value))
            with self.assertRaises(ValueError):
                origins.audit_native_origins(self.payload, manifest)
        manifest.write_bytes(b'{"format":1,"format":1}')
        with self.assertRaisesRegex(ValueError, "duplicate"):
            origins.audit_native_origins(self.payload, manifest)

    def test_exact_notice_join_requires_all_identity_fields_and_original_bytes(self):
        _, manifest, _, _ = self.capture()
        archive = self.root / "source.tar"
        with tarfile.open(archive, "w") as output:
            data = b"original provider license\r\n"
            member = tarfile.TarInfo("source/LICENSE")
            member.size = len(data)
            output.addfile(member, io.BytesIO(data))
        notice_root = self.payload / "notices"
        collection = collector.collect(archive, "https://selected.invalid/source.tar",
                                       hashlib.sha256(archive.read_bytes()).hexdigest(),
                                       self.provider, notice_root)
        self.assertEqual((1, 1), origins.audit_native_origins(self.payload, manifest, notice_root))
        external = self.root / "external-notices"
        shutil.copytree(notice_root, external)
        with self.assertRaisesRegex(ValueError, "notices escape"):
            origins.audit_native_origins(self.payload, manifest, external)
        (collection / "notices/source/LICENSE").write_bytes(b"changed original terms")
        with self.assertRaisesRegex(ValueError, "content hash"):
            origins.audit_native_origins(self.payload, manifest, notice_root)
        (collection / "notices/source/LICENSE").write_bytes(data)
        for field in origins.IDENTITY_KEYS:
            other = dict(self.provider, **{field: self.provider[field] + "-different"})
            other_root = self.payload / field
            collector.collect(archive, "https://selected.invalid/source.tar",
                              hashlib.sha256(archive.read_bytes()).hexdigest(), other, other_root)
            with self.subTest(field=field), self.assertRaisesRegex(ValueError, "lack exact original notices"):
                origins.audit_native_origins(self.payload, manifest, other_root)
        with self.assertRaisesRegex(ValueError, "missing or linked"):
            origins.audit_native_origins(self.payload, manifest, self.root / "absent")

    def test_invalid_native_headers_and_unknown_architectures_fail(self):
        for data in (b"MZ", b"\x7fELF", b"\xcf\xfa\xed\xfe", b"\xca\xfe\xba\xbe",
                     b"\xce\xfa\xed\xfe", binary("ELF")[:18] + b"\xff\xff" + bytes(108)):
            with self.subTest(header=data[:4]), self.assertRaises(ValueError):
                origins.native_header(data)

    def keg(self, name="python@3.14"):
        keg = self.root / "Cellar" / name / "1.2_1"
        keg.mkdir(parents=True, exist_ok=True)
        receipt = dict(built_as_bottle=True, poured_from_bottle=True, arch="arm64",
                       source=dict(tap="homebrew/core", versions=dict(stable="1.2")))
        (keg / "INSTALL_RECEIPT.json").write_text(json.dumps(receipt))
        (keg / "sbom.spdx.json").write_text(json.dumps(dict(packages=[dict(
            SPDXID="SPDXRef-Bottle-selected", name=name, versionInfo=keg.name,
            downloadLocation="https://ghcr.io/v2/homebrew/core/{}/blobs/sha256:{}".format(name.replace("@", "/"), "a" * 64),
            checksums=[dict(algorithm="SHA256", checksumValue="a" * 64)])])))
        source = keg / "runtime"
        source.write_bytes(binary())
        return source, keg

    def test_homebrew_identity_comes_from_the_exact_keg_and_rejects_mismatch(self):
        source, keg = self.keg()
        provider, root = homebrew_origin(source, "arm64")
        self.assertEqual(keg, root)
        self.assertEqual("homebrew/core:1.2_1:" + "a" * 64, provider["revision"])
        with self.assertRaisesRegex(RuntimeError, "architecture differs"):
            homebrew_origin(source, "x86_64")
        (self.sdk / "unknown").write_bytes(binary())
        with self.assertRaisesRegex(RuntimeError, "unknown original native provider"):
            homebrew_origin(self.sdk / "unknown", "arm64")

    def test_legacy_homebrew_sbom_version_still_binds_actual_keg_revision_and_bottle_hash(self):
        source, keg = self.keg()
        metadata = json.loads((keg / "sbom.spdx.json").read_bytes())
        metadata["packages"][0]["versionInfo"] = "1.2"
        (keg / "sbom.spdx.json").write_text(json.dumps(metadata))
        provider, _ = homebrew_origin(source, "arm64")
        self.assertEqual("homebrew/core:1.2_1:" + "a" * 64, provider["revision"])
        metadata["packages"][0]["versionInfo"] = "different-upstream-version"
        (keg / "sbom.spdx.json").write_text(json.dumps(metadata))
        with self.assertRaisesRegex(RuntimeError, "receipt/bottle/architecture differs"):
            homebrew_origin(source, "arm64")

    def test_external_content_mount_is_not_our_native_payload(self):
        _, manifest, _, _ = self.capture()
        content = self.payload / "content-provider"
        content.mkdir()
        original = Path.lstat
        def mounted(path):
            value = original(path)
            if path == content:
                fields = list(value)
                fields[2] += 1  # st_dev is a different mounted filesystem.
                return type(value)(fields)
            return value
        with patch.object(Path, "lstat", mounted), patch.object(origins.os, "walk", return_value=[
                (str(self.payload), [content.name], ["arbitrary filename Ω", "origins.json"])]):
            self.assertEqual((1, 1), origins.audit_native_origins(self.payload, manifest))

    def test_stager_captures_initial_sources_before_mutation_and_seals_after_signing(self):
        source, _ = self.keg()
        libraries, python_root, build = self.payload / "lib", self.payload / "python", self.root / "build"
        libraries.mkdir()
        python_root.mkdir()
        build.mkdir()
        runtime, app, original_app = libraries / "libpython.dylib", self.payload / "app", build / "app"
        for path in (runtime, app, original_app):
            path.write_bytes(binary())
        manifest = self.payload / "origins.json"
        stager = RuntimeStager(self.payload, python_root, libraries, "otool", "install_name_tool", "codesign",
                               app, False, [], False, origin_manifest=manifest, origin_platform="macos-arm64",
                               source_files=[(runtime, source)], project_files=[(app, original_app)],
                               project_root=build, project_revision="a" * 40 + "+diff" + "b" * 64, project_version="3.0",
                               relocate_python_runtime=(str(runtime), "original Python ID"))
        def command(args):
            if args[0] in ("install_name_tool", "codesign"):
                self.assertEqual(2, len(stager.origins.files))
                if args[0] == "install_name_tool" or "--sign" in args:
                    path = Path(args[-1])
                    path.write_bytes(path.read_bytes() + b"rewritten")
            return ""
        with patch.object(stager, "_loads", return_value=[]), patch.object(stager, "_rpaths", return_value=[]), \
                patch.object(stager, "_run", side_effect=command):
            stager.stage()
        self.assertEqual((2, 2), origins.audit_native_origins(self.payload, manifest))
        self.assertEqual(binary(), source.read_bytes())
        self.assertEqual(binary(), original_app.read_bytes())
        with self.assertRaisesRegex(ValueError, "lack exact original notices|notice directory"):
            origins.audit_native_origins(self.payload, manifest, self.root / "no-notices")

    def test_stager_unknown_initial_file_fails_before_any_mutation(self):
        libraries, python_root = self.payload / "lib", self.payload / "python"
        libraries.mkdir()
        python_root.mkdir()
        app = self.payload / "app"
        app.write_bytes(binary())
        stager = RuntimeStager(self.payload, python_root, libraries, "otool", "install_name_tool", "codesign",
                               app, False, [], False, origin_manifest=self.payload / "origins.json",
                               origin_platform="macos-arm64", project_root=self.sdk,
                               project_revision="a" * 40 + "+diff" + "b" * 64, project_version="3.0")
        with patch.object(stager, "_run") as command:
            with self.assertRaisesRegex(RuntimeError, "unknown original native file"):
                stager.stage()
            command.assert_not_called()

    def test_bundle_does_not_mislabel_install_prefix_byte_hashes_as_final_app_coverage(self):
        source = (ROOT / "scripts/macos-install/createBundle.sh").read_text()
        self.assertLess(source.index("audit_native_origins(root, sys.argv[2])"), source.index('ditto "$prefix/bin/librecad"'))
        self.assertLess(source.index("native-origins-install-prefix.json"), source.index('"$macdeployqt_bin" "$bundle"'))
        self.assertIn('mv "$resources/librecad/licenses/providers/native-origins.json" "$retained_origins"', source)


if __name__ == "__main__":
    unittest.main()
