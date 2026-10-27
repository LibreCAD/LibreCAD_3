#!/usr/bin/env bash
#
# Every DXF read callback must run inside DXFimpl::guarded().
#
# libdxfrw's read path is not a function-try-block the way its write path is:
# an exception thrown inside a callback leaves dxfRW::read() with
# getError() == BAD_NONE, so the caller is told the file read cleanly while the
# process is already unwinding.  LibreCAD's own kernel throws from inside these
# callbacks, and one bad record used to end the program.
#
# A new callback added without the guard restores exactly that behaviour, and
# no test can catch it: with the known crashes fixed there is no DXF record
# left that reaches a throw, so an unguarded callback looks fine until a file
# in the wild finds it.  This check is the only thing that notices.
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
impl="$root/persistence/libdxfrw/dxfimpl.cpp"
unguarded=()

# Read callbacks are the add*/link* entry points; endBlock and setBlock touch
# no kernel constructor and are exempt by name.
while read -r line; do
    method="${line%%(*}"
    method="${method##*::}"
    case "$method" in
        endBlock|setBlock) continue ;;
    esac

    # The first statement of the body must be the guard.
    if ! awk -v want="^void DXFimpl::${method}\\\\(" '
        $0 ~ want { found = 1; next }
        found { print; exit }
    ' "$impl" | grep -q 'guarded('; then
        unguarded+=("$method")
    fi
done < <(grep -E '^void DXFimpl::(add|link)[A-Za-z0-9]*\(' "$impl")

if [ ${#unguarded[@]} -ne 0 ]; then
    echo "error: these DXF read callbacks do not run inside DXFimpl::guarded():" >&2
    printf '  %s\n' "${unguarded[@]}" >&2
    exit 1
fi

echo "every DXF read callback is guarded"
