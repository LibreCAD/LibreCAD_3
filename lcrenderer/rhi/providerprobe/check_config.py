#!/usr/bin/env python3
"""Run bounded native configure, loader and output negatives for a restored SDK."""
import argparse
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("cmake", "source", "prefix", "version", "output", "other-qt", "app", "plugins"):
        parser.add_argument("--" + name, required=True)
    args = parser.parse_args()
    root = Path(tempfile.mkdtemp(prefix="config-negatives-", dir=args.output))
    prefix = Path(args.prefix).resolve(strict=True)
    mac = sys.platform == "darwin"
    qt = prefix / "lib/cmake/Qt6" if mac else next(prefix.glob("lib/*/cmake/Qt6/Qt6Config.cmake")).parent
    other = Path(args.other_qt).resolve(strict=True)
    if other == qt or not (other / "Qt6Config.cmake").is_file():
        raise RuntimeError("Mixed-provider negative requires a distinct existing Qt installation")
    base = [args.cmake, "-S", args.source, "-DBUILD_TESTING=OFF",
            "-DPROBE_QT_PREFIX=" + str(prefix), "-DPROBE_QT_VERSION=" + args.version,
            "-DCMAKE_PREFIX_PATH=" + str(prefix), "-DQt6_DIR=" + str(qt)]
    cases = {
        "missing-prefix": (["-DPROBE_QT_PREFIX=" + str(root / "absent")], "Provider prefix must"),
        "missing-svg": (["-DCMAKE_DISABLE_FIND_PACKAGE_Qt6Svg=TRUE"], "Qt6Svg"),
        "wrong-version": (["-DPROBE_QT_VERSION=99.0.0"], "99.0.0"),
        "mixed-provider": (["-DQt6_DIR=" + str(other)], "Provider prefix mismatch"),
    }
    records = []
    for name, (flags, reason) in cases.items():
        command = base + ["-B", str(root / name)] + flags
        result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                text=True, timeout=45, check=False)
        (root / (name + ".log")).write_text(result.stdout)
        if result.returncode == 0 or reason not in result.stdout:
            raise RuntimeError("Configure negative did not fail for its reason: " + name)
        records.append({"case": name, "command": command, "exit_code": result.returncode,
                        "required_diagnostic": reason})
    command = [args.app, str(prefix), args.plugins]
    base_env = os.environ.copy()
    base_env.update(QT_QPA_PLATFORM="offscreen", QT_SCALE_FACTOR="1")
    runtime = {
        "foreign-runtime": ({"DYLD_FRAMEWORK_PATH" if mac else "LD_LIBRARY_PATH": str(other.parent.parent)}, command,
                            "loaded-qt-paths-private"),
        "foreign-plugins": ({}, [args.app, str(prefix), str(other.parents[2] / "plugins" if mac else other.parent.parent / "qt6/plugins")],
                            "Invalid private provider paths"),
        "wrong-qpa": ({"QT_QPA_PLATFORM": "xcb"}, command, "explicit offscreen QPA"),
    }
    for name, (changes, invocation, reason) in runtime.items():
        env = base_env.copy()
        env.update(changes)
        result = subprocess.run(invocation, env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                text=True, timeout=30, check=False)
        (root / (name + ".log")).write_text(result.stdout + result.stderr)
        if result.returncode != 1 or reason not in result.stderr or result.stdout:
            raise RuntimeError("Runtime negative did not fail without a report: " + name)
        records.append({"case": name, "command": invocation, "environment": changes,
                        "exit_code": result.returncode, "required_diagnostic": reason})
    if mac:
        read_fd, write_fd = os.pipe()
        os.close(read_fd)
        full = os.fdopen(write_fd, "wb")
    else:
        full = open("/dev/full", "wb")
    with full:
        result = subprocess.run(command, env=base_env, stdout=full, stderr=subprocess.PIPE,
                                text=True, timeout=30, check=False)
    (root / "report-write-error.log").write_text(result.stderr)
    if result.returncode != 1 or "report-write-failed" not in result.stderr:
        raise RuntimeError("Report write failure was accepted")
    records.append({"case": "report-write-error", "command": command, "stdout": "closed-pipe" if mac else "/dev/full",
                    "exit_code": result.returncode, "required_diagnostic": "report-write-failed"})
    (root / "checks.json").write_text(json.dumps(records, indent=2) + "\n")
    if len(records) != 8:
        raise RuntimeError("Missing native provider negatives")
    print("R0 native provider negatives: 4 configure + 4 runtime/output; " + str(root))


if __name__ == "__main__":
    main()
