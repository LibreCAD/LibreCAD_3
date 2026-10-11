#!/usr/bin/env python3
"""Validate the Python-binding roadmap and synchronize PR paths with its gates."""

from __future__ import annotations

import argparse
from datetime import date
import html
import json
import os
import re
import subprocess
import sys
from pathlib import Path
from typing import Dict, Iterable, List, NamedTuple, Optional, Sequence, Set, Tuple


REPOSITORY_ROOT = Path(__file__).resolve().parent.parent
PLAN_RELATIVE_PATH = "PYTHON_BINDING_IMPLEMENTATION_PLAN.md"
PLAN_PATH = REPOSITORY_ROOT / PLAN_RELATIVE_PATH

IMPLEMENTATION_GATE_IDS = tuple("S{}".format(number) for number in range(8))
PLATFORM_GATE_IDS = (
    "LINUX-X64",
    "LINUX-ARM64",
    "APPIMAGE-X64",
    "SNAP-AMD64",
    "WIN-X64",
    "MAC-ARM64",
    "MAC-X64",
)
REQUIRED_GATE_IDS = frozenset(
    IMPLEMENTATION_GATE_IDS + ("DOC", "PLAN-SYNC") + PLATFORM_GATE_IDS
)
ALLOWED_STATES = frozenset(("planned", "active", "implemented", "validated", "blocked"))
EVIDENCE_FIELDS = (
    "date",
    "revision",
    "environment",
    "command",
    "counts",
    "skips",
    "result",
    "report",
    "blocker",
)
PACKAGE_EVIDENCE_FIELDS = ("artifact_sha256", "runtime")
REQUIRED_RULES = frozenset(
    (
        PLAN_RELATIVE_PATH,
        "scripts/validate_python_binding_plan.py",
        "scripts/tests/test_validate_python_binding_plan.py",
        ".github/workflows/python-binding-plan.yml",
    )
)


class PathRule(NamedTuple):
    pattern: str
    gates: frozenset


def _cells(line: str) -> List[str]:
    if not line.strip().startswith("|"):
        return []
    return [cell.strip() for cell in line.strip().strip("|").split("|")]


def _strip_code(value: str) -> str:
    value = value.strip()
    if len(value) >= 2 and value[0] == "`" and value[-1] == "`":
        return value[1:-1]
    return value


def _section(text: str, start: str, end: Optional[str] = None) -> str:
    start_at = text.find(start)
    if start_at < 0:
        return ""
    start_at += len(start)
    if end is None:
        return text[start_at:]
    end_at = text.find(end, start_at)
    return text[start_at:] if end_at < 0 else text[start_at:end_at]


def _table_rows(section: str) -> Iterable[List[str]]:
    for line in section.splitlines():
        cells = _cells(line)
        if cells and not all(set(cell) <= {"-", ":", " "} for cell in cells):
            yield cells


def _ledger_rows(text: str, known_ids: Set[str]) -> List[List[str]]:
    section = _section(
        text,
        "### Work and platform ledgers",
        "### Evidence and revalidation rules",
    )
    return [
        row
        for row in _table_rows(section)
        if row and row[0] in known_ids
    ]


def _review_log_rows(text: str) -> Optional[List[List[str]]]:
    lines = text.splitlines()
    for index, line in enumerate(lines):
        if _cells(line)[:4] != ["Review date", "Affected gates", "Plan change", "Validation scope"]:
            continue
        rows: List[List[str]] = []
        for receipt_line in lines[index + 1 :]:
            if not receipt_line.strip().startswith("|"):
                break
            row = _cells(receipt_line)
            if not row or all(set(cell) <= {"-", ":", " "} for cell in row):
                continue
            rows.append(row)
        return rows
    return None


def parse_path_rules(text: str) -> Tuple[List[PathRule], List[str]]:
    errors: List[str] = []
    rules: List[PathRule] = []
    section = _section(
        text,
        "#### Automated minimum path rules",
        "#### Explicit path exemptions",
    )
    if not section:
        return [], ["missing automated minimum path rules table"]

    for row in _table_rows(section):
        if len(row) < 2 or row[0] == "Exact path or prefix":
            continue
        pattern = _strip_code(row[0])
        gate_tokens = {token.strip() for token in row[1].split(",") if token.strip()}
        if not pattern:
            errors.append("path-rule entry has an empty path pattern")
            continue
        if "*" in pattern[:-1]:
            errors.append("path rule {!r} uses a non-trailing wildcard".format(pattern))
        unknown = gate_tokens - REQUIRED_GATE_IDS - {"ALL_PLATFORMS"}
        if unknown:
            errors.append(
                "path rule {!r} names unknown gates: {}".format(
                    pattern, ", ".join(sorted(unknown))
                )
            )
        if not gate_tokens:
            errors.append("path rule {!r} has no minimum gates".format(pattern))
        expanded = set(gate_tokens - {"ALL_PLATFORMS"})
        if "ALL_PLATFORMS" in gate_tokens:
            expanded.update(PLATFORM_GATE_IDS)
        rules.append(PathRule(pattern, frozenset(expanded)))

    seen: Set[str] = set()
    for rule in rules:
        if rule.pattern in seen:
            errors.append("duplicate path rule: {}".format(rule.pattern))
        seen.add(rule.pattern)
    missing = REQUIRED_RULES - seen
    if missing:
        errors.append("missing required PLAN-SYNC path rules: {}".format(", ".join(sorted(missing))))
    return rules, errors


def _rule_specificity(pattern: str) -> int:
    return len(pattern[:-1] if pattern.endswith(("/", "*")) else pattern)


def _matches(path: str, pattern: str) -> bool:
    if pattern.endswith("/") or pattern.endswith("*"):
        return path.startswith(pattern[:-1] if pattern.endswith("*") else pattern)
    return path == pattern


def map_paths(paths: Sequence[str], rules: Sequence[PathRule]) -> Tuple[Set[str], Set[str]]:
    mapped_gates: Set[str] = set()
    unmapped_paths: Set[str] = set()
    for path in paths:
        matches = [rule for rule in rules if _matches(path, rule.pattern)]
        if not matches:
            unmapped_paths.add(path)
            continue
        specificity = max(_rule_specificity(rule.pattern) for rule in matches)
        for rule in matches:
            if _rule_specificity(rule.pattern) == specificity:
                mapped_gates.update(rule.gates)
    return mapped_gates, unmapped_paths


def parse_exemptions(text: str) -> Tuple[Set[str], List[str]]:
    section = _section(
        text,
        "#### Explicit path exemptions",
        "Every receipt must name all minimum gates",
    )
    errors: List[str] = []
    exemptions: Set[str] = set()
    if not section:
        return exemptions, ["missing explicit path exemptions table"]
    for row in _table_rows(section):
        if len(row) < 3 or row[0] == "Exact path":
            continue
        path = _strip_code(row[0])
        rationale = row[1].strip()
        if not path or not rationale:
            errors.append("path exemption requires an exact path and rationale")
            continue
        if path.endswith("/") or "*" in path:
            errors.append("path exemption must name one exact repository path: {}".format(path))
        if path in exemptions:
            errors.append("duplicate path exemption: {}".format(path))
        exemptions.add(path)
    return exemptions, errors


def _review_receipt(text: str) -> Tuple[Optional[List[str]], str, List[str]]:
    rows = _review_log_rows(text)
    if rows is None:
        return None, "", ["missing review-log synchronization receipt table"]
    if not rows:
        return None, "", ["review log has no receipt rows"]
    row = rows[0]
    if len(row) < 4:
        return None, "", ["newest review-log receipt has fewer than four fields"]
    raw_ids = row[1]
    ids = set(re.findall(r"(?<![A-Za-z0-9-])(?:S[0-7]|DOC|PLAN-SYNC|LINUX-X64|LINUX-ARM64|APPIMAGE-X64|SNAP-AMD64|WIN-X64|MAC-ARM64|MAC-X64)(?![A-Za-z0-9-])", raw_ids))
    unknown_tokens = [token.strip() for token in raw_ids.split(",") if token.strip() and token.strip() not in ids]
    errors = []
    if not re.fullmatch(r"\d{4}-\d{2}-\d{2}", row[0]):
        errors.append("newest review-log receipt date must use YYYY-MM-DD")
    else:
        try:
            date.fromisoformat(row[0])
        except ValueError:
            errors.append("newest review-log receipt date is not a valid calendar date")
    if not row[2]:
        errors.append("newest review-log receipt must describe the plan change")
    if not row[3]:
        errors.append("newest review-log receipt must record validation scope or blocker")
    if not ids:
        errors.append("newest review-log receipt names no gate IDs")
    if unknown_tokens:
        errors.append("newest review-log receipt has unknown gate text: {}".format(", ".join(unknown_tokens)))
    return sorted(ids), " ".join(row[2:]), errors


def _check_local_links(text: str, repository_root: Path) -> List[str]:
    errors: List[str] = []
    for target in re.findall(r"\[[^\]]+\]\(([^)]+)\)", text):
        target = target.strip().strip("<>")
        if target.startswith(("https://", "http://", "mailto:", "#")):
            continue
        local_target = target.split("#", 1)[0].split("?", 1)[0]
        if not local_target:
            continue
        if not (repository_root / local_target).is_file():
            errors.append("broken local link: {}".format(target))
    return errors


def _validate_evidence(gate_id: str, evidence: str, package: bool = False) -> List[str]:
    errors: List[str] = []
    fields = {}
    for part in evidence.split(";"):
        if "=" not in part:
            continue
        key, value = part.split("=", 1)
        fields[key.strip().lower()] = value.strip()
    missing = [field for field in EVIDENCE_FIELDS if not fields.get(field)]
    if package:
        missing.extend(field for field in PACKAGE_EVIDENCE_FIELDS if not fields.get(field))
    if missing:
        errors.append(
            "validated gate {} lacks evidence fields: {}".format(
                gate_id, ", ".join(sorted(set(missing)))
            )
        )
        return errors

    if not re.fullmatch(r"\d{4}-\d{2}-\d{2}", fields["date"]):
        errors.append("validated gate {} has invalid date evidence".format(gate_id))
    if fields["result"].lower() != "pass":
        errors.append("validated gate {} must record result=pass".format(gate_id))
    if fields["blocker"].lower() != "none":
        errors.append("validated gate {} still records a blocker".format(gate_id))

    counts = re.fullmatch(r"(\d+)/(\d+)/(\d+)", fields["counts"])
    if not counts:
        errors.append("validated gate {} counts must be discovered/executed/skipped".format(gate_id))
    else:
        discovered, executed, skipped = (int(value) for value in counts.groups())
        if discovered == 0 or executed == 0:
            errors.append("validated gate {} cannot have zero discovered or executed checks".format(gate_id))
        if executed + skipped != discovered:
            errors.append("validated gate {} counts do not reconcile".format(gate_id))
        skip_note = fields["skips"].lower()
        if skipped == 0 and skip_note != "none":
            errors.append("validated gate {} must record skips=none when no checks were skipped".format(gate_id))
        if skipped > 0 and not skip_note.startswith("optional:"):
            errors.append("validated gate {} has skipped checks that are not explicitly optional".format(gate_id))

    if package and not re.fullmatch(r"[0-9a-fA-F]{64}", fields["artifact_sha256"]):
        errors.append("validated package gate {} needs a 64-digit artifact SHA-256".format(gate_id))
    return errors


def validate_plan_structure(text: str, repository_root: Path = REPOSITORY_ROOT) -> List[str]:
    errors: List[str] = []
    for line_number, line in enumerate(text.splitlines(), 1):
        if line.rstrip(" \t") != line:
            errors.append("trailing whitespace at plan line {}".format(line_number))
        if re.match(r"^(<<<<<<<|=======|>>>>>>)(?:\s|$)", line):
            errors.append("merge-conflict marker at plan line {}".format(line_number))

    required_headings = (
        "## 6. Plan self-updating and validation",
        "### Self-update triggers",
        "#### Operational self-update and validation",
        "#### Automatic candidate discovery",
        "#### Plan-edit validation transaction",
        "#### Validation layers and fail-closed rule",
        "### Work and platform ledgers",
        "### Evidence and revalidation rules",
        "### Validate the plan and the implementation separately",
    )
    for heading in required_headings:
        if heading not in text:
            errors.append("missing required plan heading: {}".format(heading))

    maintenance_protocol = _section(
        text,
        "### Maintenance protocol",
        "#### Per-batch checkpoint record",
    ).lower()
    if "at each user-turn boundary" not in maintenance_protocol:
        errors.append("maintenance protocol is missing the user-turn sync rule")
    turn_sync_rule = _section(
        text,
        "At each user-turn boundary,",
        "An explicit request to revise this plan",
    ).lower()
    turn_sync_rule = " ".join(turn_sync_rule.split())
    for detail in (
        "classify new input",
        "scope change, new evidence or blocker, or no change",
        "with a receipt before continuing implementation",
        "leaving incomplete evidence provisional",
        "no ledger churn",
    ):
        if detail not in turn_sync_rule:
            errors.append(
                "user-turn sync rule is missing required detail: {}".format(detail)
            )

    operational_steps = list(_table_rows(
        _section(
            text,
            "#### Operational self-update and validation",
            "#### Automatic candidate discovery",
        )
    ))
    operational_moments = [row[0].lower() for row in operational_steps if row]
    required_operational_moments = (
        "at each user-turn boundary",
        "resume or start a batch",
        "after each meaningful implementation/test/ci result or blocker",
        "pr-ready handoff",
    )
    for moment in required_operational_moments:
        matching_rows = [
            row
            for row in operational_steps
            if row and moment in row[0].lower()
        ]
        if not matching_rows:
            errors.append(
                "operational self-update table is missing checkpoint: {}".format(moment)
            )
            continue
        row = matching_rows[0]
        if len(row) < 3 or not row[1].strip():
            errors.append(
                "operational self-update checkpoint is missing required update: {}".format(
                    moment
                )
            )
        if len(row) < 3 or not row[2].strip():
            errors.append(
                "operational self-update checkpoint is missing required validation: {}".format(
                    moment
                )
            )

    trigger_rows = list(_table_rows(
        _section(
            text,
            "#### Self-update triggers",
            "#### Gate-state update contract",
        )
    ))
    required_trigger_categories = (
        "target branch advances",
        "source, api, build flags",
        "test selection, fixtures, or ci workflow",
        "a test/ci result, failure, or blocker",
        "external evidence arrives",
        "a capability, platform, architecture, or runtime-support decision",
        "user scope, priority, or acceptance criteria change",
        "plan, checker, checker tests, or plan workflow changes",
        "plan-only edit",
    )
    for category in required_trigger_categories:
        matching_rows = [
            row
            for row in trigger_rows
            if row and category in row[0].lower()
        ]
        if not matching_rows:
            errors.append(
                "self-update trigger table is missing required category: {}".format(
                    category
                )
            )
            continue
        row = matching_rows[0]
        trigger_title = row[0].lower()
        if len(row) < 3 or not row[1].strip():
            errors.append(
                "self-update trigger is missing required update: {}".format(
                    trigger_title
                )
            )
        if len(row) < 3 or not row[2].strip():
            errors.append(
                "self-update trigger is missing required validation: {}".format(
                    trigger_title
                )
            )

    plan_edit_transaction = _section(
        text,
        "#### Plan-edit validation transaction",
        "#### Gate-state update contract",
    ).lower()
    required_transaction_details = (
        "draft receipt",
        "checker regression suite",
        "git diff --check",
        "--worktree --base-ref",
        "final receipt",
        "append-only",
        "preserve prior receipts",
    )
    for detail in required_transaction_details:
        if detail not in plan_edit_transaction:
            errors.append(
                "plan-edit validation transaction is missing required detail: {}".format(
                    detail
                )
            )

    validation_rows = _table_rows(
        _section(
            text,
            "#### Validation layers and fail-closed rule",
            "Any nonzero checker/test/lint result",
        )
    )
    validation_layers = {row[0].lower() for row in validation_rows if row}
    for layer in (
        "plan structure",
        "checker regression",
        "change coverage",
        "product acceptance",
    ):
        if layer not in validation_layers:
            errors.append(
                "validation layer table is missing required layer: {}".format(layer)
            )

    gate_rows = _ledger_rows(text, set(REQUIRED_GATE_IDS))
    gate_counts: Dict[str, int] = {}
    states: Dict[str, str] = {}
    for row in gate_rows:
        gate_id = row[0]
        gate_counts[gate_id] = gate_counts.get(gate_id, 0) + 1
        if len(row) < 4:
            errors.append("gate {} ledger row has fewer than four fields".format(gate_id))
            continue
        state = row[1]
        states[gate_id] = state
        if state not in ALLOWED_STATES:
            errors.append("gate {} has unknown state {!r}".format(gate_id, state))
        if state == "validated":
            errors.extend(_validate_evidence(gate_id, row[3], package=gate_id == "S6"))

    for gate_id in sorted(REQUIRED_GATE_IDS):
        count = gate_counts.get(gate_id, 0)
        if count == 0:
            errors.append("missing required gate row: {}".format(gate_id))
        elif count > 1:
            errors.append("duplicate required gate row: {}".format(gate_id))

    platform_rows = _ledger_rows(text, set(PLATFORM_GATE_IDS))
    platform_counts: Dict[str, int] = {}
    for row in platform_rows:
        gate_id = row[0]
        platform_counts[gate_id] = platform_counts.get(gate_id, 0) + 1
        if len(row) < 4:
            errors.append("platform {} row has fewer than four fields".format(gate_id))
            continue
        if row[1] not in ALLOWED_STATES:
            errors.append("platform {} has unknown state {!r}".format(gate_id, row[1]))
        if row[1] == "validated":
            errors.extend(_validate_evidence(gate_id, row[3], package=True))
    for gate_id in PLATFORM_GATE_IDS:
        count = platform_counts.get(gate_id, 0)
        if count == 0:
            errors.append("missing required platform row: {}".format(gate_id))
        elif count > 1:
            errors.append("duplicate required platform row: {}".format(gate_id))

    rules, rule_errors = parse_path_rules(text)
    errors.extend(rule_errors)
    exemptions, exemption_errors = parse_exemptions(text)
    errors.extend(exemption_errors)
    del exemptions
    for rule in rules:
        unknown = rule.gates - REQUIRED_GATE_IDS
        if unknown:
            errors.append("path rule {} expands to unknown gates: {}".format(rule.pattern, ", ".join(sorted(unknown))))
    errors.extend(_check_local_links(text, repository_root))

    receipt_ids, _, receipt_errors = _review_receipt(text)
    errors.extend(receipt_errors)
    if receipt_ids:
        unknown_receipt_ids = set(receipt_ids) - REQUIRED_GATE_IDS
        if unknown_receipt_ids:
            errors.append("review receipt names unknown gate IDs: {}".format(", ".join(sorted(unknown_receipt_ids))))
    return errors


def validate_changed_paths(
    text: str,
    changed_paths: Sequence[str],
    repository_root: Path = REPOSITORY_ROOT,
    excluded_path_rules: Sequence[str] = (),
    base_plan_text: Optional[str] = None,
) -> Tuple[List[str], Set[str], Set[str]]:
    errors = validate_plan_structure(text, repository_root)
    rules, rule_errors = parse_path_rules(text)
    errors.extend(rule_errors)
    exemptions, exemption_errors = parse_exemptions(text)
    errors.extend(exemption_errors)
    mapped_gates, unmatched_paths = map_paths(changed_paths, rules)
    receipt_ids, receipt_text, receipt_errors = _review_receipt(text)
    errors.extend(receipt_errors)
    receipt_gate_ids = set(receipt_ids or ())

    plan_changed = PLAN_RELATIVE_PATH in changed_paths
    if changed_paths and (mapped_gates or unmatched_paths.intersection(exemptions)) and not plan_changed:
        errors.append("mapped or exempted changes require a same-change plan update")

    if plan_changed and base_plan_text is not None:
        previous_receipts = _review_log_rows(base_plan_text)
        current_receipts = _review_log_rows(text)
        if previous_receipts is None:
            errors.append("base plan has no review-log synchronization receipt table")
        elif current_receipts is None:
            errors.append("updated plan has no review-log synchronization receipt table")
        elif len(current_receipts) <= len(previous_receipts) or (
            previous_receipts
            and current_receipts[-len(previous_receipts) :] != previous_receipts
        ):
            errors.append(
                "same-change plan update must prepend a new receipt and preserve prior receipts unchanged"
            )

    unapproved = unmatched_paths - exemptions
    if unapproved:
        errors.append("unmapped paths need exact exemptions: {}".format(", ".join(sorted(unapproved))))
    cited_exemptions = unmatched_paths.intersection(exemptions)
    for path in sorted(cited_exemptions):
        if not plan_changed or path not in receipt_text:
            errors.append("exempt path {} must be cited in this change's newest plan receipt".format(path))

    if excluded_path_rules:
        if not plan_changed:
            errors.append("explicit worktree exclusions require a same-change plan update")
        for rule in excluded_path_rules:
            if not plan_changed or rule not in receipt_text:
                errors.append(
                    "excluded path rule {} must be cited in this change's newest plan receipt".format(rule)
                )

    if plan_changed:
        missing_receipt_gates = mapped_gates - receipt_gate_ids
        if missing_receipt_gates:
            errors.append("newest plan receipt omits mapped gates: {}".format(", ".join(sorted(missing_receipt_gates))))

    current_states = {}
    for row in _ledger_rows(text, set(REQUIRED_GATE_IDS)):
        if len(row) >= 2:
            current_states[row[0]] = row[1]
    stale = sorted(gate for gate in mapped_gates if current_states.get(gate) == "validated")
    if stale:
        errors.append("changed inputs invalidate validated gates; downgrade and revalidate: {}".format(", ".join(stale)))
    return errors, mapped_gates, unmatched_paths


def _quoted_path(path: str) -> str:
    """Keep unusual filenames on one unambiguous, safe report line."""
    return json.dumps(path, ensure_ascii=True)


def render_step_summary(
    status: str,
    change_source: str,
    changed_paths: Sequence[str],
    mapped_gates: Set[str],
    unmapped_paths: Set[str],
    plan_changed: bool,
    stale_evidence: Sequence[str],
    errors: Sequence[str],
    excluded_paths: Sequence[str] = (),
    exclusion_rules: Sequence[str] = (),
) -> str:
    """Render a GitHub job summary while escaping PR-controlled text."""
    lines = [
        "Change source: {}".format(change_source),
        "Changed paths:",
    ]
    lines.extend("  {}".format(_quoted_path(path)) for path in changed_paths)
    if not changed_paths:
        lines.append("  none")
    lines.append("Excluded paths:")
    lines.extend("  {}".format(_quoted_path(path)) for path in excluded_paths)
    if not excluded_paths:
        lines.append("  none")
    lines.append("Exclusion rules:")
    lines.extend("  {}".format(_quoted_path(rule)) for rule in exclusion_rules)
    if not exclusion_rules:
        lines.append("  none")
    lines.extend(
        (
            "Mapped gates: {}".format(", ".join(sorted(mapped_gates)) or "none"),
            "Unmapped paths: {}".format(", ".join(sorted(unmapped_paths)) or "none"),
            "Plan changed: {}".format("yes" if plan_changed else "no"),
            "Stale validated evidence: {}".format(", ".join(stale_evidence) or "none"),
            "Errors:",
        )
    )
    lines.extend("  {}".format(error) for error in errors)
    if not errors:
        lines.append("  none")
    return "## Python binding plan sync: {}\n\n<pre>\n{}\n</pre>\n".format(
        status.upper(), html.escape("\n".join(lines))
    )


def append_step_summary(
    summary_file: str,
    status: str,
    change_source: str,
    changed_paths: Sequence[str],
    mapped_gates: Set[str],
    unmapped_paths: Set[str],
    plan_changed: bool,
    stale_evidence: Sequence[str],
    errors: Sequence[str],
    excluded_paths: Sequence[str] = (),
    exclusion_rules: Sequence[str] = (),
) -> None:
    with Path(summary_file).open("a", encoding="utf-8") as output:
        output.write(
            render_step_summary(
                status,
                change_source,
                changed_paths,
                mapped_gates,
                unmapped_paths,
                plan_changed,
                stale_evidence,
                errors,
                excluded_paths,
                exclusion_rules,
            )
        )


def _git_paths(arguments: Sequence[str], repository_root: Path) -> List[str]:
    result = subprocess.run(
        list(arguments),
        cwd=str(repository_root),
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if result.returncode != 0:
        error = os.fsdecode(result.stderr).strip()
        raise RuntimeError("git command {} failed: {}".format(" ".join(arguments), error))
    paths = [os.fsdecode(path) for path in result.stdout.split(b"\0") if path]
    if os.sep != "/":
        paths = [path.replace(os.sep, "/") for path in paths]
    return sorted(set(paths))


def _git_changed_paths(base_ref: str, repository_root: Path) -> List[str]:
    return _git_paths(
        ["git", "diff", "--name-only", "--no-renames", "-z", base_ref, "--"],
        repository_root,
    )


def _git_base_plan_text(base_ref: str, repository_root: Path) -> Optional[str]:
    result = subprocess.run(
        ["git", "show", "{}:{}".format(base_ref, PLAN_RELATIVE_PATH)],
        cwd=str(repository_root),
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if result.returncode != 0:
        return None
    return result.stdout.decode("utf-8", errors="replace")


def _git_worktree_paths(base_ref: str, repository_root: Path) -> List[str]:
    tracked = _git_paths(
        ["git", "diff", "--name-only", "--no-renames", "-z", base_ref, "--"],
        repository_root,
    )
    untracked = _git_paths(
        ["git", "ls-files", "--others", "--exclude-standard", "-z"],
        repository_root,
    )
    return sorted(set(tracked).union(untracked))


def _filter_worktree_paths(
    candidate_paths: Sequence[str], exclusion_rules: Sequence[str]
) -> Tuple[List[str], List[str]]:
    candidates = sorted(set(candidate_paths))
    rules = list(
        dict.fromkeys(
            rule.replace(os.sep, "/") if os.sep != "/" else rule
            for rule in exclusion_rules
        )
    )
    if len(rules) != len(exclusion_rules):
        raise RuntimeError("duplicate --exclude-path rule")

    for rule in rules:
        invalid = (
            not rule
            or rule.startswith("/")
            or re.match(r"^[A-Za-z]:/", rule)
            or ".." in rule.split("/")
            or any(character in rule for character in "*?[]")
            or rule in (".", "./")
        )
        if invalid:
            raise RuntimeError(
                "exclusion rules must be repository-relative exact paths or directory prefixes: {}".format(rule)
            )

    excluded = set()
    for rule in rules:
        if rule.endswith("/"):
            matching = {path for path in candidates if path.startswith(rule)}
        else:
            matching = {rule} if rule in candidates else set()
        if not matching:
            raise RuntimeError("exclusion rule does not match a candidate path: {}".format(rule))
        if PLAN_RELATIVE_PATH in matching:
            raise RuntimeError("the plan file cannot be excluded from worktree validation")
        excluded.update(matching)
    included = [path for path in candidates if path not in excluded]
    return included, sorted(excluded)


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--full", action="store_true", help="validate the complete plan structure only")
    mode.add_argument("--base-ref", help="compare the working tree with this git base ref and enforce synchronization")
    mode.add_argument(
        "--paths",
        nargs="+",
        metavar="REPO_PATH",
        help="validate an explicit candidate path set, including untracked files",
    )
    parser.add_argument(
        "--worktree",
        action="store_true",
        help="include non-ignored untracked paths with --base-ref",
    )
    parser.add_argument(
        "--exclude-path",
        action="append",
        default=[],
        metavar="REPO_PATH",
        help="for --worktree, exclude an exact candidate path or an explicit directory prefix ending in /",
    )
    parser.add_argument(
        "--summary-file",
        help="append a GitHub Actions job summary report to this file",
    )
    args = parser.parse_args(argv)

    if not (args.full or args.base_ref or args.worktree or args.paths):
        parser.error("choose --full, --base-ref, --worktree --base-ref, or --paths")
    if args.worktree and not args.base_ref:
        parser.error("--worktree requires --base-ref")
    if args.exclude_path and not args.worktree:
        parser.error("--exclude-path requires --worktree")

    changed_paths: List[str] = []
    excluded_paths: List[str] = []
    mapped_gates: Set[str] = set()
    unmapped_paths: Set[str] = set()
    errors: List[str] = []
    change_source = "full plan structure"
    text = ""
    fatal_error: Optional[str] = None
    try:
        text = PLAN_PATH.read_text(encoding="utf-8")
        if args.full:
            errors = validate_plan_structure(text)
        else:
            if args.worktree:
                change_source = "complete worktree against base ref {}".format(args.base_ref)
                candidates = _git_worktree_paths(args.base_ref, REPOSITORY_ROOT)
                changed_paths, excluded_paths = _filter_worktree_paths(
                    candidates, args.exclude_path
                )
            elif args.base_ref:
                change_source = "base ref {}".format(args.base_ref)
                changed_paths = _git_changed_paths(args.base_ref, REPOSITORY_ROOT)
            else:
                change_source = "explicit candidate paths"
                changed_paths = sorted(
                    {path.replace("\\", "/") for path in args.paths}
                )
                invalid_paths = [
                    path
                    for path in changed_paths
                    if not path
                    or path.startswith("/")
                    or re.match(r"^[A-Za-z]:/", path)
                    or ".." in path.split("/")
                ]
                if invalid_paths:
                    raise RuntimeError(
                        "candidate paths must be repository-relative: {}".format(
                            ", ".join(invalid_paths)
                        )
                    )
            base_plan_text = (
                _git_base_plan_text(args.base_ref, REPOSITORY_ROOT)
                if args.base_ref and PLAN_RELATIVE_PATH in changed_paths
                else None
            )
            errors, mapped_gates, unmapped_paths = validate_changed_paths(
                text,
                changed_paths,
                excluded_path_rules=args.exclude_path,
                base_plan_text=base_plan_text,
            )
    except (OSError, RuntimeError) as error:
        fatal_error = str(error)
        errors.append(fatal_error)
        print("ERROR: {}".format(fatal_error), file=sys.stderr)

    print("Python binding plan sync: {}".format("FAIL" if errors else "PASS"))
    if args.base_ref or args.paths:
        if args.paths:
            print("Change source: explicit candidate paths")
        elif args.worktree:
            print("Change source: {}".format(change_source))
        print("Changed paths:")
        for path in changed_paths:
            print("  {}".format(_quoted_path(path)))
        if not changed_paths:
            print("  none")
        print("Excluded paths:")
        for path in excluded_paths:
            print("  {}".format(_quoted_path(path)))
        if not excluded_paths:
            print("  none")
        if args.worktree:
            print(
                "Exclusion rules: {}".format(
                    ", ".join(_quoted_path(rule) for rule in args.exclude_path) or "none"
                )
            )
    gate_states = (
        {
            row[0]: row[1]
            for row in _ledger_rows(text, set(REQUIRED_GATE_IDS))
            if len(row) >= 2
        }
        if text
        else {}
    )
    stale_evidence = sorted(
        gate for gate in mapped_gates if gate_states.get(gate) == "validated"
    )
    if args.base_ref or args.paths:
        print("Mapped gates: {}".format(", ".join(sorted(mapped_gates)) or "none"))
        print("Unmapped paths: {}".format(", ".join(sorted(unmapped_paths)) or "none"))
        print("Plan changed: {}".format("yes" if PLAN_RELATIVE_PATH in changed_paths else "no"))
        print("Stale validated evidence: {}".format(", ".join(stale_evidence) or "none"))
    for error in errors:
        if error != fatal_error:
            print("ERROR: {}".format(error))
    if args.summary_file:
        try:
            append_step_summary(
                args.summary_file,
                "fail" if errors else "pass",
                change_source,
                changed_paths,
                mapped_gates,
                unmapped_paths,
                PLAN_RELATIVE_PATH in changed_paths,
                stale_evidence,
                errors,
                excluded_paths,
                args.exclude_path,
            )
        except OSError as error:
            print("cannot write plan job summary: {}".format(error), file=sys.stderr)
            return 1
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
