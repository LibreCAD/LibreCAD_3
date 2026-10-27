"""Regression tests for the Python-binding plan checker."""

import io
import sys
import subprocess
import tempfile
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest.mock import patch


SCRIPT_DIRECTORY = Path(__file__).resolve().parents[1]
REPOSITORY_ROOT = SCRIPT_DIRECTORY.parent
sys.path.insert(0, str(SCRIPT_DIRECTORY))

import validate_python_binding_plan as plan_checker  # noqa: E402


PLAN_PATH = REPOSITORY_ROOT / plan_checker.PLAN_RELATIVE_PATH


def replace_latest_receipt(text, gates=None, add_plan_note=None):
    lines = text.splitlines()
    for index, line in enumerate(lines):
        if plan_checker._cells(line)[:4] != [
            "Review date",
            "Affected gates",
            "Plan change",
            "Validation scope",
        ]:
            continue
        for row_index in range(index + 1, len(lines)):
            cells = plan_checker._cells(lines[row_index])
            if not cells or all(set(cell) <= {"-", ":", " "} for cell in cells):
                continue
            parts = lines[row_index].split("|")
            if gates is not None:
                parts[2] = " {} ".format(gates)
            if add_plan_note:
                parts[3] = " {} {} ".format(parts[3].strip(), add_plan_note)
            lines[row_index] = "|".join(parts)
            return "\n".join(lines) + "\n"
    raise AssertionError("could not find a review-log receipt")


def replace_gate_evidence(text, gate_id, evidence, state="validated"):
    lines = text.splitlines()
    for index, line in enumerate(lines):
        cells = plan_checker._cells(line)
        if len(cells) < 4 or cells[0] != gate_id:
            continue
        parts = line.split("|")
        parts[2] = " {} ".format(state)
        parts[4] = " {} ".format(evidence)
        lines[index] = "|".join(parts)
        return "\n".join(lines) + "\n"
    raise AssertionError("could not find gate {}".format(gate_id))


class PythonBindingPlanValidationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.plan = PLAN_PATH.read_text(encoding="utf-8")

    def test_current_plan_passes_full_validation(self):
        self.assertEqual([], plan_checker.validate_plan_structure(self.plan, REPOSITORY_ROOT))

    def test_self_update_and_validation_sections_are_required(self):
        required_sections = (
            "### Self-update triggers",
            "#### Operational self-update and validation",
            "#### Validation layers and fail-closed rule",
        )
        for heading in required_sections:
            with self.subTest(heading=heading):
                changed = self.plan.replace(heading, "#### Removed section", 1)
                errors = plan_checker.validate_plan_structure(changed, REPOSITORY_ROOT)
                self.assertIn("missing required plan heading: {}".format(heading), errors)

    def test_self_update_contract_requires_operational_invariants(self):
        start = self.plan.index("## 6. Plan self-updating and validation")
        end = self.plan.index("## 7.", start)
        section = self.plan[start:end]
        for marker in plan_checker.REQUIRED_SYNC_CONTRACT_MARKERS:
            with self.subTest(marker=marker):
                changed = self.plan[:start] + section.replace(marker, "[removed]") + self.plan[end:]
                errors = plan_checker.validate_plan_structure(changed, REPOSITORY_ROOT)
                self.assertIn(
                    "plan self-update contract is missing required detail: {}".format(marker),
                    errors,
                )

    def test_operational_self_update_checkpoints_are_required(self):
        required_moments = (
            "Resume or start a batch",
            "After each meaningful implementation/test/CI result or blocker",
            "PR-ready handoff",
        )
        for moment in required_moments:
            with self.subTest(moment=moment):
                changed = "\n".join(
                    line
                    for line in self.plan.splitlines()
                    if not (
                        plan_checker._cells(line)
                        and plan_checker._cells(line)[0].startswith(moment)
                    )
                )
                errors = plan_checker.validate_plan_structure(changed, REPOSITORY_ROOT)
                self.assertIn(
                    "operational self-update table is missing checkpoint: {}".format(
                        moment.lower()
                    ),
                    errors,
                )

    def test_step_summary_reports_sync_details_and_escapes_paths(self):
        summary = plan_checker.render_step_summary(
            "fail",
            "base ref main",
            ["scripts/<img src=x>.py"],
            {"PLAN-SYNC"},
            {"unexpected<&.txt"},
            True,
            ["S2"],
            ["unmapped path requires an exemption"],
        )

        self.assertIn("## Python binding plan sync: FAIL", summary)
        self.assertIn("Changed paths:", summary)
        self.assertIn("scripts/&lt;img src=x&gt;.py", summary)
        self.assertNotIn("<img src=x>", summary)
        self.assertIn("Mapped gates: PLAN-SYNC", summary)
        self.assertIn("Unmapped paths: unexpected&lt;&amp;.txt", summary)
        self.assertIn("Stale validated evidence: S2", summary)
        self.assertIn("unmapped path requires an exemption", summary)

    def test_step_summary_appends_to_existing_file(self):
        with tempfile.TemporaryDirectory() as temporary_directory:
            summary_file = Path(temporary_directory) / "summary.md"
            summary_file.write_text("existing summary\n", encoding="utf-8")
            plan_checker.append_step_summary(
                str(summary_file),
                "pass",
                "base ref main",
                ["PYTHON_BINDING_IMPLEMENTATION_PLAN.md"],
                {"PLAN-SYNC"},
                set(),
                True,
                [],
                [],
            )

            output = summary_file.read_text(encoding="utf-8")
            self.assertTrue(output.startswith("existing summary\n"))
            self.assertIn("Python binding plan sync: PASS", output)
            self.assertIn("Plan changed: yes", output)

    def test_duplicate_gate_id_is_rejected(self):
        duplicate = "| S0 | planned | duplicate fixture | Not run |\n"
        changed = self.plan.replace("| S1 | active |", duplicate + "| S1 | active |", 1)
        errors = plan_checker.validate_plan_structure(changed, REPOSITORY_ROOT)
        self.assertTrue(any("duplicate required gate row: S0" in error for error in errors))

    def test_unknown_state_is_rejected(self):
        changed = self.plan.replace("| S0 | active |", "| S0 | finished |", 1)
        errors = plan_checker.validate_plan_structure(changed, REPOSITORY_ROOT)
        self.assertTrue(any("unknown state 'finished'" in error for error in errors))

    def test_missing_platform_row_is_rejected(self):
        changed = "\n".join(
            line for line in self.plan.splitlines() if not line.startswith("| MAC-X64 |")
        )
        errors = plan_checker.validate_plan_structure(changed, REPOSITORY_ROOT)
        self.assertTrue(any("missing required platform row: MAC-X64" in error for error in errors))

    def test_validated_gate_requires_complete_evidence(self):
        changed = self.plan.replace("| S0 | active |", "| S0 | validated |", 1)
        errors = plan_checker.validate_plan_structure(changed, REPOSITORY_ROOT)
        self.assertTrue(any("validated gate S0 lacks evidence fields" in error for error in errors))

    def test_zero_discovered_or_executed_checks_cannot_validate(self):
        evidence = (
            "date=2026-10-09; revision=abc123; environment=macOS arm64; "
            "command=ctest; counts=0/0/0; skips=none; result=pass; "
            "report=https://example.test/report; blocker=none"
        )
        changed = replace_gate_evidence(self.plan, "S0", evidence)
        errors = plan_checker.validate_plan_structure(changed, REPOSITORY_ROOT)
        self.assertTrue(any("zero discovered or executed checks" in error for error in errors))

    def test_required_skips_cannot_validate(self):
        evidence = (
            "date=2026-10-09; revision=abc123; environment=macOS arm64; "
            "command=ctest; counts=2/1/1; skips=required: TestFoo; result=pass; "
            "report=https://example.test/report; blocker=none"
        )
        changed = replace_gate_evidence(self.plan, "S0", evidence)
        errors = plan_checker.validate_plan_structure(changed, REPOSITORY_ROOT)
        self.assertTrue(any("not explicitly optional" in error for error in errors))

    def test_optional_skip_can_validate_with_complete_evidence(self):
        evidence = (
            "date=2026-10-09; revision=abc123; environment=macOS arm64; "
            "command=ctest; counts=2/1/1; skips=optional: TestFoo; result=pass; "
            "report=https://example.test/report; blocker=none"
        )
        changed = replace_gate_evidence(self.plan, "S0", evidence)
        self.assertEqual([], plan_checker.validate_plan_structure(changed, REPOSITORY_ROOT))

    def test_validated_platform_requires_artifact_digest_and_runtime(self):
        evidence = (
            "date=2026-10-09; revision=abc123; environment=macOS arm64; "
            "command=smoke; counts=1/1/0; skips=none; result=pass; "
            "report=https://example.test/report; blocker=none"
        )
        changed = replace_gate_evidence(self.plan, "MAC-X64", evidence)
        errors = plan_checker.validate_plan_structure(changed, REPOSITORY_ROOT)
        self.assertTrue(any("artifact_sha256" in error for error in errors))

        changed = replace_gate_evidence(
            self.plan,
            "MAC-X64",
            evidence + "; artifact_sha256=abc; runtime=Python 3.x",
        )
        errors = plan_checker.validate_plan_structure(changed, REPOSITORY_ROOT)
        self.assertTrue(any("64-digit artifact SHA-256" in error for error in errors))

    def test_base_ref_collects_tracked_worktree_changes(self):
        with tempfile.TemporaryDirectory() as temporary_directory:
            repository = Path(temporary_directory)
            subprocess.run(["git", "init", "-q"], cwd=repository, check=True)
            subprocess.run(
                ["git", "config", "user.email", "plan-check@example.test"],
                cwd=repository,
                check=True,
            )
            subprocess.run(
                ["git", "config", "user.name", "Plan Check Test"],
                cwd=repository,
                check=True,
            )
            tracked_file = repository / "tracked.py"
            tracked_file.write_text("before\n", encoding="utf-8")
            subprocess.run(["git", "add", "tracked.py"], cwd=repository, check=True)
            subprocess.run(["git", "commit", "-qm", "fixture base"], cwd=repository, check=True)
            tracked_file.write_text("after\n", encoding="utf-8")

            changed_paths = plan_checker._git_changed_paths("HEAD", repository)

        self.assertEqual(["tracked.py"], changed_paths)

    def test_worktree_inventory_unions_staged_unstaged_and_untracked_paths(self):
        with tempfile.TemporaryDirectory() as temporary_directory:
            repository = Path(temporary_directory)
            subprocess.run(["git", "init", "-q"], cwd=repository, check=True)
            subprocess.run(
                ["git", "config", "user.email", "plan-check@example.test"],
                cwd=repository,
                check=True,
            )
            subprocess.run(
                ["git", "config", "user.name", "Plan Check Test"],
                cwd=repository,
                check=True,
            )
            (repository / ".gitignore").write_text("ignored-output/\n", encoding="utf-8")
            staged = repository / "staged.py"
            unstaged = repository / "unstaged.py"
            staged.write_text("before\n", encoding="utf-8")
            unstaged.write_text("before\n", encoding="utf-8")
            subprocess.run(
                ["git", "add", ".gitignore", "staged.py", "unstaged.py"],
                cwd=repository,
                check=True,
            )
            subprocess.run(["git", "commit", "-qm", "fixture base"], cwd=repository, check=True)

            staged.write_text("staged change\n", encoding="utf-8")
            subprocess.run(["git", "add", "staged.py"], cwd=repository, check=True)
            unstaged.write_text("unstaged change\n", encoding="utf-8")
            new_staged = repository / "new-staged.py"
            new_staged.write_text("new staged\n", encoding="utf-8")
            subprocess.run(["git", "add", "new-staged.py"], cwd=repository, check=True)
            (repository / "untracked file.py").write_text("untracked\n", encoding="utf-8")
            ignored_directory = repository / "ignored-output"
            ignored_directory.mkdir()
            (ignored_directory / "generated.py").write_text("ignored\n", encoding="utf-8")

            changed_paths = plan_checker._git_worktree_changed_paths(repository)

        self.assertEqual(
            ["new-staged.py", "staged.py", "unstaged.py", "untracked file.py"],
            changed_paths,
        )

    def test_worktree_exclusion_is_exact_reported_and_must_match_candidates(self):
        candidates = ["src/main.cpp", "build/generated/app", "build/generated/lib", "build-notes.md"]
        remaining, excluded = plan_checker._exclude_candidate_paths(
            candidates, ["build/generated/"]
        )
        self.assertEqual(["src/main.cpp", "build-notes.md"], remaining)
        self.assertEqual(["build/generated/"], excluded)

        with self.assertRaisesRegex(RuntimeError, "matches no worktree candidates"):
            plan_checker._exclude_candidate_paths(candidates, ["missing/"])
        with self.assertRaisesRegex(RuntimeError, "repository-relative"):
            plan_checker._exclude_candidate_paths(candidates, ["../outside/"])

    def test_candidate_path_mode_accepts_plan_file_when_base_has_no_plan(self):
        output = io.StringIO()
        with patch.object(plan_checker, "_git_plan_at_ref", return_value=None):
            with redirect_stdout(output):
                result = plan_checker.main(["--paths", plan_checker.PLAN_RELATIVE_PATH])

        self.assertEqual(0, result)
        self.assertIn("Change source: explicit candidate paths", output.getvalue())
        self.assertIn(plan_checker.PLAN_RELATIVE_PATH, output.getvalue())

    def test_candidate_path_mode_requires_sync_for_untracked_code_with_spaces(self):
        candidate = "lcadpythonscript/new untracked binding.cpp"
        output = io.StringIO()
        with redirect_stdout(output):
            result = plan_checker.main(["--paths", candidate])

        self.assertEqual(1, result)
        self.assertIn(candidate, output.getvalue())
        self.assertIn("same-change plan update", output.getvalue())

    def test_candidate_path_mode_rejects_paths_outside_repository(self):
        error = io.StringIO()
        with redirect_stderr(error):
            result = plan_checker.main(["--paths", "../outside.cpp"])

        self.assertEqual(1, result)
        self.assertIn("must be repository-relative", error.getvalue())

    def test_worktree_exclusions_cannot_be_used_without_worktree_inventory(self):
        error = io.StringIO()
        with redirect_stderr(error):
            result = plan_checker.main(["--full", "--exclude-path", "build/"])

        self.assertEqual(1, result)
        self.assertIn("--exclude-path is valid only with --worktree", error.getvalue())

    def test_broken_local_link_is_rejected(self):
        changed = self.plan.replace("(doxygen/Scripting.md)", "(missing/Scripting.md)", 1)
        errors = plan_checker.validate_plan_structure(changed, REPOSITORY_ROOT)
        self.assertTrue(any("broken local link" in error for error in errors))

    def test_most_specific_path_rule_wins(self):
        rules, errors = plan_checker.parse_path_rules(self.plan)
        self.assertEqual([], errors)
        gates, unmapped = plan_checker.map_paths(
            ["scripts/validate_python_binding_plan.py"], rules
        )
        self.assertEqual(set(), unmapped)
        self.assertEqual({"PLAN-SYNC"}, gates)

    def test_mapped_change_without_plan_update_fails(self):
        errors, mapped, _ = plan_checker.validate_changed_paths(
            self.plan, ["lcadpythonscript/bridge/new_binding.cpp"], REPOSITORY_ROOT
        )
        self.assertIn("S2", mapped)
        self.assertTrue(any("same-change plan update" in error for error in errors))

    def test_receipt_must_name_every_mapped_gate(self):
        changed_plan = replace_latest_receipt(self.plan, gates="PLAN-SYNC")
        errors, mapped, _ = plan_checker.validate_changed_paths(
            changed_plan,
            ["lcadpythonscript/bridge/new_binding.cpp", plan_checker.PLAN_RELATIVE_PATH],
            REPOSITORY_ROOT,
        )
        self.assertIn("S2", mapped)
        self.assertTrue(any("newest plan receipt omits mapped gates" in error for error in errors))

    def test_same_change_requires_a_receipt_newer_than_the_comparison_base(self):
        errors, _, _ = plan_checker.validate_changed_paths(
            self.plan,
            [plan_checker.PLAN_RELATIVE_PATH],
            REPOSITORY_ROOT,
            baseline_text=self.plan,
        )

        self.assertTrue(
            any("receipt is unchanged from the comparison base" in error for error in errors)
        )

    def test_fresh_same_change_receipt_passes_comparison_base_check(self):
        changed_plan = replace_latest_receipt(
            self.plan, add_plan_note="Fresh receipt for comparison-base regression test."
        )
        errors, _, _ = plan_checker.validate_changed_paths(
            changed_plan,
            [plan_checker.PLAN_RELATIVE_PATH],
            REPOSITORY_ROOT,
            baseline_text=self.plan,
        )

        self.assertEqual([], errors)

    def test_changed_input_cannot_leave_gate_validated(self):
        changed_plan = self.plan.replace("| S2 | active |", "| S2 | validated |", 1)
        changed_plan = replace_latest_receipt(
            changed_plan,
            gates="S1, S2, S3, S4, S5, S6, S7, PLAN-SYNC, LINUX-X64, "
            "LINUX-ARM64, APPIMAGE-X64, SNAP-AMD64, WIN-X64, MAC-ARM64, MAC-X64",
        )
        errors, mapped, _ = plan_checker.validate_changed_paths(
            changed_plan,
            ["lcadpythonscript/bridge/changed.cpp", plan_checker.PLAN_RELATIVE_PATH],
            REPOSITORY_ROOT,
        )
        self.assertIn("S2", mapped)
        self.assertTrue(any("invalidate validated gates" in error for error in errors))

    def test_unmapped_path_requires_explicit_exact_exemption(self):
        errors, _, unmapped = plan_checker.validate_changed_paths(
            self.plan, ["research/one-off.txt"], REPOSITORY_ROOT
        )
        self.assertEqual({"research/one-off.txt"}, unmapped)
        self.assertTrue(any("unmapped paths need exact exemptions" in error for error in errors))

    def test_exact_exemption_must_be_cited_in_new_receipt(self):
        changed = self.plan.replace(
            "| Exact path | Rationale | Review date |\n|---|---|---|\n",
            "| Exact path | Rationale | Review date |\n|---|---|---|\n"
            "| `research/one-off.txt` | Historical note, outside implementation scope | 2026-10-09 |\n",
            1,
        )
        changed = replace_latest_receipt(changed, add_plan_note="Exemption reviewed.")
        errors, mapped, unmapped = plan_checker.validate_changed_paths(
            changed,
            ["research/one-off.txt", plan_checker.PLAN_RELATIVE_PATH],
            REPOSITORY_ROOT,
        )
        self.assertEqual({"research/one-off.txt"}, unmapped)
        self.assertIn("PLAN-SYNC", mapped)
        self.assertTrue(any("must be cited" in error for error in errors))

    def test_exact_exemption_citation_passes(self):
        changed = self.plan.replace(
            "| Exact path | Rationale | Review date |\n|---|---|---|\n",
            "| Exact path | Rationale | Review date |\n|---|---|---|\n"
            "| `research/one-off.txt` | Historical note, outside implementation scope | 2026-10-09 |\n",
            1,
        )
        changed = replace_latest_receipt(
            changed,
            gates="S2, MAC-ARM64, PLAN-SYNC",
            add_plan_note="Reviewed exemption research/one-off.txt.",
        )
        errors, _, _ = plan_checker.validate_changed_paths(
            changed,
            ["research/one-off.txt", plan_checker.PLAN_RELATIVE_PATH],
            REPOSITORY_ROOT,
        )
        self.assertEqual([], errors)


if __name__ == "__main__":
    unittest.main()
