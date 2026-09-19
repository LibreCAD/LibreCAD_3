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
# One more thing has to be true before the pack can resolve anything at all.
# This runner's apt is configured for the x86-64-v3 micro-architecture variant:
#
#   APT::Architecture "amd64";
#   APT::Architecture-Variants "amd64v3";
#
# and its Ubuntu sources carry no Architectures: line, so every Ubuntu index on
# disk is binary-amd64v3 and there is no binary-amd64 index for those suites at
# all. Host apt copes, because the variant setting tells it to. snapcraft's
# staging cache is a separate apt rootdir resolving for plain amd64; it finds
# no index it can use, and reports the first name out of an unordered set:
#
#   Stage package not found in part 'librecad': <whichever came first>
#
# which reads like a typo in snapcraft.yaml and is not one. Three runs named
# three different packages -- python3, qt6-svg-dev, libpython3.14 -- and every
# one of them was installed on that very host at the time.
#
# So give apt a plain-amd64 view of the same mirror: same URIs, same keyring,
# same suites, with only the architecture spelled out.
# Unconditionally, and deliberately so. The first attempt guarded this on
# "does a *_binary-amd64_Packages list already exist", which is always true on
# this image and says nothing: the runner carries third-party repositories
# (Microsoft's is published under dists/resolute/) whose names match that
# pattern while the Ubuntu suites remain amd64v3-only. Writing the source every
# time costs one apt-get update and cannot be fooled by a neighbour's filename.
. /etc/os-release
sudo tee /etc/apt/sources.list.d/librecad-plain-amd64.sources >/dev/null <<SOURCE
Types: deb
URIs: mirror+file:/etc/apt/apt-mirrors.txt
Suites: ${VERSION_CODENAME} ${VERSION_CODENAME}-updates ${VERSION_CODENAME}-security
Components: main universe restricted multiverse
Architectures: amd64
Signed-By: /usr/share/keyrings/ubuntu-archive-keyring.gpg
SOURCE
sudo apt-get update

# And say whether it worked, because the failure mode is silent.
echo "Ubuntu plain-amd64 indexes now present:"
ls /var/lib/apt/lists/ | grep -E "apt-mirrors.*_binary-amd64_Packages" || \
    echo "  NONE -- the stage-package resolve below will find nothing"

if ! sudo env "PATH=$PATH" snapcraft pack --destructive-mode --output librecad.snap; then
    # "Stage package not found in part 'librecad': <name>" names whichever
    # package an unordered set yielded first, which says nothing about why.
    # These three lines say whether apt could see the archive at all, which is
    # the difference between a wrong name and an unreachable component.
    # "Stage package not found in part 'librecad': <name>" names whichever
    # package an unordered set yielded first, which says nothing about why --
    # and two different names have now been reported for packages that both
    # exist in the archive, which points at the staging cache seeing nothing
    # rather than at a wrong name. These dumps separate the two.
    echo "--- snapcraft pack failed ---"
    echo "--- apt, as this shell sees it ---"
    apt-cache policy libqt6svg6 python3 libpython3.14 || true
    echo "--- apt sources, in full ---"
    cat /etc/apt/sources.list 2>/dev/null || true
    cat /etc/apt/sources.list.d/*.sources /etc/apt/sources.list.d/*.list 2>/dev/null || true
    echo "--- apt architecture ---"
    apt-config dump 2>/dev/null | grep -iE "^APT::Architecture" || true
    echo "--- package lists apt has actually fetched ---"
    ls /var/lib/apt/lists/ 2>/dev/null | grep -i packages | head -20 || true
    echo "--- snapcraft's own execution log ---"
    # The glob has to be expanded as root, or the unprivileged shell resolves
    # it against a directory it cannot read and tail gets the literal pattern.
    sudo sh -c 'tail -120 /root/.local/state/snapcraft/log/*.log' 2>/dev/null || true
    exit 1
fi

# The pack ran as root, so the snap belongs to root; the upload step reads it
# as the runner user.
sudo chown "$(id -u):$(id -g)" librecad.snap

sudo snap install ./librecad.snap --devmode --dangerous
