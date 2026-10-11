#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    echo "Usage: $0 <AppImage> (inside the disposable runtime container)" >&2
    exit 2
fi
if [[ ! -r "$1" || ! -f "$1" ]]; then
    echo "AppImage is missing or unreadable: $1" >&2
    exit 1
fi
if [[ $(uname -m) != x86_64 ]]; then
    echo "The AppImage runtime check requires x86_64" >&2
    exit 1
fi
if command -v python3 >/dev/null 2>&1; then
    echo "Clean runtime unexpectedly contains a system Python executable" >&2
    exit 1
fi
system_libraries=$(ldconfig -p)
if grep -q 'libpython[0-9]' <<< "$system_libraries"; then
    echo "Clean runtime unexpectedly contains a system libpython" >&2
    exit 1
fi
packages=$(dpkg-query -W -f='${binary:Package}\n')
if grep -Eq '^(lib)?python[0-9][^[:space:]]*-dev(:[^[:space:]]+)?$' <<< "$packages"; then
    echo "Clean runtime unexpectedly contains a Python SDK" >&2
    exit 1
fi

sha256sum "$1"
printf 'Clean runtime: %s / %s; no system Python executable, library or SDK\n' \
    "$(uname -m)" "$(getconf GNU_LIBC_VERSION)"
runtime_dir=$(mktemp -d '/tmp/LibreCAD runtime Ω.XXXXXX')
trap 'rm -rf -- "$runtime_dir"' EXIT
cp "$1" "$runtime_dir/LibreCAD.AppImage"
chmod +x "$runtime_dir/LibreCAD.AppImage"
cd /tmp
timeout --kill-after=10s 180s xvfb-run -a env -u QT_PLUGIN_PATH -u QT_QPA_PLATFORM_PLUGIN_PATH \
    -u LD_LIBRARY_PATH -u LD_PRELOAD \
    QT_QPA_PLATFORM=xcb \
    PYTHONHOME=/__librecad_python_poison__ \
    PYTHONPATH=/__librecad_python_poison__ \
    LIBRECAD_REQUIRE_BUNDLED_PYTHON=1 \
    LIBRECAD_REQUIRE_PYTHON_EXTENSION_CLOSURE=1 \
    LIBRECAD_SETTINGS_DIR="$runtime_dir/settings" \
    "$runtime_dir/LibreCAD.AppImage" --appimage-extract-and-run --python-smoke
