"""Regression tests for the Python-binding plan checker."""

import io
import re
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


def prepend_review_receipt(text, receipt):
    lines = text.splitlines()
    for index, line in enumerate(lines):
        if plan_checker._cells(line)[:4] == [
            "Review date",
            "Affected gates",
            "Plan change",
            "Validation scope",
        ]:
            lines.insert(index + 2, receipt)
            return "\n".join(lines) + "\n"
    raise AssertionError("could not find a review-log receipt table")


def blank_table_cell(text, section_heading, next_heading, row_title, cell_index):
    lines = text.splitlines()
    start = next(
        index for index, line in enumerate(lines) if line.strip() == section_heading
    )
    end = next(
        index
        for index in range(start + 1, len(lines))
        if lines[index].strip() == next_heading
    )
    for index in range(start + 1, end):
        cells = plan_checker._cells(lines[index])
        if cells and cells[0] == row_title:
            parts = lines[index].split("|")
            parts[cell_index + 1] = " "
            lines[index] = "|".join(parts)
            return "\n".join(lines) + "\n"
    raise AssertionError("could not find row {!r}".format(row_title))


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
            "#### Automatic candidate discovery",
            "#### Plan-edit validation transaction",
            "#### Validation layers and fail-closed rule",
        )
        for heading in required_sections:
            with self.subTest(heading=heading):
                changed = self.plan.replace(heading, "#### Removed section", 1)
                errors = plan_checker.validate_plan_structure(changed, REPOSITORY_ROOT)
                self.assertIn("missing required plan heading: {}".format(heading), errors)

    def test_user_turn_boundary_sync_rule_is_required(self):
        start = self.plan.index("At each user-turn boundary,")
        end = self.plan.index("An explicit request to revise this plan", start)
        changed = self.plan[:start] + self.plan[end:]
        self.assertNotEqual(self.plan, changed)
        errors = plan_checker.validate_plan_structure(changed, REPOSITORY_ROOT)
        self.assertIn("maintenance protocol is missing the user-turn sync rule", errors)

    def test_user_turn_sync_requires_classification_and_receipt_before_resume(self):
        required_details = (
            "classify new input",
            "scope change, new evidence or blocker, or no change",
            "with a receipt before continuing implementation",
            "leaving incomplete evidence provisional",
            "no ledger churn",
        )
        start = self.plan.index("At each user-turn boundary,")
        end = self.plan.index("An explicit request to revise this plan", start)
        rule = self.plan[start:end]
        for detail in required_details:
            with self.subTest(detail=detail):
                changed = self.plan.replace(
                    rule,
                    re.sub(
                        re.escape(detail).replace(r"\ ", r"\s+"),
                        "removed requirement",
                        rule,
                        count=1,
                    ),
                    1,
                )
                errors = plan_checker.validate_plan_structure(changed, REPOSITORY_ROOT)
                self.assertIn(
                    "user-turn sync rule is missing required detail: {}".format(detail),
                    errors,
                )

    def test_operational_self_update_checkpoints_are_required(self):
        required_moments = (
            "At each user-turn boundary",
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

    def test_operational_checkpoints_require_update_and_validation_actions(self):
        checkpoints = (
            "At each user-turn boundary",
            "Resume or start a batch",
            "After each meaningful implementation/test/CI result or blocker",
            "PR-ready handoff",
        )
        for checkpoint in checkpoints:
            for field_index, field_name in ((1, "update"), (2, "validation")):
                with self.subTest(checkpoint=checkpoint, field=field_name):
                    changed = blank_table_cell(
                        self.plan,
                        "#### Operational self-update and validation",
                        "#### Automatic candidate discovery",
                        checkpoint,
                        field_index,
                    )
                    errors = plan_checker.validate_plan_structure(
                        changed, REPOSITORY_ROOT
                    )
                    self.assertIn(
                        "operational self-update checkpoint is missing required {}: {}".format(
                            field_name, checkpoint.lower()
                        ),
                        errors,
                    )

    def test_self_update_trigger_table_requires_each_trigger_category(self):
        required_categories = (
            "Target branch advances",
            "Source, API, build flags",
            "Test selection, fixtures, or CI workflow",
            "A test/CI result, failure, or blocker",
            "External evidence arrives",
            "A capability, platform, architecture, or runtime-support decision",
            "User scope, priority, or acceptance criteria change",
            "Plan, checker, checker tests, or plan workflow changes",
            "Plan-only edit",
        )
        for category in required_categories:
            with self.subTest(category=category):
                changed = "\n".join(
                    line
                    for line in self.plan.splitlines()
                    if not (
                        plan_checker._cells(line)
                        and plan_checker._cells(line)[0].startswith(category)
                    )
                )
                errors = plan_checker.validate_plan_structure(changed, REPOSITORY_ROOT)
                self.assertIn(
                    "self-update trigger table is missing required category: {}".format(
                        category.lower()
                    ),
                    errors,
                )

    def test_self_update_triggers_require_update_and_validation_actions(self):
        categories = (
            "Target branch advances, or work is rebased/ported to another base",
            "Source, API, build flags, dependency/runtime, or package layout changes",
            "Test selection, fixtures, or CI workflow changes",
            "A test/CI result, failure, or blocker arrives",
            "External evidence arrives (pasted logs, hosted CI, review feedback, package/artifact report)",
            "A capability, platform, architecture, or runtime-support decision changes",
            "User scope, priority, or acceptance criteria change",
            "Plan, checker, checker tests, or plan workflow changes",
            "Plan-only edit",
        )
        for category in categories:
            for field_index, field_name in ((1, "update"), (2, "validation")):
                with self.subTest(category=category, field=field_name):
                    changed = blank_table_cell(
                        self.plan,
                        "#### Self-update triggers",
                        "#### Plan-edit validation transaction",
                        category,
                        field_index,
                    )
                    errors = plan_checker.validate_plan_structure(
                        changed, REPOSITORY_ROOT
                    )
                    self.assertIn(
                        "self-update trigger is missing required {}: {}".format(
                            field_name, category.lower()
                        ),
                        errors,
                    )

    def test_plan_edit_transaction_requires_two_pass_validation_details(self):
        required_details = (
            "draft receipt",
            "checker regression suite",
            "git diff --check",
            "--worktree --base-ref",
            "final receipt",
            "append-only",
            "preserve prior receipts",
        )
        for detail in required_details:
            with self.subTest(detail=detail):
                transaction_start = self.plan.index(
                    "#### Plan-edit validation transaction"
                )
                transaction_end = self.plan.index(
                    "#### Gate-state update contract", transaction_start
                )
                transaction = self.plan[transaction_start:transaction_end]
                changed = self.plan.replace(
                    transaction,
                    transaction.replace(detail, "removed detail", 1),
                    1,
                )
                errors = plan_checker.validate_plan_structure(changed, REPOSITORY_ROOT)
                self.assertIn(
                    "plan-edit validation transaction is missing required detail: {}".format(
                        detail
                    ),
                    errors,
                )

    def test_validation_layer_table_requires_all_four_layers(self):
        for layer in (
            "Plan structure",
            "Checker regression",
            "Change coverage",
            "Product acceptance",
        ):
            with self.subTest(layer=layer):
                changed = "\n".join(
                    line
                    for line in self.plan.splitlines()
                    if not (
                        plan_checker._cells(line)
                        and plan_checker._cells(line)[0] == layer
                    )
                )
                errors = plan_checker.validate_plan_structure(changed, REPOSITORY_ROOT)
                self.assertIn(
                    "validation layer table is missing required layer: {}".format(
                        layer.lower()
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
            ["local-only/generated <cache>.bin\nnext.bin"],
            ["local-only/"],
        )

        self.assertIn("## Python binding plan sync: FAIL", summary)
        self.assertIn("Changed paths:", summary)
        self.assertIn("scripts/&lt;img src=x&gt;.py", summary)
        self.assertNotIn("<img src=x>", summary)
        self.assertIn("Mapped gates: PLAN-SYNC", summary)
        self.assertIn("Unmapped paths: unexpected&lt;&amp;.txt", summary)
        self.assertIn("Stale validated evidence: S2", summary)
        self.assertIn("Excluded paths:", summary)
        self.assertIn(r"local-only/generated &lt;cache&gt;.bin\nnext.bin", summary)
        self.assertIn("Exclusion rules:", summary)
        self.assertIn("local-only/", summary)
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

    def test_worktree_discovery_collects_tracked_and_untracked_paths_nul_safely(self):
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
            baseline_names = (
                "staged file.txt",
                "unstaged file.txt",
                "deleted file.txt",
            )
            for name in baseline_names:
                (repository / name).write_text("baseline\n", encoding="utf-8")
            (repository / ".gitignore").write_text("ignored file.txt\n", encoding="utf-8")
            subprocess.run(["git", "add", "."], cwd=repository, check=True)
            subprocess.run(["git", "commit", "-qm", "fixture base"], cwd=repository, check=True)
            base_ref = subprocess.run(
                ["git", "rev-parse", "HEAD"],
                cwd=repository,
                check=True,
                stdout=subprocess.PIPE,
                universal_newlines=True,
            ).stdout.strip()

            (repository / "committed change.txt").write_text("branch commit\n", encoding="utf-8")
            subprocess.run(["git", "add", "committed change.txt"], cwd=repository, check=True)
            subprocess.run(["git", "commit", "-qm", "branch change"], cwd=repository, check=True)
            (repository / "staged file.txt").write_text("staged change\n", encoding="utf-8")
            subprocess.run(["git", "add", "staged file.txt"], cwd=repository, check=True)
            (repository / "unstaged file.txt").write_text("unstaged change\n", encoding="utf-8")
            (repository / "deleted file.txt").unlink()
            (repository / "untracked file\nwith newline.txt").write_text("new\n", encoding="utf-8")
            (repository / "ignored file.txt").write_text("ignored\n", encoding="utf-8")

            changed_paths = plan_checker._git_worktree_paths(base_ref, repository)

        self.assertEqual(
            {
                "committed change.txt",
                "deleted file.txt",
                "staged file.txt",
                "unstaged file.txt",
                "untracked file\nwith newline.txt",
            },
            set(changed_paths),
        )
        self.assertNotIn("ignored file.txt", changed_paths)

    def test_worktree_exclusions_must_match_and_report_exact_candidates(self):
        included, excluded = plan_checker._filter_worktree_paths(
            ["cache/one.bin", "cache/two.bin", "source.cpp", "untracked file.txt"],
            ["cache/", "untracked file.txt"],
        )
        self.assertEqual(["source.cpp"], included)
        self.assertEqual(["cache/one.bin", "cache/two.bin", "untracked file.txt"], excluded)

        with self.assertRaisesRegex(RuntimeError, "does not match a candidate"):
            plan_checker._filter_worktree_paths(["source.cpp"], ["cache/"])
        with self.assertRaisesRegex(RuntimeError, "repository-relative"):
            plan_checker._filter_worktree_paths(["source.cpp"], ["../cache/"])
        with self.assertRaisesRegex(RuntimeError, "plan file cannot be excluded"):
            plan_checker._filter_worktree_paths(
                [plan_checker.PLAN_RELATIVE_PATH], [plan_checker.PLAN_RELATIVE_PATH]
            )

    def test_worktree_mode_requires_receipt_citation_for_exclusions(self):
        errors, _, _ = plan_checker.validate_changed_paths(
            self.plan,
            [plan_checker.PLAN_RELATIVE_PATH],
            REPOSITORY_ROOT,
            excluded_path_rules=["local-only/"],
        )
        self.assertTrue(any("must be cited" in error for error in errors))

        changed_plan = replace_latest_receipt(
            self.plan,
            add_plan_note="Excluded local-only/ because it contains generated local fixtures.",
        )
        errors, _, _ = plan_checker.validate_changed_paths(
            changed_plan,
            [plan_checker.PLAN_RELATIVE_PATH],
            REPOSITORY_ROOT,
            excluded_path_rules=["local-only/"],
        )
        self.assertEqual([], errors)

    def test_worktree_cli_discovers_and_reports_exclusions(self):
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
            (repository / "scripts").mkdir()
            (repository / "scripts/validate_python_binding_plan.py").write_text(
                "base\n", encoding="utf-8"
            )
            plan_path = repository / plan_checker.PLAN_RELATIVE_PATH
            plan_path.write_text(self.plan, encoding="utf-8")
            subprocess.run(["git", "add", "."], cwd=repository, check=True)
            subprocess.run(["git", "commit", "-qm", "fixture base"], cwd=repository, check=True)
            base_ref = subprocess.run(
                ["git", "rev-parse", "HEAD"],
                cwd=repository,
                check=True,
                stdout=subprocess.PIPE,
                universal_newlines=True,
            ).stdout.strip()
            (repository / "scripts/validate_python_binding_plan.py").write_text(
                "changed\n", encoding="utf-8"
            )
            plan_path.write_text(
                prepend_review_receipt(
                    self.plan,
                    "| 2026-10-10 | PLAN-SYNC | Exercised worktree exclusion reporting | "
                    "Excluded local-only/ because it contains a generated fixture. |",
                ),
                encoding="utf-8",
            )
            (repository / "local-only").mkdir()
            excluded_candidate = repository / "local-only/generated fixture.bin"
            excluded_candidate.write_text("generated\n", encoding="utf-8")

            output = io.StringIO()
            with patch.object(plan_checker, "REPOSITORY_ROOT", repository), patch.object(
                plan_checker, "PLAN_PATH", plan_path
            ), redirect_stdout(output):
                result = plan_checker.main(
                    ["--worktree", "--base-ref", base_ref, "--exclude-path", "local-only/"]
                )

        self.assertEqual(0, result, output.getvalue())
        self.assertIn("Change source: complete worktree against base ref", output.getvalue())
        self.assertIn("scripts/validate_python_binding_plan.py", output.getvalue())
        self.assertIn("Excluded paths:", output.getvalue())
        self.assertIn("local-only/generated fixture.bin", output.getvalue())
        self.assertIn('Exclusion rules: "local-only/"', output.getvalue())

    def test_worktree_cli_requires_base_ref_and_only_allows_exclusions_in_worktree_mode(self):
        for arguments, expected in (
            (["--worktree"], "--worktree requires --base-ref"),
            (["--base-ref", "HEAD", "--exclude-path", "cache/"], "--exclude-path requires --worktree"),
        ):
            with self.subTest(arguments=arguments), redirect_stderr(io.StringIO()):
                with self.assertRaises(SystemExit) as error:
                    plan_checker.main(arguments)
                self.assertEqual(2, error.exception.code)

    def test_candidate_path_mode_accepts_an_untracked_plan_file(self):
        output = io.StringIO()
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

    def test_newest_receipt_requires_complete_metadata(self):
        header_index = next(
            index
            for index, line in enumerate(self.plan.splitlines())
            if plan_checker._cells(line)[:4]
            == ["Review date", "Affected gates", "Plan change", "Validation scope"]
        )
        receipt_index = header_index + 2
        receipt = self.plan.splitlines()[receipt_index]
        for column, expected_error in (
            (2, "must describe the plan change"),
            (3, "must record validation scope or blocker"),
        ):
            with self.subTest(column=column):
                lines = self.plan.splitlines()
                parts = receipt.split("|")
                parts[column + 1] = " "
                lines[receipt_index] = "|".join(parts)
                errors = plan_checker.validate_plan_structure(
                    "\n".join(lines) + "\n", REPOSITORY_ROOT
                )
                self.assertTrue(
                    any(expected_error in error for error in errors), errors
                )

    def test_newest_receipt_requires_a_valid_iso_date(self):
        lines = self.plan.splitlines()
        header_index = next(
            index
            for index, line in enumerate(lines)
            if plan_checker._cells(line)[:4]
            == ["Review date", "Affected gates", "Plan change", "Validation scope"]
        )
        receipt_index = header_index + 2
        parts = lines[receipt_index].split("|")
        parts[1] = " 2026-02-30 "
        lines[receipt_index] = "|".join(parts)
        errors = plan_checker.validate_plan_structure(
            "\n".join(lines) + "\n", REPOSITORY_ROOT
        )
        self.assertTrue(
            any("not a valid calendar date" in error for error in errors), errors
        )

    def test_tracked_plan_change_requires_a_new_receipt(self):
        errors, mapped, _ = plan_checker.validate_changed_paths(
            self.plan,
            [plan_checker.PLAN_RELATIVE_PATH],
            REPOSITORY_ROOT,
            base_plan_text=self.plan,
        )

        self.assertEqual({"PLAN-SYNC"}, mapped)
        self.assertTrue(
            any("must prepend a new receipt" in error for error in errors), errors
        )

    def test_new_receipt_preserves_existing_review_history(self):
        receipt = (
            "| 2026-10-10 | PLAN-SYNC | Added append-only receipt freshness validation | "
            "`--full`, checker tests, and complete worktree coverage pass. |"
        )
        changed = prepend_review_receipt(self.plan, receipt)
        errors, mapped, _ = plan_checker.validate_changed_paths(
            changed,
            [plan_checker.PLAN_RELATIVE_PATH],
            REPOSITORY_ROOT,
            base_plan_text=self.plan,
        )

        self.assertEqual({"PLAN-SYNC"}, mapped)
        self.assertEqual([], errors)

    def test_new_receipt_cannot_rewrite_prior_review_history(self):
        receipt = (
            "| 2026-10-10 | PLAN-SYNC | Added receipt freshness validation | "
            "Plan checks pass. |"
        )
        changed = prepend_review_receipt(self.plan, receipt)
        lines = changed.splitlines()
        header_index = next(
            index
            for index, line in enumerate(lines)
            if plan_checker._cells(line)[:4]
            == ["Review date", "Affected gates", "Plan change", "Validation scope"]
        )
        prior_receipt = lines[header_index + 3].split("|")
        prior_receipt[3] = prior_receipt[3] + " edited after the fact"
        lines[header_index + 3] = "|".join(prior_receipt)
        changed = "\n".join(lines) + "\n"
        errors, _, _ = plan_checker.validate_changed_paths(
            changed,
            [plan_checker.PLAN_RELATIVE_PATH],
            REPOSITORY_ROOT,
            base_plan_text=self.plan,
        )

        self.assertTrue(
            any("preserve prior receipts unchanged" in error for error in errors), errors
        )

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
