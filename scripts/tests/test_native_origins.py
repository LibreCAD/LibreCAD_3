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
import bundleNativeOrigins as bundle_origins


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

    def test_nonloadable_elf_objects_are_not_part_of_runtime_origin_coverage(self):
        _, manifest, _, _ = self.capture("appimage-x86_64")
        data = bytearray(binary("ELF", "x86_64"))
        data[16:18] = (1).to_bytes(2, "little")
        (self.payload / "python.o").write_bytes(data)
        self.assertIsNone(origins.native_header(data))
        self.assertEqual((1, 1), origins.audit_native_origins(self.payload, manifest))
        for kind in (0, 4):
            data[16:18] = kind.to_bytes(2, "little")
            with self.subTest(kind=kind), self.assertRaises(ValueError):
                origins.native_header(data)
        with self.assertRaises(ValueError):
            origins.native_header(b"\x7fELF")
        data[16:18] = (1).to_bytes(2, "little")
        data[18:20] = (0xffff).to_bytes(2, "little")
        with self.assertRaises(ValueError):
            origins.native_header(data)

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

    def cached_bottle(self, architecture="arm64", name="jpeg-turbo"):
        source, keg = self.keg(name)
        source.write_bytes(binary(arch=architecture))
        receipt = json.loads((keg / "INSTALL_RECEIPT.json").read_bytes())
        receipt.update(arch=architecture, homebrew_version="6.0.6", changed_files=["lib/pkgconfig/input.pc"],
                       source_modified_time=1782840343, compiler="clang", runtime_dependencies=[],
                       built_on=dict(os="Macintosh", os_version="macOS 26", xcode="26.4"))
        cache = self.root / ("original cache " + name)
        # The old API response need not survive. Never replace it with today's API.
        receipt["source"]["path"] = str(cache / "api/internal/packages.selected.jws.json")
        (keg / "INSTALL_RECEIPT.json").write_text(json.dumps(receipt))
        (keg / "sbom.spdx.json").write_text(json.dumps(dict(packages=[dict(
            SPDXID="SPDXRef-Archive-source", name=name + "-src", versionInfo="1.2")])))
        downloads = cache / "downloads"
        downloads.mkdir(parents=True)
        tag = "arm64_tahoe" if architecture == "arm64" else "sonoma"
        archive = downloads / ("a" * 64 + "--{}--1.2_1.{}.bottle.tar.gz".format(name, tag))
        archive.write_bytes(b"exact original compressed bottle input")
        digest = hashlib.sha256(archive.read_bytes()).hexdigest()
        build_fields = ("homebrew_version", "changed_files", "source_modified_time", "compiler",
                        "runtime_dependencies", "arch", "built_on")
        descriptor = dict(platform=dict(os="darwin", architecture="arm64" if architecture == "arm64" else "amd64"),
                          annotations={"org.opencontainers.image.ref.name": "1.2_1." + tag,
                                       "sh.brew.bottle.digest": digest,
                                       "sh.brew.bottle.size": str(archive.stat().st_size),
                                       "sh.brew.tab": json.dumps({key: receipt[key] for key in build_fields})})
        metadata = dict(schemaVersion=2, annotations={"com.github.package.type": "homebrew_bottle",
                        "org.opencontainers.image.vendor": "homebrew", "org.opencontainers.image.title": name,
                        "org.opencontainers.image.version": keg.name, "org.opencontainers.image.ref.name": keg.name},
                        manifests=[descriptor])
        manifest = downloads / ("b" * 64 + "--{}-1.2_1.bottle_manifest.json".format(name))
        manifest.write_text(json.dumps(metadata))
        return source, keg, archive, manifest, metadata, digest

    def test_source_only_sbom_uses_exact_cached_bottle_and_all_receipt_build_fields(self):
        for architecture in ("arm64", "x86_64"):
            with self.subTest(architecture=architecture):
                source, keg, _, _, _, digest = self.cached_bottle(architecture, "jpeg-" + architecture)
                provider, root = homebrew_origin(source, architecture)
                self.assertEqual(keg, root)
                self.assertEqual("homebrew/core:1.2_1:" + digest, provider["revision"])
                self.assertEqual(architecture, provider["architecture"])

    def test_legacy_bottle_without_sbom_requires_exact_retained_inputs_without_creating_metadata(self):
        for architecture in ("arm64", "x86_64"):
            with self.subTest(architecture=architecture):
                source, keg, archive, manifest, _, digest = self.cached_bottle(architecture, "no-sbom-" + architecture)
                sbom = keg / "sbom.spdx.json"
                sbom.unlink()
                originals = {file: file.read_bytes() for file in (archive, manifest, keg / "INSTALL_RECEIPT.json")}
                provider, selected = homebrew_origin(source, architecture)
                self.assertEqual(keg, selected)
                self.assertEqual("homebrew/core:1.2_1:" + digest, provider["revision"])
                self.assertEqual(architecture, provider["architecture"])
                self.assertFalse(sbom.exists() or sbom.is_symlink())
                for file, original in originals.items():
                    self.assertEqual(original, file.read_bytes())

    def test_absent_sbom_does_not_waive_original_inputs_or_allow_invalid_present_sboms(self):
        for architecture in ("arm64", "x86_64"):
            for mutation in ("missing-manifest", "missing-archive", "changed-archive", "source-built",
                             "wrong-architecture", "wrong-cache", "missing-build-field", "empty-sbom",
                             "malformed-sbom", "linked-sbom", "dangling-sbom"):
                with self.subTest(architecture=architecture, mutation=mutation):
                    source, keg, archive, manifest, _, _ = self.cached_bottle(architecture, architecture + mutation)
                    sbom = keg / "sbom.spdx.json"
                    sbom.unlink()
                    file = keg / "INSTALL_RECEIPT.json"
                    receipt = json.loads(file.read_bytes())
                    if mutation == "missing-manifest":
                        manifest.unlink()
                    elif mutation == "missing-archive":
                        archive.unlink()
                    elif mutation == "changed-archive":
                        archive.write_bytes(archive.read_bytes() + b"changed")
                    elif mutation == "source-built":
                        receipt["poured_from_bottle"] = False
                    elif mutation == "wrong-architecture":
                        receipt["arch"] = "x86_64" if architecture == "arm64" else "arm64"
                    elif mutation == "wrong-cache":
                        receipt["source"]["path"] = str(self.root / "unrelated-api.json")
                    elif mutation == "missing-build-field":
                        del receipt["built_on"]
                    elif mutation == "empty-sbom":
                        sbom.touch()
                    elif mutation == "malformed-sbom":
                        sbom.write_text("{not-json")
                    elif mutation == "linked-sbom":
                        target = self.root / (architecture + "-outside-sbom.json")
                        target.write_text('{"packages": []}')
                        sbom.symlink_to(target)
                    else:
                        sbom.symlink_to(self.root / "absent-sbom.json")
                    file.write_text(json.dumps(receipt))
                    with self.assertRaises((RuntimeError, ValueError, OSError)):
                        homebrew_origin(source, architecture)

    def test_sbom_presence_io_errors_are_not_mistaken_for_legacy_absence(self):
        exists, is_symlink, lstat = Path.exists, Path.is_symlink, Path.lstat
        for architecture in ("arm64", "x86_64"):
            for error in (PermissionError("metadata denied"), OSError("metadata I/O failed")):
                with self.subTest(architecture=architecture, error=type(error).__name__):
                    source, keg, _, _, _, _ = self.cached_bottle(architecture, architecture + type(error).__name__.lower())
                    sbom = keg / "sbom.spdx.json"
                    original = sbom.read_bytes()
                    def stat_or_error(path, *args, **kwargs):
                        if path == sbom:
                            raise error
                        return lstat(path, *args, **kwargs)
                    # Python 3.14 predicates hide OSError; simulate that policy
                    # on older test interpreters too. A strict presence probe
                    # must distinguish unreadable metadata from ENOENT.
                    with patch.object(Path, "exists", lambda path: False if path == sbom else exists(path)), \
                            patch.object(Path, "is_symlink", lambda path: False if path == sbom else is_symlink(path)), \
                            patch.object(Path, "lstat", stat_or_error), self.assertRaises(OSError):
                        homebrew_origin(source, architecture)
                    self.assertEqual(original, sbom.read_bytes())

    def test_legacy_homebrew_vendor_spelling_keeps_original_bytes_and_exact_other_identity(self):
        for architecture in ("arm64", "x86_64"):
            with self.subTest(architecture=architecture):
                source, _, _, manifest, metadata, digest = self.cached_bottle(architecture, "legacy-" + architecture)
                metadata["annotations"]["org.opencontainers.image.vendor"] = "Homebrew"
                manifest.write_text(json.dumps(metadata))
                before = manifest.read_bytes()
                self.assertTrue(homebrew_origin(source, architecture)[0]["revision"].endswith(digest))
                self.assertEqual(before, manifest.read_bytes())
                metadata["annotations"]["org.opencontainers.image.title"] = "another-formula"
                manifest.write_text(json.dumps(metadata))
                with self.assertRaises(RuntimeError):
                    homebrew_origin(source, architecture)

    def test_cached_oci_identity_diagnostics_reject_unknown_vendor_and_list_actual_difference(self):
        for index, vendor in enumerate(("HOMEBREW", "homeBrew", "different-vendor")):
            with self.subTest(vendor=vendor):
                source, _, _, manifest, metadata, _ = self.cached_bottle(name="vendor-" + str(index))
                metadata["annotations"]["org.opencontainers.image.vendor"] = vendor
                manifest.write_text(json.dumps(metadata))
                with self.assertRaises(RuntimeError) as error:
                    homebrew_origin(source, "arm64")
                self.assertIn("org.opencontainers.image.vendor", str(error.exception))
                self.assertIn(vendor, str(error.exception))

    def test_cache_failure_distinguishes_absent_and_ambiguous_inputs(self):
        for mutation, count in (("missing", "0"), ("ambiguous", "2+")):
            with self.subTest(mutation=mutation):
                source, _, _, manifest, _, _ = self.cached_bottle(name="jpeg-" + mutation)
                if mutation == "missing":
                    manifest.unlink()
                else:
                    shutil.copy2(manifest, manifest.with_name("c" * 64 + manifest.name[64:]))
                with self.assertRaises(RuntimeError) as error:
                    homebrew_origin(source, "arm64")
                self.assertIn("cached input is " + mutation + ":", str(error.exception))
                self.assertIn("matches=" + count, str(error.exception))
                self.assertIn(str(manifest.parent), str(error.exception))

    def test_cached_bottle_optional_build_fields_must_match_the_selected_receipt(self):
        source, keg, _, manifest, metadata, digest = self.cached_bottle()
        receipt_file = keg / "INSTALL_RECEIPT.json"
        receipt = json.loads(receipt_file.read_bytes())
        receipt["stdlib"] = "libc++"
        receipt_file.write_text(json.dumps(receipt))
        tab = json.loads(metadata["manifests"][0]["annotations"]["sh.brew.tab"])
        tab["stdlib"] = "libc++"
        metadata["manifests"][0]["annotations"]["sh.brew.tab"] = json.dumps(tab)
        manifest.write_text(json.dumps(metadata))
        self.assertTrue(homebrew_origin(source, "arm64")[0]["revision"].endswith(digest))
        tab["stdlib"] = "another stdlib"
        metadata["manifests"][0]["annotations"]["sh.brew.tab"] = json.dumps(tab)
        manifest.write_text(json.dumps(metadata))
        with self.assertRaises(RuntimeError):
            homebrew_origin(source, "arm64")
        tab["stdlib"] = "libc++"
        tab["unknown-build-field"] = "not ignored"
        metadata["manifests"][0]["annotations"]["sh.brew.tab"] = json.dumps(tab)
        manifest.write_text(json.dumps(metadata))
        with self.assertRaises(RuntimeError):
            homebrew_origin(source, "arm64")

    def post_pour_fixture(self, suffix="updated", architecture="arm64"):
        source, keg, archive, manifest, metadata, digest = self.cached_bottle(architecture, "zstd-" + suffix)
        native, dependency = self.keg("xz-" + suffix)
        native.write_bytes(binary(arch=architecture))
        file = dependency / "INSTALL_RECEIPT.json"
        value = json.loads(file.read_bytes())
        value["arch"] = architecture
        file.write_text(json.dumps(value))
        installed = dict(full_name=dependency.parent.name, version="1.2", revision=1,
                         pkg_version="1.2_1", bottle_rebuild=2, declared_directly=True)
        receipt = json.loads((keg / "INSTALL_RECEIPT.json").read_bytes())
        receipt["runtime_dependencies"] = [installed]
        (keg / "INSTALL_RECEIPT.json").write_text(json.dumps(receipt))
        tab = json.loads(metadata["manifests"][0]["annotations"]["sh.brew.tab"])
        tab["runtime_dependencies"] = [dict(installed, version="1.1", pkg_version="1.1_1", bottle_rebuild=1)]
        metadata["manifests"][0]["annotations"]["sh.brew.tab"] = json.dumps(tab)
        manifest.write_text(json.dumps(metadata))
        return source, keg, dependency, manifest, metadata, digest

    def test_post_pour_runtime_versions_are_verified_against_actual_selected_dependency_kegs(self):
        source, _, _, _, _, digest = self.post_pour_fixture()
        self.assertTrue(homebrew_origin(source, "arm64")[0]["revision"].endswith(digest))

    def test_post_pour_recomputed_names_and_declarations_bind_actual_kegs_not_old_parent_tables(self):
        for architecture in ("arm64", "x86_64"):
            for mutation in ("addition", "replacement", "declaration", "new-direct"):
                with self.subTest(architecture=architecture, mutation=mutation):
                    suffix = architecture + mutation
                    source, keg, _, manifest, _, digest = self.post_pour_fixture(suffix, architecture)
                    native, added = self.keg("json-c-" + suffix)
                    native.write_bytes(binary(arch=architecture))
                    file = added / "INSTALL_RECEIPT.json"
                    value = json.loads(file.read_bytes())
                    value["arch"] = architecture
                    file.write_text(json.dumps(value))
                    file = keg / "INSTALL_RECEIPT.json"
                    value = json.loads(file.read_bytes())
                    entry = dict(full_name=added.parent.name, version="1.2", revision=1,
                                 pkg_version="1.2_1", declared_directly=mutation in ("replacement", "new-direct"))
                    if mutation == "replacement":
                        value["runtime_dependencies"] = [entry]
                    elif mutation == "declaration":
                        value["runtime_dependencies"][0]["declared_directly"] = False
                    else:
                        value["runtime_dependencies"].append(entry)
                    file.write_text(json.dumps(value))
                    before = manifest.read_bytes()
                    self.assertTrue(homebrew_origin(source, architecture)[0]["revision"].endswith(digest))
                    self.assertEqual(before, manifest.read_bytes())

    def test_build_field_diagnostics_identify_each_candidate_descriptor(self):
        source, _, _, manifest, metadata, _ = self.cached_bottle()
        descriptor = metadata["manifests"][0]
        tab = json.loads(descriptor["annotations"]["sh.brew.tab"])
        tab["homebrew_version"] = "another-build"
        descriptor["annotations"]["sh.brew.tab"] = json.dumps(tab)
        other = json.loads(json.dumps(descriptor))
        other["annotations"]["org.opencontainers.image.ref.name"] = "1.2_1.arm64_sequoia"
        metadata["manifests"].append(other)
        manifest.write_text(json.dumps(metadata))
        with self.assertRaises(RuntimeError) as error:
            homebrew_origin(source, "arm64")
        self.assertIn("1.2_1.arm64_tahoe: homebrew_version", str(error.exception))
        self.assertIn("1.2_1.arm64_sequoia: homebrew_version", str(error.exception))

    def test_post_pour_dependency_changes_do_not_allow_unknown_or_inconsistent_inputs(self):
        for mutation in ("name", "missing", "source-built", "architecture", "version", "duplicate",
                         "pkg-version", "declaration", "unknown-field", "linked-keg", "malformed-source",
                         "negative-rebuild", "bool-revision", "compatibility"):
            with self.subTest(mutation=mutation):
                source, keg, dependency, _, _, _ = self.post_pour_fixture(mutation)
                file = keg / "INSTALL_RECEIPT.json"
                receipt = json.loads(file.read_bytes())
                dep = receipt["runtime_dependencies"][0]
                dep_file = dependency / "INSTALL_RECEIPT.json"
                dep_receipt = json.loads(dep_file.read_bytes())
                if mutation == "name":
                    dep["full_name"] = "another-package"
                elif mutation == "missing":
                    dep_file.unlink()
                elif mutation == "source-built":
                    dep_receipt["poured_from_bottle"] = False
                elif mutation == "architecture":
                    dep_receipt["arch"] = "x86_64"
                elif mutation == "version":
                    dep_receipt["source"]["versions"]["stable"] = "9.9"
                elif mutation == "duplicate":
                    receipt["runtime_dependencies"].append(dep)
                elif mutation == "pkg-version":
                    dep["pkg_version"] = "1.2_9"
                elif mutation == "declaration":
                    dep["declared_directly"] = "not-a-boolean"
                elif mutation == "linked-keg":
                    moved = self.root / "external dependency"
                    dependency.rename(moved)
                    dependency.symlink_to(moved, target_is_directory=True)
                elif mutation == "malformed-source":
                    dep_receipt["source"] = []
                elif mutation == "negative-rebuild":
                    dep["bottle_rebuild"] = -1
                elif mutation == "bool-revision":
                    dep["revision"] = True
                elif mutation == "compatibility":
                    dep["compatibility_version"] = 3
                else:
                    dep["unknown-field"] = "unverified"
                file.write_text(json.dumps(receipt))
                if mutation != "missing":
                    dep_file.write_text(json.dumps(dep_receipt))
                with self.assertRaises((RuntimeError, ValueError, OSError)):
                    homebrew_origin(source, "arm64")

    def test_cached_bottle_rebuild_names_bind_index_descriptor_and_archive(self):
        for architecture, rebuild in (("arm64", 1), ("x86_64", 2)):
            with self.subTest(architecture=architecture, rebuild=rebuild):
                source, _, archive, manifest, metadata, digest = self.cached_bottle(architecture, "rebuilt-" + architecture)
                metadata["annotations"]["org.opencontainers.image.ref.name"] += "-" + str(rebuild)
                metadata["manifests"][0]["annotations"]["org.opencontainers.image.ref.name"] += "." + str(rebuild)
                target = manifest.with_name(manifest.name.replace(".bottle_manifest", "-{}.bottle_manifest".format(rebuild)))
                manifest.rename(target)
                target.write_text(json.dumps(metadata))
                archive.rename(archive.with_name(archive.name.replace(".bottle.tar", ".bottle.{}.tar".format(rebuild))))
                self.assertTrue(homebrew_origin(source, architecture)[0]["revision"].endswith(digest))
                metadata["manifests"][0]["annotations"]["org.opencontainers.image.ref.name"] += ".99"
                target.write_text(json.dumps(metadata))
                with self.assertRaises(RuntimeError):
                    homebrew_origin(source, architecture)

    def test_other_cached_pkg_versions_do_not_create_false_rebuild_ambiguity(self):
        source, _, _, manifest, _, digest = self.cached_bottle()
        shutil.copy2(manifest, manifest.with_name(manifest.name.replace("1.2_1", "1.2_10")))
        self.assertTrue(homebrew_origin(source, "arm64")[0]["revision"].endswith(digest))

    def test_multiple_matching_bottle_rebuilds_remain_ambiguous(self):
        source, _, archive, manifest, metadata, _ = self.cached_bottle()
        metadata["annotations"]["org.opencontainers.image.ref.name"] += "-1"
        metadata["manifests"][0]["annotations"]["org.opencontainers.image.ref.name"] += ".1"
        manifest.with_name(manifest.name.replace(".bottle_manifest", "-1.bottle_manifest")).write_text(json.dumps(metadata))
        shutil.copy2(archive, archive.with_name(archive.name.replace(".bottle.tar", ".bottle.1.tar")))
        with self.assertRaises(RuntimeError):
            homebrew_origin(source, "arm64")

    def test_cached_bottle_provenance_fails_closed_on_missing_ambiguous_or_changed_inputs(self):
        for index, mutation in enumerate(("missing-archive", "tampered-archive", "linked-archive", "duplicate-archive",
                                          "duplicate-manifest", "wrong-architecture", "wrong-os", "wrong-version",
                                          "missing-tab", "changed-build", "partial-tab", "bad-digest", "bad-size",
                                          "missing-manifest", "linked-manifest", "invalid-manifest-name", "duplicate-descriptor",
                                          "wrong-cache-context", "source-built", "index-type", "tab-type",
                                          *("field-" + key for key in ("homebrew_version", "changed_files",
                                            "source_modified_time", "compiler", "runtime_dependencies", "arch")))):
            with self.subTest(mutation=mutation):
                source, _, archive, manifest, metadata, _ = self.cached_bottle(name="jpeg-" + str(index))
                descriptor = metadata["manifests"][0]
                if mutation == "missing-archive":
                    archive.unlink()
                elif mutation == "tampered-archive":
                    archive.write_bytes(archive.read_bytes() + b"changed")
                elif mutation == "linked-archive":
                    target = self.root / "external input"
                    archive.rename(target)
                    archive.symlink_to(target)
                elif mutation == "duplicate-archive":
                    shutil.copy2(archive, archive.with_name("c" * 64 + archive.name[64:]))
                elif mutation == "duplicate-manifest":
                    shutil.copy2(manifest, manifest.with_name("c" * 64 + manifest.name[64:]))
                elif mutation == "invalid-manifest-name":
                    target = manifest.with_name("invalid" + manifest.name[64:])
                    manifest.rename(target)
                    manifest = target
                elif mutation == "duplicate-descriptor":
                    metadata["manifests"].append(descriptor)
                elif mutation == "wrong-architecture":
                    descriptor["platform"]["architecture"] = "amd64"
                elif mutation == "wrong-os":
                    descriptor["platform"]["os"] = "linux"
                elif mutation == "wrong-version":
                    metadata["annotations"]["org.opencontainers.image.version"] = "today's version"
                elif mutation == "missing-tab":
                    descriptor["annotations"].pop("sh.brew.tab")
                elif mutation in ("changed-build", "partial-tab"):
                    tab = json.loads(descriptor["annotations"]["sh.brew.tab"])
                    if mutation == "changed-build":
                        tab["built_on"]["xcode"] = "other build"
                    else:
                        tab.pop("runtime_dependencies")
                    descriptor["annotations"]["sh.brew.tab"] = json.dumps(tab)
                elif mutation == "bad-digest":
                    descriptor["annotations"]["sh.brew.bottle.digest"] = "not-a-sha256"
                elif mutation == "bad-size":
                    descriptor["annotations"]["sh.brew.bottle.size"] = "999999999999"
                elif mutation in ("wrong-cache-context", "source-built"):
                    receipt_file = source.parent / "INSTALL_RECEIPT.json"
                    receipt = json.loads(receipt_file.read_bytes())
                    if mutation == "wrong-cache-context":
                        receipt["source"]["path"] = "/today/api/formula.json"
                    else:
                        receipt["poured_from_bottle"] = False
                    receipt_file.write_text(json.dumps(receipt))
                elif mutation == "tab-type":
                    descriptor["annotations"]["sh.brew.tab"] = {}
                elif mutation.startswith("field-"):
                    tab = json.loads(descriptor["annotations"]["sh.brew.tab"])
                    tab[mutation.removeprefix("field-")] = "different original build"
                    descriptor["annotations"]["sh.brew.tab"] = json.dumps(tab)
                if mutation == "index-type":
                    metadata = []
                manifest.write_text(json.dumps(metadata))
                if mutation == "missing-manifest":
                    manifest.unlink()
                elif mutation == "linked-manifest":
                    target = self.root / "external manifest"
                    manifest.rename(target)
                    manifest.symlink_to(target)
                with self.assertRaises((RuntimeError, ValueError)):
                    homebrew_origin(source, "arm64")

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

    def test_final_bundle_origins_are_captured_before_deployment_and_sealed_outside_after_signing(self):
        source = (ROOT / "scripts/macos-install/createBundle.sh").read_text()
        prepare = source.index('"$script_dir/bundleNativeOrigins.py" prepare')
        finalize = source.index('"$script_dir/bundleNativeOrigins.py" finalize')
        self.assertLess(prepare, source.index('ditto "$prefix/bin/librecad"'))
        self.assertLess(source.index('"$codesign_bin" --verify --deep --strict "$bundle"'), finalize)
        self.assertIn('final_origins="$bundle.native-origins.json"', source)
        self.assertIn('--origin-capture "$origin_capture" "$origin_staged"', source)

    def bundle_capture(self, dependency=None):
        ledger, old_report, _, installed = self.capture()
        app = self.payload / "bin/librecad"
        app.parent.mkdir()
        installed.rename(app)
        ledger.files["bin/librecad"] = dict(ledger.files.pop(installed.name), path="bin/librecad")
        old_report.unlink()
        report = self.payload / "share/librecad/licenses/providers/native-origins.json"
        report.parent.mkdir(parents=True)
        report.write_bytes(origins.canonical_json(ledger.seal()))
        _, keg = self.keg("qtbase")
        libraries, plugins = keg / "lib", keg / "plugins"
        plugin = plugins / "platforms/libqoffscreen.dylib"
        plugin.parent.mkdir(parents=True)
        plugin.write_bytes(binary())
        framework = libraries / "QtCore.framework/Versions/A/QtCore"
        framework.parent.mkdir(parents=True)
        framework.write_bytes(binary())
        contents = self.root / "LibreCAD.app/Contents"
        contents.mkdir(parents=True)
        capture = self.root / "before.json"
        def loads(stager, binary_path):
            return [str(dependency)] if dependency is not None and binary_path == framework else []
        with patch.object(RuntimeStager, "_loads", loads):
            bundle_origins.prepare(self.payload, contents, libraries, plugins, capture)
        return contents, capture, app, framework

    def test_predeployment_captures_transitive_original_from_resolved_load_not_copy_basename(self):
        original, _ = self.keg("brotli")
        soname = original.parent / "libbrotli.1.dylib"
        soname.symlink_to(original.name)
        contents, capture, _, _ = self.bundle_capture(soname)
        ledger = origins.read_origin_capture(contents, capture)
        entry = ledger.files["Frameworks/libbrotli.1.dylib"]
        self.assertEqual(origins.binary_fingerprint(original), entry["original"])
        self.assertEqual(original.name, entry["source_path"])
        self.assertEqual("brotli", ledger.providers[entry["provider"]]["name"])

    def test_bundle_carries_originals_and_snapshots_qt_before_later_copy_rewrites(self):
        contents, capture, app, framework = self.bundle_capture()
        state = origins.read_origin_capture(contents, capture)
        self.assertEqual(3, len(state.files))
        self.assertEqual({"MacOS/librecad"}, state.required)
        carried = state.files["MacOS/librecad"]
        source = json.loads((self.payload / "share/librecad/licenses/providers/native-origins.json").read_bytes())
        self.assertEqual(source["files"][0]["original"], carried["original"])
        self.assertEqual(source["providers"], {carried["provider"]: state.providers[carried["provider"]]})
        self.assertEqual(origins.binary_fingerprint(framework), state.files["Frameworks/QtCore.framework/Versions/A/QtCore"]["original"])
        with self.assertRaisesRegex(ValueError, "coverage differs|empty"):
            origins.audit_native_origins(contents, capture)
        for relative, original in (("MacOS/librecad", app), ("Frameworks/QtCore.framework/Versions/A/QtCore", framework)):
            destination = contents / relative
            destination.parent.mkdir(parents=True)
            shutil.copy2(original, destination)
            destination.write_bytes(destination.read_bytes() + b"deployment/signature")
        # Optional Qt plugin candidate is deliberately not deployed.
        output = self.root / "after.json"
        stager = RuntimeStager(contents, self.sdk, contents / "Frameworks", "otool", "install_name_tool", "codesign",
                               contents / "MacOS/librecad", False, [], True, origin_capture=(capture, output))
        self.assertEqual(2, len(stager.origins.files))
        def command(args):
            self.assertEqual(2, len(stager.origins.files))
            if args[0] == "install_name_tool" or "--sign" in args:
                path = Path(args[-1])
                path.write_bytes(path.read_bytes() + b"stager rewrite/signature")
            return ""
        with patch.object(stager, "_framework_binaries", return_value=[contents / "Frameworks/QtCore.framework/Versions/A/QtCore"]), \
                patch.object(stager, "_loads", return_value=[]), patch.object(stager, "_rpaths", return_value=[]), \
                patch.object(stager, "_run", side_effect=command):
            stager.stage()
        self.assertEqual(2, len(origins.read_origin_capture(contents, output).files))
        # Final deep signing is another change, not the staged checkpoint hash.
        final_app = contents / "MacOS/librecad"
        final_app.write_bytes(final_app.read_bytes() + b"final deep signature")
        final = self.root / "LibreCAD.app.native-origins.json"
        bundle_origins.finalize(contents, output, final)
        self.assertEqual((2, 2), origins.audit_native_origins(contents, final))
        self.assertEqual(0, origins.main([str(contents), str(final)]))
        self.assertEqual(origins.binary_fingerprint(final_app), json.loads(final.read_bytes())["files"][1]["final"])
        with self.assertRaisesRegex(ValueError, "fresh and outside"):
            bundle_origins.finalize(contents, output, final)
        with self.assertRaisesRegex(ValueError, "fresh and outside"):
            bundle_origins.finalize(contents, output, contents / "embedded.json")
        final_app.write_bytes(final_app.read_bytes() + b"tampered after final seal")
        self.assertEqual(1, origins.main([str(contents), str(final)]))

    def test_bundle_unknown_native_copy_fails_before_any_stager_command(self):
        contents, capture, app, _ = self.bundle_capture()
        (contents / "unknown.native").write_bytes(binary())
        with patch.object(RuntimeStager, "_run") as command:
            with self.assertRaisesRegex(RuntimeError, "unknown deployed native origin"):
                RuntimeStager(contents, self.sdk, contents, "otool", "install_name_tool", "codesign",
                              None, False, [], False, origin_capture=(capture, self.root / "after.json"))
            command.assert_not_called()

    def test_optional_sdk_pruning_must_not_hide_a_missing_required_prefix_native_copy(self):
        contents, capture, _, framework = self.bundle_capture()
        destination = contents / "Frameworks/QtCore.framework/Versions/A/QtCore"
        destination.parent.mkdir(parents=True)
        shutil.copy2(framework, destination)
        with patch.object(RuntimeStager, "_run") as command:
            with self.assertRaisesRegex(RuntimeError, "required installed native copy"):
                RuntimeStager(contents, self.sdk, contents / "Frameworks", "otool", "install_name_tool", "codesign",
                              None, False, [], False, origin_capture=(capture, self.root / "after.json"))
            command.assert_not_called()

    def test_original_capture_is_distinct_strict_and_not_final_acceptance(self):
        ledger, final, _, _ = self.capture()
        capture = self.root / "capture.json"
        capture.write_bytes(origins.canonical_json(ledger.capture_state()))
        self.assertEqual(ledger.capture_state(), origins.read_origin_capture(self.payload, capture).capture_state())
        with self.assertRaisesRegex(ValueError, "malformed or is a final-byte|invalid required paths"):
            origins.read_origin_capture(self.payload, final)
        with self.assertRaisesRegex(ValueError, "final native bytes"):
            origins.audit_native_origins(self.payload, capture)
        state = ledger.capture_state()
        state["extra"] = "unbound"
        capture.write_bytes(origins.canonical_json(state))
        with self.assertRaisesRegex(ValueError, "malformed"):
            origins.read_origin_capture(self.payload, capture)
        state.pop("extra")
        state["required"] = ["not-in-capture"]
        capture.write_bytes(origins.canonical_json(state))
        with self.assertRaisesRegex(ValueError, "invalid required paths"):
            origins.read_origin_capture(self.payload, capture)

    def test_both_mac_lanes_retain_and_audit_final_report_with_archive(self):
        source = (ROOT / ".github/workflows/python-binding-macos.yml").read_text()
        self.assertIn("-DLIBRECAD_RECORD_NATIVE_ORIGINS=ON", source)
        self.assertIn('-DLIBRECAD_NATIVE_ORIGIN_PROJECT_REVISION="$(git rev-parse HEAD)"', source)
        self.assertIn('LibreCAD.app LibreCAD.app.native-origins.json', source)
        self.assertEqual(3, source.count('/lcUIPy/_nativeorigins.py"'))
        self.assertIn('--evidence build/macos-native-origins.log', source)


if __name__ == "__main__":
    unittest.main()
