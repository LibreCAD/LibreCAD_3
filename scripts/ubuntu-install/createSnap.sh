#!/usr/bin/env bash
set -euo pipefail

# GitHub runners receive LXD during the job, so their existing shell has not
# picked up the newly created lxd-group membership.  `sg` starts the group
# aware shell required by LXD without relying on an interactive logout/login.
getent group lxd >/dev/null || sudo groupadd --system lxd
if ! id -nG "$(id -un)" | tr ' ' '\n' | grep -qx lxd; then
    sudo usermod -aG lxd "$(id -un)"
fi

# Build in a core26 LXD environment. `pack` is the explicit Snapcraft 9
# command, and a fixed output name makes the install step depend on success.
sg lxd -c 'lxd init --minimal'
sg lxd -c 'snapcraft pack --use-lxd --output librecad.snap'

sudo snap install ./librecad.snap --devmode --dangerous
