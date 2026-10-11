"""Guard the Snap package's Qt plugin and Python smoke contract."""

import re
import unittest
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
SNAPCRAFT = REPOSITORY_ROOT / "snapcraft.yaml"
SNAP_SCRIPT = REPOSITORY_ROOT / "scripts/ubuntu-install/createSnap.sh"
WORKFLOW = REPOSITORY_ROOT / ".github/workflows/main.yml"


class SnapRuntimeContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.snapcraft = SNAPCRAFT.read_text(encoding="utf-8")
        cls.snap_script = SNAP_SCRIPT.read_text(encoding="utf-8")
        cls.workflow = WORKFLOW.read_text(encoding="utf-8")

    def test_stages_qt_plugins_and_resolves_them_from_inside_snap(self):
        self.assertRegex(self.snapcraft, r"(?m)^base: core26$")
        for declaration in (
            "  QT_PLUGIN_PATH: $SNAP/usr/lib/x86_64-linux-gnu/qt6/plugins",
            "  QT_QPA_PLATFORM_PLUGIN_PATH: $SNAP/usr/lib/x86_64-linux-gnu/qt6/plugins/platforms",
        ):
            with self.subTest(declaration=declaration):
                self.assertIn(declaration, self.snapcraft)

        for package in ("libqt6gui6", "qt6-qpa-plugins", "qt6-svg-plugins"):
            with self.subTest(package=package):
                self.assertRegex(
                    self.snapcraft,
                    r"(?m)^      - {}$".format(re.escape(package)),
                )

    def test_ci_packs_installs_and_smokes_the_same_snap(self):
        script = self.snap_script
        pack = script.index("snapcraft pack --destructive-mode --output librecad.snap")
        install = script.index("snap install ./librecad.snap --devmode --dangerous")
        self.assertLess(pack, install)

        create_snap = self.workflow.index("scripts/ubuntu-install/createSnap.sh")
        smoke = self.workflow.index("snap run librecad --python-smoke")
        self.assertLess(create_snap, smoke)
        self.assertIn("QT_QPA_PLATFORM=xcb", self.workflow[smoke - 400 : smoke])

    def test_installed_smoke_requires_packaged_libpython(self):
        smoke = self.workflow.index("snap run librecad --python-smoke")
        step_start = self.workflow.rfind("- name:", 0, smoke)
        self.assertIn("LIBRECAD_REQUIRE_BUNDLED_PYTHON=1", self.workflow[step_start:smoke])

    def test_libpython_guard_uses_package_generic_linux_root(self):
        smoke_source = (REPOSITORY_ROOT / "lcUI/python/pyruntimesmoke.cpp").read_text(
            encoding="utf-8"
        )
        strict_check = smoke_source.split(
            'if os.environ.get("LIBRECAD_REQUIRE_BUNDLED_PYTHON") == "1":', 1
        )[1].split("import lcgui", 1)[0]
        self.assertIn('if sys.platform.startswith("linux"):', strict_check)
        self.assertIn("_package_usr", strict_check)
        self.assertIn("packaged usr tree", strict_check)
        self.assertNotIn("AppDir", strict_check)

    def test_staged_python_root_maps_to_snap_usr(self):
        staged_python = Path("/snap/librecad/current/usr/share/librecad/python")
        self.assertEqual(Path("/snap/librecad/current/usr"), staged_python.parents[2])

    def test_temporary_apt_source_refuses_overwrite_and_is_removed(self):
        guard = self.snap_script.index('if sudo test -e "$plain_amd64_sources"; then')
        cleanup = self.snap_script.index("trap cleanup_plain_amd64_sources EXIT")
        source_write = self.snap_script.index('sudo tee "$plain_amd64_sources"')

        self.assertLess(guard, cleanup)
        self.assertLess(cleanup, source_write)
        self.assertIn('sudo rm -f "$plain_amd64_sources"', self.snap_script)

    def test_workflow_refreshes_existing_snapcraft_in_latest_stable_channel(self):
        setup_start = self.workflow.index("name: Install or refresh Snapcraft (latest/stable)")
        create_snap = self.workflow.index("scripts/ubuntu-install/createSnap.sh")
        setup = self.workflow[setup_start:create_snap]

        self.assertLess(setup_start, create_snap)
        self.assertIn("sudo snap list snapcraft", setup)
        self.assertIn("sudo snap refresh snapcraft --channel=latest/stable", setup)
        self.assertIn(
            "sudo snap install snapcraft --classic --channel=latest/stable",
            setup,
        )
        self.assertIn("snapcraft --version", setup)
        self.assertNotIn("samuelmeuli/action-snapcraft", setup)


if __name__ == "__main__":
    unittest.main()
