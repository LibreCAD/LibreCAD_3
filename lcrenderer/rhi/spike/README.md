# Standalone R0 rendering proof

This builds the migration plan's feasibility executables without the application,
kernel, scripting runtime, legacy GL renderer or CAD dependencies. It does not
implement `LcPainter`, change a default, or qualify production rendering/packages.
The shared scene uses triangles and an embedded texture atlas, offline `.qsb`
shaders and explicit readback. All fixture geometry, bitmap glyphs and image pixels
are original project fixtures under the repository [LICENSE](../../../LICENSE);
no external font/image is downloaded. Bitmap `CAD` text is not shaping/fallback proof.

## Build

Use CMake 3.21+, C++17 and a matching Qt 6.8+ Gui/private-header/ShaderTools/qsb
provider. Widgets is required only with `WITH_SPIKE_WIDGET=ON` (the default).
Linux/Windows also need a Vulkan-enabled Qt, Vulkan development headers/loader and
a working ICD; macOS needs Metal-enabled Qt and an accessible native GPU.
No SDK/compiler is invoked by the running executables. Do not configure in-source.

```sh
cmake -S lcrenderer/rhi/spike -B /tmp/librecad-rhi-build \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/absolute/qt-prefix
cmake --build /tmp/librecad-rhi-build --config Release --parallel 4
```

The targets are `lcrhispike_offscreen` and `lcrhispike_widget`. Multi-configuration
generators place executables under `Release/`. To prove Gui-only compilation,
configure a fresh build with `-DWITH_SPIKE_WIDGET=OFF`. Build inputs, shader-tool
identity and effective spike options are embedded in each binary; source edits
trigger regeneration. The source-tree validator rejects stale/incomplete input
manifests. These development reports deliberately do not certify relocated packages.

### Restore the minimum Mac provider

The dedicated `scripts/restore_renderer_qt683_macos.py` helper restores the pinned
Qt 6.8.3 required-module inputs without downloading, compiling or globally
installing dependencies. Supply `qtbase.7z`, `qtshadertools.7z`, `qtsvg.7z` and
`qttools.7z` from the URLs or
retained inputs in `lcrenderer/rhi/providers/qt683-macos.json`:

```sh
python3 -B scripts/restore_renderer_qt683_macos.py \
  --archives /absolute/provider-archives --destination /absolute/new-qt-sdk \
  > /absolute/provider-restore.json
cmake -S lcrenderer/rhi/spike -B /absolute/new-spike-build \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/absolute/new-qt-sdk \
  -DQt6_DIR=/absolute/new-qt-sdk/lib/cmake/Qt6
```

The destination must not exist. Every archive size/hash is checked before
extraction; 22 reference binaries/private headers/CMake files are checked before
and after the native `qsb --version` probe. The tool does not inherit DYLD/plugin
overrides. Changed inputs, failed extraction/tool execution
or a mismatched version fail without a success JSON record. Failed new SDK
directories remain for diagnosis; use a fresh destination for retries.

The Mac R0 CI legs use this cold-restoration route, restore a second private SDK
from the same verified inputs for native mixed-provider negatives, and retain
helper/pin/input identities without SDK payloads. Windows retains its separate
provider. Native Qt tool execution
may need access unavailable in a restricted local sandbox; an abort is not a pass
and must not be hidden with CPU-feature overrides or modified provider binaries.
The universal input archives do not prove native Intel execution, graphical
sessions, deployment-floor testing or
redistribution/release approval; those remain separate provider gates.

### Restore the Windows Vulkan input candidate

The pinned [SwiftShader input record](../providers/swiftshader-windows-x64.json)
selects only the prebuilt loader, ICD, original manifest and license notices from
the versioned official Chrome for Testing archive. The browser and its other
programs are never extracted or executed. Restore the downloaded archive offline:

```sh
python3 -B scripts/restore_renderer_swiftshader_windows.py \
  --archive /absolute/chrome-headless-shell-win64.zip \
  --destination /absolute/new-vulkan-inputs > /absolute/vulkan-inputs.json
```

Static restoration works without executing Windows code. It checks the raw archive
and all four member sizes/hashes before writing a fresh prefix, checks PE32+/AMD64
DLL headers and preserves the manifest unchanged. Existing prefixes, duplicate or
linked members, foreign architectures and changed inputs fail. The manifest's
legacy `1.0.5` is not an observed driver API version or an accepted deployment floor.

On native Windows x64, add `--native-probe` to require a checked architecture query,
actual loader path and successful loader-version call. WOW64/ARM64 emulation and
missing/foreign/changed modules fail. This creates no Vulkan instance, device or
window and does not qualify the ICD, graphics or presentation. The Windows R0 CI
lane now defines that native loader preflight; local Mac/Linux restoration and
protocol mocks do not establish its native execution.

Next qualify the same inputs in forced-software Windows offscreen/widget suites,
including observed actual loader/ICD paths and validation. Qt 6.8.3 can fall back
from a failed `QT_VULKAN_LIB` override to its default loader; an environment string
alone is not provenance. Windows Qt/CRT/OS closure, hardware, notices/redistribution,
deployment-floor and final package gates remain open. No global driver or registry
installation is performed. See [SwiftShader usage](https://github.com/google/swiftshader/blob/master/README.md)
and [Qt's loader selection](https://github.com/qt/qtbase/blob/v6.8.3/src/gui/vulkan/qbasicvulkanplatforminstance.cpp).

## Run and inspect

From the repository root on native macOS, with a graphical login for widget mode:

```sh
python3 -B scripts/run_renderer_checks.py --spike \
  --app /tmp/librecad-rhi-build/lcrhispike_offscreen \
  --backend metal --device hardware --mode offscreen \
  --report-dir /tmp/librecad-rhi-build/evidence-offscreen
python3 -B scripts/run_renderer_checks.py --spike \
  --app /tmp/librecad-rhi-build/lcrhispike_widget.app/Contents/MacOS/lcrhispike_widget \
  --backend metal --device hardware --mode widget \
  --report-dir /tmp/librecad-rhi-build/evidence-widget
```

Use `--backend vulkan` on Windows/Linux. Forced requests never fall back. Software
qualification additionally requires `--device software --icd-manifest /path/to/icd.json`;
select that exact ICD through `VK_DRIVER_FILES` (or the loader's legacy
`VK_ICD_FILENAMES`) and record its library provenance. `--device any` is useful for
local CTest but does not satisfy a software/hardware-specific gate. Use
`--expected-revision FULL_SHA --expected-arch arm64|x86_64` to bind a candidate.
Use `--expected-qpa cocoa|xcb|wayland` to require the actual graphical session.
Linux CI additionally requires the pinned prebuilt Khronos validation layer
1.4.341.0-1 for every XCB/managed-Wayland case. To use the same core and
synchronization profile, add both paths to the existing runner command:

```sh
  --vulkan-validation-manifest /absolute/provider/share/vulkan/explicit_layer.d/VkLayer_khronos_validation.json \
  --vulkan-validation-library /absolute/provider/lib/libVkLayer_khronos_validation.so
```

The paths may refer to a privately extracted binary package. The runner changes
only the app's environment, writes private settings, removes inherited Vulkan
filters/disables/log redirection, and requires actual library initialization,
instance/device insertion and active synchronization with no disabled checks.
Manifest/library/settings and both output streams are hashed and checked again
after supervised cleanup. VUID, synchronization and unassigned validation errors
reject pixels and controlled terminal-failure results alike. Informational messages
remain in the logs. This Linux profile does not qualify Windows layers, hardware,
GPU-assisted validation or performance. Configuration follows the
[validation-layer settings](https://github.com/KhronosGroup/Vulkan-ValidationLayers/blob/vulkan-sdk-1.4.341.0/docs/khronos_validation_layer.md).

The Linux CI aggregate requires the exact 60 distinct tuples across QPA, build,
mode, background, scenario, observed DPR and terminal-failure point. It binds the
native JSON and executable rather than counting embedded summaries alone.
Duplicated positives/failures cannot replace a missing case, and a main build
cannot stand in for Gui-only proof. DPR normalization uses the runner's existing
0.001 tolerance; missing/mixed/nonfinite or boolean DPR values fail.

The owned Vulkan host now explicitly requests API 1.1 when the loader supports it,
as [QRhi requires for full functionality](https://doc.qt.io/qt-6.8/qrhivulkaninitparams.html).
Older loaders receive an explicit 1.0 request and remain subject to capability and
validation gates; this does not qualify a 1.0 provider or choose a deployment floor.
The prior unspecified-version host produced valid-looking pixels but emitted
extension-dependency and shader-capability VUIDs under validation. Those runs are
historical pixel evidence, not layer-clean or current-source acceptance.

For widget DPI checks, set `QT_SCALE_FACTOR=1.25|1.5|2` and also pass the corresponding
`--expected-dpr`; the validator checks the observed view DPR, not just that variable.
The macOS widget target is a development `.app` with its own application identity.
It requires a logged-in, unlocked console graphical session; detected locked or
unavailable sessions fail before opening test windows. Lifecycle mode also requires
actual focus. It does not unlock the desktop, force application activation, or
substitute synthetic focus for a real focused window.
`ctest --test-dir /tmp/librecad-rhi-build --output-on-failure` runs the offscreen
opaque/transparent and standard/lifecycle/recovery cases; presentation requires the separate
explicit widget command.

Add `--background transparent` to either invocation to check straight-alpha
readback from a premultiplied target. The renderer verifies RGB ≤ alpha on GPU
bytes before one explicit conversion; half-alpha geometry, stacked layers and a
translucent image texel have independent expected pixels. Default background is opaque.
Add `--scenario lifecycle` for three real owned-device create/render/readback/destroy
iterations in offscreen mode. In widget mode it verifies actual focus, synthetic
mouse/key routing and an observed context popup/action, requests logical close
after recording readback but before submission, drops the queued invalid consumer
payload, and renders a replacement surface plus the surviving view. It records
whether GPU work was still pending at actual destruction; a safe pre-submission
invalidation is not a physical-device-loss or pending-GPU-destruction claim.

`--scenario recovery` is offscreen-only. It substitutes four host-visible failure
results (device creation, texture creation, frame start and submission), verifies
that each failed attempt releases its owned device and accepts no image, then
creates a new device and validates its pixels. Each run creates/destroys seven
QRhi owners and checks four recovered outputs. Every initialized attempt records
its actual API/device/classification; all must match a required hardware/software
run, not just the final device. For submission, the real synchronous
frame/readback finishes before its success status is replaced by `FrameOpDeviceLost`;
this tests host rejection/recreation, not an actual driver reset, GPU allocation
failure or physical device loss. Unexpected native failures still fail the run.
The widget host and runner reject this scenario for borrowed devices.

`--scenario reparent` is widget-only. It tests two existing surfaces through
initial, resumed, detached and returned stages, retaining pixel output for both
surfaces at every stage. The moved widget must release its scene resources, bind
to the second window's distinct Qt-owned QRhi, then return to the original one;
the surviving view must keep its device and render fresh pixels throughout.
Each capture records device, resource-release and readback generations. These
are Qt surface/device-binding transitions, not physical GPU resets.

The same case observes at least 200 ms without scene draws while hidden and
another 200 ms while visible at logical size 0×0, then verifies resumed output.
This is not the production idle/power budget. A controlled completed readback is
held until a recapture request invalidates it; delivering the old CPU payload
must leave the image unset. Resize also invalidates the snapshot, and readiness
requires its size to match the current render target. No borrowed QRhi/frame/target
is destroyed by the test widget. The offscreen host/runner reject this scenario.
The move follows Qt's [early resource release contract](https://doc.qt.io/qt-6.8/qrhiwidget.html#releaseResources)
and [temporary subwindow child ownership](https://doc.qt.io/qt-6.8/qmdisubwindow.html#setWidget).

Before allocating scene resources or drawing, both hosts require RGBA8 sampling/render-target/readback,
single-sample support, a 64-byte uniform range and in-range target/atlas dimensions.
The owned offscreen host also checks before allocating its target; Qt creates the
widget's borrowed target under its own ownership.
The report records those actual capabilities; unsupported requirements fail rather
than silently choosing another format/sample count or API.

Every run writes into a fresh evidence subdirectory: process logs, `app.json`,
normalized top-left RGBA sidecars, PNGs and `checks.json` binding the executable,
report and executed landmark IDs. The validator independently checks PNG pixels
against the RGBA bytes and committed fixture, including all 15 landmarks per view.
It rejects wrong APIs/devices/providers/architectures, incomplete frames, blank or
stale output and timeouts. The runner also rejects unreported files in its fresh
evidence directory, including accidental failed-frame images. Widget mode requires two exposed/submitted MDI views,
resized targets, completed asynchronous readbacks and ordinary-widget composition.
Reparent mode reports eight stage snapshots of those two surfaces, not eight
independent documents or devices; each of its 128 check IDs has a fixed scenario.
Its bounded shutdown wait is prototype scaffolding, not production lifetime design.
App and runner reports use schema version 3 and the unchanged v2 fixture; older R0
reports remain historical evidence, not current-source validation. Production and
package evidence kinds are still unsupported rather than accepted by this schema.

## Terminal widget failures

The widget proof latches its first terminal failure. New rendering/capture and
successful report/image serialization are rejected thereafter, even if another
path requests exit code zero. Normal widget output is retained as CPU snapshots;
accepted PNGs/RGBA/report are written only after windows and QApplication teardown.
This proves process cleanup, not a requirement for interactive screenshots to wait
until an application's exit.

Pending `QRhiReadbackResult` storage stays owned while a drain runs. Failed or
incomplete draining transfers it to a fixed two-slot CPU holder constructed before
QApplication; no borrowed QRhi/frame/target is retained there. Its last ownership
is released after QApplication destruction. Completion after terminal failure drops
delivery directly, without queuing new GUI work. Qt cleanup callbacks alone are
not completion barriers: the qualified Qt backend still has to finish destruction.

Run three separate standard-widget expected-failure cases, with a working
initialized/exposed native API and the same provider/architecture/ICD assertions
as success mode. For example, on the qualified Linux software provider:

```sh
VK_DRIVER_FILES=/absolute/qualified/icd.json QT_QPA_PLATFORM=xcb \
  python3 -B scripts/run_renderer_checks.py --spike --xvfb-run /usr/bin/xvfb-run \
  --app /absolute/build/lcrhispike_widget --backend vulkan --device software \
  --mode widget --scenario standard --background transparent --expect-failure shutdown \
  --icd-manifest /absolute/qualified/icd.json --expected-qpa xcb \
  --expected-arch arm64 --expected-revision FULL_SHA \
  --report-dir /absolute/build/evidence-terminal-shutdown
```

Use `render`, `readback` and `shutdown` for `--expect-failure`. The first injects
before drawing on an exposed initialized device, the second after recording a
readback, and the third substitutes a failed drain result **after a real successful
drain/completion** during teardown. The last exercises CPU-retirement ownership but
does not claim physically pending GPU work at destruction, an actual failed native
drain or device loss. Both opaque/transparent cases run in the Linux CI scaffold,
separately from its 24 successful invocations **per QPA**.

Each injection deliberately probes both successful writers and a later zero-exit
request. The final result must stay failed. App diagnostic `failure.json` has kind
`r0-spike-failure`; runner `checks.json` has kind `r0-spike-expected-failure`, empty
`executed_smoke_ids` and separate `expected_failure_ids` (three assertions per run).
No `app.json`, PNG or RGBA is allowed. The validator requires exact exit 1, requested/
reached point, real device/provider/exposure, original diagnostic without secondary
errors, completion/retirement observations and successful teardown/output guards.
Timeout, crash, missing ICD/session, unexpected status or stale identity fails;
none can substitute for reaching the controlled failure. The flags are rejected for
offscreen hosts and nonstandard scenarios. Native Mac/Windows widget execution
remains a separate qualification gate; helper tests and compilation do not close it.

## Managed Wayland proof

Use a matching Qt Wayland plugin plus prebuilt Weston 14.0.2. The private
[session helper](../../../scripts/renderer_wayland.py) owns a fresh compositor,
short mode-0700 socket directory and bounded shutdown. It forces the CAD app to
`wayland`, removes the app's `DISPLAY`/inherited Wayland socket, and records the
observed compositor backend, Pixman renderer, inputs, liveness and log hashes.
The app still has to report actual Vulkan and validate its pixels; Weston's CPU
composition is not an application fallback to OpenGL or XCB.

```sh
VK_DRIVER_FILES=/absolute/qualified/icd.json \
  python3 -B scripts/run_renderer_checks.py --spike --xvfb-run /usr/bin/xvfb-run \
  --app /absolute/build/lcrhispike_widget --backend vulkan --device software \
  --mode widget --scenario lifecycle --background transparent \
  --weston /usr/bin/weston --weston-backend x11 --weston-input-tool /usr/bin/xdotool \
  --icd-manifest /absolute/qualified/icd.json --expected-qpa wayland \
  --expected-arch arm64 --expected-revision FULL_SHA \
  --report-dir /absolute/build/evidence-wayland-lifecycle
```

`x11` here names **Weston's** output/input backend inside Xvfb; the app connects
only through Wayland. The lifecycle proof needs a real compositor input serial:
direct Qt events or XSendEvent are insufficient for a grabbing Wayland popup.
The qualified input tool focuses only the owned Weston window on a verified local
Xvfb server and sends mapped F12 input through XTest. After the synthetic mouse/
Space checks, the widget constructs a keyboard context event inside the spontaneous
F12 callback while its Wayland input serial is current. It writes an atomic
`wayland-input.json` acknowledgment so the helper stops seeding keys. The menu must
be visible, active and actually exposed before its action is triggered. This is
virtual keyboard-popup evidence, not a physical keyboard/monitor or native mouse-grab test.

`--weston-backend headless` is available for rendering without an input seat;
it cannot replace lifecycle focus/popup acceptance. Missing/unqualified versions,
non-Xvfb input, startup/socket timeout, premature compositor exit and changed
provider binaries fail. The runner executes and checks the app independently:
Weston's own `-- program` launcher can return zero when that program failed and
is not used as the acceptance result. The owned-process supervisor below also
checks display-file and descendant retirement before publishing accepted output.

For an extracted Ubuntu provider, `--weston /prefix/bin/weston` derives `/prefix`
(or pass `--weston-prefix`); the matching libweston-14 modules and helpers must be
present. The [ARM64 provider pin](../providers/weston14-linux-arm64.json) records
verified binary package inputs, not full deployment/redistribution qualification.
No dependency is compiled from source. Weston logs/metadata are retained with each
app result, including failures; source CI defines separate XCB and Wayland matrices.
Native hardware/desktop sessions and installed artifacts remain separate gates.

## Owned Linux test processes

Pass `--xvfb-run /usr/bin/xvfb-run` directly to the runner, not around it. The private
[supervisor](../../../scripts/renderer_process.py) requires a fresh, single-threaded,
child-free Linux CLI with default SIGCHLD handling and pidfd/waitid support. It
enables subreaper adoption before launching the existing xvfb-run tool, keeps the
supervisor alive after that tool exits, and waits for all adopted descendants.
TERM then KILL cleanup is bounded; signals use pidfds and checked process-start/
parent identities, never a system-wide process sweep. Double-fork/setsid children
cannot bypass this cleanup by leaving their original process group.

The display has private authentication under a short mode-0700 runtime directory.
`display.json` binds its actual Xvfb/provider/launcher and lock/socket inode identities.
After the process tree settles, the runner requires those owned file identities
and its authentication directory to have disappeared. A new unrelated server reusing
the display number is preserved, not removed or signaled. Existing container
zombies are not restarted, killed or counted as newly reaped test children.

Workers write `candidate.json`, not a passing `checks.json`. Only after cleanup
does the parent revalidate source/helper/binary/report/pixel or expected-failure
identity, reject late unreported outputs, bind `processes.json` and launcher-log
hashes, then atomically publish `checks.json`. App failure, timeout, interrupt,
cleanup failure or changed tools cannot publish an accepted candidate. Failure
diagnostics remain in the fresh evidence directory. SIGKILL of the supervisor
itself cannot be recovered; it leaves no completed cleanup acceptance record.

App/runner schema 3 remains unchanged; process scope is an additive schema-1
`r0-process-scope` record. Ordinary Mac/Windows or unsupervised/manual reports do
not claim this Linux virtual-display cleanup gate. Native desktop/hardware input
qualification remains separate; the supervisor never targets a user's real display.

CPU tests include native Linux double-fork, stubborn-child, crash, timeout,
interruption, preserved unrelated work, real Xvfb/app failure and escaped-child
compositor startup failure. Linux CI requires every one to execute without skips:

```sh
python3 -B -m unittest scripts.tests.test_renderer_process -v
```

## Painter and consumer source register

M1 adds a bounded canonical signature and source-role validator to the existing
inventory helper. It covers all 60 current painter signatures, including overloads
and qualifiers, and binds the whole header digest so changed defaults are visible.
It rejects unrecognized declarations instead of ignoring them, audits drawable
source/header and known host/event/factory/binding files, and prevents CPU/build-only
role labels from bypassing graphics requirements. A lexical guard flags new direct
painter callers in the existing production roots; it is not C++ alias analysis.

```sh
python3 -B -m unittest scripts.tests.test_renderer_cases
```

These are CPU register/input/mutation and independent oracle checks. M2 now adds
the [single requirements manifest](../../../unittest/rendering/renderer_cases.json)
and [contract corpus/provenance](../../../unittest/rendering/fixtures/README.md):
56 cases / 223 proposed variants, six pinned fixtures, typed observations and five
workloads. Scripts remain inert during metadata validation; real font metrics and
original PNG/geometry expectations have independent CPU checks. Nonempty text also
requires actual glyph readback coverage in its future native producer.
M3 now validates 189 logical tuples and stage prerequisites, with explicit
Python-OFF/ON and UI-OFF profiles and an inventory-only CLI documented with the
corpus. Requirements schema 2 adds case prerequisites; corpus schema 1 and R0
report schema 3 are unchanged. All selected bindings remain proposed, not executed
production, package or platform acceptance. The report CLI above is unchanged.
Changed helper bytes invalidate old runner bindings even when the spike's compiled
inputs are unchanged. Refresh GPU evidence at stable helpers in M4, not by editing
old report hashes. No production factory or default changes in M1–M3.

## Minimum Qt provider

### Linux ARM64 candidate

The [Linux 6.8.3 candidate pin](../providers/qt683-linux-arm64.json) records the
official Base/ShaderTools/Wayland/ICU archives and raw reference identities.
It is diagnostic-only: mandatory Wayland widget closure fails on the tested
Ubuntu 26.04 / Mesa 26.0.8 / Weston 14.0.2 software stack. Do not select it as a
passing required CI provider or replace the Wayland tests with XCB/GL.

Verify every archive size/SHA256 before extracting or executing tools. Base,
ShaderTools and Wayland extract to a new SDK root; the separate ICU archive's
top-level libraries extract to its `lib/`, not the root. Use a trusted 7z-capable
extractor and verify the pinned references before running native `bin/qsb`.
Force the SDK in both `CMAKE_PREFIX_PATH` and `Qt6_DIR`, plus its plugin path at
runtime. No provider binary was patched or dependency compiled in this audit.

Main/Gui-only native builds, the XCB matrix and Wayland offscreen checks pass.
Ordinary and lifecycle Wayland widgets instead receive SIGSEGV during close.
A tiny Qt-only Vulkan widget with no LibreCAD resources reproduces it; the same
source on current Qt 6.10.2 closes normally on the same software-display path.
The stack reaches Qt backing-store swapchain reset, Mesa and `wl_display_flush`.
New Qt-only observations with a privately extracted Vulkan validation layer
confirm `VUID-vkDestroySurfaceKHR-surface-01266`: the candidate destroys its
Wayland Vulkan surface while a swapchain still exists. Its hide path resets the
native surface; the matching 6.10.2 path preserves it until later teardown.
See the [Vulkan lifetime requirement](https://docs.vulkan.org/refpages/latest/refpages/source/vkDestroySurfaceKHR.html).

The new plain candidate observation exits zero but warns of attached Mesa Wayland
proxies; with validation it exits 139 and also reports present-layout VUID 01430.
The 6.10.2 control exits zero with/without that same layer and reports no VUIDs.
These four diagnostics use no LibreCAD resources and remain rejected by the
strict R0 runner. They do not close the candidate gate or establish every later
Mesa memory fault's cause. Require a fixed prebuilt Qt/runtime tuple and rerun
the full matrices; normal exit or adding a validation layer is not a runtime fix.

The migration plan retains the original failure, Qt-only source/control, tool
inputs, partial matrix and exact process baseline. The Qt-only control is not an
R0 report; the renderer runner correctly rejects its missing smoke evidence.
D2/D4 must qualify a fixed minimum Linux tuple. Existing Qt 6.10.2 Linux R0 CI
definitions stay unchanged; no support floor or platform gate is accepted here.

### Mac provider

The [macOS 6.8.3 provider pin](../providers/qt683-macos.json) records four
official prebuilt archives, published checksums, SHA256 identities and raw payload
reference hashes. Base/ShaderTools supply this standalone proof; SVG and Tools
also supply the current application's required module boundary. The shared
[module preflight](../providerprobe/README.md) checks that boundary without a
graphics device or visible window. CI cold-restores all four inputs and builds
both the spike and module target for each requested Mac architecture. Its native
CPU module gate requires exact unskipped CTests, matching host/producer architecture,
all module check IDs and private Qt runtime paths. The separate RHI target is
built, not executed by that gate; hosted results, graphics/session, deployment and
package acceptance remain separate.

Qt 6.8/6.9 may export an AGL consumer link item even when the selected Apple SDK
has no AGL link stub. The Metal-only R0 build removes just that obsolete item,
following [Qt 6.10's removal](https://raw.githubusercontent.com/qt/qtbase/v6.10.0/cmake/FindWrapOpenGL.cmake).
It checks the actual compiler SDK rather than a possibly still-present OS runtime
bundle, preserves every other dependency, and records
`build.options.legacy_agl_link_workaround`. SDK files are not patched; Qt's runtime
OpenGL/AGL dependencies may remain. This is not a GL-free Qt distribution.

The [compatibility module](cmake/LegacyQtMacSdk.cmake) is required source, not a
generated CMake file. Its narrow ignore exception and the source-visibility test
ensure it is included in clean CI candidates.

For an already checksum-verified, extracted SDK:

```sh
cmake -S lcrenderer/rhi/spike -B /tmp/librecad-rhi-minqt \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/absolute/qt683-sdk \
  -DQt6_DIR=/absolute/qt683-sdk/lib/cmake/Qt6 \
  -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0
cmake --build /tmp/librecad-rhi-minqt --parallel 4
```

On this host, Qt 6.8.3 tools need an execution context permitting CPU-feature
queries; the sandbox denied the NEON query, while native `qsb --version` and shader
compilation passed. Do not mask this with feature-disable overrides. Native ARM64
Metal offscreen checks pass, including a fresh Widgets-OFF build. Intel targets
cross-build on ARM64, but no native Intel or Rosetta runtime result is claimed.
The 12.0 deployment flag is build metadata, not macOS 12 runtime qualification;
Mac GUI sessions, redistribution review and final package gates remain open.

```sh
python3 -B -m unittest scripts.tests.test_renderer_inventory
```

Qt's imported provider may still link its own OpenGL dependency (the macOS provider
links Apple's framework); this is not a no-OpenGL-library distribution claim.
Native Windows/Linux x86_64/macOS x86_64, minimum-Qt qualification, physical input,
monitor-DPI changes, device loss/recovery, numeric budgets and production
renderer/test/package gates remain tracked in the migration plan. Do not treat
these R0 landmark checks as the later feature-complete rendering suites.
