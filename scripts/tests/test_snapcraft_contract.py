"""Static safeguards for the amd64-only Python-enabled Snap configuration."""

import re
import unittest
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]


class SnapcraftContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.manifest = (REPOSITORY_ROOT / "snapcraft.yaml").read_text(encoding="utf-8")

    def test_core26_snap_declares_only_amd64_build_and_runtime(self):
        self.assertRegex(self.manifest, r"(?m)^base:\s*core26$")
        self.assertRegex(
            self.manifest,
            r"(?ms)^platforms:\n\s+amd64:\n\s+build-on:\s+\[amd64\]\n\s+build-for:\s+\[amd64\]$",
        )

    def test_runtime_library_path_uses_selected_architecture_triplet(self):
        self.assertRegex(
            self.manifest,
            r"(?m)^\s+LD_LIBRARY_PATH:.*\$SNAP/usr/lib/\$CRAFT_ARCH_TRIPLET_BUILD_FOR(?:[:/]|$)",
        )
        self.assertNotIn("$SNAP/usr/lib/x86_64-linux-gnu", self.manifest)

    def test_snap_stages_system_python_runtime(self):
        stage_packages = re.search(
            r"(?ms)^    stage-packages:\n(.*?)(?=^    [A-Za-z0-9_-]+:)",
            self.manifest,
        )
        self.assertIsNotNone(stage_packages)
        self.assertRegex(stage_packages.group(1), r"(?m)^      - python3$")

    def test_libdxfrw_part_uses_the_pinned_patched_provider(self):
        provider = re.search(
            r"(?ms)^  libdxfrw:\n(.*?)(?=^  [A-Za-z0-9_-]+:|\Z)", self.manifest
        )
        self.assertIsNotNone(provider)
        provider = provider.group(1)
        self.assertIn("source: https://github.com/LibreCAD/libdxfrw.git", provider)
        self.assertIn("source-type: git", provider)
        revision = re.search(r"(?m)^    source-commit: ([0-9a-f]{40})$", provider)
        self.assertIsNotNone(revision)
        patch_helper = (REPOSITORY_ROOT / "scripts/patch-libdxfrw.sh").read_text(
            encoding="utf-8"
        )
        self.assertIn(f"expected_revision={revision.group(1)}", patch_helper)
        build_override = provider.split("override-build: |", 1)[1]
        patch_command = (
            'bash "$CRAFT_PROJECT_DIR/scripts/patch-libdxfrw.sh" "$CRAFT_PART_SRC"'
        )
        self.assertIn(patch_command, build_override)
        self.assertIn("craftctl default", build_override)
        self.assertLess(
            build_override.index(patch_command), build_override.index("craftctl default")
        )
        self.assertIn('"-DCMAKE_INSTALL_PREFIX=/usr"', provider)
        self.assertIn('"-DBUILD_SHARED_LIBS=ON"', provider)
        self.assertIn('"-DWITH_BUNDLED_PYTHON_RUNTIME=ON"', self.manifest)
        self.assertNotIn("source: persistence/libdxfrw", provider)

    def test_librecad_part_builds_after_libdxfrw_provider(self):
        parts = self.manifest.split("\nparts:\n", 1)[1]
        application = re.search(
            r"(?ms)^  librecad:\n(.*?)(?=^  [A-Za-z0-9_-]+:|\Z)", parts
        )
        self.assertIsNotNone(application)
        self.assertRegex(application.group(1), r"(?m)^    after:\s*\[libdxfrw\]$")

    def test_pack_script_uses_explicit_snapcraft_command(self):
        script = (REPOSITORY_ROOT / "scripts/ubuntu-install/createSnap.sh").read_text(
            encoding="utf-8"
        )
        self.assertIn("snapcraft pack --destructive-mode --output librecad.snap", script)
        self.assertIn("sudo snap install ./librecad.snap --devmode --dangerous", script)

    def test_installed_snap_smoke_requires_libpython_from_the_package(self):
        workflow = (REPOSITORY_ROOT / ".github/workflows/main.yml").read_text(
            encoding="utf-8"
        )
        start = workflow.index("      - name: Smoke installed snap Python runtime")
        end = workflow.find("\n      - name:", start + 1)
        step = workflow[start : end if end >= 0 else len(workflow)]
        self.assertIn("LIBRECAD_REQUIRE_BUNDLED_PYTHON=1", step)
        self.assertIn("snap run librecad --python-smoke", step)

    def test_loaded_libpython_diagnostic_is_package_generic(self):
        smoke_source = (REPOSITORY_ROOT / "lcUI/python/pyruntimesmoke.cpp").read_text(
            encoding="utf-8"
        )
        strict_check = smoke_source.split(
            'if os.environ.get("LIBRECAD_REQUIRE_BUNDLED_PYTHON") == "1":', 1
        )[1].split("import lcgui", 1)[0]
        self.assertIn("_package_usr", strict_check)
        self.assertIn("packaged usr tree", strict_check)
        self.assertNotIn("AppDir", strict_check)

    def test_staged_python_path_maps_to_snap_usr_tree(self):
        smoke_source = (REPOSITORY_ROOT / "lcUI/python/pyruntimesmoke.cpp").read_text(
            encoding="utf-8"
        )
        strict_check = smoke_source.split(
            'if os.environ.get("LIBRECAD_REQUIRE_BUNDLED_PYTHON") == "1":', 1
        )[1].split("import lcgui", 1)[0]
        self.assertIn(".resolve().parents[2]", strict_check)
        staged_python = Path("/snap/librecad/current/usr/share/librecad/python")
        self.assertEqual(Path("/snap/librecad/current/usr"), staged_python.parents[2])


if __name__ == "__main__":
    unittest.main()
