#!/usr/bin/env bash
# Refresh preinstalled CI kegs too: their original bottle cache may be absent.
set -euo pipefail

if [[ "${GITHUB_ACTIONS:-}" != "true" ]]; then
    echo "CI-only dependency refresh; refusing to replace a local Homebrew SDK." >&2
    exit 2
fi

export HOMEBREW_NO_INSTALL_CLEANUP=1
export HOMEBREW_NO_AUTO_UPDATE=1
export HOMEBREW_NO_INSTALLED_DEPENDENTS_CHECK=1

roots=(cmake boost eigen freetype glew glfw glm googletest lua@5.4 rapidjson qt python@3.14)
# --os selects current declared dependencies, not stale installed runtime tabs.
# Capture separately so a failed discovery cannot disappear in process substitution.
dependencies="$(brew deps --union --topological --os=macos "${roots[@]}")"
formulae=()
while IFS= read -r formula; do
    formulae+=("$formula")
done < <(printf '%s\n' "$dependencies" "${roots[@]}" | awk 'NF && !seen[$0]++')

# Every root/dependency is an explicit bottle-only argument, including existing
# kegs. Retain the newly poured receipt and its actual OCI/archive inputs together.
brew reinstall --formula --force-bottle "${formulae[@]}"
