"""Exercise the clean-runtime gate without requiring Docker for regressions."""

import os
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "scripts/ubuntu-install/smokeAppImageRuntime.sh"


@unittest.skipUnless(os.name == "posix" and shutil.which("bash"), "Bash fixture needs POSIX")
class CleanAppImageRuntimeTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.commands = self.root / "bin"
        self.commands.mkdir()
        for name in ("cp", "chmod", "mktemp", "rm", "grep", "env"):
            (self.commands / name).symlink_to(shutil.which(name))
        self.stub("uname", "printf 'x86_64\\n'")
        self.stub("getconf", "printf 'glibc 2.43\\n'")
        self.stub("ldconfig", "printf 'libc.so.6\\n'")
        self.stub("dpkg-query", "printf 'libc6:amd64\\n'")
        self.stub("sha256sum", "printf 'fixture-digest\\n'")
        self.stub("timeout", 'test "$1" = --kill-after=10s; test "$2" = 180s; shift 2; exec "$@"')
        self.stub("xvfb-run", 'test "$1" = -a; shift; exec "$@"')
        self.artifact = self.root / "LibreCAD artifact Ω.AppImage"
        self.artifact.write_text(
            '#!/bin/sh\nset -eu\n'
            'test "$1" = --appimage-extract-and-run\n'
            'test "$2" = --python-smoke\n'
            'test "$PYTHONHOME" = /__librecad_python_poison__\n'
            'test "$PYTHONPATH" = /__librecad_python_poison__\n'
            'test "$LIBRECAD_REQUIRE_BUNDLED_PYTHON" = 1\n'
            'test "$LIBRECAD_REQUIRE_PYTHON_EXTENSION_CLOSURE" = 1\n'
            'test "$LIBRECAD_REQUIRE_FULL_ELF_CLOSURE" = 1\n'
            'test "$QT_QPA_PLATFORM" = xcb\n'
            'test -z "${QT_PLUGIN_PATH:-}${QT_QPA_PLATFORM_PLUGIN_PATH:-}${LD_LIBRARY_PATH:-}${LD_PRELOAD:-}"\n'
            'printf "fixture launcher passed\\n"\n', encoding="utf-8",
        )

    def stub(self, name, body):
        path = self.commands / name
        path.write_text("#!/bin/sh\nset -eu\n" + body + "\n", encoding="utf-8")
        path.chmod(0o755)

    def run_smoke(self, *arguments):
        environment = dict(os.environ, PATH=str(self.commands),
                           QT_PLUGIN_PATH="/external", QT_QPA_PLATFORM_PLUGIN_PATH="/external",
                           LD_LIBRARY_PATH="/external", LD_PRELOAD="/external")
        return subprocess.run([shutil.which("bash"), str(SCRIPT), *map(str, arguments)],
                              env=environment, text=True, capture_output=True, check=False)

    def test_isolated_launcher_and_poisoned_environment(self):
        result = self.run_smoke(self.artifact)
        self.assertEqual(0, result.returncode, result.stdout + result.stderr)
        self.assertIn("no system Python executable, library or SDK", result.stdout)
        self.assertIn("fixture launcher passed", result.stdout)

    def test_system_python_executable_is_rejected(self):
        self.stub("python3", "exit 0")
        result = self.run_smoke(self.artifact)
        self.assertEqual(1, result.returncode)
        self.assertIn("system Python executable", result.stderr)

    def test_system_python_library_is_rejected(self):
        self.stub("ldconfig", "printf 'libpython3.14.so.1.0\\n'")
        result = self.run_smoke(self.artifact)
        self.assertEqual(1, result.returncode)
        self.assertIn("system libpython", result.stderr)

    def test_python_sdk_is_rejected(self):
        self.stub("dpkg-query", "printf 'libpython3.14-dev:amd64\\n'")
        result = self.run_smoke(self.artifact)
        self.assertEqual(1, result.returncode)
        self.assertIn("Python SDK", result.stderr)

    def test_launcher_failure_is_not_masked(self):
        self.artifact.write_text("#!/bin/sh\nexit 7\n", encoding="utf-8")
        self.assertEqual(7, self.run_smoke(self.artifact).returncode)

    def test_launcher_timeout_is_not_masked(self):
        self.stub("timeout", "exit 124")
        self.assertEqual(124, self.run_smoke(self.artifact).returncode)

    def test_missing_argument_or_artifact_fails(self):
        self.assertEqual(2, self.run_smoke().returncode)
        self.assertEqual(1, self.run_smoke(self.root / "missing").returncode)

    def test_image_and_workflow_enforce_runtime_only_isolation(self):
        dockerfile = (SCRIPT.parent / "appimage-runtime.Dockerfile").read_text(encoding="utf-8")
        self.assertIn("ARG RUNTIME_BASE=ubuntu:26.04", dockerfile)
        self.assertIn("FROM ${RUNTIME_BASE}", dockerfile)
        self.assertIn("USER 65534:65534", dockerfile)
        packages = dockerfile.split("apt-get install", 1)[1].split("&& rm", 1)[0]
        self.assertNotIn("python", packages)
        self.assertNotIn("-dev ", packages)
        workflow = (ROOT / ".github/workflows/main.yml").read_text(encoding="utf-8")
        for required in ("Smoke AppImage without host Python", "--network none --read-only",
                         'docker pull --platform linux/amd64 ubuntu:26.04',
                         '--build-arg "RUNTIME_BASE=$runtime_base"',
                         "--cap-drop ALL --security-opt no-new-privileges",
                         '"$appimage:/artifact/LibreCAD.AppImage:ro"',
                         "build/appimage-clean-runtime.log", "build/appimage-runtime-container.txt"):
            self.assertIn(required, workflow)


if __name__ == "__main__":
    unittest.main()
