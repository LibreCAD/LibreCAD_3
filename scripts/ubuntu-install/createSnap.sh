#!/usr/bin/env bash
set -euo pipefail

# The workflow uses an Ubuntu 26.04 runner, matching core26 directly.  This
# avoids a nested LXD image download, which GitHub-hosted runners can block.
# `pack` is the explicit Snapcraft 9 command, and a fixed output name makes
# the install step depend on success.
snapcraft pack --destructive-mode --output librecad.snap

sudo snap install ./librecad.snap --devmode --dangerous
