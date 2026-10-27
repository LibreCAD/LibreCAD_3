#!/usr/bin/env python3
"""Validate the Python-binding roadmap and synchronize PR paths with its gates."""

from __future__ import annotations

import argparse
import html
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
REQUIRED_SYNC_CONTRACT_MARKERS = (
    "same-change",
    "git status --short",
    "--worktree",
    "After each meaningful implementation/test/CI result or blocker",
    "non-ignored untracked paths",
    "--paths",
    "complete-candidate `--paths`",
    "--base-ref",
    "checker regression suite",
    "newest plan receipt is unchanged from the comparison base",
    "branch protection",
    "Target branch advances, or work is rebased/ported to another base",
    "Source, API, build flags, dependency/runtime, runner ABI, source-vs-prebuilt policy, or package layout changes",
    "Test selection, fixtures, or CI workflow changes",
    "A test/CI result, failure, or blocker arrives",
    "External evidence arrives",
    "A capability, platform, architecture, or runtime-support decision changes",
    "A user turn changes scope, priority, acceptance criteria, or supplies evidence",
    "Plan, checker, checker tests, or plan workflow changes",
    "Plan-only edit",
    "python3 scripts/validate_python_binding_plan.py --full",
    "python3 -m unittest discover -s scripts/tests -p 'test_validate_python_binding_plan.py' -v",
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
    lines = text.splitlines()
    for index, line in enumerate(lines):
        header = _cells(line)
        if header[:4] != ["Review date", "Affected gates", "Plan change", "Validation scope"]:
            continue
        for receipt_line in lines[index + 1 :]:
            row = _cells(receipt_line)
            if not row:
                continue
            if all(set(cell) <= {"-", ":", " "} for cell in row):
                continue
            if len(row) < 4:
                return None, "", ["newest review-log receipt has fewer than four fields"]
            raw_ids = row[1]
            ids = set(re.findall(r"(?<![A-Za-z0-9-])(?:S[0-7]|DOC|PLAN-SYNC|LINUX-X64|LINUX-ARM64|APPIMAGE-X64|SNAP-AMD64|WIN-X64|MAC-ARM64|MAC-X64)(?![A-Za-z0-9-])", raw_ids))
            unknown_tokens = [token.strip() for token in raw_ids.split(",") if token.strip() and token.strip() not in ids]
            errors = []
            if not ids:
                errors.append("newest review-log receipt names no gate IDs")
            if unknown_tokens:
                errors.append("newest review-log receipt has unknown gate text: {}".format(", ".join(unknown_tokens)))
            return sorted(ids), " ".join(row[2:]), errors
        return None, "", ["review log has no receipt rows"]
    return None, "", ["missing review-log synchronization receipt table"]


def _latest_review_receipt_row(text: str) -> Optional[Tuple[str, ...]]:
    """Return the newest receipt verbatim enough to detect stale receipt reuse."""
    lines = text.splitlines()
    for index, line in enumerate(lines):
        if _cells(line)[:4] != [
            "Review date",
            "Affected gates",
            "Plan change",
            "Validation scope",
        ]:
            continue
        for receipt_line in lines[index + 1 :]:
            row = _cells(receipt_line)
            if not row or all(set(cell) <= {"-", ":", " "} for cell in row):
                continue
            return tuple(row)
        return None
    return None


def _git_plan_at_ref(ref: str, repository_root: Path) -> Optional[str]:
    """Read the plan at a commit, returning None if that revision predates it."""
    verified = subprocess.run(
        ["git", "rev-parse", "--verify", "--quiet", "{}^{{commit}}".format(ref)],
        cwd=str(repository_root),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True,
    )
    if verified.returncode != 0:
        raise RuntimeError("cannot resolve plan comparison ref {!r}".format(ref))

    result = subprocess.run(
        ["git", "show", "{}:{}".format(ref, PLAN_RELATIVE_PATH)],
        cwd=str(repository_root),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True,
    )
    return result.stdout if result.returncode == 0 else None


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
        "#### Validation layers and fail-closed rule",
        "### Work and platform ledgers",
        "### Evidence and revalidation rules",
        "### Validate the plan and the implementation separately",
    )
    for heading in required_headings:
        if heading not in text:
            errors.append("missing required plan heading: {}".format(heading))

    sync_section = _section(text, "## 6. Plan self-updating and validation", "## 7.")
    for marker in REQUIRED_SYNC_CONTRACT_MARKERS:
        if marker not in sync_section:
            errors.append(
                "plan self-update contract is missing required detail: {}".format(marker)
            )

    operational_steps = _table_rows(
        _section(
            text,
            "#### Operational self-update and validation",
            "#### Automatic candidate discovery",
        )
    )
    operational_moments = [row[0].lower() for row in operational_steps if row]
    required_operational_moments = (
        "resume or start a batch",
        "after each meaningful implementation/test/ci result or blocker",
        "pr-ready handoff",
    )
    for moment in required_operational_moments:
        if not any(moment in title for title in operational_moments):
            errors.append(
                "operational self-update table is missing checkpoint: {}".format(moment)
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
    baseline_text: Optional[str] = None,
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

    unapproved = unmatched_paths - exemptions
    if unapproved:
        errors.append("unmapped paths need exact exemptions: {}".format(", ".join(sorted(unapproved))))
    cited_exemptions = unmatched_paths.intersection(exemptions)
    for path in sorted(cited_exemptions):
        if not plan_changed or path not in receipt_text:
            errors.append("exempt path {} must be cited in this change's newest plan receipt".format(path))

    if plan_changed:
        missing_receipt_gates = mapped_gates - receipt_gate_ids
        if missing_receipt_gates:
            errors.append("newest plan receipt omits mapped gates: {}".format(", ".join(sorted(missing_receipt_gates))))
        if baseline_text is not None:
            current_receipt = _latest_review_receipt_row(text)
            baseline_receipt = _latest_review_receipt_row(baseline_text)
            if current_receipt == baseline_receipt:
                errors.append(
                    "newest plan receipt is unchanged from the comparison base; "
                    "append a fresh same-change receipt"
                )

    current_states = {}
    for row in _ledger_rows(text, set(REQUIRED_GATE_IDS)):
        if len(row) >= 2:
            current_states[row[0]] = row[1]
    stale = sorted(gate for gate in mapped_gates if current_states.get(gate) == "validated")
    if stale:
        errors.append("changed inputs invalidate validated gates; downgrade and revalidate: {}".format(", ".join(stale)))
    return errors, mapped_gates, unmatched_paths


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
) -> str:
    """Render a GitHub job summary while escaping PR-controlled text."""
    lines = [
        "Change source: {}".format(change_source),
        "Changed paths:",
    ]
    lines.extend("  {}".format(path) for path in changed_paths)
    if not changed_paths:
        lines.append("  none")
    lines.extend(
        (
            "Mapped gates: {}".format(", ".join(sorted(mapped_gates)) or "none"),
            "Unmapped paths: {}".format(", ".join(sorted(unmapped_paths)) or "none"),
            "Plan changed: {}".format("yes" if plan_changed else "no"),
            "Excluded paths: {}".format(", ".join(excluded_paths) or "none"),
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
            )
        )


def _git_changed_paths(base_ref: str, repository_root: Path) -> List[str]:
    result = subprocess.run(
        ["git", "diff", "--name-only", "--no-renames", base_ref, "--"],
        cwd=str(repository_root),
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True,
    )
    if result.returncode != 0:
        raise RuntimeError("git diff failed for base {!r}: {}".format(base_ref, result.stderr.strip()))
    return sorted({line.strip().replace("\\", "/") for line in result.stdout.splitlines() if line.strip()})


def _git_worktree_changed_paths(repository_root: Path) -> List[str]:
    """Return staged, unstaged, and non-ignored untracked paths without rename hiding."""
    commands = (
        ["git", "diff", "--name-only", "-z", "--no-renames", "--"],
        ["git", "diff", "--cached", "--name-only", "-z", "--no-renames", "--"],
        ["git", "ls-files", "--others", "--exclude-standard", "-z"],
    )
    changed_paths = set()
    for command in commands:
        result = subprocess.run(
            command,
            cwd=str(repository_root),
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        if result.returncode != 0:
            raise RuntimeError(
                "git path inventory failed for {}: {}".format(
                    " ".join(command), os.fsdecode(result.stderr).strip()
                )
            )
        changed_paths.update(
            os.fsdecode(path).replace("\\", "/")
            for path in result.stdout.split(b"\0")
            if path
        )
    return sorted(changed_paths)


def _exclude_candidate_paths(
    candidates: Sequence[str], raw_exclusions: Sequence[str]
) -> Tuple[List[str], List[str]]:
    excluded_paths: List[str] = []
    baseline_text: Optional[str] = None
    for raw_path in raw_exclusions:
        path = raw_path.replace("\\", "/")
        if (
            not path
            or path.startswith("/")
            or re.match(r"^[A-Za-z]:/", path)
            or ".." in path.split("/")
        ):
            raise RuntimeError("excluded paths must be repository-relative: {}".format(raw_path))
        if path in excluded_paths:
            raise RuntimeError("duplicate excluded path: {}".format(path))
        matching = [
            candidate
            for candidate in candidates
            if candidate == path or (path.endswith("/") and candidate.startswith(path))
        ]
        if not matching:
            raise RuntimeError("excluded path matches no worktree candidates: {}".format(path))
        excluded_paths.append(path)
    included_paths = [
        candidate
        for candidate in candidates
        if not any(
            candidate == excluded
            or (excluded.endswith("/") and candidate.startswith(excluded))
            for excluded in excluded_paths
        )
    ]
    return included_paths, excluded_paths


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--full", action="store_true", help="validate the complete plan structure only")
    mode.add_argument("--base-ref", help="compare the working tree with this git base ref and enforce synchronization")
    mode.add_argument(
        "--worktree",
        action="store_true",
        help="inventory staged, unstaged, and non-ignored untracked paths",
    )
    mode.add_argument(
        "--paths",
        nargs="+",
        metavar="REPO_PATH",
        help="validate an explicit candidate path set, including untracked files",
    )
    parser.add_argument(
        "--exclude-path",
        action="append",
        default=[],
        metavar="REPO_PATH_OR_PREFIX/",
        help="exclude one exact generated path or repository-relative directory prefix from --worktree (repeatable)",
    )
    parser.add_argument(
        "--summary-file",
        help="append a GitHub Actions job summary report to this file",
    )
    args = parser.parse_args(argv)

    changed_paths: List[str] = []
    mapped_gates: Set[str] = set()
    unmapped_paths: Set[str] = set()
    errors: List[str] = []
    change_source = "full plan structure"
    text = ""
    fatal_error: Optional[str] = None
    excluded_paths: List[str] = []
    try:
        text = PLAN_PATH.read_text(encoding="utf-8")
        if args.exclude_path and not args.worktree:
            raise RuntimeError("--exclude-path is valid only with --worktree")
        if args.full:
            errors = validate_plan_structure(text)
        else:
            if args.base_ref:
                change_source = "base ref {}".format(args.base_ref)
                changed_paths = _git_changed_paths(args.base_ref, REPOSITORY_ROOT)
            elif args.worktree:
                change_source = "worktree inventory (staged, unstaged, non-ignored untracked)"
                candidates = _git_worktree_changed_paths(REPOSITORY_ROOT)
                changed_paths, excluded_paths = _exclude_candidate_paths(
                    candidates, args.exclude_path
                )
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
            baseline_ref = args.base_ref or "HEAD"
            baseline_text = _git_plan_at_ref(baseline_ref, REPOSITORY_ROOT)
            errors, mapped_gates, unmapped_paths = validate_changed_paths(
                text, changed_paths, baseline_text=baseline_text
            )
    except (OSError, RuntimeError) as error:
        fatal_error = str(error)
        errors.append(fatal_error)
        print("ERROR: {}".format(fatal_error), file=sys.stderr)

    print("Python binding plan sync: {}".format("FAIL" if errors else "PASS"))
    if args.base_ref or args.paths or args.worktree:
        if args.paths:
            print("Change source: explicit candidate paths")
        elif args.worktree:
            print("Change source: worktree inventory (staged, unstaged, non-ignored untracked)")
            print("Excluded paths: {}".format(", ".join(excluded_paths) or "none"))
        print("Changed paths:")
        for path in changed_paths:
            print("  {}".format(path))
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
    if args.base_ref or args.paths or args.worktree:
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
            )
        except OSError as error:
            print("cannot write plan job summary: {}".format(error), file=sys.stderr)
            return 1
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
