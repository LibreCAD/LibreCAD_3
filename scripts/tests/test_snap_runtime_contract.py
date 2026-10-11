"""Guard the Snap package's Qt plugin and Python smoke contract."""

import re
import os
import shutil
import subprocess
import sys
import tempfile
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

    def test_declares_the_only_supported_snap_platform(self):
        self.assertRegex(
            self.snapcraft,
            r"(?m)^platforms:\n  amd64:\n    build-on: \[amd64\]\n    build-for: \[amd64\]\n",
        )

    def test_stages_non_graphics_dependencies_reported_by_the_elf_linter(self):
        stage_packages = self.snapcraft.split("    stage-packages:", 1)[1].split(
            "    cmake-parameters:", 1
        )[0]
        for package in ("libgdbm6t64", "libxml2-16"):
            with self.subTest(package=package):
                self.assertRegex(stage_packages, r"(?m)^      - {}$".format(package))

    def test_retains_and_validates_the_snap_specific_provider(self):
        pack = self.snap_script.index("snapcraft pack --destructive-mode --output librecad.snap")
        inventory = self.snap_script.index("snap_inventory=parts/librecad/build/python-runtime-inventory.txt")
        validate = self.snap_script.index("python3 scripts/validate_python_inventory.py")
        install = self.snap_script.index("snap install ./librecad.snap --devmode --dangerous")
        self.assertLess(pack, inventory)
        self.assertLess(inventory, validate)
        self.assertLess(validate, install)
        for contract in ('sudo test -s "$snap_inventory"',
                         'sudo cat "$snap_inventory" | tee build/snap-python-runtime-inventory.txt',
                         '--expected-root /usr --expected-version 3.14 --expected-architecture x86_64',
                         'snap list core26 mesa-2604 snapcraft'):
            self.assertIn(contract, self.snap_script)
        upload = self.workflow.split("name: Uploading snap", 1)[1].split("  linux-arm64:", 1)[0]
        self.assertIn("build/snap-python-runtime-inventory.txt", upload)
        self.assertIn("build/snap-runtime-identity.txt", upload)

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


@unittest.skipUnless(os.name == "posix" and shutil.which("bash"), "Bash fixture needs POSIX")
class SnapInventoryRetentionTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.inventory = self.root / "parts/librecad/build/python-runtime-inventory.txt"
        self.inventory.parent.mkdir(parents=True)
        (self.root / "scripts").symlink_to(REPOSITORY_ROOT / "scripts", target_is_directory=True)
        self.commands = self.root / "commands"
        self.commands.mkdir()
        (self.commands / "python3").symlink_to(sys.executable)
        for name, body in (
            ("sudo", 'exec "$@"'),
            ("snap", 'printf "%s\\n" "$*" >> "$SNAP_TEST_CALLS"; printf "snap-fixture\\n"'),
            ("sha256sum", 'printf "artifact-digest  librecad.snap\\n"'),
        ):
            command = self.commands / name
            command.write_text("#!/bin/sh\nset -eu\n" + body + "\n", encoding="utf-8")
            command.chmod(0o755)
        self.calls = self.root / "snap-calls.txt"
        self.block = SNAP_SCRIPT.read_text(encoding="utf-8").split(
            "snap_inventory=", 1
        )[1]
        self.block = "snap_inventory=" + self.block

    def provider(self, **overrides):
        fields = dict(
            provider_implementation="CPython", provider_version="3.14.4",
            provider_architecture="x86_64", provider_pointer_bits="64",
            provider_debug="false", provider_gil_disabled="false",
            executable="/usr/bin/python3.14", include_dirs="/usr/include/python3.14",
            link_library="/usr/lib/x86_64-linux-gnu/libpython3.14.so", prefix="/usr",
            stdlib="/usr/lib/python3.14", stdarch="/usr/lib/python3.14",
            dynamic_modules="/usr/lib/python3.14/lib-dynload",
        )
        fields.update(overrides)
        text = "".join(f"{key}={value}\n" for key, value in fields.items())
        self.inventory.write_text(text, encoding="utf-8")
        return text

    def run_block(self):
        return subprocess.run(
            [shutil.which("bash"), "-euo", "pipefail", "-c", self.block], cwd=self.root,
            env=dict(os.environ, PATH=str(self.commands) + os.pathsep + os.environ["PATH"],
                     SNAP_TEST_CALLS=str(self.calls)), capture_output=True, text=True,
            check=False,
        )

    def test_retains_exact_snap_inventory_and_records_runtime_identity(self):
        inventory = self.provider()
        result = self.run_block()
        self.assertEqual(0, result.returncode, result.stdout + result.stderr)
        self.assertEqual(inventory, (self.root / "build/snap-python-runtime-inventory.txt").read_text())
        self.assertIn("artifact-digest", (self.root / "build/snap-runtime-identity.txt").read_text())
        self.assertEqual(["list core26 mesa-2604 snapcraft", "install ./librecad.snap --devmode --dangerous"],
                         self.calls.read_text().splitlines())

    def test_missing_snap_inventory_cannot_fall_back_to_system_build(self):
        (self.root / "build").mkdir()
        (self.root / "build/python-runtime-inventory.txt").write_text("not-the-snap-provider\n")
        self.assertNotEqual(0, self.run_block().returncode)
        self.assertFalse(self.calls.exists())

    def test_mixed_provider_stops_before_snap_install(self):
        self.provider(link_library="/outside/libpython3.14.so")
        result = self.run_block()
        self.assertEqual(1, result.returncode)
        self.assertIn("outside the selected provider root", result.stderr)
        self.assertFalse(self.calls.exists())


if __name__ == "__main__":
    unittest.main()
