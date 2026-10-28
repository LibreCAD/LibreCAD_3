# Linux and macOS Qt provider preflight

This R0 target checks the full Qt module set needed by the current application,
UI tests and retained legacy surface, independently of production integration.
It creates no application graphics device, shows no window and sends no desktop
input. Native ARM64 proof does not qualify other architectures or deployment floors.

The executable checks matching build/runtime Qt, UiTools form/properties, an actual
Qt Test signal, original 4×4 red SVG pixels through the SVG renderer/image/icon
plugins, private RHI headers and legacy OpenGL/OpenGLWidgets **linkage only**.
Every loaded Qt library/plugin must resolve under the supplied private prefix.
It emits 32 named checks; a nonzero process exit invalidates any partial stdout.
Use the separate RHI spike for Vulkan/Metal execution and graphical sessions.

## Native macOS inputs and checks

[qt683-macos.json](../providers/qt683-macos.json) pins the four official Qt 6.8.3
Base, ShaderTools, SVG and Tools archives and 22 raw reference files. Use the
[shared restoration command](../spike/README.md#restore-the-minimum-mac-provider)
with a new SDK directory. It verifies all inputs before extraction and all
references before/after native qsb, refusing input/target drift and foreign tool
environment overrides. Twenty-six mocked CPU fixtures are not native qualification.

The native CPU probe observes actual frameworks/plugins with dyld; it never
creates an OpenGL context, Metal device or visible window. UiTools imports
OpenGLWidgets in this provider too; module linkage does not qualify a legacy GL
context. Reuse the existing AGL consumer-link correction on newer Apple SDKs;
do not patch Qt binaries or weaken signatures. See [Qt UI Tools](https://doc.qt.io/qt-6.8/qtuitools-index.html).

Supply a second distinct, same-version restored SDK for the mixed-provider
negatives. It must contain real frameworks and Qt6 CMake files, not a synthetic
configuration file. With native ARM64 execution access:

```sh
cmake -S lcrenderer/rhi/providerprobe -B /absolute/new-mac-module-build \
  -DCMAKE_BUILD_TYPE=Release -DPROBE_QT_VERSION=6.8.3 \
  -DPROBE_QT_PREFIX=/absolute/new-sdk -DCMAKE_PREFIX_PATH=/absolute/new-sdk \
  -DQt6_DIR=/absolute/new-sdk/lib/cmake/Qt6 \
  -DPROBE_OTHER_QT=/absolute/other-sdk/lib/cmake/Qt6 \
  -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build /absolute/new-mac-module-build --parallel 3
ctest --test-dir /absolute/new-mac-module-build --no-tests=error \
  --output-on-failure --output-junit /absolute/mac-module-tests.xml
QT_QPA_PLATFORM=offscreen QT_SCALE_FACTOR=1 \
  /absolute/new-mac-module-build/lcrhiproviderprobe /absolute/new-sdk \
  /absolute/new-sdk/plugins > /absolute/mac-modules.json
```

Require the same exact two CTest IDs and unskipped XML as Linux. Mac negatives
use `DYLD_FRAMEWORK_PATH` and a closed output pipe instead of Linux's loader
variable and `/dev/full`; neither test exhausts a disk. Native Intel execution
requires an Intel host and the corresponding build architecture. The Mac CI
legs restore a second SDK from the same verified inputs, build this target with
`BUILD_TESTING=ON`, and require exact discovery/XML, all 32 check IDs, matching
host/producer architecture, a non-translated process and private loaded Qt files.
The native-process guard follows [Apple's Rosetta check](https://developer.apple.com/documentation/apple-silicon/about-the-rosetta-translation-environment);
unavailable query permission fails rather than accepting an architecture label.
Both restorations and
their references are rechecked after the build. Retained identities/diagnostics
exclude SDK payloads. These definitions do not constitute hosted execution,
graphics/session qualification or Windows module coverage.

## Restore retained Linux binary inputs

[qt6102-linux-arm64.json](../providers/qt6102-linux-arm64.json) pins 51 raw Ubuntu
packages and 19 reference files. Obtain those exact HTTPS inputs with their
recorded filenames, or use the retained archive directory. Never substitute a
newer apt candidate. Use the same targeted offline helper as the ARM64 R0 CI leg:

```sh
python3 -B scripts/restore_renderer_qt6102_linux_arm64.py \
  --archives /absolute/archives --destination /absolute/new-sdk \
  > /absolute/provider-restore.json
```

The helper requires native Linux ARM64. It verifies every raw package before
extraction, refuses existing destinations, checks the restored references, then
runs the exact qsb and verifies its actual private Qt loader paths. Changes to
inputs/references/pin/helper, failed extraction/tool execution or a mismatched
version fail without success JSON. It runs no maintainer scripts, installs
nothing globally and never compiles dependencies. Keep failed new directories
for diagnosis; retry with a different new destination. Twenty-five CPU fixtures
exercise failure paths with mocked tools, not native provider acceptance.

The R0 workflow fetches only the pinned HTTPS inputs for ARM64, shares this helper,
requires the native module CTests below, and passes the restored prefix to both
RHI builds/matrices. It retains restore/module identities and diagnostics, not
unreviewed SDK payloads. Linux x64 keeps the existing distro route; it has no
qualified full-module pin yet. YAML definitions are not hosted execution evidence.

The input hash check is not a redistribution/license review. Non-Qt system
libraries, the compiler/CRT, loader/ICD, compositor and OS remain separate inputs.

## Build and validate

Use a native ARM64 Linux environment with the matching non-Qt prerequisites:

```sh
provider_prefix=/absolute/new-sdk/usr
export LD_LIBRARY_PATH="$provider_prefix/lib/aarch64-linux-gnu"
export QT_QPA_PLATFORM=offscreen QT_SCALE_FACTOR=1
cmake -S lcrenderer/rhi/providerprobe -B /absolute/new-provider-build \
  -DCMAKE_BUILD_TYPE=Release -DPROBE_QT_VERSION=6.10.2 \
  -DPROBE_QT_PREFIX="$provider_prefix" -DCMAKE_PREFIX_PATH="$provider_prefix" \
  -DQt6_DIR="$provider_prefix/lib/aarch64-linux-gnu/cmake/Qt6"
cmake --build /absolute/new-provider-build --parallel 3
/absolute/new-provider-build/lcrhiproviderprobe "$provider_prefix" \
  "$provider_prefix/lib/aarch64-linux-gnu/qt6/plugins" > /absolute/modules.json
ctest --test-dir /absolute/new-provider-build --no-tests=error \
  --output-on-failure --output-junit /absolute/provider-tests.xml
```

Require exactly `r0-qt-required-modules` and `r0-qt-provider-negatives` in discovery
and successful, unskipped XML. The latter retains four configure failures (missing
prefix/module, wrong version, foreign Qt root) and four runtime/output failures
(foreign runtime/plugin path, wrong QPA, failed report write). Its comparison
requires the existing system Qt installation at the recorded native triplet;
missing comparison capacity is a failure, not a skipped pass. Logs/commands remain
in a new `config-negatives-*` build subdirectory. No real disk exhaustion is used.

Retain `provider-imports.txt`, resolved non-Qt libraries, raw observations, source
and tool hashes. The package's UiTools imports OpenGL/OpenGLWidgets transitively;
that is not a hidden LibreCAD legacy surface or a successful legacy context test.
These module records cannot satisfy RHI/production/GTest/package evidence kinds.
Repeat the existing [spike](../spike/README.md) main/Gui-only builds and both
XCB/managed-Wayland software matrices with this prefix, retaining actual loader
paths. Do not select this current provider as the Linux minimum, waive the failed
6.8.3 Wayland candidate, or change a production default from these results.
