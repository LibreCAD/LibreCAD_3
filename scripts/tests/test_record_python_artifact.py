"""Exact artifact/runtime/evidence identity, not native qualification."""

import hashlib
import json
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "scripts/record_python_artifact.py"


class PythonArtifactIdentityTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name).resolve()
        self.artifact = self.root / "LibreCAD package Ω.bin"
        self.artifact.write_bytes(b"exact inner artifact\x00\r\n")
        self.inventory = self.root / "python-runtime-inventory.txt"
        self.evidence = self.root / "installed-smoke.log"
        self.evidence.write_bytes(b"retained smoke output\r\n")
        self.receipt = self.root / "identity.json"
        self.write_inventory()

    def write_inventory(self, architecture="x86_64", **overrides):
        fields = dict(provider="CMake FindPython / Python::Python", version="3.14.8",
                      provider_implementation="CPython", provider_version="3.14.8",
                      provider_architecture=architecture, provider_pointer_bits="64",
                      provider_debug="false", provider_gil_disabled="false",
                      executable="/selected/bin/python3.14", include_dirs="/selected/include/python3.14",
                      link_library="/selected/lib/libpython3.14.so", prefix="/selected",
                      stdlib="/selected/lib/python3.14", stdarch="/selected/lib/python3.14",
                      dynamic_modules="/selected/lib/python3.14/lib-dynload")
        fields.update(overrides)
        self.inventory.write_text("".join(key + "=" + value + "\n" for key, value in fields.items()))

    def run_tool(self, *extra, success=True, platform="appimage-x86_64"):
        result = subprocess.run(
            [sys.executable, "-B", str(SCRIPT), "--artifact", str(self.artifact),
             "--inventory", str(self.inventory), "--evidence", str(self.evidence),
             "--revision", "a" * 40, "--platform", platform, *map(str, extra)],
            capture_output=True, text=True, timeout=30)
        if success:
            self.assertEqual(0, result.returncode, result.stdout + result.stderr)
        else:
            self.assertNotEqual(0, result.returncode, result.stdout + result.stderr)
        return result

    def test_exact_inner_artifact_inventory_and_evidence_hashes_are_deterministic(self):
        self.run_tool("--output", self.receipt)
        first = self.receipt.read_bytes()
        record = json.loads(first)
        self.assertEqual(1, record["format"])
        self.assertEqual("a" * 40, record["revision"])
        self.assertEqual("appimage-x86_64", record["platform"])
        for field, file in (("artifact", self.artifact), ("inventory", self.inventory)):
            self.assertEqual(file.name, record[field]["name"])
            self.assertEqual(file.stat().st_size, record[field]["size_bytes"])
            self.assertEqual(hashlib.sha256(file.read_bytes()).hexdigest(), record[field]["sha256"])
        self.assertEqual(hashlib.sha256(self.evidence.read_bytes()).hexdigest(),
                         record["evidence"][0]["sha256"])
        self.assertEqual("/selected/lib/libpython3.14.so", record["selected_python"]["link_library"])
        self.assertNotIn("qualified", record)
        self.run_tool("--output", self.receipt)
        self.assertEqual(first, self.receipt.read_bytes())
        self.run_tool("--verify", self.receipt)

    def test_every_supported_artifact_architecture_and_alias(self):
        for platform, architecture in (("appimage-x86_64", "x86_64"), ("snap-amd64", "amd64"),
                                       ("windows-x64", "AMD64"), ("macos-arm64", "aarch64"),
                                       ("macos-x86_64", "x86_64")):
            with self.subTest(platform=platform):
                self.write_inventory(architecture)
                self.run_tool("--output", self.receipt, platform=platform)
                self.run_tool("--verify", self.receipt, platform=platform)

    def test_missing_empty_and_linked_inputs_fail_without_writing_a_receipt(self):
        for field in ("artifact", "inventory", "evidence"):
            file = getattr(self, field)
            original = file.read_bytes()
            for kind in ("missing", "empty", "symlink"):
                with self.subTest(field=field, kind=kind):
                    file.unlink()
                    if kind == "empty":
                        file.write_bytes(b"")
                    elif kind == "symlink":
                        outside = self.root / "alias-target"
                        outside.write_bytes(original)
                        file.symlink_to(outside)
                    self.run_tool("--output", self.receipt, success=False)
                    self.assertFalse(self.receipt.exists())
                    if file.exists() or file.is_symlink():
                        file.unlink()
                    file.write_bytes(original)

    def test_missing_malformed_duplicate_and_incompatible_provider_metadata_fail(self):
        for kind in ("missing", "malformed", "duplicate", "architecture", "version", "debug", "gil"):
            with self.subTest(kind=kind):
                self.write_inventory()
                text = self.inventory.read_text()
                if kind == "missing":
                    text = text.replace("stdlib=/selected/lib/python3.14\n", "")
                elif kind == "malformed":
                    text += "not a field\n"
                elif kind == "duplicate":
                    text += "provider_version=3.13.0\n"
                elif kind == "architecture":
                    text = text.replace("provider_architecture=x86_64", "provider_architecture=arm64")
                elif kind == "version":
                    text = text.replace("provider_version=3.14.8", "provider_version=3.13.0")
                elif kind == "debug":
                    text = text.replace("provider_debug=false", "provider_debug=true")
                else:
                    text = text.replace("provider_gil_disabled=false", "provider_gil_disabled=true")
                self.inventory.write_text(text)
                self.run_tool("--output", self.receipt, success=False)
                self.assertFalse(self.receipt.exists())

    def test_verification_rejects_changed_inputs_or_receipt(self):
        for field in ("artifact", "inventory", "evidence", "receipt"):
            with self.subTest(field=field):
                self.run_tool("--output", self.receipt)
                file = getattr(self, field)
                before = file.read_bytes()
                if field == "receipt":
                    record = json.loads(before)
                    record["revision"] = "b" * 40
                    file.write_text(json.dumps(record))
                elif field == "inventory":
                    file.write_bytes(before.replace(b"/selected", b"/different"))
                else:
                    file.write_bytes(before + b"\nmodified bytes\n")
                self.run_tool("--verify", self.receipt, success=False)
                file.write_bytes(before)

    def test_output_cannot_overwrite_an_input_or_alias_it(self):
        for file in (self.artifact, self.inventory, self.evidence):
            before = file.read_bytes()
            self.run_tool("--output", file, success=False)
            self.assertEqual(before, file.read_bytes())
        self.receipt.symlink_to(self.artifact)
        self.run_tool("--output", self.receipt, success=False)
        self.assertEqual(b"exact inner artifact\x00\r\n", self.artifact.read_bytes())
        self.receipt.unlink()
        os.link(self.artifact, self.receipt)
        self.run_tool("--output", self.receipt, success=False)
        self.assertEqual(b"exact inner artifact\x00\r\n", self.artifact.read_bytes())

    def test_ambiguous_evidence_names_and_duplicate_files_fail(self):
        other = self.root / "other"
        other.mkdir()
        collision = other / self.evidence.name
        collision.write_bytes(b"different output")
        for file in (self.evidence, collision):
            self.run_tool("--evidence", file, "--output", self.receipt, success=False)
        alias = other / "distinct-name.log"
        os.link(self.evidence, alias)
        self.run_tool("--evidence", alias, "--output", self.receipt, success=False)
        self.assertFalse(self.receipt.exists())

    def test_short_revision_and_unknown_platform_fail(self):
        self.run_tool("--revision", "abcdef", "--output", self.receipt, success=False)
        self.run_tool("--output", self.receipt, success=False, platform="windows-arm64")

    def test_workflows_record_and_retain_identities_after_required_smokes(self):
        linux = (ROOT / ".github/workflows/main.yml").read_text()
        mac = (ROOT / ".github/workflows/python-binding-macos.yml").read_text()
        for platform, report, gate in (
                ("appimage-x86_64", "appimage-artifact-identity.json", "Smoke AppImage without host Python"),
                ("snap-amd64", "snap-artifact-identity.json", "Smoke installed snap Python runtime"),
                ("windows-x64", "windows-artifact-identity.json", "Install and smoke the isolated Python runtime")):
            with self.subTest(platform=platform):
                start = linux.index("- name: " + gate)
                record = linux.index("--platform " + platform, start)
                self.assertLess(start, record)
                self.assertGreaterEqual(linux.count("build/" + report), 2)
                self.assertIn("scripts/record_python_artifact.py", linux[start:record])
        start = mac.index("- name: Archive the verified app and runtime identity")
        self.assertLess(mac.index('"$extracted/LibreCAD.app/Contents/MacOS/librecad" --python-smoke', start),
                        mac.index('scripts/record_python_artifact.py', start))
        self.assertIn("--platform macos-${{ matrix.arch }}", mac)
        self.assertGreaterEqual(mac.count("build/macos-artifact-identity.json"), 2)
        # v4 uploads are immutable: Linux/Windows must neither collide on
        # upload nor merge and overwrite their same-named provider inventory.
        self.assertIn("name: assets-linux-x86_64", linux)
        self.assertIn("name: assets-windows-x64", linux)
        self.assertNotIn("name: assets\n", linux)
        publisher = linux[linux.index("  UpdateAssets:"):]
        self.assertIn("pattern: assets-*", publisher)
        self.assertIn("path: release-artifacts", publisher)
        self.assertNotIn("merge-multiple: true", publisher)
        self.assertIn("release-artifacts/assets-linux-x86_64/LibreCAD*.AppImage", publisher)
        self.assertIn("release-artifacts/assets-windows-x64/bin/LibreCAD3*.exe", publisher)


if __name__ == "__main__":
    unittest.main()
