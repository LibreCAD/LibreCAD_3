#!/usr/bin/env bash
#
# LibreCAD 3 reads DWG and does not write it. See docs/dwg-support.md.
#
# This asserts the two halves of that position still agree with each other:
#
#   * LibreCAD offers no writable DWG variant, and
#   * libdxfrw does not claim its DWG writing is qualified.
#
# The failure worth catching is those two drifting apart -- LibreCAD writing a
# format the library itself still calls experimental. The reverse (the library
# promoting its claims) is not a failure; it is the news that gate 1 has moved,
# and the job reports it.
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
status_file="$root/third_party/libdxfrw/metadata/qualified-format-status-v1.json"
claims_file="$root/third_party/libdxfrw/metadata/qualified-format-claims-v1.json"

if [ ! -f "$status_file" ] || [ ! -f "$claims_file" ]; then
    echo "error: the libdxfrw format ledger is missing; is the submodule checked out?" >&2
    exit 1
fi

# Does LibreCAD offer to write DWG? The format table is the single place that
# decides, so it is the single place to ask.
if grep -E '^\s*\{"dwg' "$root/persistence/format.cpp" | grep -qE ',\s*true,\s*[a-zA-Z0-9_ !=]*,\s*true,'; then
    writable=yes
else
    writable=no
fi

promoted=$(python3 - "$status_file" "$claims_file" <<'PY'
import json, sys, collections

status = json.load(open(sys.argv[1]))
claims = json.load(open(sys.argv[2]))

# The status file records a status per claim id and says nothing about what the
# claim is for; the direction lives in the claims file. Counting every PROMOTED
# entry watched the wrong thing: every claim in the ledger is direction "read",
# so the gate could be tripped by the reader becoming qualified and could never
# be moved by anything about writing -- the one thing it exists to watch.
direction = {claim["id"]: claim.get("tuple", {}).get("direction")
             for claim in claims.get("claims", [])}

counts = collections.Counter()
write_claims = 0
for entry in status.get("claimStatus", []):
    if direction.get(entry["claimId"]) != "write":
        continue
    write_claims += 1
    counts[entry["status"]] += 1

if write_claims == 0:
    print("  the ledger carries no DWG write claim at all", file=sys.stderr)
else:
    for name in sorted(counts):
        print(f"  {name}: {counts[name]}", file=sys.stderr)
print(counts.get("PROMOTED", 0))
PY
)

echo "libdxfrw DWG format claims:" >&2
echo "LibreCAD offers a writable DWG variant: $writable"
echo "libdxfrw DWG write claims promoted: $promoted"

if [ "$writable" = "yes" ] && [ "$promoted" -eq 0 ]; then
    echo "error: LibreCAD offers DWG writing while libdxfrw promotes no DWG format claim." >&2
    echo "       See docs/dwg-support.md for the three gates that have to be met first." >&2
    exit 1
fi

if [ "$promoted" -gt 0 ]; then
    echo "note: libdxfrw now promotes $promoted DWG format claim(s)." >&2
    echo "      Gate 1 in docs/dwg-support.md has moved; gates 2 and 3 are human." >&2
fi

echo "DWG write position holds"
