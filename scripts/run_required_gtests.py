#!/usr/bin/env python3
"""Run a required GoogleTest filter and reject empty, failed, or skipped runs."""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
from typing import Iterable, Sequence


_RUN_SUMMARY = re.compile(
    r"^\[==========\]\s+(\d+)\s+tests? from \d+ test suites? ran\.", re.MULTILINE
)
_SKIPPED = re.compile(r"^\[\s*SKIPPED\s*\]", re.MULTILINE)


def _discovered_count(output: str) -> int:
    return sum(
        1
        for line in output.splitlines()
        if line.startswith("  ")
        and line.strip()
        and not line.lstrip().startswith("#")
    )


def _discovered_suites(output: str) -> set[str]:
    return {
        line.strip()
        for line in output.splitlines()
        if line and not line[0].isspace() and line.strip().endswith(".")
    }


def _suite_names_from_filter(test_filter: str) -> set[str]:
    positive_filter = test_filter.partition("-")[0]
    suites = set()
    for pattern in positive_filter.split(":"):
        suite_pattern, separator, _test_pattern = pattern.partition(".")
        if (
            separator
            and suite_pattern
            and not any(wildcard in suite_pattern for wildcard in "*?")
        ):
            suites.add(suite_pattern + ".")
    return suites


def _invoke(
    command: Sequence[str], timeout_seconds: int
) -> subprocess.CompletedProcess:
    return subprocess.run(
        list(command),
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        errors="replace",
        check=False,
        timeout=timeout_seconds,
    )


def _write_partial_output(error: subprocess.TimeoutExpired) -> None:
    output = error.stdout if error.stdout is not None else error.output
    if isinstance(output, bytes):
        output = output.decode(errors="replace")
    if output:
        sys.stdout.write(output)


def run_required_filter(
    binary: str,
    test_filter: str,
    command_prefix: Iterable[str] = (),
    timeout_seconds: int = 1800,
    required_suites: Iterable[str] = (),
) -> int:
    if not test_filter:
        print("ERROR: required GoogleTest filter is empty", file=sys.stderr)
        return 2
    if timeout_seconds <= 0:
        print("ERROR: timeout must be a positive number of seconds", file=sys.stderr)
        return 2

    prefix = list(command_prefix)
    listing_command = prefix + [
        binary,
        "--gtest_list_tests",
        "--gtest_filter=" + test_filter,
    ]
    try:
        listing = _invoke(listing_command, timeout_seconds)
    except subprocess.TimeoutExpired as error:
        _write_partial_output(error)
        print(
            "ERROR: GoogleTest listing timed out after {} seconds: {}".format(
                timeout_seconds, test_filter
            ),
            file=sys.stderr,
        )
        return 1
    except OSError as error:
        print("ERROR: unable to list required tests: {}".format(error), file=sys.stderr)
        return 2

    if listing.returncode != 0:
        sys.stdout.write(listing.stdout)
        print(
            "ERROR: GoogleTest listing exited with {}".format(listing.returncode),
            file=sys.stderr,
        )
        return listing.returncode if listing.returncode > 0 else 1

    discovered = _discovered_count(listing.stdout)
    if discovered == 0:
        print(
            "ERROR: required GoogleTest filter discovered zero tests: {}".format(
                test_filter
            ),
            file=sys.stderr,
        )
        return 1
    expected_suites = set(required_suites) | _suite_names_from_filter(test_filter)
    missing_suites = sorted(expected_suites - _discovered_suites(listing.stdout))
    if missing_suites:
        print(
            "ERROR: required GoogleTest suites were not discovered: {}".format(
                ", ".join(missing_suites)
            ),
            file=sys.stderr,
        )
        return 1

    print(
        "Required GoogleTest filter discovered {} tests: {}".format(
            discovered, test_filter
        )
    )
    run_command = prefix + [
        binary,
        "--gtest_filter=" + test_filter,
        "--gtest_brief=1",
    ]
    try:
        result = _invoke(run_command, timeout_seconds)
    except subprocess.TimeoutExpired as error:
        _write_partial_output(error)
        print(
            "ERROR: required GoogleTest filter timed out after {} seconds: {}".format(
                timeout_seconds, test_filter
            ),
            file=sys.stderr,
        )
        return 1
    except OSError as error:
        print("ERROR: unable to run required tests: {}".format(error), file=sys.stderr)
        return 2

    sys.stdout.write(result.stdout)
    if result.returncode != 0:
        print(
            "ERROR: required GoogleTest filter exited with {}: {}".format(
                result.returncode, test_filter
            ),
            file=sys.stderr,
        )
        return result.returncode if result.returncode > 0 else 1

    summary = _RUN_SUMMARY.search(result.stdout)
    if summary is None:
        print(
            "ERROR: GoogleTest run summary was missing for filter: {}".format(
                test_filter
            ),
            file=sys.stderr,
        )
        return 1
    executed = int(summary.group(1))
    if executed == 0:
        print(
            "ERROR: required GoogleTest filter executed zero tests: {}".format(
                test_filter
            ),
            file=sys.stderr,
        )
        return 1
    if _SKIPPED.search(result.stdout):
        print(
            "ERROR: required GoogleTest filter skipped one or more tests: {}".format(
                test_filter
            ),
            file=sys.stderr,
        )
        return 1

    print(
        "Required GoogleTest filter passed: discovered {}, executed {}, skipped 0".format(
            discovered, executed
        )
    )
    return 0


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", required=True, help="GoogleTest executable")
    parser.add_argument("--filter", required=True, help="GoogleTest filter expression")
    parser.add_argument(
        "--require-suite",
        action="append",
        default=[],
        help="suite name (including trailing dot) that must appear in discovery; repeat as needed",
    )
    parser.add_argument(
        "--command-prefix",
        action="append",
        default=[],
        help="executable/argument to prepend, repeat once per argument",
    )
    parser.add_argument(
        "--timeout-seconds",
        type=int,
        default=1800,
        help="maximum time for test listing and execution (default: 1800)",
    )
    args = parser.parse_args(argv)
    return run_required_filter(
        args.binary,
        args.filter,
        args.command_prefix,
        args.timeout_seconds,
        args.require_suite,
    )


if __name__ == "__main__":
    raise SystemExit(main())
