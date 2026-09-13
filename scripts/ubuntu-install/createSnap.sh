#!/usr/bin/env bash
set -euo pipefail

# Build in a core26 LXD environment.  `pack` is the explicit Snapcraft 9
# command, and a fixed output name makes the install step depend on success.
snapcraft pack --use-lxd --output librecad.snap

sudo snap install ./librecad.snap --devmode --dangerous
