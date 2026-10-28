"""Exercise the clean-runtime gate without requiring Docker for regressions."""

import os
import re
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
        self.assertIn("ARG RUNTIME_BASE=ubuntu:24.04", dockerfile)
        self.assertIn("FROM ${RUNTIME_BASE}", dockerfile)
        self.assertIn("USER 65534:65534", dockerfile)
        packages = dockerfile.split("apt-get install", 1)[1].split("&& rm", 1)[0]
        self.assertNotIn("python", packages)
        self.assertNotIn("-dev ", packages)
        workflow = (ROOT / ".github/workflows/main.yml").read_text(encoding="utf-8")
        for required in ("Smoke AppImage without host Python", "--network none --read-only",
                         'docker pull --platform linux/amd64 "$APPIMAGE_RUNTIME_BASE"',
                         '--build-arg "RUNTIME_BASE=$runtime_base"',
                         "--cap-drop ALL --security-opt no-new-privileges",
                         '"$appimage:/artifact/LibreCAD.AppImage:ro"',
                         "build/appimage-clean-runtime.log", "build/appimage-runtime-container.txt"):
            self.assertIn(required, workflow)

    def test_appimage_is_built_gated_and_smoked_on_its_declared_floor(self):
        # Built on the newest runner, the AppImage needed glibc 2.43 and an
        # x86-64-v3 CPU, and passed every check because every check ran there.
        workflow = (ROOT / ".github/workflows/main.yml").read_text(encoding="utf-8")
        appimage, linux = job(workflow, "appimage"), job(workflow, "linux")
        self.assertIn("runs-on: ubuntu-24.04\n", appimage)
        base = re.search(r"\n      APPIMAGE_RUNTIME_BASE: (\S+)\n", appimage).group(1)
        dockerfile = (SCRIPT.parent / "appimage-runtime.Dockerfile").read_text(encoding="utf-8")
        self.assertEqual("ubuntu:24.04", base)
        self.assertIn("ARG RUNTIME_BASE={}\n".format(base), dockerfile)
        for floor in ("APPIMAGE_GLIBC_FLOOR: '2.39'", "APPIMAGE_GLIBCXX_FLOOR: 3.4.33",
                      "APPIMAGE_CXXABI_FLOOR: 1.3.15", "APPIMAGE_GCC_FLOOR: 14.0.0",
                      "APPIMAGE_ISA_FLOOR: x86-64-baseline", "APPIMAGE_PYTHON_VERSION: '3.12'",
                      "APPIMAGE_NEWEST_RUNTIME_BASE: ubuntu:26.04"):
            self.assertIn(floor, appimage)

        ordered = ("Refuse an apt that installs an x86-64 micro-architecture variant",
                   "installDependenciesAndBuildRepo.sh",
                   "- name: Verify selected Linux Python runtime inventory", "createAppImage.sh",
                   "scripts/check_elf_floor.py", "- name: Smoke final AppImage Python runtime",
                   "- name: Smoke AppImage without host Python",
                   "- name: Smoke AppImage on the newest runtime", "scripts/record_python_artifact.py",
                   "name: assets-linux-x86_64")
        positions = [appimage.index(step) for step in ordered]
        self.assertEqual(sorted(positions), positions)
        # The bundled CPython is the floor system's, not whichever the runner offers.
        self.assertIn('--expected-root /usr --expected-version "$APPIMAGE_PYTHON_VERSION" '
                      '--expected-architecture x86_64', appimage)
        gate = appimage[appimage.index("- name: Check AppImage ELF floor"):
                        appimage.index("- name: Smoke final AppImage Python runtime")]
        for required in ('"$appimage" --appimage-extract', '"$extracted/squashfs-root" "$appimage"',
                         '--max "GLIBC_${APPIMAGE_GLIBC_FLOOR}"',
                         '--max "GLIBCXX_${APPIMAGE_GLIBCXX_FLOOR}"',
                         '--max "CXXABI_${APPIMAGE_CXXABI_FLOOR}"', '--max "GCC_${APPIMAGE_GCC_FLOOR}"',
                         '--max-isa "$APPIMAGE_ISA_FLOOR"', "--report build/appimage-elf-floor.txt"):
            self.assertIn(required, gate)
        smoke = appimage[appimage.index("- name: Smoke AppImage without host Python"):
                         appimage.index("- name: Smoke AppImage on the newest runtime")]
        for required in ('test "$runtime_glibc" = "glibc $APPIMAGE_GLIBC_FLOOR"',
                         "libstdc++.so.6.0.${APPIMAGE_GLIBCXX_FLOOR#3.4.}",
                         '-qF "CXXABI_${APPIMAGE_CXXABI_FLOOR}" /usr/lib/x86_64-linux-gnu/libstdc++.so.6',
                         '-qF "GCC_${APPIMAGE_GCC_FLOOR}" /usr/lib/x86_64-linux-gnu/libgcc_s.so.1'):
            self.assertIn(required, smoke)
        newest = appimage[appimage.index("- name: Smoke AppImage on the newest runtime"):
                          appimage.index("- name: Record AppImage artifact identity")]
        for required in ('docker pull --platform linux/amd64 "$APPIMAGE_NEWEST_RUNTIME_BASE"',
                         "-t librecad-appimage-runtime-newest", "--network none --read-only",
                         "build/appimage-newest-runtime.log"):
            self.assertIn(required, newest)
        for evidence in ("appimage-elf-floor.txt", "appimage-newest-runtime.log",
                         "appimage-newest-runtime-container.txt"):
            self.assertIn("--evidence build/" + evidence, appimage)
            self.assertIn("${{ github.workspace }}/build/" + evidence, appimage)

        # The snap stays on 26.04 with the native build; nothing AppImage does.
        self.assertIn("runs-on: ubuntu-26.04\n", linux)
        self.assertIn("createSnap.sh", linux)
        for absent in ("createAppImage.sh", "assets-linux-x86_64", "check_elf_floor.py"):
            self.assertNotIn(absent, linux)
        publisher = job(workflow, "UpdateAssets")
        self.assertRegex(publisher, r"needs: \[[^\]]*\bappimage\b[^\]]*\]")


def job(workflow, job_id):
    """The text of one top-level job, up to the next job key."""
    start = workflow.index("\n  {}:\n".format(job_id))
    following = re.search(r"\n  [A-Za-z][A-Za-z0-9_-]*:\n", workflow[start + 1:])
    return workflow[start:start + 1 + following.start()] if following else workflow[start:]


if __name__ == "__main__":
    unittest.main()
