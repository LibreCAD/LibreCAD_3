#!/usr/bin/env bash
# Single definition of the CI configure arguments.
#
# The CodeQL workflow configures the SAME build directory twice: once before
# "Initialize CodeQL" to build the third-party libdxfrw target outside the
# traced window, and once during the traced build. If the two invocations
# differ by even one flag, CMake rewrites flags.make, every libdxfrw
# translation unit recompiles inside the traced window, and third-party
# findings reappear as alerts on this repository -- silently. Sourcing one
# script from both steps is what keeps them identical.
# pipefail matters to the caller: the traced build pipes make through tee, and
# without it a failed build would be masked by tee's exit status.
set -e
set -o pipefail

mkdir -p build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_SHARED_LIBS=On
