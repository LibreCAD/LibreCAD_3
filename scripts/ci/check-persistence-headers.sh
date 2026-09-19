#!/usr/bin/env bash
#
# The public persistence headers must name no libdxfrw type.
#
# persistence links libdxfrw PRIVATE (persistence/CMakeLists.txt) so that the
# library's C++17 requirement stops there.  persistence/file.h is included
# transitively by lcluascript and lcpythonscript, which are built as gnu++14 and
# fail on kaguya's missing FunctionSignature specialisation for noexcept members
# the moment the standard is raised.  A DRW_* or dxfRW name in one of these
# headers would force every consumer to find libdxfrw's include path, and then
# to compile as C++17 -- a breakage that shows up far from the edit that caused
# it, in a target nobody was touching.
#
# Only the top-level headers are checked.  persistence/libdxfrw/ is the
# adapter: naming those types is its job.
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
offenders=()

# Comments may name these types -- explaining why a header must not depend on
# them is the whole point of the rule -- so only code lines are checked.
strip_comments() {
    sed -e 's://.*::' -e 's:/\*.*\*/::' "$1" | grep -vE '^[[:space:]]*\*'
}

for header in "$root"/persistence/*.h; do
    [ -e "$header" ] || continue
    if strip_comments "$header" | grep -Eq '(\bDRW_|\bdxfRW\b|\bDRW::)'; then
        offenders+=("$header")
    fi
done

if [ ${#offenders[@]} -ne 0 ]; then
    echo "error: libdxfrw types must not appear in the public persistence headers:" >&2
    for header in "${offenders[@]}"; do
        echo "  ${header#"$root"/}" >&2
        strip_comments "$header" | grep -En '(\bDRW_|\bdxfRW\b|\bDRW::)' | sed 's/^/    /' >&2
    done
    exit 1
fi

echo "persistence headers name no libdxfrw type"
