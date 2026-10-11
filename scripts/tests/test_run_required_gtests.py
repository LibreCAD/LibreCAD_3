"""Behavior tests for the cross-platform required GoogleTest runner."""

import io
import subprocess
import unittest
from contextlib import redirect_stderr, redirect_stdout
from types import SimpleNamespace
from unittest.mock import call, patch

from scripts.run_required_gtests import run_required_filter


class RunRequiredGoogleTests(unittest.TestCase):
    def _run(
        self,
        results,
        test_filter="Sample.*",
        command_prefix=(),
        timeout_seconds=1800,
        required_suites=(),
    ):
        output = io.StringIO()
        errors = io.StringIO()
        with patch("scripts.run_required_gtests._invoke", side_effect=results) as invoke:
            with redirect_stdout(output), redirect_stderr(errors):
                status = run_required_filter(
                    "lcunittest",
                    test_filter,
                    command_prefix,
                    timeout_seconds,
                    required_suites,
                )
        return status, output.getvalue(), errors.getvalue(), invoke

    def test_success_requires_discovery_and_execution_and_preserves_prefix(self):
        status, output, errors, invoke = self._run(
            [
                SimpleNamespace(returncode=0, stdout="Sample.\n  First\n  Second\n"),
                SimpleNamespace(
                    returncode=0,
                    stdout=(
                        "[==========] 2 tests from 1 test suite ran. (1 ms total)\n"
                        "[  PASSED  ] 2 tests.\n"
                    ),
                ),
            ],
            command_prefix=("xvfb-run", "-a"),
            required_suites=("Sample.",),
        )

        self.assertEqual(0, status)
        self.assertEqual("", errors)
        self.assertIn("discovered 2, executed 2, skipped 0", output)
        self.assertEqual(
            call(
                [
                    "xvfb-run",
                    "-a",
                    "lcunittest",
                    "--gtest_list_tests",
                    "--gtest_filter=Sample.*",
                ],
                1800,
            ),
            invoke.call_args_list[0],
        )
        self.assertEqual(
            call(
                [
                    "xvfb-run",
                    "-a",
                    "lcunittest",
                    "--gtest_filter=Sample.*",
                    "--gtest_brief=1",
                ],
                1800,
            ),
            invoke.call_args_list[1],
        )

    def test_zero_discovered_tests_fail_without_running_binary(self):
        status, output, errors, invoke = self._run(
            [SimpleNamespace(returncode=0, stdout="Sample.\n")]
        )

        self.assertEqual(1, status)
        self.assertIn("discovered zero tests", errors)
        self.assertEqual(1, invoke.call_count)

    def test_missing_required_suite_fails_from_bounded_discovery(self):
        status, output, errors, invoke = self._run(
            [SimpleNamespace(returncode=0, stdout="Sample.\n  First\n")],
            required_suites=("Sample.", "RequiredSuite."),
        )

        self.assertEqual(1, status)
        self.assertIn("RequiredSuite.", errors)
        self.assertEqual(1, invoke.call_count)

    def test_filter_suite_that_matches_zero_tests_fails_closed(self):
        status, output, errors, invoke = self._run(
            [SimpleNamespace(returncode=0, stdout="Sample.\n  First\n")],
            test_filter="Sample.*:MissingSuite.*",
        )

        self.assertEqual(1, status)
        self.assertIn("MissingSuite.", errors)
        self.assertEqual(1, invoke.call_count)

    def test_nonzero_test_process_fails(self):
        status, output, errors, invoke = self._run(
            [
                SimpleNamespace(returncode=0, stdout="Sample.\n  First\n"),
                SimpleNamespace(returncode=3, stdout="[  FAILED  ] First\n"),
            ]
        )

        self.assertEqual(3, status)
        self.assertIn("[  FAILED  ] First", output)
        self.assertIn("exited with 3", errors)

    def test_signal_terminated_test_process_returns_conventional_failure(self):
        status, output, errors, invoke = self._run(
            [
                SimpleNamespace(returncode=0, stdout="Sample.\n  First\n"),
                SimpleNamespace(returncode=-11, stdout="process terminated\n"),
            ]
        )

        self.assertEqual(1, status)
        self.assertIn("exited with -11", errors)

    def test_zero_executed_tests_fail_even_after_nonempty_listing(self):
        status, output, errors, invoke = self._run(
            [
                SimpleNamespace(returncode=0, stdout="Sample.\n  First\n"),
                SimpleNamespace(
                    returncode=0,
                    stdout="[==========] 0 tests from 0 test suites ran.\n",
                ),
            ]
        )

        self.assertEqual(1, status)
        self.assertIn("executed zero tests", errors)

    def test_skipped_test_fails_even_when_gtest_returns_success(self):
        status, output, errors, invoke = self._run(
            [
                SimpleNamespace(returncode=0, stdout="Sample.\n  First\n"),
                SimpleNamespace(
                    returncode=0,
                    stdout=(
                        "[==========] 1 test from 1 test suite ran.\n"
                        "[  SKIPPED ] 1 test.\n"
                    ),
                ),
            ]
        )

        self.assertEqual(1, status)
        self.assertIn("skipped one or more tests", errors)

    def test_missing_gtest_summary_fails_closed(self):
        status, output, errors, invoke = self._run(
            [
                SimpleNamespace(returncode=0, stdout="Sample.\n  First\n"),
                SimpleNamespace(
                    returncode=0, stdout="test process produced no summary\n"
                ),
            ]
        )

        self.assertEqual(1, status)
        self.assertIn("summary was missing", errors)

    def test_listing_timeout_fails_with_partial_output(self):
        status, output, errors, invoke = self._run(
            [
                subprocess.TimeoutExpired(
                    ["lcunittest"], timeout=3, output=b"listing stalled\n"
                )
            ],
            timeout_seconds=3,
        )

        self.assertEqual(1, status)
        self.assertIn("listing stalled", output)
        self.assertIn("listing timed out after 3 seconds", errors)
        self.assertEqual(1, invoke.call_count)

    def test_test_execution_timeout_fails_with_partial_output(self):
        status, output, errors, invoke = self._run(
            [
                SimpleNamespace(returncode=0, stdout="Sample.\n  First\n"),
                subprocess.TimeoutExpired(
                    ["lcunittest"], timeout=4, output="test stalled\n"
                ),
            ],
            timeout_seconds=4,
        )

        self.assertEqual(1, status)
        self.assertIn("test stalled", output)
        self.assertIn("filter timed out after 4 seconds", errors)


if __name__ == "__main__":
    unittest.main()
