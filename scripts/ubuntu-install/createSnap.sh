#!/usr/bin/env bash
set -euo pipefail

# The workflow uses an Ubuntu 26.04 runner, matching core26 directly.  This
# avoids a nested LXD image download, which GitHub-hosted runners can block.
# `pack` is the explicit Snapcraft 9 command, and a fixed output name makes
# the install step depend on success.
# Snapcraft installs the base and every content default-provider as a
# build-snap.  In --destructive-mode it does that on the host by shelling out
# to plain `snap install`, with no sudo (craft-parts snaps.py: the argv is
# ["snap", "install", name]) -- and as the unprivileged CI user that call is
# denied.  craft-parts discards snapd's stdout and stderr, so all that reaches
# the log is its own generic line:
#
#   Error installing snap 'mesa-2604' from channel 'latest/stable'.
#
# which reads like a store outage and is not one: both snaps are published and
# current.  The first line of the same log is the real clue, emitted whenever
# --destructive-mode runs with euid != 0:
#
#   Running in destructive mode as a non-super user is not recommended
#
# Installing them here with sudo makes snapcraft skip its own attempt -- it
# only installs a build-snap when one is absent -- and any genuine failure
# then prints snapd's actual error instead of the swallowed one.
#
# Which snap trips first depends on what the runner already has: observed
# 'core26' on two runs and 'mesa-2604' on a third, so neither is assumed.
# `snap install` fails on an already-installed snap, hence the guard.
for build_snap in core26 mesa-2604; do
    if ! snap list "$build_snap" >/dev/null 2>&1; then
        sudo snap install "$build_snap"
    fi
done

snapcraft pack --destructive-mode --output librecad.snap

sudo snap install ./librecad.snap --devmode --dangerous
