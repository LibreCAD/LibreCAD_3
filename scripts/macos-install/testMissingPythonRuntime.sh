#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    echo "Usage: $0 <LibreCAD.app>" >&2
    exit 2
fi

source_app=$1
if [[ ! -d "$source_app/Contents" || ! -x "$source_app/Contents/MacOS/librecad" ]]; then
    echo "A built LibreCAD app bundle is required: $source_app" >&2
    exit 2
fi

temporary_root=$(mktemp -d "${TMPDIR:-/tmp}/librecad-missing-python.XXXXXX")
trap 'rm -rf "$temporary_root"' EXIT

test_app="$temporary_root/LibreCAD missing Python Ω.app"
ditto "$source_app" "$test_app"

python_root="$test_app/Contents/Resources/librecad/python"
if [[ ! -d "$python_root" ]]; then
    echo "Bundled CPython runtime is already missing: $python_root" >&2
    exit 1
fi
mv "$python_root" "$temporary_root/python-disabled"

poison=/__librecad_python_environment_must_not_be_used__
app_binary="$test_app/Contents/MacOS/librecad"
if output=$(env \
    QT_QPA_PLATFORM=offscreen \
    PYTHONHOME="$poison" \
    PYTHONPATH="$poison" \
    QT_PLUGIN_PATH="$poison" \
    DYLD_LIBRARY_PATH="$poison" \
    LIBRECAD_SETTINGS_DIR="$temporary_root/settings" \
    "$app_binary" --python-smoke 2>&1); then
    echo "LibreCAD unexpectedly started without its staged CPython runtime" >&2
    exit 1
else
    status=$?
fi

if [[ $status -ne 1 ]]; then
    printf 'Expected a clean startup failure with exit 1; got %s:\n%s\n' \
        "$status" "$output" >&2
    exit 1
fi

expected="staged CPython runtime is missing beside the application"
if [[ "$output" != *"$expected"* ]]; then
    printf 'Startup failed without the expected runtime diagnostic:\n%s\n' \
        "$output" >&2
    exit 1
fi

printf '%s\n' "$output"
echo "Missing-runtime smoke passed: startup failed closed with exit 1."
