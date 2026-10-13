"""Exercise the CI bottle refresh without touching the host Homebrew SDK."""

import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]
INSTALLER = ROOT / "scripts/macos-install/installDependencies.sh"
ROOTS = "cmake boost eigen freetype glew glfw glm googletest lua@5.4 rapidjson qt python@3.14".split()


class MacOSDependencyInstallTests(unittest.TestCase):
    def run_installer(self, dependencies="zstd\nmpdecimal\nfreetype\nqtbase\nzstd\n", failure="", ci=True):
        with tempfile.TemporaryDirectory(prefix="LibreCAD bottles Ω ") as directory:
            temporary = Path(directory)
            log = temporary / "brew.jsonl"
            brew = temporary / "brew"
            brew.write_text("#!" + sys.executable + "\n" + '''
import json, os, sys
from pathlib import Path
args = sys.argv[1:]
with Path(os.environ["BREW_TEST_LOG"]).open("a") as stream:
    stream.write(json.dumps(dict(args=args, env={key: os.environ.get(key) for key in (
        "HOMEBREW_NO_INSTALL_CLEANUP", "HOMEBREW_NO_AUTO_UPDATE",
        "HOMEBREW_NO_INSTALLED_DEPENDENTS_CHECK")})) + "\\n")
if args[0] == os.environ["BREW_TEST_FAILURE"]:
    sys.exit(93)
if args[0] == "deps":
    sys.stdout.write(os.environ["BREW_TEST_DEPENDENCIES"])
elif args[0] != "reinstall" or "--force-bottle" not in args:
    sys.exit("unexpected command or dependency source-build risk")
''')
            brew.chmod(0o755)
            environment = dict(os.environ, PATH=str(temporary) + os.pathsep + os.environ["PATH"],
                               BREW_TEST_LOG=str(log), BREW_TEST_DEPENDENCIES=dependencies,
                               BREW_TEST_FAILURE=failure, GITHUB_ACTIONS="true" if ci else "false",
                               HOMEBREW_NO_INSTALL_CLEANUP="",
                               HOMEBREW_NO_AUTO_UPDATE="", HOMEBREW_NO_INSTALLED_DEPENDENTS_CHECK="")
            result = subprocess.run(["bash", str(INSTALLER)], env=environment,
                                    text=True, capture_output=True, timeout=15)
            calls = [json.loads(line) for line in log.read_text().splitlines()] if log.exists() else []
            return result, calls

    def test_refreshes_preinstalled_runtime_closure_and_all_roots_from_bottles(self):
        result, calls = self.run_installer()
        self.assertEqual(0, result.returncode, result.stderr)
        self.assertEqual(["deps", "--union", "--topological", "--os=macos", *ROOTS], calls[0]["args"])
        self.assertEqual(["reinstall", "--formula", "--force-bottle", "zstd", "mpdecimal", "freetype", "qtbase",
                          *(name for name in ROOTS if name != "freetype")], calls[1]["args"])
        self.assertEqual(2, len(calls))

    def test_retains_inputs_and_freezes_metadata_without_upgrading_unrelated_dependents(self):
        result, calls = self.run_installer()
        self.assertEqual(0, result.returncode, result.stderr)
        for call in calls:
            self.assertEqual({"HOMEBREW_NO_INSTALL_CLEANUP": "1", "HOMEBREW_NO_AUTO_UPDATE": "1",
                              "HOMEBREW_NO_INSTALLED_DEPENDENTS_CHECK": "1"}, call["env"])

    def test_dependency_discovery_failure_prevents_installation(self):
        result, calls = self.run_installer(failure="deps")
        self.assertEqual(93, result.returncode, result.stderr)
        self.assertEqual(["deps"], [call["args"][0] for call in calls])

    def test_missing_bottle_or_refresh_failure_propagates_without_source_fallback(self):
        result, calls = self.run_installer(failure="reinstall")
        self.assertEqual(93, result.returncode, result.stderr)
        self.assertEqual(["deps", "reinstall"], [call["args"][0] for call in calls])
        self.assertIn("--force-bottle", calls[1]["args"])

    def test_empty_dependency_union_still_refreshes_every_root(self):
        result, calls = self.run_installer(dependencies="")
        self.assertEqual(0, result.returncode, result.stderr)
        self.assertEqual(["reinstall", "--formula", "--force-bottle", *ROOTS], calls[1]["args"])

    def test_refuses_to_mutate_a_non_ci_homebrew_installation(self):
        result, calls = self.run_installer(ci=False)
        self.assertEqual(2, result.returncode, result.stderr)
        self.assertIn("CI-only", result.stderr)
        self.assertEqual([], calls)

    def test_both_native_workflow_architectures_use_the_bottle_refresh(self):
        workflow = (ROOT / ".github/workflows/python-binding-macos.yml").read_text()
        install = workflow.split("      - name: Install native dependencies\n", 1)[1].split("      - name:", 1)[0]
        self.assertIn("bash scripts/macos-install/installDependencies.sh", install)
        self.assertNotIn("brew install ", install)
        for runner in ("macos-15", "macos-15-intel"):
            self.assertIn("runner: " + runner, workflow)


if __name__ == "__main__":
    unittest.main()
