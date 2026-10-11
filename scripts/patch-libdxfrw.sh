#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    echo "usage: $0 <libdxfrw-source-directory>" >&2
    exit 2
fi

source_dir=$(cd "$1" && pwd)
repository_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
expected_revision=f7debf4c5f30734a70c5e828cf05247aabc2d04b
actual_revision=$(git -C "$source_dir" rev-parse HEAD)

if [[ "$actual_revision" != "$expected_revision" ]]; then
    echo "libdxfrw revision mismatch: expected $expected_revision, got $actual_revision" >&2
    exit 1
fi

for patch_file in "$repository_root"/patches/libdxfrw-*.patch; do
    if [[ ! -f "$patch_file" ]]; then
        echo "No libdxfrw patches found under $repository_root/patches" >&2
        exit 1
    fi

    if git -C "$source_dir" apply --check "$patch_file"; then
        git -C "$source_dir" apply "$patch_file"
    elif git -C "$source_dir" apply --reverse --check "$patch_file"; then
        echo "libdxfrw patch already applied: $(basename "$patch_file")"
    else
        echo "libdxfrw patch no longer applies cleanly: $patch_file" >&2
        exit 1
    fi
done
