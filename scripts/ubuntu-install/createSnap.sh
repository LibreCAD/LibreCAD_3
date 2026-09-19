#!/usr/bin/env bash
set -euo pipefail

# The workflow uses an Ubuntu 26.04 runner, matching core26 directly.  This
# avoids a nested LXD image download, which GitHub-hosted runners can block.
# `pack` is the explicit Snapcraft 9 command, and a fixed output name makes
# the install step depend on success.
#
# --destructive-mode builds on the host instead of in a container, which means
# snapcraft itself has to do the things a container would have done as root:
# install the base and every content default-provider as a build-snap, and
# fetch the part's stage-packages through apt.  Neither works as the
# unprivileged CI user, and neither says so.
#
#   * Build-snaps go through a plain `snap install` with no sudo, and
#     craft-parts discards snapd's stdout and stderr, so all that reaches the
#     log is its own generic line:
#
#       Error installing snap 'mesa-2604' from channel 'latest/stable'.
#
#     which reads like a store outage and is not one -- both snaps are
#     published and current.
#
#   * Stage-packages go through craft_parts.packages.deb, whose
#     refresh_packages_list() returns early with
#
#       Packages list not refreshed, not running as superuser.
#
#     and then fails on the first name it cannot resolve out of an unordered
#     set:
#
#       Stage package not found in part 'librecad': python3
#
#     which reads like a missing dependency and is not one.
#
# The first line of the same log is the real clue, emitted whenever
# --destructive-mode runs with euid != 0:
#
#   Running in destructive mode as a non-super user is not recommended
#
# So run the pack as root and both symptoms go away together.  `env PATH=`
# because sudo replaces PATH from secure_path, and snapcraft lives in
# /snap/bin.
# sudo drops -E on this host ("preserving the entire environment is not
# supported"), so PATH is passed explicitly: sudo replaces it from secure_path
# and snapcraft lives in /snap/bin.
if ! sudo env "PATH=$PATH" snapcraft pack --destructive-mode --output librecad.snap; then
    # "Stage package not found in part 'librecad': <name>" names whichever
    # package an unordered set yielded first, which says nothing about why.
    # These three lines say whether apt could see the archive at all, which is
    # the difference between a wrong name and an unreachable component.
    echo "--- snapcraft pack failed; what apt can see from here ---"
    apt-cache policy libqt6svg6 python3 || true
    grep -rhE "^(deb|Suites:|Components:|URIs:)" /etc/apt/sources.list /etc/apt/sources.list.d/ 2>/dev/null || true
    exit 1
fi

# The pack ran as root, so the snap belongs to root; the upload step reads it
# as the runner user.
sudo chown "$(id -u):$(id -g)" librecad.snap

sudo snap install ./librecad.snap --devmode --dangerous
