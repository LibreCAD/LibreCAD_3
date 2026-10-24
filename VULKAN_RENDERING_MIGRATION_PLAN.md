# Vulkan rendering migration plan

Status: reviewed 2026-10-24; R0 remains active and unaccepted.
Provisional R1 implementation is authorized; R2–R7 remain proposed.
Baseline: LibreCAD 3
`132265a0573253980451facf9109678f8bffe743`, branch `fix/msvc-eigen-include`.
All platform acceptance gates below remain open. R0 is implementing standalone
feasibility executables, not changing production factories or defaults.
R1 acceptance remains gated on R0's decision record,
prototype results and agreed scope/budgets. Preserve this checkout's unrelated
changes; distinguish that checkout from the published R1 candidate below.

On 2026-10-22 the user explicitly approved provisional R1 build/factory/test
integration using QRhi, Vulkan on Windows/Linux and Metal on macOS, with legacy
remaining the default and all native-platform, D1–D6 and release gates open.
This changes implementation order, not acceptance order or platform scope.
Earlier requirements to finish R0 before starting R1 now govern R1 acceptance;
the exception does not authorize R2, cutover, retirement or desktop interaction.
The subsequent branch/push/Actions authorization is limited to the R1 candidate
in `dxli/LibreCAD_3:codex/r1-platform-ci`; it does not authorize a merge, new PR,
release workflow change or acceptance-policy change. Independent review remains pending.

### Current checkpoint and next decision — 2026-10-24

#### Wayland popup serial-race correction — full native validation complete

The user authorizes a targeted fix, the full unchanged Vulkan validation, commit
and push to `codex/r1-platform-ci`. The original index/checkout remain preserved.
Protocol tracing of the unchanged earlier candidate reproduces the DPR 1.25
failure on invocation 11: Weston has received F12 release before Qt dispatches
the press and sends `xdg_popup.grab`; `popup_done` precedes exposure. Rendering
and readback succeed. This is an input-serial race, not evidence of a GPU error.

The compact correction uses an atomic focused-widget readiness marker, then one
native XTest F12 keydown on the existing owned Xvfb/Weston seat. The key stays
down until the original visible/active/window/exposed popup predicates pass;
only then is the existing acknowledgment written. Every exit/error path releases
the key and still reaps the app, including partial input-tool failure. No case,
assertion, input proof, provider pin or timeout is removed or relaxed. The 20s
native, 30s invocation and 300s CI-block deadlines remain unchanged.

Final clean source `965e4465b7a55c85f433499bf3344508fe75357b` passes the entire
60-tuple Vulkan replay and **50 consecutive unretried Wayland DPR 1.25 lifecycle
runs**. Each QPA passes 24 positives / 1,384 checks and six controlled failures /
18 separate assertions. All 50 repeated runs pass their 34 original checks and
retain native protocol traces: held F12 precedes popup grab, with no early release
or `popup_done`. Core/synchronization validation reports zero errors, no ordinary
checks fail or skip, and all owned descendants retire. Qt/private/qsb 6.10.2,
Weston 14.0.2 and Lavapipe stay the existing software/VM providers. Make/private
binary paths are the only replay setup differences; no dependency is built.

The source fix is b0618650; final refinement 965e4465 registers the readiness
artifact only for input sessions, preserving fail-closed unrelated sessions.
The first fix's full60 and 50 repetitions pass too, but final proof is separately
bound to 965e4465. Six added CPU regressions cover readiness/ack ordering and
failure/cleanup paths. Final Linux scripts pass 622/622 without skips; Mac passes
611 plus 11 Linux-only skips. The initial offline Linux run lacks four exact
submodule fixtures (license notices and a linked header); restore their pinned
local Git objects without downloading/building dependencies, retain that failure
and pass the complete unchanged suite. This is not a test suppression or retry
of a failed final rendering case.

Evidence is retained separately under
`build/renderer-evidence/r1-popup-fix-20261024.fDteaO/`: original failing trace,
first-fix proof, final native proof/traces, scripts, identities and readback review.
The old failures and all 43 renderer/357 Python receipts remain immutable.
All 110 retained reports independently revalidate pixels, check IDs, source/helper/
app identities, core/synchronization logs, native input ordering and owned cleanup.
Full-plan, checker42/42, complete-worktree mapping116 paths/16 gates, inventory190,
both workflow lints, whitespace, 50 local links and preservation of all prior
receipts/gate states and 1,076 unrelated files pass. Repeat maintenance checks on
this finalized receipt before the authorized documentation commit and push.
The published head includes documentation-only descendants of tested 965e4465;
no compiled/helper/fixture input changes follow native validation. Earlier
all-platform hosted success below belongs only to d4e9650e, not this changed
helper; a new push starts a fresh source-CI cohort and is not predeclared green.
Independent review, hardware, production viewport, wider widget reliability,
providers/deployment/packages and R0/R1/D1–D6 gates stay open.

Use this checkpoint, the completed **R1 all-platform CI continuation** and the
local Vulkan follow-up below as the current resume point. The detailed R0/R1
evidence paragraphs below are chronological snapshots;
their former missing-host/CI descriptions are not a claim that later execution
did not occur. Historical results retain their original input identities.

Last completed all-platform source-CI candidate: `d4e9650e5dd03b8396923e7001dbee1d7c0bd3fb`,
[run 40076688956](https://github.com/dxli/LibreCAD_3/actions/runs/40076688956).
Attempt 1 completed **success**: all **60/60 source jobs**, the protocol/plan job
and the source aggregate pass, **62/62 jobs** in one exact-SHA cohort. Source-CI
execution and self-review are complete; independent acceptance and R0/R1 phase
acceptance are not. These closeout receipts are local documentation-only updates
after dispatch; the tested/published code remains the exact commit above.

| Native source platform | Passed / required jobs | Source-CI scope and remaining acceptance |
|---|---|---|
| Linux x86_64, Ubuntu 26.04 | 12/12 | Both library groups, independent hosts and software offscreen XCB/Wayland pass; hardware, presentation and packages stay open. |
| Linux arm64, Ubuntu 26.04 | 12/12 | Same source coverage on native ISA; VM/software proof is not physical-GPU or installed-package acceptance. |
| Windows x64, MSVC 2022 | 12/12 | Library/host, exports and relocated CLI checks pass; full Python/NSIS/runtime and hardware acceptance stay open. |
| macOS arm64, macOS 15 runner | 12/12 | Library/host and hardware-class Metal offscreen source checks pass; widget/session/deployment/bundle/signing acceptance stays open. |
| macOS x86_64, macOS 15 Intel runner | 12/12 | Verified virtual Metal for source/offscreen only; hardware, widget, performance, deployment and bundle/signing gates stay open. |

All 60 unique source artifacts are locally retained and reviewed against their
run/attempt/full SHA, effective profiles, helper/source identities, discovery/XML,
device attestation and graph/cleanup records. The hosted protocol job passes
616/616 regressions without skips. Current proof:
`build/renderer-evidence/r1-ui-partition-20261024.kJGirf/hosted/cohort-review.json`,
with supplementary payload readback in `hosted/payload-audit.json`. Local retention
is not an external durable backup or full provider/license/deployment closure.
Required SDK files omitted from uploaded payloads retain hash metadata only.

Both Intel library/tests-ON artifacts execute all 22 original RHI IDs with zero
failures/skips. Their selected registry IDs, `4294968041` and `4294968042`, match
independent `system_profiler` entries and exact `AppleParavirtAccelerator` native
attestation; raw Qt type remains `0`, not hardware. Intel UI and combined-host
tests-ON each execute 197 non-widget IDs, compile/discover 97 unexecuted widget
IDs, and retain all 75 UI units: bindings 3, root 28, widgets 44. The File API
confirms one MOC/resource owner and shared C++17/export usage. Command/profile/job
caps remain 300 seconds / 1,200 seconds / 60 minutes. No default, R2 or release
workflow change follows from this green source cohort.

#### Committed Vulkan rendering follow-up — 2026-10-24

Local commit `2b876010ea0e76e02afa566caf0810f900257ec8` records the synchronized
CI-closeout plans on `codex/r1-platform-ci`; it is not pushed. A fresh clean
snapshot of that exact commit builds and runs the existing standalone R0 Vulkan
proof on native-ISA Linux ARM64 in the existing VM/container, using Qt/private/qsb
6.10.2 and Lavapipe (`llvmpipe (LLVM 21.1.8, 128 bits)`). This is actual Vulkan
software execution, not Metal or an OpenGL fallback, and not the application's
still-legacy viewport or a complete `LcPainter` implementation.

The fresh complete cohort passes all six selected existing CI shell blocks and
all **60 required native tuples**: on **each** XCB and managed Wayland, 24 positive
invocations / 1,384 checks plus six controlled failures / 18 separate assertions.
It covers Widgets-ON and Gui-only builds, opaque/transparent pixels, lifecycle,
reparent, recovery and DPR 1/1.25/1.5/2. Actual core/synchronization validation is
active, with zero Vulkan errors, and all owned descendants retire. Generator and
provider-path adjustments use existing Make/private binary packages; no dependency
install, source change, skipped case or budget expansion is introduced.

Retain the earlier failures too. The initial configure selected absent system Qt
private headers; selecting the existing private packages explicitly fixes setup.
The first full replay then fails Wayland lifecycle at DPR 1.25 while waiting for
popup exposure, after successful Vulkan render/readback. Its unchanged isolated
rerun and the fresh complete cohort pass. This is a **known intermittent interaction
failure**, not reliable widget acceptance or a failure to suppress. The post-run
one-use summary reader also needed its nested report-key corrected after all six
actual CI blocks passed; no test or rendering code was changed for that readback.

Evidence is retained under `build/renderer-evidence/r1-vulkan-20261024.L6azJJ/`:
`final-vulkan.tar.gz`, `partial-vulkan-and-diagnostic.tar.gz`,
`failed-configure.tar.gz`, original PNG/RGBA/logs and `review.json`. Payload review
and final plan/checker/preservation validation are completed before handoff.
Independent review, external durable retention, hardware/performance, missing
platforms/providers/deployment/packages and R0/R1/D1–D6 acceptance remain open.
Next investigate the fractional-DPR Wayland popup's visibility/active-popup/
exposure and native-input ordering; preserve its failing logs, assertions and
deadlines. No source fix, R2, merge, push or release/default change is authorized
by this validation receipt.

Historical predecessor failures remain retained, not rebased onto this result.
On predecessor `ded3b55a` / run 40071386912, both Intel library/tests-ON jobs
pass all 22 RHI IDs with verified virtual
Metal. The combined Intel host job instead times out in ordinary UI compilation
at 93/98 edges. A locally verified three-group follow-up preserves the exact 75
units (bindings 3, root 28, widgets 44), one Qt generator owner and the original
command/profile/job caps; that follow-up is now published above. The following completed
cohort is the historical failure baseline, not this candidate's result.

Historical failure baseline, before the approved virtual-tuple change:
`cb91cf0160ac5e0c4d6e3c16ca98c8a058f79b72`.
[Run 40021957419](https://github.com/dxli/LibreCAD_3/actions/runs/40021957419)
completed **failure**: 58/60 source jobs succeed, the protocol/plan job succeeds,
and the source aggregate correctly fails. All 60 expected source tuples and 60
nonexpired artifact names were checked against the run metadata; there are no
missing, skipped or cancelled source jobs in this run.

| Native source platform | Passed / required jobs | Remaining source-CI result |
|---|---|---|
| Linux x86_64, Ubuntu 26.04 | 12/12 | Green for this R1 source scope; hardware, presentation and packages remain separate. |
| Linux arm64, Ubuntu 26.04 | 12/12 | Green for this R1 source scope; native ISA is not a physical-GPU claim. |
| Windows x64, MSVC 2022 | 12/12 | Earlier CPU-preflight deadline failure is resolved; full NSIS/runtime qualification is still open. |
| macOS arm64, macOS 15 runner | 12/12 | Green for this R1 source scope, not Cocoa interaction or final bundle qualification. |
| macOS x86_64, macOS 15 Intel runner | 10/12 | Both tests-ON library groups fail the Metal device-class assertion. UI-only tests-ON now passes. |

The retained Intel `system_profiler` reports **Apple Paravirtualized Graphics
Device** / `AppleParavirtAccelerator`; the test reports **Apple Paravirtual device**,
classified `unknown`, where the unchanged CI request is `hardware`. Each failed
job executes 22 RHI IDs: eight pass and 14 fail at device setup, with zero skips.
This identifies the adapter and the failing precondition; it does **not** prove
that the 14 rendering/lifetime cases would pass on a virtual device. Expected
negative build-contract XML failures are separate harness probes, not additional
ordinary test failures.

**Owner decision approved 2026-10-24:** allow verified virtual Metal for Intel
Mac R1 source CI; keep hardware acceptance gates open and continue the five-platform
cohort. Implement the explicit source/offscreen tuple, binding the selected Metal
device's registry ID to the native Apple paravirtual accelerator, not its name
alone. No virtual widget/performance/package acceptance is authorized.
Do not call `unknown` hardware, infer device class from Metal availability,
allow `any`, skip the 14 tests, or change all Mac/hardware requirements. This plan
review records both routes; the approved virtual route is now active, and a
hardware-backed runner remains the fallback if verification fails.

The bounded implementation now binds the selected `QRhiMetalNativeHandles::dev`
registry ID to IOKit's exact `AppleParavirtAccelerator` entry, rechecks that ID,
and retains raw QRhi type/registry ID/class in XML. Only unknown/virtual QRhi
types on native Intel Metal can use that positive native identity; other unknown
devices stay unknown. Names and environment variables cannot attest a device.
The additive `r1-source-virtual-metal` configuration is source/offscreen only,
with R2–R7/widget/package selections rejected and every hardware tuple preserved.
All 22 original GTest IDs remain; the existing factory test gains classifier
negatives without replacing device-backed tests. Required uploads now fail if
no evidence is found. Current local proof is at
`build/renderer-evidence/r1-virtual-metal-20261024.AirkWi/`; the successful current
Intel device verification and full cohort are retained in the UI-partition proof
above. Every hardware tuple and the full 190-configuration inventory remain intact.

The historical four-payload failure-review packet is retained at
`build/renderer-evidence/r1-plan-review-20261024.P1Sh1I/index.json`: exact run/job/
artifact metadata plus four downloaded source artifacts (both Intel failures,
Windows core/tests-ON and Intel UI/tests-ON), logs and report hashes. The Windows
artifact executes all 22 RHI IDs without failures/skips; Intel UI executes 197
non-widget IDs without failures/skips, not its widget cases. The remaining 56
artifact payloads have inventory metadata only in that historical packet. It is
not the current successful cohort's archive: all 60 current payloads are retained
and reviewed separately above. Preserve these failed/superseded records and the
initial local pinned-Qt UI header-conflict attempt. Hosted clean Qt 6.8.3 Intel UI
now passes; that does not erase the failed local attempt or qualify deployment
OS floors. An expiring Actions listing or local packet is not external durable
retention; independent review and durable retention remain open.

No runtime, CI, default, source-submodule pin or release policy is changed by this
documentation review. R0/R1, D1–D6, the full painter/consumer contract, native
hardware/session/deployment floors and every final package gate remain open.

The current source contains the M1–M3 manifest/corpus and inventory CLI, validating
56 cases / 223 proposed variants / 60 signatures / 75 consumers. Earlier M2/M3
draft descriptions below are retained context, not the next implementation task.
Provisional R1.1 now separates the CPU/Cairo viewer from `lcviewer_opengl`, with
explicit consumer links, conditional discovery and build-tree generated metadata.
Native ARM64 CPU builds/tests pass on Linux and Mac; Linux additionally builds
the legacy GUI/CLI and passes the optional Cairo factory regression. Provisional
R1.2 adds a private owned offscreen factory/executor, with historical native ARM64
pixel proof. R1.4 now implements the existing indexed-quad, zero-size and injected
texture-policy contracts. Its current native runs and shared-runner integration
are recorded below; this is not the full painter or an application viewport.

R0 closeout continues separately: review the case manifest
and graphics decisions, retain/extend the locally verified process cleanup, qualify missing
native providers/sessions, measure workloads and accept D1–D6.
The current schema-3 prototype proves selected pixels and bounded lifecycle/fault scenarios;
it is not a complete painter, production viewport, performance baseline or package.
Terminal-failure hardening and XCB/managed-Wayland software proofs exist in
the standalone implementation. The retained supervised runs bind their execution-time
inputs, not the changed draft validator now in the worktree. The M1–M3 manifest
work is implemented; refresh helper-bound R0 execution and review its acceptance
before claiming current-run qualification.
R1 alone may proceed provisionally. Local Linux cleanup/pixels do not close
missing native widget, hardware or installed-package gates.

## 1. Migration decision and completion contract

Replace the application-owned OpenGL renderer with a compact Qt Rendering
Hardware Interface (QRhi) backend. The working platform policy is **Vulkan on
Windows and Linux, Metal on macOS**. This removes the application's direct OpenGL
dependency; it is not a claim of Vulkan execution on macOS. R0 must explicitly
confirm that policy before acceptance; the provisional R1 exception above permits
only the scoped integration work. If Vulkan on every platform
or a Qt-independent GPU backend is mandatory, use the direct-Vulkan subplan in
section 9 and re-estimate the work before proceeding beyond R0.

Keep the CAD kernel, entities, document operations, geometry queries, persistence,
and scripting APIs independent of the graphics API. Preserve the existing
`LcPainter` and drawable boundary initially; introduce only the command data and
backend ownership needed for this renderer. Do not build a general rendering
plugin framework or couple this migration to a full UI or CAD-kernel rewrite.

The migration is complete when:

- All painter capabilities and user workflows in section 3 pass their contracts;
  existing OpenGL stubs are not proof of completeness. Unsupported requests must
  be reported, not silently ignored. A limitation must define its accepted input
  domain and tested failure behavior; removing a required feature/platform requires
  an explicit scope decision, not a rewritten passing criterion.
- Required native platforms, offscreen rendering, the CLI where supported, and
  installed packages pass the functional and visual checks in sections 6–7.
- Required tests actually execute on their requested backend; compilation, Python
  smoke, a Null renderer, or an unnoticed fallback cannot satisfy a render gate.
- The new default meets agreed correctness, interaction, memory, and performance
  budgets. The legacy renderer is removed only after the separate retirement gate.
- Build options, dependency providers, CI workflows, package scripts, support
  documentation, and renderer diagnostics agree about what is built and supported.

The separate SMLib/NURBS roadmap remains independent of this migration.
Both efforts should share a backend-neutral curve sampling boundary; Vulkan must
not become the source of exact CAD/NURBS geometry. Do not require unfinished SMLib
integration to deliver this migration, or remove supported spline forms if that
integration lands first.

Required source architectures are Linux x86_64/arm64, Windows x64 and macOS
arm64/x86_64. Deliverables are the two Linux installs, x86_64 AppImage, amd64
Snap, Windows x64 NSIS and both native Mac bundles in section 7. This does not
introduce Windows ARM64, mobile platforms, universal Mac binaries or ARM64
AppImage/Snap. D2 must name each deployment OS/compiler/driver floor separately
from its build host; a newer CI image does not establish support for older OSes.

## 2. Baseline gaps and edit boundaries

| Boundary | Existing code and behavior | Migration responsibility |
|---|---|---|
| Public painter | [lcpainter.h](lcviewernoqt/painters/lcpainter.h), [DocumentCanvas](lcviewernoqt/documentcanvas.cpp) | Retain drawing semantics and visible-entity filtering; specify state, frame submission, diagnostics, and readback. |
| GPU implementation | [OpenGL backend](lcviewernoqt/painters/opengl/renderer.cpp), its manager, entity packs, buffers and fonts | Replace GL ownership/submission; retain reusable CPU preparation only after separating GL types and assumptions. |
| Strokes and curves | [shader resources](lcviewernoqt/painters/opengl/resources/shaders/thickline_shader.shader), [openglpainter.cpp](lcviewernoqt/painters/opengl/openglpainter.cpp) | Geometry-shader strokes need a portable replacement; fixed 1,000-point sampling needs a tolerance-aware boundary. |
| Application viewport | [lcadviewer.cpp](lcUI/lcadviewer.cpp), [main.cpp](lcUI/main.cpp), [About dialog](lcUI/dialogs/aboutdialog.cpp) | Replace GL callbacks/current-context assumptions; preserve events, overlays, MDI behavior and startup semantics. |
| Alternate consumers | [CLI](luacmdinterface/main.cpp), [rendering tests](unittest/rendering/renderingtest.cpp), optional Cairo painter | Remove hard-coded GLFW/GL creation and readback from migrated consumers; preserve optional vector/CPU output paths. |
| Call-site compatibility | [image drawable](lcviewernoqt/drawitems/lcimage.cpp), [MText](lcviewernoqt/drawitems/lcvmtext.cpp), canvas cache callbacks | Image acquisition is repeated during drawing; text metrics are needed before submission; cache removal occurs in document callbacks. Preserve CPU-query availability and retain queued resources independently of public handles. |
| Build ownership | Root, viewer, UI, CLI and [test CMake](unittest/CMakeLists.txt); [Conan](conanfile.py) | OpenGL/GLEW are currently mandatory in several places; options must control discovery, headers, linking, assets and tests consistently. |
| Hidden legacy dependencies | [viewer settings](lcviewernoqt/viewersettings.cpp) and [Lua target](lcadluascript/CMakeLists.txt) | Viewer settings reference generated GL font/shader globals outside the GL directory; the MSVC Lua branch discovers/links GLEW. Include these in the R1 split and negative-discovery audit, not just renderer source lists. |
| CI qualification | [main.yml](.github/workflows/main.yml), [macOS workflow](.github/workflows/python-binding-macos.yml), platform scripts | Windows/macOS disable existing rendering tests; Linux leaves non-core tests informational. Add explicit blocking renderer coverage. |

## 3. Required feature completeness

The current `LcPainter` has 60 active pure-virtual declarations, counting both
`clear` and `scale` overloads, plus a virtual destructor. The commented-out arrow
declaration is not an API. The register below assigns every active method to a
feature group. R0 checks drift and attaches exact fixtures/test cases; no method
may disappear between groups. Keep baseline-output comparisons separate from
fixes to currently missing or incorrect behavior.

| ID | Capability and required behavior | Required tests |
|---|---|---|
| F1 | Path construction: move/line, new path/subpath, close, rectangle, point, empty/degenerate paths, and repeated stroke/fill lifecycle. | `PainterContractTest`, `RenderGeometryTest` |
| F2 | Strokes: model/pixel widths, hairlines, width compensation, joins/caps/miter limits, dash offset and odd/even patterns, closed-path phase, scaled/unscaled patterns. Validate zero/negative/invalid dash inputs. | `RenderStrokeTest` |
| F3 | Arcs in both directions, circles, rotated/partial ellipses, quadratic/cubic Béziers, current splines and supported NURBS sampling; bounded display error and stable endpoints/seams across zoom/DPI changes. | `RenderCurveTest` |
| F4 | Solid fills, concave boundaries, holes/disconnected regions, hatch patterns and solid hatches, multi-stop linear gradients and degenerate gradient axes; define fill rule and alpha behavior. | `RenderFillTest` |
| F5 | Text/font selection, font enumeration/loading, metrics, rotation, model/device-size text, UTF-8 decoding and declared font fallback; dimensions, MText and insertion transforms remain correct. Document supported shaping rather than claiming a new shaping engine. | `RenderTextTest` |
| F6 | Raster image creation/drawing/destruction, transforms/UVs, transparency, image-format plugins and invalid handles/files; real antialias enable/disable semantics, including a defined fallback if MSAA is unavailable. | `RenderImageTest`, `RenderQualityTest` |
| F7 | RGB/RGBA/clear, all transforms and coordinate/distance conversions, complete save/restore, reset and translation queries; style snapshots, viewport clipping and background/document/foreground ordering. | `PainterContractTest`, `RenderTransformTest` |
| F8 | Entity caching, edit/remove/undo invalidation, layer/block changes, selection recoloring, view-dependent tessellation, multiple documents and bounded memory. Geometry and live appearance must not get stale. | `RenderCacheTest` |
| F9 | Grid/paper/background, cursor, snap markers, grips, selection rectangles, operation previews, pan/zoom, keyboard/focus/context menus, high DPI and MDI lifecycle. | `RenderWidgetTest` |
| F10 | Explicit offscreen render/readback, documented pixel format/stride/origin/alpha, image-buffer lifetime, CLI output dimensions and error codes, screenshot parity and supported export paths. | `RenderReadbackTest`, `RenderCliTest` |
| F11 | Lua/Python-driven drawing/document edits, custom entities, callbacks, plugins and preview/commit/cancel/undo continue to work; renderer smoke itself does not require Python. | Existing scripting suites plus `RenderScriptingTest` |
| F12 | Startup/backend selection, missing driver/device, out-of-memory and device-loss handling, resource recreation, orderly shutdown and repeated open/close. Errors are diagnosable and preserve unsaved documents. | `RenderDeviceTest`, `RenderLifetimeTest` |

These suite names are proposed new tests. Add fixtures for every entity drawable,
not only lines: images, inserts, polylines/bulges, text, dimensions, hatches and
splines. Reconcile unavailable features and estimates at R0; do not later declare
the migration complete merely because the currently implemented GL subset passes.

### Drawable and consumer coverage

Use a single committed selection manifest, proposed
`unittest/rendering/renderer_cases.json`, extended with the tests rather than a
second registry of rendering behavior. Map each case to F-groups, actual test IDs,
required configurations, input/font hashes and numeric/image assertions. R0 must
inventory these existing consumers; R2–R4 implement and execute their cases.
Define its minimal schema in R0: stable case ID, feature IDs, canonical painter
signatures/consumer, input digests, expected test IDs, required backend/mode/platform
configurations, oracle/threshold and expected success or diagnostic. Distinguish
overloads, negative cases and an explicitly out-of-domain input. Validate unique
IDs and bidirectional coverage: required cases execute, and an executed test cannot
replace a missing required case merely by increasing the total. Keep workload
metadata here too; do not maintain a second handwritten list of expected counts.

| Existing family or consumer | Required representative cases beyond the spike |
|---|---|
| Line, point, arc, circle, ellipse and LWPolyline drawables | Every primitive, CW/CCW sweeps, bulges, open/closed paths, joins/caps/dashes, degenerates and clipping; transformed and selected variants. |
| `LCVSpline` and its current Bézier decomposition | Line/quadratic/cubic spans, endpoint/tangent continuity, periodic/closed and high-curvature cases, zoom/DPR and large origins. Record unsupported document spline forms rather than inferring NURBS support from a displayed cubic. |
| Hatch, text, MText and image drawables | Nested/disconnected hatch loops, pattern transforms; metrics/multiline/fallback; affine/mirrored images and alpha. Invalid assets must have a tested error path. |
| Dimensions, endcaps and inserts | All five concrete dimension families (aligned, angular, diametric, linear, radial), endcaps; nested/transformed inserts, layer/block inheritance, recursion limits and viewport filtering. |
| Model/paper viewers and canvas drawables | Paper/background/grid, cursor/location, drag points/temp entities, model/paper tabs, selection and preview/commit/cancel/undo; z-order and scissor boundaries between views. |
| UI, CLI, scripting, Cairo and exports | Real viewport/input and screenshot; CLI raster dimensions/orientation/errors; Lua/Python custom draw/edit hooks; preserve existing Cairo/vector consumers without adding a new print/PDF subsystem. Inventory actual export call sites before promising output parity. |

For each family include empty, ordinary and resource-limit cases. Require cached
versus uncached equivalence and affected/unaffected-view assertions for edits,
layer/block changes and undo. A method-count check or a single composite image
cannot stand in for this consumer coverage.

M1 now implements the source-register component in
`scripts/validate_renderer_inventory.py`: all 60 canonical signatures, header/default
digest and 75 current consumer source/header roles. Full public-declaration parsing
rejects unknown forms rather than skipping them; role validation prevents a caller
being relabeled CPU/build-only to weaken coverage. The focused
`scripts/tests/test_renderer_cases.py` tests cover this component, not rendering.

Consumer discovery includes nested drawable source/header files and fixed host,
factory, event and binding boundaries. Its lexical guard detects new direct painter
references in existing production roots, across conditional branches while masking
comments/literals. It is not compiler preprocessing or alias/dependency analysis;
new host roots/backend splits still need a reviewed audit and mapping update.
Configuration/case validation remains draft. `validate_case_inputs()` and typed
value/fixture helpers exist, but `unittest/rendering/renderer_cases.json` and its
contract-input corpus do not. Only the draft OFL notice is present under the new
fixture directory; the 34 focused tests still exercise M1, not M2. The older
`validate_case_manifest()` repeats parsing and disagrees with the newer input path.
M2/M3 must finish and test those contracts; passing M1 does not establish case
completeness, numeric assertions, selection or production execution.

Keep the manifest small and declarative: one signature register, source consumers,
hashed fixtures/notices, valid configuration templates, cases and workload metadata.
Cases can share signatures, consumers and inputs; do not duplicate geometry or
invent a test executable per family. Specify these rules before completing the draft:

- Canonical signatures distinguish return/parameter types, overloads and qualifiers;
  the header digest also guards changed defaults. A bounded parser must reject an
  unfamiliar declaration, not silently omit it. Method counts remain a secondary check.
- Discover drawable sources and audit host/factory/binding call sites. A source's
  role cannot be relabeled CPU/build-only to bypass its graphics coverage. Changes
  to consumers or their supported domains require a reviewed mapping update.
- Expand only declared valid tuples. Reject unknown, empty, duplicate, unreachable
  or silently discarded selections, unsafe paths, missing inputs and unreferenced
  required rows. Keep source, installed, package, CPU/Cairo, offscreen, CLI and
  widget requirements distinct; dual builds also need explicit legacy-process proof.
- Fixtures contain concrete operands and typed expected assertions, not just a
  nonempty object or suite name. Pin font/image/drawing bytes and matching notices;
  define units, tolerances, reference/oracle and diagnostic reason. A resource-limit
  case crosses a declared limit deterministically without exhausting a runner.
- R0 records proposed production bindings, never executed results. At R1–R5 select
  the cases assigned to that stage and its completed prerequisites; later cases
  remain visible and mandatory at their owning gate. The reviewed CI gate chooses
  the stage/configuration, not test discovery or an app's self-reported readiness.
  R6/R7 rollout/retirement cases belong to their own gates, not R5's prerequisite set.

Run exact-ID equality on the selected GTest set, including parameterized IDs;
unrelated suites outside that filter are not excess failures. Runner-only smoke
IDs, process-contract IDs and GTest IDs have different producers and cannot replace
one another. Installed apps need not ship a GTest binary: package smoke binds to
the source qualification separately through the tested artifact manifest.

Each assertion also needs an identified observation from its native test or
runner: geometry/CPU state, rendered landmark/readback, application event, resource
counter or diagnostic. A well-typed expected value, test name or renderer-reported
"pass" is not that observation. Review every method's defaults, units, mutation,
query timing and failure behavior, including both overloads; attaching all methods
to one case is not semantic coverage. Reuse small native fixtures and ordinary
GTests, not a JSON scene interpreter or a generic assertion virtual machine.
The corpus is data; tests must call the actual painter/consumer and compare its
observed result with the independent oracle. Empty/limit cases must reach that
consumer's real operation/guard, not just assert a precomputed zero or injected label.

### Complete painter method register

| Group | Active methods covered |
|---|---|
| F1 | `new_path`, `close_path`, `new_sub_path`, `move_to`, `line_to`, `rectangle`, `point`, `stroke` |
| F2 | `lineWidthCompensation`, `line_width`, `set_dash`, `dash_destroy` |
| F3 | `arc`, `arcNegative`, `circle`, `ellipse`, `quadratic_curve_to`, `curve_to` |
| F4 | `fill`, `pattern_create_linear`, `pattern_add_color_stop_rgba`, `set_pattern_source`, `pattern_destroy` |
| F5 | `font_size`, `select_font_face`, `text`, `text_extends`, `getFontList`, `addFontsFromPath` |
| F6 | `image_create`, `image_destroy`, `image`, `disable_antialias`, `enable_antialias` |
| F7 | `clear` (RGB/RGBA), `source_rgb`, `source_rgba`, `scale` (query/set), `rotate`, `translate`, `user_to_device`, `device_to_user`, `user_to_device_distance`, `device_to_user_distance`, `save`, `restore`, `reset_transformations`, `getTranslate` |
| F8 | `isCachingEnabled`, `startcaching`, `finishcaching`, `getCacherpainter`, `isEntityCached`, `renderEntityCached`, `deleteEntityCached` |
| F10 | `data` |
| F12 | `create_resources`, `new_device_size`, destructor/shutdown ordering |

F9 and F11 test consumers of these methods rather than introducing additional
painter declarations. The existing renderer contract test checks 60 declarations
and 58 distinct method names against this register; M1 additionally checks the
canonical overload signatures and header/default digest. M2 must bind these to
actual operands and expected observations. An API change needs a same-change
register and expected-test update, not merely a new total count.

## 4. Compact architecture and invariants

### Drawing data and backend ownership

Keep CPU drawing preparation in `lcviewernoqt`, using ordinary C++/LibreCAD data.
Propose a small path/mesh and draw-command representation under its painter
directory, with explicit style, transform, origin, ordering, texture references,
and cache/revision keys. Recording a command cannot depend on a current GPU context.

Add one separate target, proposed `lcrenderer_rhi` under `lcrenderer/rhi/`, for QRhi
objects, pipelines, uploads, fonts/textures and submission. Link Qt Gui/private
headers only there and in the widget adapter, not into `lckernel` or public painter
headers. Initially support one render owner thread per device; CPU workers may
prepare immutable commands without touching QRhi or mutable document state.

Snapshot document/style inputs on their owning thread before asynchronous CPU
preparation; workers cannot traverse a mutating document. Bound queued preparation
and discard outdated document/view generations before upload. GPU caches belong
to one QRhi instance and its generation, not a global generation number alone.
Two top-level windows/offscreen hosts may share immutable CPU assets, never QRhi
buffers, textures or shader-resource bindings. Test simultaneous owners with the
same entity IDs and local generation numbers, then destroy/recreate one owner.
[QRhi threading and resource sharing](https://doc.qt.io/qt-6.8/qrhi.html#threading)

Keep link dependencies one-way: application/CLI/test hosts → selected renderer
targets → `lcviewernoqt` → kernel. Concrete RHI and legacy factories belong to
their backend targets and are called by hosts; the CPU viewer must not call/link
back into RHI. Move the current OpenGL factory with its implementation rather
than creating a circular library dependency. Test both static/shared linkage
where supported; public headers carry no Qt-private/native graphics types.

Keep the public `LCADViewer` event/document interface. A QWidget wrapper can own
either a private QRhiWidget surface or a temporary legacy surface, avoiding application-wide
conditional inheritance. Preserve focus and input coordinates through the wrapper;
audit all promoted widgets, bindings, public casts, signals and tests. Do not
promise unchanged C++ binary ABI when changing a public base class or layout.

Maintain a CPU `ViewState` for transforms, inverse queries, font metrics and visible
bounds even before the first GPU frame or while a surface is unavailable. Audit
legacy Y inversions in drawables/canvas; do not add a second flip during migration.
Before replacing a document/painter, detach the old canvas painter reference and
disconnect old document callbacks; cover never-initialized and document-less teardown.

### Frame and pass ownership

Use the following host/executor contract, not an immediate Vulkan interpretation
of each `LcPainter` call:

1. Record background, document and foreground commands in their existing order.
   Resolve CPU metrics, tessellation and cached command references without GPU
   submission. `stroke`, `fill` and image/text operations append commands.
2. On the render owner thread, prepare resources and one resource-update batch
   before beginning the render pass. Submit uploads with `beginPass` or a legal
   out-of-pass update; do not upload through `resourceUpdate` inside a pass.
3. Bind the current target-compatible pipelines, resources, viewport and scissor,
   then execute ordered commands. Do not begin three clearing passes for the three
   canvas layers. Handle any later `clear` in command order, not by moving all
   clears to the start. Preserve earlier content if an extra pass is necessary.
4. End the pass, schedule any resolved-texture readback, and return/submit under
   the host's ownership. Hold command/resource references through submission;
   no renderer method may start a nested frame.

QRhiWidget owns its QRhi/frame/target; use the callback's borrowed command buffer
and never call `beginFrame`/`endFrame` or destroy the borrowed device/target there.
The offscreen host instead owns its QRhi/target and pairs `beginOffscreenFrame`
with `endOffscreenFrame`. A headless host needs a suitable Qt platform plugin and,
where necessary, a virtual display even without a visible window.

Pipeline keys include render-pass compatibility, target format/sample count and
shader/binding layout. Recreate target-dependent state on resize/format/MSAA changes;
zero-sized/minimized surfaces suspend drawing without invalid allocations. Do not
cache a QRhiWidget target pointer or device across a device-generation change.
See [command recording](https://doc.qt.io/qt-6.8/qrhicommandbuffer.html) and
[widget lifecycle](https://doc.qt.io/qt-6.8/qrhiwidget.html).

Allocate distinct aligned per-draw uniform ranges, or an equivalent immutable
instance-data layout, before executing the pass. Repeatedly writing one dynamic
buffer range between draws does not preserve earlier draw values in QRhi's deferred
model. Test several overlapping draws with different transforms/colors/dash state
in one frame, plus two views and consecutive frames; every draw must retain its own
snapshot. Keep referenced resource objects/layouts unchanged until submission;
do not grow/recreate a referenced buffer midway through recording.
[Deferred command and dynamic-buffer semantics](https://doc.qt.io/qt-6.8/qrhi.html#command-buffers-and-deferred-command-execution)

### Portable geometry and style

Render strokes with triangle topology and vertex/fragment shaders, using CPU
topology generation or instanced vertex expansion. Do not require geometry shaders,
wide hardware lines, non-solid polygon modes, triangle fans, or GPU float64. Keep
join/cap behavior and dash distance accumulation explicit. Preserve painter order
when batching translucent content; do not sort arbitrary entities just by material.

Use doubles for model coordinates, camera/inverse transforms, path lengths and
CPU tolerances. Upload local-origin-relative floats and test tiny detail at large
world coordinates. Define logical pixels versus framebuffer pixels once; apply
backend clip-space/orientation corrections consistently to rendering and readback,
without making snapping or hit testing dependent on GPU rounding.
Use conservative CPU culling bounds that include stroke/cap/antialias expansion
at the current scale; final target scissoring must not hide a visible thick stroke,
point, rotated glyph or image crossing the nominal entity/view boundary.

Allocate the accepted screen-error budget across curve flattening, local-origin
float conversion and stroke expansion; independent bounds must cover their total,
not allow each step the full 0.25 pixel. D2/D3 must state the tested model-coordinate,
zoom and DPR domain. Include cancellation near large origins, reflected transforms
and singular/near-singular inverses; outside that domain report a bounded failure
rather than return nonfinite geometry or silently omit detail.

Tessellate curves adaptively with bounded screen error, span/end/cusp handling and
finite subdivision/output limits. Cache by curve revision and required tolerance;
rebuild when zoom/DPI invalidates the error bound. Print/export uses its own
world-space tolerance. A sampled display path never replaces exact document data.
Reuse existing CPU hatch preparation only after validating finite/nondegenerate
bounds, loop topology and bounded pattern expansion. In
[lcvhatch.cpp](lcviewernoqt/drawitems/lcvhatch.cpp), the solid-hatch loop derives its
step from bounding-box height without a zero-step guard and uses a fixed 300-step
float approximation. It cannot establish the new tolerance/large-origin contract.
R2 must guard degenerate input and adapt or replace that local preparation where
required for F4; porting the GPU fill alone is insufficient. Profile hatch CPU work
separately, without requiring a CAD-region/kernel rewrite.

Capture all state needed by queued commands. `save`/`restore` must include color,
dash, line width, font, transform and other advertised state, not just a matrix.
Keep pattern/image handles as validated opaque IDs, not native pointers cast to
`long` (whose width differs on Windows). Specify invalid-handle behavior and
image/readback ownership without silently returning successful empty output.
Test handle exhaustion, generation rollover and stale-ID reuse under Windows'
32-bit `long` contract as well as the 64-bit Unix contract; a small injected ID
space makes these deterministic without billions of allocations.
Keep the current path separate from saved graphics state; document whether each
stroke/fill consumes it and test subpaths/closed paths explicitly. Image transforms
are the affine basis vectors and insertion point supplied by `LCImage`, not an
assumption that UV arguments are normalized texture coordinates. Test skew/mirror.

Use bounded handle IDs representable by `long` on all platforms. Repeated
`image_create` for the same asset/version reuses decoded content without accumulating
an unbounded reference per draw; `LCImage` currently does not balance each draw with
`image_destroy`. Commands/cache entries retain their content after public-handle
destruction until no recording/submission uses it. Define asset-change detection,
eviction and stale-handle errors; apply the same retained-snapshot rule to gradients.

### GPU lifetime and assets

Use persistent geometry and bounded caches, transient upload/uniform storage, and
QRhi-supported deferred retirement. Edits/removals cannot free resources still used
by submitted frames. Invalidate GPU objects by device generation; view reparenting
or device recreation must never reuse handles belonging to the old device. Retain
CPU definitions sufficient to rebuild. Avoid whole-device waits per entity/edit.
Distinguish Qt object lifetime from native GPU retirement: objects referenced by
the frame being recorded survive to submission, using `QRhiResource::deleteLater`
where appropriate; QRhi defers native destruction for in-flight GPU work. Do not
add a parallel Vulkan fence allocator to the RHI implementation. All QRhi resource
creation/use/destruction stays on its owner thread. [Resource lifetime contract](https://doc.qt.io/qt-6.8/qrhiresource.html)
Disconnect document/async consumers before releasing painter caches, release the
executor's resources while its device is alive, then let the widget/owned offscreen
device shut down. Generation guards must also cover callbacks arriving after close.

Use the same idempotent release routine from the derived widget destructor and
its early-release callback. The callback handles reparent/device changes but does
not replace destructor cleanup; drop old resources before returning, not at the
next initialize call. Query borrowed targets only in their allowed callbacks and
preserve QRhiWidget resize handling. [Early-release contract](https://doc.qt.io/qt-6.8/qrhiwidget.html#releaseResources)

Use a small explicit host state machine: uninitialized → ready → suspended
(zero-size/hidden target), or failed/recreating → ready/failed, then closing →
closed. Document state and CPU view queries survive every GPU transition. A
failed frame produces no successful smoke/readback result. Invalidate the old
device generation, release its resources before Qt's device dies, and rebuild
only after the host supplies a valid replacement. Bound retries and expose the
failure to the UI/CLI; never loop initialization or silently change APIs. Test
initialization/allocation/submit failures separately from normal target resize,
same-device surface replacement and complete device recreation.

Owned offscreen recreation is not a widget recovery implementation. For borrowed
devices, R0 must identify the exact Qt/provider notification and host/window
recreation path that can supply a replacement. If safe in-process recreation is
unavailable, D3/D6 must accept an explicit rendering-failed UI with document
save/undo and user-directed restart, rather than promise transparent recovery.
No renderer path may destroy/recreate Qt's borrowed QRhi itself.

Do not treat `QRhiWidget::renderFailed()` as a universal device-loss notification:
its documented case is rendering without an available QRhi. Qualify the specific
provider's frame/window failure signals separately, and route both through the
same latched host status and document-preserving UI. A submitted-frame signal is
not proof that readback or display presentation completed.
[Widget failure signal](https://doc.qt.io/qt-6.8/qrhiwidget.html#renderFailed)

Latch the first terminal host failure independently of the event-loop exit code.
Stop new rendering/capture/retry work, invalidate deliveries and retain the original
diagnostic; a later timer, completion or `exit(0)` cannot clear it. For proof/CLI
processes, decide final status and publish accepted outputs only after the relevant
windows, scene resources and devices have torn down successfully. Interactive
screenshots need completed-frame validity, not application shutdown; their status
must not masquerade as a successful process-lifetime test.

Cache identity includes the document, entity change generation, block/insert
context, geometry-affecting style, tolerance/DPI and device generation. A renderer
generation counter may be required: do not assume the current entities expose a
revision API. Layer/block edits must invalidate affected instances; live selection
color must remain live in cached and uncached paths. Eviction cannot invalidate
queued commands; remove/undo/reinsert of the same ID must not resurrect stale meshes.

Replace per-glyph GL objects with owned glyph atlas pages and batched textured
quads while preserving metrics/fallback behavior. Define straight versus premultiplied
alpha, color-space conversions, clipping, antialias coverage and multisample resolve
as tested contracts. Do not hide unsupported content as a blank drawing.
Start with single-sample rendering and analytic stroke coverage, so per-command
antialias controls do not change a target's sample count halfway through a frame.
Qualify MSAA separately; read its resolved texture, not a multisample attachment.
R0 fixes SDR target/blend-space and alpha conventions with transparent/opaque
background fixtures; HDR/color-management expansion is out of scope. Premultiply
exactly once where selected and do not infer alpha semantics from a QImage label.

### Readback and CPU data lifetime

Add an explicit host-level readback result carrying dimensions, stride, channel
order, origin and alpha convention. Normalize encoded image output deliberately;
do not reinterpret Cairo ARGB32, GL BGR and RHI RGBA buffers as one implicit format.
For the new painter, `data()` accesses the most recent completed host-requested CPU
snapshot, valid until the next completed snapshot, resize or painter destruction;
before one exists it reports a defined not-ready error. It must not block/nest a
frame from inside drawable recording. Existing Cairo buffer behavior is unchanged.

An onscreen readback may complete later: keep its `QRhiReadbackResult` and payload
alive, guard consumer lifetime and associate a device/frame generation. A completed
offscreen frame can provide synchronous data, but that guarantee must not be applied
to normal widget submission. Test close/resize during pending readback and drop
stale logical results without use-after-free. [Readback result](https://doc.qt.io/qt-6.8/qrhireadbackresult.html)

Completion runs on the QRhi owner thread. Copy/retain the CPU payload before
queuing delivery to the GUI or encoder; an invalid GUI consumer must not free a
still-pending GPU result. Settle outstanding readbacks on teardown using the
qualified Qt backend's completion/shutdown contract, then discard stale delivery.
Do not cancel by destroying the result or calling a borrowed device after release.
The production host needs a bounded teardown test, not the spike's wait as its
design. Define `data()` not-ready as `nullptr` plus host status/diagnostic without
changing the existing raw-pointer signature; the typed host snapshot owns bytes.

A failed `finish()` is not a completion guarantee. Keep pending CPU result storage
outside the closing widget until confirmed completion or the qualified backend's
destruction has ended; do not keep borrowed GPU objects alive through that owner.
In Qt 6.8.3, cleanup callbacks run and are cleared **before** backend destruction,
so a cleanup callback alone is not a GPU-completion/last-owner barrier.
[Versioned Qt teardown source](https://github.com/qt/qtbase/blob/v6.8.3/src/gui/rhi/qrhi.cpp#L7753)
R0 can use a bounded CPU-only retirement holder that outlives its QApplication;
the production host must separately qualify early device replacement and shutdown.
Record acquisition, invalidation, actual completion and last-owner release order;
a deliberate status substitution after completion does not prove pending-GPU teardown.

Build shaders offline through `qt_add_shaders()`/matching `qsb`; ship `.qsb` assets
or embed them as resources. Do not require a shader compiler or SDK on end-user
machines, or load absolute source-tree shader paths. Pin matching Qt development,
private-header and tool packages per tested platform; record their exact versions.
QRhi compatibility across Qt minor versions is a qualification responsibility.
Treat the exact tested build/private-header/tool/runtime tuple as the default
supported tuple: QRhi has no general binary-compatibility promise. D2 must qualify
any different runtime patch/minor rather than relying on ordinary public Qt ABI
expectations. Rebuild and rerun after provider changes; distro packages must not
silently upgrade private headers/runtime independently. [QRhi compatibility](https://doc.qt.io/qt-6.8/qrhi.html)
Only add a persistent pipeline cache if cold-start measurements justify it; key
it by backend/device/driver, Qt and shader compatibility, bound its size and store
it in a writable user cache, not the bundle. Invalid cache data is a cache miss,
not a reason to fail startup or change APIs.

### Backend selection and optional builds

Propose `WITH_RHI_RENDERER`, `WITH_LEGACY_OPENGL_RENDERER` and
`WITH_RENDERER_TESTS`, initially keeping the old default until R6. Fresh defaults
are RHI OFF and legacy ON; new renderer tests default ON only with unit tests and
RHI enabled, and old rendering tests default ON only with unit tests and legacy
enabled. Conflicting explicit/cached requests are errors, not forced OFF options.
All new names/options in this plan are future interfaces. Keep old tests separate from new
renderer tests so the old `WITH_RENDERING_UNITTESTS=OFF` cannot disable the latter.
Explicitly requested missing dependencies/backends must fail configuration or startup;
do not silently compile away the renderer or replace a forced Vulkan run with GL.

Propose `--renderer=auto|vulkan|metal|rhi-opengl|legacy-opengl`. Unsupported platform
or compiled-out choices fail clearly. Before R6, `auto` chooses legacy when it is
compiled; an explicit RHI-only build chooses the platform RHI policy. At R6,
`auto` chooses Vulkan/Metal in dual builds too. `rhi-opengl` is optional and remains
rejected unless D1/D6 explicitly retain and qualify it; it is not an implicit
fallback. Log the effective choice and cutover-policy version. Any
compatibility retry must log its reason and construct a fresh eligible window.
Choose one API before constructing the top-level widget hierarchy. Backend changes
require an explicit restart/recreation strategy, not hot-swapping MDI children.
Set `QRhiWidget::setApi()` explicitly on each RHI surface before exposure; Qt's
default is not this policy (it defaults to D3D11 on Windows and OpenGL on Linux).
Mixed API requests within one top-level widget hierarchy are invalid. [Qt API selection](https://doc.qt.io/qt-6.8/qrhiwidget.html#setApi)
Parse renderer selection before QApplication-dependent graphics setup; enable
`AA_ShareOpenGLContexts` only for the selected legacy configuration when needed.
Construct exactly one surface implementation per viewer; a hidden legacy widget
must not force GL composition in a Vulkan/Metal window. Test compatibility backends
in separate processes. A startup retry is bounded and must not loop; forced API
and CI modes never retry another API.

RHI-only builds must configure/link with legacy discovery and factories disabled:
no application GL/GLEW/GLFW calls, headers, generated GL resources or direct links.
Qt itself may retain an optional OpenGL backend/dependency; do not confuse removing
LibreCAD's legacy backend with promising a Qt distribution containing no GL code.
When both GPU backends are disabled, kernel/CPU tests remain buildable without Qt
or GL; GPU executables are either explicitly disabled or rejected at configure time.
An offscreen RHI CLI may use Qt Gui without Widgets when `WITH_QT_UI=OFF`.

Build-option truth table, using `RHI`, `legacy`, `UI`, `CLI` and `tests` as
abbreviations for the named CMake options:

| RHI / legacy | UI / CLI | Valid configuration and test ownership |
|---|---|---|
| OFF / OFF | OFF / OFF | CPU/kernel and painter-contract tests only; no Qt/GL discovery. Either GPU test switch ON is an error. |
| ON / OFF | ON or OFF / ON or OFF | RHI is explicit; Qt Gui/private + ShaderTools required. Widgets required only for UI/widget tests. RHI offscreen tests remain available with UI OFF. |
| OFF / ON | ON or OFF / ON or OFF | Legacy host/GL requirements remain; new renderer tests cannot be enabled. Old rendering-test switch controls only old tests. |
| ON / ON | ON or OFF / ON or OFF | Both compiled, one selected per process; each gets its own test run. Link only enabled factories/targets. |
| OFF / OFF | either host ON | Configure error. CPU/Cairo library/export tests may exist, but an unspecified CPU GUI/CLI backend is not part of this migration. |

Explicit Python-OFF never means graphics-OFF. Requested test/executable targets
must either exist with their expected suites or fail configuration. No provider
branch may override a cache option silently. If legacy is OFF, an explicit old
rendering-test request is an error even when RHI is ON. Record verified effective
flags from CMake, not just the workflow command line.

## 5. Implementation stages and exit gates

R0 is **active**; R1 implementation is **provisional**, without acceptance;
R2–R7 remain **proposed**. Acceptance order is R0 → R1 → R2 → R3/R4 → R5 → R6 →
R7. R0 feasibility scaffolding exists; production build/test CI begins in R1 and
grows with every stage. R5 is final platform/package qualification, not the first
time CI runs the renderer.
Each numbered item is a small implementation/review boundary.

### R0 Feasibility and acceptance baseline

1. Reconcile this baseline with the actual implementation branch. Inventory every
   painter method, drawable, backend consumer, GL dependency and platform/provider.
   Capture representative drawings and existing images; distinguish intentional
   behavior from bugs/stubs. Agree on text/shaping, vector output and image scope.
2. Prove a minimal RHI widget and offscreen path on Vulkan Windows/Linux and Metal
   on native macOS arm64/x86_64. Include thick/dashed joined strokes, text, large
   coordinates, resize/DPI, readback and two MDI views. Verify Qt has the requested
   backend enabled; an SDK's presence is not evidence that its Qt build supports it.
3. Qualify exact Qt/private-header/qsb providers and software ICDs for hosted CI;
   identify real GPU runner access. Keep an unavailable architecture/device as an
   open gate rather than treating compilation or emulation as native acceptance.
   Review redistribution licenses/notices for the selected Qt modules, software
   driver, shader tools and bundled fonts/images; retain source and artifact provenance.
4. Record CPU/GPU frame timing, allocation/upload counts, cache warm/cold behavior,
   idle behavior, memory and artifact size on reference workloads/hardware. Set
   numeric visual-error, memory and interaction budgets before renderer implementation.

**Exit:** accepted API/platform policy, complete method inventory, reproducible
spike, explicit hardware/CI provisioning plan and recorded budgets. If the RHI
private-API or platform policy is unacceptable, choose section 9 now. No default
change or performance improvement is implied by this gate.

R0 accepts the production acceptance specification and proves feasibility; it
does not require the not-yet-built production renderer or final packages to pass.
Native prototype/provider/session requirements belong here; feature execution
lands in R1–R4 and final installed/hardware/package qualification closes in R5.
Do not create a dependency cycle by demanding R5 artifacts before R1 can start,
or reverse it by accepting R1 without R0's required decisions and prototype proof.

### R0 decision record and first implementation batch

The following decisions are **open**, not implicit permissions to drop scope.
Close them before R1 with measured/reproducible evidence and explicit acceptance;
partial spike results do not implicitly accept a decision.

| Decision | Concrete output required before R1 |
|---|---|
| D1 API policy | Confirm Vulkan Windows/Linux and Metal macOS, or select section 9. Record the reason and resulting support claims. |
| D2 Provider and capability floor | Per-platform Qt/private headers/qsb versions, compiler/CRT, deployment OS, shader target set and minimum device limits. Probe formats, sample counts, buffer/texture limits and feature availability; reject unsupported devices before partial rendering. |
| D3 Graphics contracts | Ratify path consumption/fill rule, width/dash units, SDR blend/alpha/readback format, font/fallback support and all 60 method mappings. No new complex-script shaper or HDR requirement is implied. |
| D4 Runnable CI | Exact native runner/image, Qt plugin, software ICD digest or real device, graphical-session/Wayland setup, and command for each required architecture. List missing runner access as an unresolved gate. |
| D5 Acceptance budgets | Pin fixtures/reference hardware and numeric geometry, raster, memory, latency and cold-start thresholds. Proposed adaptive-curve error is at most 0.25 physical pixel; qualify it at each tested DPR/zoom instead of a fixed vertex count. |
| D6 Rollout | Identify the compatibility release, observation/rollback criteria, explicit fallback policy and authority for changing defaults/retiring legacy code. |

D2 must record both the CMake/compiler floor and the full production Qt module set,
not just the spike's Gui/Widgets/ShaderTools subset. D3 must choose a bounded fill
tessellation strategy, stroke topology/coverage rules and CPU font/metrics provider,
with dependency/license impact and independent oracles. These are R0 decisions,
not unspecified research silently carried into an otherwise ready R2/R3 task.

#### D3 implementation inputs

Prepare the following compact decision packet with the case manifest. These are
review candidates, not accepted algorithms or implemented painter features. Each
choice names its input domain, error policy, finite work/byte limits, dependency
revision/license and the stage that implements it. Approving prose alone cannot
prove an algorithm's robustness or a platform's behavior.

| Boundary | Candidate implementation and concrete acceptance input |
|---|---|
| Path and style | One CPU path/state recorder, consumed by both hosts; decide path consumption, defaults and save/restore independently. Use a 3–4–5 segment, nested transforms, empty/repeated stroke/fill and singular/nonfinite inputs with analytic positions and unchanged later valid state. |
| Curves and strokes | Double-precision bounded subdivision plus ordered triangle strokes. Specify the existing API's default caps/joins/miter policy without inventing public setters. Include CW/CCW quarter circles, rotated ellipses, bulges, quadratic/cubic endpoints, cusp/inflection and dash seams. Use independent bounds/subdivision verification, not only the renderer's own samples. |
| Fills and hatches | Select one bounded CPU tessellator/adapter, not a new CAD Boolean engine. Decide winding, touching/self-intersection domain and pattern limits. A 4×4 outer square with a 2×2 hole must have area 12 and correct interior/hole membership; include reversed/disconnected loops and zero-height hatch bounds. |
| Text and assets | Prefer reusing FreeType CPU metrics with owned glyph pages if its audited behavior meets D3; choose a bounded UTF-8 decoder/fallback policy first. Pin a real font and design-unit bearings/advances, then test placement in dimensions/MText. Keep Qt image decoding, if chosen, inside the private Qt-dependent target; neither QImage nor QRhi types enter CPU public headers. |
| Raster and readback | Fix SDR transfer/blending, alpha and origin/stride conventions. Use original odd-sized RGBA input with transparent edges, affine/mirror transforms and known overlapping colors; test checked size/stride arithmetic, incomplete completion and failed output writes. Fully transparent RGB is compared only under the declared normalization. |
| Consumers and caching | Include every concrete dimension family and all endcap enum values, nested inserts/layers, paper/grid/cursor/preview and Lua/Python edit/undo. Compare cached/uncached and affected/unaffected views; a shared primitive fixture does not prove the caller's transforms, event routing or invalidation. |
| Failure and limits | Define diagnostics and last-valid-frame behavior for invalid geometry/assets, exhausted declared budgets and unavailable devices. Small injected limits test rejection/chunking without real memory exhaustion; document/save/undo survive a renderer failure. Shader packages are trusted application assets, never accepted from a drawing or unchecked cache. |

The existing `LCVSpline` sends two-, three- and four-control-point Bézier spans
to line/quadratic/cubic painter methods. Test that path now; rational/higher-order
support requires an actual document sampling provider, not a new Vulkan primitive.
Keep any future SMLib sampling integration conditional and backend-neutral.
The existing ABeeZee font is a possible fixture, not yet the pinned production
oracle: verify the exact binary/version and corresponding redistribution notice
before reuse, rather than attaching a newer upstream font's metadata to old bytes.
The new `unittest/rendering/fixtures/ABeeZee-OFL.txt` is a draft notice, not a
validated fixture binding. Distinguish raw font design-unit advances/bearings from
hinted/scaled raster metrics and supported shaping; licensing alone establishes
none of those rendering behaviors. Independently check the pinned binary's cmap
and metrics, with explicit replacement-glyph and malformed-UTF-8 expectations.

Audit existing consumer defects before fixing baseline expectations. In
`drawitems/endcaps.h`, `TICK` is enum value 7, but `EndCaps` does not register it
and `render()` uses `map::at`. Closed caps also call `fill()` followed by `stroke()`
on the same path. D3 must specify intended tick geometry and filled/outlined caps;
R2/R3 must test/fix these callers under the accepted path-consumption rule, not
preserve a throw or a missing outline as golden behavior. No such code fix is
included in this plan-only review.

The standalone `lcrhispike_offscreen` and `lcrhispike_widget` targets now exist under
`lcrenderer/rhi/spike/`; see the [spike instructions](lcrenderer/rhi/spike/README.md).
They do not change production factories/defaults. Their shared scene uses original
project-licensed bitmap glyph/image fixtures, joined/dashed triangles, alpha overlap
and large-coordinate placement. Bitmap text proves atlas sampling, not production
font loading/shaping. The offscreen executable owns its device; the widget executable
uses two MDI views, Qt-owned frames, asynchronous readback and real target resize.

| Batch | Files or boundary | Required result |
|---|---|---|
| A Provider and standalone build | New spike CMake/scene/shaders and plan-map entry; selected prebuilt Qt/tool/ICD manifests. | Cold-cache native configure/build; correct shader package/tool versions and dependency closure; explicit failure with unsupported provider/backend. |
| B Functional proof | Spike executables plus a small deterministic image/landmark check using the proposed shared scripts. | Forced Vulkan software and hardware runs where required, native Metal on both Mac architectures, no fallback/Null pass; correct strokes, text, alpha, coordinate origin and readback. |
| C UI and decision closeout | Widget input/lifecycle checks, evidence files and D1–D6 records. | MDI focus/context menu/resize/DPI and close-during-readback pass; all required runners identified; budgets and direct/RHI route explicitly ratified. |

The standalone build commands are available; they are not the application's
production renderer options. The README documents the implemented executable flags:

```sh
cmake -S lcrenderer/rhi/spike -B build-renderer-spike \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="/absolute/qualified-qt-prefix"
cmake --build build-renderer-spike --config Release
```

Run these executables through `scripts/run_renderer_checks.py --spike`, forcing
the API/device/mode and expected source/architecture. R0 reports have explicit
`r0-spike` and `r0-spike-runner` success kinds, with separate `r0-spike-failure` and
`r0-spike-expected-failure` terminal-failure kinds; none satisfies production feature,
GTest or package acceptance. The runner binds executable/report/image hashes,
checks actual PNG/RGBA pixels against the committed fixture, validates source-input
freshness, and rejects missing/excess views, unexposed widget capture or fallback.
Native Windows/macOS Intel execution, Linux x64 and Linux hardware/desktop sessions
still need D4 qualification. Managed Wayland software evidence now exists on
ARM64; minimum-Qt CI build definitions are not execution evidence. If a decision
cannot close, stop at that gate instead of advancing to production integration.

### R0 implementation checkpoint

| Work package | Current state | Next unmet acceptance |
|---|---|---|
| A Provider/build | Partial: native ARM64 macOS Qt 6.8.3 and 6.12.0, native-ISA Linux ARM64 VM Qt 6.10.2, separate Widgets-OFF builds and pinned minimum-Mac SDK/tool inputs. Native Intel R1 source CI now passes, but native Intel R0 runtime remains unqualified. Mac widget preflight rejects a locked console. | R0 hosted/minimum-Qt Windows/Linux and native Mac Intel/runtime UI, provider redistribution, deployment OS/CRT and capability-floor qualification. Neither source CI nor cross-builds substitute for required R0 execution. |
| B Functional proof | Partial: fixture v2 has 15 landmarks, opaque/transparent targets and straight-alpha output. The retained supervised cohort records each Metal provider (Qt 6.8.3 and 6.12.0) at 12 offscreen invocations / 512 checks. ARM64 software Vulkan separately records 24 / 1,384 on XCB and 24 / 1,384 on actual Wayland clients, including five reparent cases per QPA. These sample F1/F2/F5/F6/F7/F9/F10/F12, not their complete contracts. | Refresh changed helper bindings after the draft is complete; native Windows/Linux x64/Mac Intel, Linux hardware/desktop, production geometry/fonts, physical DPI, quality/MSAA and production/window loss recovery remain open. |
| C UI/decisions | Partial: retained XCB/managed-Wayland executions pass input/focus, exposed popup, close delivery, suspension, reparent/survivor and DPR checks. Each QPA records six terminal failures / 18 assertions for persistent failure, blocked success writers/zero-exit override and bounded CPU retirement. Owned-process cleanup has separate execution-time proof. Wayland popup input is virtual keyboard input, not a native pointer-grab test. | Stable-helper refresh, missing native/hosted providers/sessions, physical input/monitor DPI, window-loss recovery, workloads/budgets and D1–D6 remain open. Shutdown substitution after real completion is not pending-GPU destruction proof. |
| CI | [renderer.yml](.github/workflows/renderer.yml) retains R0 runtime jobs/CPU rejection fixtures and calls the same-commit [R1 source workflow](.github/workflows/renderer-source.yml). R1 now passes all 60 source jobs plus protocol/aggregate on all five native architectures, with all current artifacts retained/reviewed. Only the pinned patched libdxfrw submodule builds as an external dependency; other providers stay binary-only. The R0 **per Linux architecture** XCB/Wayland matrices still require 48 positives / 2,768 IDs and 12 expected failures / 36 IDs. Native process-contract tests must execute without skips. | R1 source execution is green, not R0 hosted runtime or phase acceptance. Full R0/provider/hardware/desktop/deployment/package/release qualification and independent review remain open; this is not the final renderer-required gate. |

The working route remains RHI/Vulkan Windows+Linux and Metal macOS. D1–D6 remain
open until their full outputs above are accepted. Existing production renderer,
scripting and package behavior is unchanged. Qt's imported provider still supplies
an OpenGL framework link on macOS; this prototype does not claim an OpenGL-free
Qt distribution or completion of R1's application dependency audit.

The widget proof now latches terminal failure outside the event-loop exit status,
guards rendering/delivery/output, and defers normal PNG/RGBA/report acceptance until
window and QApplication teardown. Failed/incomplete drain retains CPU capture state
in two fixed slots beyond Qt destruction, never borrowed GPU objects. Controlled
tests attempt both successful writers and a zero-exit request after failure. Real
native drain failure and physically pending GPU teardown remain unobserved; the
tested shutdown failure is substituted after actual successful completion.

#### Retained schema 3 supervised evidence

Revision `132265a0573253980451facf9109678f8bffe743`, unchanged dirty compiled-input
SHA256 `a151d043776423225289e6a588e321d9d101fc6ae934803208e6d5b2999350bd`.
Only Python runner/supervision, tests, CI recipes, source visibility and documentation
changed in that completed batch; no C++/shader/fixture/provider SDK rebuild was needed.
Those runs bind their execution-time process/runner/validator/Wayland helpers.

Freshness review: sampled minimum/current Mac `supervised-v4-*` reports bind validator
SHA256 `6646017b0859eb0a1bdabeb580c84af7b3337cbe92d26ed3b806064094114689`.
M1's locally tested validator was
`806997d45e1dd5ca163b3a08376ff36a0764ea18bf5e1ba46586953e52ac5406`;
the unfinished M2 draft is now
`83f5183b3fd0ce4be0773c7286d39039aaf7e5e3be4fa05e826e13c6c90a013b`.
The compiled spike input remains unchanged, but neither M1 nor the partial M2
helper has a current GPU execution cohort. Finish M2/M3 and their negatives, then
rerun available required matrices once at stable helper bytes; never replace the
hash inside an old receipt. Preserve unavailable native/session/provider gates
and retain the old reports as history. This documentation review changes neither
helper nor compiled inputs.

| Provider and retained temporary roots | Result recorded by the completed cleanup batch |
|---|---|
| Linux ARM64 native-ISA VM / Qt 6.10.2 / Lavapipe / XCB+Xvfb | `/tmp/librecad-rhi-r0.ItHtgR` plus `nogui`, `supervised-v4-xcb-*`: 24 positives / 1,384 IDs plus six expected failures / 18 separate IDs, zero failed/skipped. All 30 scopes verify launcher status, owned-process and display/auth retirement. |
| Same Linux software provider / actual Wayland client / Weston X11+Pixman inside Xvfb | Same roots, `supervised-v4-wayland-*`: separate 24 / 1,384 plus six / 18, zero failed/skipped. All 30 scopes also bind compositor/input/log identity. This preserves the virtual keyboard-popup, not physical-input/native-pointer-grab, scope below. |
| Native ARM64 Metal/Cocoa / Qt 6.8.3 and Qt 6.12.0 | Existing minimum `/tmp/librecad-qt683-r0.7It56j/{build-arm64,nogui-arm64}` and current `/tmp/librecad-rhi-spike.UDfPi5` plus `nogui`, prefix `supervised-v4-{opaque,transparent}-{standard,lifecycle,recovery}`: each provider separately passes 12 offscreen / 512, zero failed/skipped. C++-unchanged CTest 6/6 per provider; no Mac widget or Intel runtime claim. |

The [supervisor](scripts/renderer_process.py) runs in a fresh, single-threaded,
child-free Linux CLI with default SIGCHLD handling. It becomes a subreaper before
launch, uses checked start/parent identities and pidfds, and stays until adopted
children are waited for or bounded cleanup fails. Private Xauthority, the owned
Xvfb lock/socket file identities and short runtime directory must retire too;
unrelated replacements are never deleted/signaled. Workers stage candidates only;
the parent revalidates helpers/binary/report/pixels or failure assertions and rejects
late unreported output before publishing `checks.json`. Process scope is additive
schema 1, `r0-process-scope`; app/runner schema 3 and production boundaries stay intact.

That batch's [process tests](scripts/tests/test_renderer_process.py) passed 19/19 on Linux:
double-fork/setsid and TERM-resistant descendants, app crash/failure/timeout,
supervisor interruption, preserved unrelated work, SIGCHLD/thread guards, real Xvfb
failure/timeout and escaped-child compositor startup failure. These are CPU fixtures,
not GPU success cases. Its full Linux script regressions passed 195/195 without skips;
the Mac run had 195 with 185 passed and ten explicitly native-Linux cases skipped.

The source-inventory audit also found the required compatibility CMake module
hidden by the root `*.cmake` ignore rule. A single-file exception now includes it
in candidate discovery, with a regression checking Git visibility of every spike
source input. The module's bytes and compiled-input digest are unchanged; generated
CMake files stay ignored. The Python plan maps `.gitignore` to the affected build,
source/install and platform gates rather than exempting it.

That batch's final report read-back verified its helpers, binaries, app/failure reports,
pixels and all 60 process/display manifests. Cleanup reaped 60 Xvfb, 30 Weston
keyboard and 30 desktop-shell descendants after launcher exit; two TERM signals
were needed, no KILL in the graphics matrix. The baseline's 328 pre-existing
defunct display/compositor identities remained, with no new ones, live test display
or remaining private runtime directory. CPU fixtures separately cover KILL escalation.
That local qualification closed the cleanup batch at its recorded inputs, not R0 or the
missing hosted/x64/native desktop/hardware/package/decision gates. The earlier
unreaped PID-1 children remain preserved, not misreported as newly cleaned up.

#### Historical schema 3 Wayland software evidence

The following compiled cohort is unchanged, but its older helper-bound reports
are superseded by the supervised runs above.

Revision `132265a0573253980451facf9109678f8bffe743`, dirty compiled-input SHA256
`a151d043776423225289e6a588e321d9d101fc6ae934803208e6d5b2999350bd`.
Fixture v2 and scene/shader geometry are unchanged. The source changes since the
terminal-failure cohort concern popup exposure/native-input dispatch and timeout
diagnostics. The runner also binds the new Wayland helper hash. The matrices below
were executed and revalidated in that earlier batch at its source/helper/binary
identities, including actual PNG/RGBA or failure assertions. They are not
current-helper validation for this documentation review.

| Provider and evidence root | Executed result for the current inputs |
|---|---|
| Native ARM64 Qt 6.8.3 / Apple M2 Max Metal / Cocoa | `/tmp/librecad-qt683-r0.7It56j/{build-arm64,nogui-arm64}`, prefix `wayland-v3-{opaque,transparent}-{standard,lifecycle,recovery}`: 12 offscreen invocations / 512 checks, zero failed/skipped. Retained CTest result is 6/6. Both Intel targets rebuild but remain cross-build only. |
| Native ARM64 Qt 6.12.0 / Apple M2 Max Metal / Cocoa | `/tmp/librecad-rhi-spike.UDfPi5` plus `nogui`, same `wayland-v3-*` cohort: separate 12 / 512, zero failed/skipped; retained CTest 6/6. No current-source Mac widget run is accepted. |
| Linux ARM64 native-ISA Ubuntu 26.04 VM, Qt 6.10.2 / Lavapipe / XCB+Xvfb | `/tmp/librecad-rhi-r0.ItHtgR` plus `nogui`, `wayland-v3-xcb-*`: 24 successes / 1,384 IDs and separate six failures / 18 assertions, zero failed/skipped. |
| Same Linux software provider / Wayland client / Weston 14.0.2 X11 backend and Pixman compositor inside Xvfb | Same roots, `wayland-v3-wayland-*`: separate 24 successes / 1,384 IDs and six failures / 18 assertions, zero failed/skipped. All 30 compositor records verify the selected backend/renderer, private socket, app DISPLAY removal, provider/log hashes, liveness through app exit and compositor return 0. Five lifecycle cases retain a checked native F12 acknowledgment. |

Per QPA the 24 positive invocations comprise eight background × standard/lifecycle
× offscreen/widget cases, two owned offscreen recovery, two widget reparent, six
Gui-only offscreen and six fractional-DPR lifecycle/reparent cases. The six failures
are both backgrounds × render/readback/shutdown. Do not combine positive IDs,
expected-failure IDs, QPAs or Qt providers into one feature-completeness count.

The [Wayland helper](scripts/renderer_wayland.py) starts a separate compositor and
checks the app's own status; compositor success cannot mask app failure. It uses a
short private socket path independent of the evidence-directory depth. On verified
owned Xvfb only, XTest sends F12 to Weston's window. After observed focus/synthetic
mouse and Space checks, the widget requests its keyboard context menu **inside the
spontaneous F12 handler**, not a later timer; it requires active, visible and exposed
popup state. An atomic acknowledgment stops further key seeding. This is real
Wayland-client/software-presentation and virtual keyboard-popup evidence, not
DRM/KMS hardware, physical-input, native pointer-grab or monitor-DPI qualification.

The [ARM64 Weston provider pin](lcrenderer/rhi/providers/weston14-linux-arm64.json)
identifies seven checksum/size-verified prebuilt archives, extracted under
`/tmp/librecad-wayland-r0.le91Br/overlay/usr`. Selected compositor modules/helpers
and xdotool are fingerprinted per run. No dependency was compiled from source or
installed globally. This subset is not a complete SDK/deployment/license closure;
versioned apt requests and a package list alone do not make hosted CI reproducible.

At that checkpoint, read-only inspection found defunct Xvfb/Weston descendants
adopted by container PID 1, with no live test display/compositor. Those reports did
not attribute/reap every descendant; `tini -s` and compositor return 0 were not
sufficient proof. The later supervised batch above adds bounded descendant cleanup
without retroactively cleaning these old PID-1 children. Preserve these rendering
results as partial functional evidence, not a clean-process qualification.

The last Mac widget preflights used the older terminal-failure cohort and rejected
a locked console; they do not qualify this changed widget source. An authorized
unlocked graphical session and new current-source runs are still required. Local
`/tmp` evidence must be retained with source/helper/provider artifacts before phase
approval. D1–D6, missing native/session/hardware, production and package gates stay open.

#### Historical schema 3 terminal failure evidence

The following cohort is superseded for current-source/helper acceptance.

Revision `132265a0573253980451facf9109678f8bffe743`, dirty compiled-input SHA256
`0590fd38fab3e1ba1b4a7da85e3d59029a8b30b8cfc2ab1ddac522a1f09d7d62`.
Fixture v2, scene/shaders and provider/tool digests are unchanged. Each runner binds
actual executable/helper/report/output hashes; expected failures carry no successful
pixel/GTest count. No dependency was installed or built from source for this batch.

| Provider and evidence root | Executed current-source result |
|---|---|
| Native ARM64 Qt 6.8.3 / Apple M2 Max Metal / Cocoa | `/tmp/librecad-qt683-r0.7It56j/{build-arm64,nogui-arm64}`, prefix `terminal-v2-{opaque,transparent}-{standard,lifecycle,recovery}`: 12 offscreen invocations / 512 checks, zero failed/skipped; CTest 6/6. Both Intel targets rebuild, but remain cross-build only. |
| Native ARM64 Qt 6.12.0 / Apple M2 Max Metal / Cocoa | `/tmp/librecad-rhi-spike.UDfPi5` and `nogui`, same `terminal-v2-*` matrix: separate 12 / 512, zero failed/skipped; CTest 6/6. |
| Linux ARM64 native-ISA VM, Qt 6.10.2 / Lavapipe / XCB+Xvfb | `/tmp/librecad-rhi-r0.ItHtgR` plus `nogui`, `terminal-v2-*`: 24 successes / 1,384 IDs, zero failed/skipped, preserving both backgrounds, standard/lifecycle/recovery/reparent and fractional DPR. Six `terminal-v2-failure-{opaque,transparent}-{render,readback,shutdown}` invocations separately pass 18 expected-failure assertions. Software/VM, not hardware/hosted qualification. |

Both shutdown reports record actual drain result 0, substituted result 1, completed
readback before/after draining, one retired/released capture and all retained owners
expired after QApplication destruction. Readback injection observes pending recording
then completion by widget release. All six failure directories contain only logs,
`failure.json` and `checks.json`: no accepted app report, PNG or RGBA. A later
zero-exit request cannot clear the persistent terminal state. This exercises host
ownership/error handling, not physical GPU loss or failed native queue completion.

Mac widget preflight remains locked for both providers, with no success record:
current `terminal-v2-widget-session/run-s_w3hyyr`, minimum
`build-arm64/terminal-v2-widget-session/run-d4n52yor`. The changed widget source
therefore remains unqualified on Mac. Helper regressions pass 163/163 (renderer
module 71), including malformed/unrelated fault, stale identity, secondary error,
success artifact, unsafe retirement and changed-helper/binary cases. Retain these
temporary reports for phase review; native/session/hardware/Wayland/provider,
workload/budget and D1–D6 acceptance remain open.

#### Historical schema 3 minimum provider evidence

The following source cohort is superseded for current-source acceptance.

Revision `132265a0573253980451facf9109678f8bffe743`, dirty compiled-input SHA256
`4978755ef2ae2ec10a3a63d81739736e38fec05c41ce511fc86eb4187c8ddb3e`.
The new CMake compatibility module is included in both generator and validator
input manifests. Fixture v2 and all scene/shader geometry remain unchanged.

| Provider and evidence root | Current executed result |
|---|---|
| Official Qt/private/qsb 6.8.3 universal SDK; native ARM64 on macOS 27.0.1, AppleClang 21.0.0.21000334, Apple SDK 27.0, CMake 4.4.4 | Root `/tmp/librecad-qt683-r0.7It56j`; fresh `build-arm64` and `nogui-arm64`, each `qualified-provider-v3-{opaque,transparent}-{standard,lifecycle,recovery}`: 12 native Metal/Cocoa hardware invocations / 512 checks, zero failed/skipped; CTest 6/6. `CMAKE_OSX_ARCHITECTURES=arm64`, deployment flag 12.0; old-AGL compatibility true. |
| Current native ARM64 Qt/private/qsb 6.12.0 / Apple M2 Max Metal / Cocoa | `/tmp/librecad-rhi-spike.UDfPi5` and `nogui`, prefix `qualified-provider-v3-*`: separate 12 / 512, zero failed/skipped; CTest 6/6; AGL compatibility false. Existing provider digests below are unchanged. |
| Linux ARM64 native-ISA VM, Qt/private/qsb 6.10.2 / Lavapipe / XCB+Xvfb | Existing `/tmp/librecad-rhi-r0.ItHtgR` container build and `nogui`, prefix `qualified-provider-v3-*`; same 24-case configuration/DPR matrix below, 1,384 checks, zero failed/skipped; AGL compatibility false. Software/VM, not native Linux hardware or hosted evidence. |

The [minimum-Mac provider input manifest](lcrenderer/rhi/providers/qt683-macos.json)
pins verified Qt Base/ShaderTools archives and qsb SHA256
`b5cfd05045ca18485ac897001e53df88d28c72472fd002e99173eb11c212eab3`.
No host package was installed and no dependency was built from source; downloaded
SDK bytes stayed under `/tmp` and were not modified. The initial native link
failed because old Qt exported AGL despite absent SDK stubs. The narrowly scoped
Metal-host CMake adjustment follows Qt 6.10's consumer-link removal, preserves all
other dependencies, records its effective flag and has six synthetic configure
cases. Qt binary runtime OpenGL/AGL dependencies are not removed.

`cross-x86_64` in the minimum-provider root successfully builds both targets and
records x86_64 Mach-O/source/provider identity, but was **not executed** on Intel
or Rosetta. The universal SDK and deployment flag 12.0 are not native Intel or
macOS 12 runtime qualification. CPU-feature queries denied in the sandbox make
old `qsb` abort there; native version/shader commands pass without feature overrides.

Current Mac widget preflights fail before window creation with a locked console:
minimum SDK `build-arm64/qualified-provider-v3-widget-session/run-zpwillkc`, current
provider `qualified-provider-v3-widget-session/run-t8_3sjkl`. Neither has a success
report. Script regressions pass 154/154; actionlint passes. Archive/tool pins and
local builds are not hosted execution, redistribution review, package closure or
D1–D6 acceptance. Retain ephemeral evidence before phase approval; R1–R7 stay gated.

#### Historical schema 3 widget evidence

The following source cohort is superseded for current-source acceptance.

Revision `132265a0573253980451facf9109678f8bffe743`, dirty compiled-input SHA256
`8f50661eb1671c4d9e7fe380738b696294be0f847ab845e6393d68b1fde8b8de`.
Fixture v2, shaders and provider versions/digests below are unchanged. Each run
retains its actual command and executable/helper/report/PNG/RGBA hashes.

| Provider and existing root | Current executed matrix |
|---|---|
| Native Apple M2 Max / Metal / Cocoa, Qt 6.12.0; `/tmp/librecad-rhi-spike.UDfPi5` | Main and `nogui`: `validated-ui-v3-{opaque,transparent}-{standard,lifecycle,recovery}`, 12 invocations / 512 checks, zero failed/skipped; CTest 6/6. |
| Native-ISA Linux ARM64 VM / Lavapipe / XCB+Xvfb, Qt 6.10.2; container root `/tmp/librecad-rhi-r0.ItHtgR` | Main: two backgrounds × standard/lifecycle × offscreen/widget (8 / 254), owned recovery (2 / 132), widget reparent (2 / 256); DPR 1.25/1.5/2 × transparent lifecycle/reparent (6 / 486); separate `nogui` offscreen backgrounds × standard/lifecycle/recovery (6 / 256). Total 24 / 1,384, zero failed/skipped. Prefix `validated-ui-v3-*`; fractional runs use `validated-ui-v3-widget-{lifecycle,reparent}-dpr-*`. Software/VM evidence, not hardware or hosted qualification. |

Reparent is a widget-only additive scenario in schema 3: eight pixel snapshots of
the same two surfaces at initial/resumed/detached/returned stages plus ordinary
widget composition and seven explicit lifecycle checks, 128 IDs per invocation.
The moved scene's observed device generations are 1/1/2/3 and resource releases
0/0/1/2; the survivor remains generation 1. Actual Qt device-sharing/window checks,
per-capture API/class/provider/capabilities, exposed frames and fresh readback
generations are validated. No borrowed device/frame/target is owned by the widget.

`validated-ui-v3-transparent-reparent-widget/run-owpjzkm4` observes hidden and
visible 0×0 intervals of 253/200 ms with zero scene draws. Its completed current
CPU payload is held before recapture invalidation, then delivered and dropped
exactly once without setting an image. Automatic resize invalidation and target-size
readiness prevent old snapshots passing as current. These are bounded prototype
observations, not the production idle/power budget, physical-input/monitor-DPI or
physical GPU-reset proof.

The five software-Vulkan reparent cases are opaque/transparent at DPR 1 and
transparent at DPR 1.25/1.5/2. Mac
`validated-ui-v3-widget-session/run-hwb1y3hd` still fails locked-console preflight
before opening a window, without a success report. Full script regressions pass
151/151; workflow actionlint passes. R0 and D1–D6 remain open; production factories
and defaults are unchanged. Retain these ephemeral `/tmp` artifacts before phase
acceptance, and qualify missing native providers rather than extrapolating results.

#### Historical schema 3 owned failure evidence

The following source cohort is superseded for current-source acceptance.

Revision `132265a0573253980451facf9109678f8bffe743`, dirty compiled-input SHA256
`620cb9023f2d62f22fd414d1c14dc99fc16701192e9827ad782a40223f089609`.
The original fixture v2 and shader geometry remain unchanged (108 vertices, 5,504
initial mesh/atlas bytes). Provider versions and qsb/Lavapipe digests are those
recorded below; current binaries/helper/output hashes are bound by each new report.

| Provider and existing root | Current matrix and evidence |
|---|---|
| Native Apple M2 Max / Metal / Cocoa, Qt 6.12.0; `/tmp/librecad-rhi-spike.UDfPi5` | Main and `nogui`: `final-v3-{opaque,transparent}-{standard,lifecycle,recovery}/run-*/checks.json`, 12 invocations / 512 checks, zero failed/skipped. Recovery is 66 checks; standard/lifecycle remain 15/47. Metal CTest passes 6/6. |
| Native-ISA Linux ARM64 VM / Lavapipe / XCB+Xvfb, Qt 6.10.2; container root `/tmp/librecad-rhi-r0.ItHtgR` | `final-v3-{opaque,transparent}-{standard,lifecycle}-{offscreen,widget}`: 8 / 254; `final-v3-{opaque,transparent}-recovery-offscreen`: 2 / 132; fractional `final-v3-widget-dpr-{1.25,1.5,2}`: 3 / 102; `nogui/final-v3-{opaque,transparent}-{standard,lifecycle,recovery}`: 6 / 256. Total 19 / 744, zero failed/skipped. This is software/VM, not hardware/hosted qualification. |

Commands retain the v2 API/device/full-revision/architecture/QPA/ICD/DPR assertions
below and add explicit `--scenario recovery` only to owned offscreen runs. The
four injected points are device creation, texture allocation, frame start and
submission status. Each verifies failure cleanup/no accepted image, then real
device recreation/render/readback. Per recovery run, seven QRhi owners are created
and destroyed; all four recovered generations have their own checked output.
Every initialized failed/recovered attempt records observed API/device/class;
the validator enforces the requested device class for each, not only the final one.
The submit case replaces a real successful synchronous submission result with
`FrameOpDeviceLost` **after** actual readback completes. Reports distinguish actual
and effective status, mark host-result substitution and reject physical-loss claims;
this is neither a driver reset nor a real allocation/device-loss observation.
CPU readback ownership now outlives device teardown even on error. The runner
rejects unexpected/unreported evidence files as well as incomplete pixel reports.

Current Mac `final-v3-widget-session/run-oggions9` again fails the locked-console
preflight before opening a window and produces no success report. Current Mac UI
acceptance stays open. Local `/tmp` evidence needs retained review/CI artifacts
before phase acceptance. New helper tests cover the capability floor, every
fault/recovered generation, rejected failed pixels, physical-loss overclaim,
schema-2 rejection and borrowed-host misuse. No production factory/default changed.

#### Historical v2 evidence

The following schema-2 results are superseded by the current compiled inputs.

Full revision `132265a0573253980451facf9109678f8bffe743` plus dirty compiled-input
SHA256 `bc3804d8ccddb3e8429c4c64e0d66b464be5db2066a1a0c1177acd8109d92095`
identifies the historical runs below. Reports use schema 2, fixture `cad-rhi-spike-v2`,
108 vertices and 5,504 initial mesh/atlas upload bytes (excluding per-frame uniform
updates). They bind actual output/helper/binary hashes, not a clean release tree.

| Provider and evidence root | Current executed matrix |
|---|---|
| Apple M2 Max / Metal / Cocoa; native ARM64, Release, AppleClang 21.0.0.21000334, Qt/private/qsb 6.12.0. Root `/tmp/librecad-rhi-spike.UDfPi5`. | Both main and `nogui` builds: `verified-v2-{opaque,transparent}-{standard,lifecycle}/run-*/checks.json`; 8 invocations, 248 checks, zero failed/skipped. Standard offscreen is 15 checks; lifecycle is 47 including three generations. Local CTest log records all 4 offscreen cases passing. |
| Ubuntu 26.04 ARM64 VM / Lavapipe / XCB+Xvfb; GCC 15.2.0, Qt/private/qsb 6.10.2, Mesa 26.0.8. Existing container `librecad-python-linux-arm64-20261010`, root `/tmp/librecad-rhi-r0.ItHtgR`. | `verified-v2-{opaque,transparent}-{standard,lifecycle}-{offscreen,widget}`: 8 invocations / 254 checks; `verified-v2-widget-dpr-{1.25,1.5,2}`: 3 / 102; `nogui/verified-v2-{opaque,transparent}-{standard,lifecycle}`: 4 / 124. Total 15 / 480, zero failed/skipped. Software/native-ISA VM evidence, not hosted or Linux hardware qualification. |

Provider qsb and Lavapipe library digests are unchanged from the v1 table below.
Each invocation forces backend/device/mode, expected full revision/ARM64/QPA and
explicit `--background`/`--scenario`; fractional widget runs additionally bind
observed `--expected-dpr` and `QT_SCALE_FACTOR`. Software runs force and declare
`/usr/share/vulkan/icd.d/lvp_icd.json`; widget runs use Xvfb 2048×1536. Full
per-run commands are retained in `checks.json`. Local `/tmp` evidence is ephemeral;
copy it to retained CI/review artifacts before relying on it for phase acceptance.

`verified-v2-transparent-lifecycle-widget/run-d1gxg3hb` records one consumer
invalidation after readback recording/before submission, one later completion and
stale-delivery drop, and one rendered surface replacement. GPU work was **not**
pending at actual destruction. This proves guarded logical delivery, not physical
device loss, pending-GPU destruction or Qt device recreation. Offscreen lifecycle
instead creates/renders/reads/destroys three owned devices; neither scenario proves
recovery after an injected submit/allocation failure.

Mac `verified-v2-widget-session/run-l8a47995` fails before opening a window with
`macOS console is locked; unlock the desktop before widget checks`; no success
report is present. Earlier Mac v1 widget results do not qualify the changed v2
widget source. An unavailable/locked session is an open validation gate, not a
passing skip or grounds for focus/activation workarounds.

#### Historical v1 baseline

The following v1 results are preserved for comparison and are superseded for
current-source acceptance. They use revision `132265a0573253980451facf9109678f8bffe743`
plus dirty compiled-input manifest SHA256
`ab3f3d4cf85a5441ca0ad6b0b4a0de8187c81697d9a553d2e2c4205b762b22a8`.
Both platforms execute the same input manifest; no clean release-tree claim is made.

| Provider / execution | Exact local evidence |
|---|---|
| Native macOS ARM64, Apple M2 Max / Metal / Cocoa | Release, AppleClang 21.0.0.21000334, Qt/private/qsb 6.12.0; qsb SHA256 `f3dafc00fab5f4b19ed349ba7b471e768f3440613e2fd56c1dd7cc3ea5333e9a`. Build `/tmp/librecad-rhi-spike.UDfPi5`; `accepted-metal-widget-dpr-{1,1.25,1.5,2}` pass 25 checks each; `accepted-metal-offscreen` and `accepted-metal-nogui` pass 12 each. CTest 1/1; actual unexposed-QPA and unsupported-API negatives fail without success reports. Native GPU execution required leaving the sandbox. |
| Linux ARM64 native ISA in local Docker VM, forced Lavapipe / XCB+Xvfb | Release, GCC 15.2.0, Qt/private/qsb 6.10.2, Mesa 26.0.8; observed software device `llvmpipe (LLVM 21.1.8, 128 bits)`. qsb SHA256 `de5a13ed06a4a47f985c78f6ce2bbf8418c0d3ac61352df834071cdf132a4a42`; `libvulkan_lvp.so` SHA256 `1ed1c5216fb9ff75453582c1ef9aa5004911e68fc99b11cf991d736e574c672d`. Existing `librecad-python-linux-arm64-20261010` container; isolated build/tool overlay `/tmp/librecad-rhi-r0.ItHtgR`, no installed package changes. `accepted-widget-dpr-{1,1.25,1.5,2}` pass 25 each; `accepted-offscreen-{build,nogui}` pass 12 each. This is software/VM evidence, not Linux hardware or hosted qualification. |

Each command is the README's `run_renderer_checks.py --spike` invocation with
`--backend metal --device hardware --expected-qpa cocoa` or
`--backend vulkan --device software --expected-qpa xcb --icd-manifest /usr/share/vulkan/icd.d/lvp_icd.json`;
widget runs additionally force `QT_SCALE_FACTOR` and `--expected-dpr` to each listed
value. All runs require the full source SHA and ARM64 architecture. JSON records
bind exact executable/report/helper/PNG/RGBA hashes and observed frames; offscreen
and Gui-only targets are distinct builds. Qt's own GL libraries remain present.
The fixture submits 96 vertices and uploads 5,120 initial bytes; single cold
device-create-to-readback samples are about 9.4 ms Metal and 26.9 ms software Vulkan,
not p95 interaction/GPU timing or accepted production budgets. The current checkpoint
above supersedes the former next tasks; remaining R0 work is ordered below.

### R0 closeout and implementation readiness

These are the next small batches, not authorization to change defaults or install
providers on a developer host. Name an accountable implementation owner and
reviewer in each batch receipt before starting it; roles below are responsibilities,
not assignments to people. Unavailable external capacity remains an open gate.
Independent R0 proofs may proceed while external provider/session access is
pending; this does not close the missing gate. Only the explicit provisional R1
exception authorizes its scoped implementation, not R1 acceptance.

#### Platform readiness

Each row requires its own accepted minimum/current provider, source runtime,
presentation and section 7 installed artifacts. This is an evidence/access summary,
not another platform-state ledger. All final platform gates remain open.

| Required tuple | Present R0 evidence | Next prerequisite and required proof |
|---|---|---|
| Linux x86_64 / Vulkan / XCB and Wayland | R1 source jobs now pass 12/12, including software offscreen under both QPAs; full R0 widget/prototype qualification is still open. | Retain/review full x64 provider and current-helper R0 case matrices; then hardware/desktop/deployment and install/AppImage/Snap proof. |
| Linux arm64 / Vulkan / XCB and Wayland | Historical d4e9650e source CI passes 12/12. Current 965e4465 clean-source R0 software proof passes all60 XCB/Wayland tuples and 50 unretried DPR1.25 lifecycle runs with active core/synchronization validation. Protocol tracing reproduces and fixes the old release-before-grab race; all assertions/deadlines remain intact. | Independently review the source-bound payloads and refresh source CI after push. Qualify wider desktop/input/DPI reliability, provider/runtime closure, deployment floor, physical hardware, then clean install. Narrow software proof does not accept widgets/platforms. No ARM64 AppImage/Snap is added. |
| Windows x64 / Vulkan / Windows QPA | R1 source jobs now pass 12/12; selected retained core artifact executes 22 RHI IDs under the software provider. This is not accepted R0/widget/hardware proof. | Complete/review retained matching MSVC/CRT/Qt/private/qsb/ICD inputs and current-helper R0 cases; obtain an authorized desktop, then hardware/deployment and installed NSIS proof. |
| macOS arm64 / Metal / Cocoa | R1 source CI passes 12/12, including hardware-class Metal offscreen. Earlier minimum/current R0 offscreen results retain older helper bindings; last widget preflight rejected a locked console. | Refresh stable-helper R0 offscreen evidence and obtain an authorized usable console for current-source widget/failure/reparent/input/DPR on both providers; qualify deployment floor and final relocated/signed bundle. Never unlock or bypass the session in a harness. |
| macOS x86_64 / Metal / Cocoa | R1 source CI passes 12/12 with verified virtual Metal; both library/tests-ON groups execute all 22 RHI IDs, and UI/combined hosts execute 197 non-widget IDs each. R0 runtime/hardware remains unaccepted. | Independently review source evidence, then qualify real supported hardware/current-helper R0 offscreen/widget/failure cases, full providers/deployment floor and final Intel bundle. Rosetta/cross-build/ARM64/virtual-source results are not substitutes. |

For D1–D6 record decision, rationale, affected F-groups/platforms, exact accepted
provider or contract, evidence, accountable approver and date. Unassigned owners,
unaccepted budgets or inaccessible sessions keep the dependent task not ready.
The implementation owner/self-review for the existing R0 work is Codex;
independent acceptance and release authority are not assigned by this plan.

#### Readiness priorities

Completeness means all required rows are assigned, not that they pass. Correctness
requires independent oracles and failure-safe ownership. Readiness requires inputs,
decisions, access and runnable acceptance. Apply that distinction to these open items:

| Priority and item | Current readiness and implementation boundary | Required closure |
|---|---|---|
| P0 R0 test-process cleanup | The shared supervisor now anchors TERM/KILL graces to signal attempts and supports Mac Python 3.12 without reaping the pinned root. Current ARM64 source, controller, clock-jump and offscreen Metal/XCB/Wayland checks pass; earlier R0 rendering matrices remain bound to their historical helpers. The recorded failed-run PID-1 zombie and 328 old display identities remain untouched. | Retain current source proof, refresh required R0 cases and qualify missing native hosts/providers. Keep failure/timeout/startup/crash, bounded exhausted-grace failure and exact ownership checks; never restart a container or normalize failed evidence. |
| P0 R0 terminal failure | Implemented in shared `support.*` and widget host; six cases **per Linux QPA** test both writers and a controlled zero-exit request after failure. | Rerun on missing native widget providers; preserve existing pixel matrices. Local proof does not establish real device-loss recovery. |
| P0 R0 readback retirement | Implemented fixed two-slot CPU-only retention beyond QApplication and direct stale delivery after terminal failure. Linux tests release one completed retained capture per substituted shutdown failure. | Native widget/provider qualification and actual pending-GPU/failing-backend behavior remain open. Do not reinterpret completed synthetic retirement as physically pending destruction. |
| P0 R0 failure evidence | Implemented additive schema-3 failure kinds, common identity validation, strict exit/artifact/error/ownership checks and helper negatives; each Linux QPA passes six cases / 18 IDs separately. | Execute the same cases on missing native backends/sessions; hosted and production acceptance remain separate. |
| P0 D2/D4 providers and sessions | The exact-SHA R1 source cohort passes on all five native architectures; verified Intel virtual Metal is approved for this source scope only. Complete R0 cases, desktop/hardware, deployment floors and reviewed full provider closure are still missing. | Independently review the retained per-configuration source/provider/command evidence and obtain missing hardware/session/deployment proof. Do not relabel source build/offscreen proof as R0 widget, physical-input or hardware qualification. |
| P1 Wayland fractional-DPR popup reliability | Targeted serial race corrected in b0618650/965e4465: an unchanged-source trace reproduces release before popup grab and popup_done before exposure. A readiness/held-key/exposure-ack handshake passes the final exact-source full60 and 50 consecutive unretried DPR1.25 cases, retaining all assertions/deadlines and zero Vulkan errors. | Review retained traces, binaries, pixels and cleanup; run new hosted source CI, then qualify wider providers/physical input/desktop DPI and all remaining widget acceptance. Prior failed artifacts stay retained. This fixes the reproduced harness race, not general hardware or production-viewport acceptance. |
| P1 D3 geometry and assets | The shared on-disk manifest/corpus and inventory CLI validate 56 cases, 223 proposed variants, 60 signatures and 75 consumers. Initial R1 source/offscreen bindings execute; structural coverage is not full painter, geometry or asset correctness. Choices/oracles are not ratified. | Extend the existing typed inputs, independent oracles and stage-appropriate bindings without dropping required cases. Resolve endcap/path-consumption defects in the authorized stage; retain the D3 decision packet and missing feature/consumer gates. |
| P1 D5 workloads and D1/D6 approval | Inputs/hardware/thresholds and policy/rollout acceptance remain open. Provisional R1 implementation is separately authorized above. | Measured workload worksheet, accepted numeric limits and D1–D6 decision receipts remain required for R1 acceptance, not its authorized provisional implementation. |

The terminal-output/CPU-retirement, managed Wayland presentation and Linux
test-process cleanup baselines have execution-time local proof. The changed
supervisor/controller boundary now has current ARM64 source proof below;
required R0 rendering-case and missing-provider refreshes remain open.
The following local batches can be prepared without missing hardware or an API
policy decision. R1 acceptance and R2–R7 remain **not ready**; provisional R1
source work has its separate authorization and closeout list. This order is not
permission to install host dependencies,
dispatch trusted CI or accept a platform/policy on a maintainer's behalf.

#### Next local implementation batches

| Order and bounded change | Implementation details and inputs | Acceptance and next action |
|---|---|---|
| 1 Test supervisor and cleanup | Keep the existing runner/private helper, fresh-owner guards, subreaper/pidfd adoption and exact status-preserving reaping. Corrected signal-phase deadlines and the Mac Python 3.12 non-reaping adapter are locally verified; Xvfb authentication/file and helper/tool/log identities remain bound to acceptance. | Current ARM64 source and managed-Wayland reruns pass. Next refresh required R0 cases and missing native/hosted providers; CPU/controller/source proofs do not close graphics/device lifetime, physical input or phase acceptance. Preserve historical failed scopes. |
| 2 Selection manifest and D3 inputs | Extend the existing `unittest/rendering/renderer_cases.json`, corpus and inventory validator/tests. The canonical register already maps all 60 signatures and 75 consumers to required staged cases with typed fixtures, IDs, thresholds and diagnostic domains. Keep unimplemented production cases required; never substitute spike IDs or create a second harness. | Preserve duplicate/orphan/unknown-tuple/missing-fixture/unit rejection tests and valid repeated references to a signature. Expand actual discovery/execution in its authorized stage, retain unsupported cases as open, and submit fill/stroke/font/alpha decisions with independent analytic oracles for D3 review. |
| 3 Provider retention and missing-session recipes | Extend existing per-platform provider pins, not a second provider framework. Separate downloaded archive hashes, resolved package inventories, runtime loader/plugin dependencies, notices and minimum-OS claims. Retain the actual tools/SDK/ICD inputs and cache-miss restore recipe. | Verify hashes before executing tools; cold restore/build/run each accessible native tuple. Record inaccessible tuple/session explicitly with its owner/unblock action. Archive current temporary source patch/manifest, binaries, helpers and reports before they disappear. Missing inputs prevent R0 acceptance. |

Batch 1 has current ARM64 controller/source proof; required R0 rendering cases and
missing native/provider bindings still need refresh and qualification.
Batch 2's single case manifest remains the next implementation task, with batch 3's
retention/provider work independent. Split batch 2 into these review-sized patches:

| Subpatch | Files and required acceptance |
|---|---|
| M1 Schema and source register | Implemented in the existing validator and focused `scripts/tests/test_renderer_cases.py`, with the same validation called by the draft manifest validator. Covers all 60 signatures, header/default identity from the same complete header bytes and 75 current source roles. All 34 focused tests pass locally; Linux full scripts pass 229/229 without skips and Mac passes 219 with ten native-Linux skips. CI definitions require a nonempty/no-skip M1 module on both Linux architectures and minimum-Qt Windows/Mac jobs; no hosted execution is claimed. Old report CLI behavior is preserved. No manifest/case-execution acceptance is implied. |
| M2 Inputs and assertions | In progress, not locally accepted. M2.1 writes the single manifest/corpus, pins actual asset/notice bytes and finishes one bounded typed-input path. M2.2 adds real-repository positive tests, malformed-value/hash/license/reference mutations and independent analytic/font/PNG oracle checks. Cover every method/F-group/consumer with meaningful ordinary/empty/limit observations and diagnostic reasons. Preserve M1 and old report behavior. No tuple or rendering acceptance is implied. |
| M3 Valid selection | M3.1 replaces the incompatible duplicate validator with selection over M2's validated data and explicit valid coverage layers. M3.2 adds the inventory-only CLI below and contract-job wiring. Test all declared stages/configurations, exact bindings, zero/duplicate/unreachable selections, Python-OFF/UI-OFF/Windows-CLI exclusions, Cairo, package runner checks and separate legacy-process proof. Output requirements only; neither arbitrary products nor missing-runner exclusions may rewrite scope. |
| M4 Stable-helper qualification | Run all old and new helper negatives, plan/path checks and workflow lint; then refresh accessible spike matrices once at final helper bytes. Bind binaries/reports/process records/outputs to those bytes, retain both cohorts and report missing tuples separately. Manifest success does not qualify production GTests, D3 approval or R0 completion. |

M1's CPU component is complete locally; next finish M2's real inputs and then
M3's valid stage/configuration selection, preserving M1 regressions. M2/M3's CPU
work can proceed without graphics access; M4 and provider retention
need their actual execution/storage prerequisites. Before writing code, assign the
batch owner/reviewer and run section 8 checks. Use the spike README's implemented
runtime commands for available providers; do not present future section 6 production
flags as runnable acceptance today. New rendering-test paths affect additional
Python-plan test gates; derive their mapping rather than copying this review's count.

#### M2 and M3 correction checklist

Resolve these observed draft inconsistencies before either component is accepted;
do not add a third validator or leave mutually incompatible schemas behind.

| Draft defect or missing input | Bounded correction and regression |
|---|---|
| No on-disk manifest/corpus or M2-specific positive test | Add the real files and require successful repository-wide `validate_case_inputs()` before mutation tests. Hash and parse each fixture/notice from the same bounded byte snapshot. Check Git visibility and LF/binary attributes on native Windows as well as Unix; preserve existing DXF binary policy. |
| `validate_case_manifest()` reparses fixtures/oracles and rejects newer `script-source`/workload-variant fields | Consume M2's already validated structures once; mutate each field through that shared path. Fixture scripts are inert input/provenance here, never executed by the metadata validator. |
| `case_configurations()` requires software × hardware across source/install/packages | Implement section 6's coverage layers. D4's explicitly qualified Windows software-or-trusted-hardware route is a declared alternative; separate hardware qualification stays mandatory. Missing access remains pending, never an automatic alternative/waiver. |
| Mixed profile/mode predicates conflict; legacy selections are checked as native Vulkan/Metal | Validate supported relations, not every cross-pair of a case's mode/profile lists. Give the legacy process its real backend and separate IDs; `any` device cannot satisfy software/hardware or performance proof. Reject genuinely unreachable requested rows rather than silently filtering them. |
| Data types cannot establish semantic or numerical correctness | Independently recompute analytic lengths/areas/transforms/curve bounds, decode the odd-sized image and check actual font metrics. Separate categorical equality, dimensional error and performance ratios/absolute limits; malformed UTF-8, huge integers, NaN, overflow and oracle-unit swaps must fail safely. |
| No inventory CLI or CI invocation of the real corpus/selector | Add the proposed mode below, preserve old report CLI tests and run every valid stage/tuple in CPU contract CI. Proposed IDs remain requirements until a later native producer discovers and executes them. |

Proposed M3 command contract, **not implemented today**:

```sh
python3 -B scripts/validate_renderer_inventory.py \
  --cases unittest/rendering/renderer_cases.json
python3 -B scripts/validate_renderer_inventory.py \
  --cases unittest/rendering/renderer_cases.json \
  --stage R2 --configuration '<declared-expanded-tuple-id>'
```

The first mode validates all inputs/coverage and reports inventory totals. The
second requires both selection arguments and returns a stable sorted JSON
requirements list containing case/variant, producer/test ID, stage, tuple and
fixture/oracle identities. Select the owning stage plus its completed required
prerequisites; keep later stages visible in the inventory, not executed prematurely.
Unknown/missing/empty/incompatible selections exit nonzero without a success
record. This mode performs no builds, application launches, test discovery,
baseline updates or gate promotion. Keep the existing report positional CLI
mutually exclusive and backward compatible. R1 now has initial exact-ID/XML
execution; R4 extends that same path to the complete application regression set.

Resolve profiles to reviewed effective RHI/legacy/UI/Python/test flags; the
`rhi-only` label alone does not establish Python OFF. Separate Lua and Python
case prerequisites, and never make package smoke depend on a shipped GTest binary.
Bind each logical tuple to its actual provider/build cohort so minimum/current
Qt and declared Debug/Release runs cannot satisfy one another merely by sharing
test IDs. Keep these bindings in the existing manifest/build records and CI matrix,
not another handwritten platform ledger.

Before R1 acceptance, the R0 reviewer must accept the resulting manifest/D3 contracts,
decision records D1–D6 and required prototype/provider evidence. Completing the
local work does not assign release authority or substitute for
missing native/session/hardware measurements. Every later batch receipt must
name its concrete files/targets, prerequisite versions, owner/reviewer, command,
expected selected IDs and stop condition; an unassigned reviewer is not acceptance.

#### Terminal widget failure batch

This batch is implemented in the existing spike/support and shared helper files,
with owner and self-review Codex; independent phase acceptance remains pending.
The following contracts remain required for every additional native qualification:

1. Add a persistent fatal latch with original/secondary diagnostics. Guard render,
   readiness, queued delivery and the final writer. Make event-loop return a request
   to finalize, not the final accepted result; never throw from a destructor.
2. Separate CPU snapshots from serialization. Stop timers and close proof windows,
   release scene resources while their borrowed devices are valid, then finish Qt
   teardown before deciding process success. If unresolved readbacks were retired,
   verify last-owner release after the qualified backend lifetime; test the chosen
   PNG/JSON serialization order on each Qt provider rather than assuming plugins
   remain usable after QApplication destruction.
3. Inject one render failure, one failure after readback recording, and one shutdown
   drain-result failure in separate standard-widget processes. Record actual and
   substituted status and whether GPU completion already occurred. Use one chosen
   view; verify another view or late success callback cannot overwrite the failure.
   Never force a physical GPU reset or fabricate pending work after a completed drain.
4. Use the implemented expected-failure record/runner contract in section 6,
   including malformed/unrelated failure negatives. On Linux run both backgrounds
   at all three points: six failures and 24 successes **per QPA**, with separate
   counts. Native Mac/Windows failures need their own runs;
   locked-session, missing ICD, timeout, crash and signal exits cannot satisfy this batch.
5. Rerun existing success matrices and helper tests at the new compiled-input digest,
   preserve old receipts as stale-source history, retain diagnostics/artifacts and
   update this checkpoint. Completion closes this batch, not R0 or device-loss recovery.

#### Remaining R0 closeout responsibilities

| Closeout responsibility | Concrete work and output | Exit proof |
|---|---|---|
| Build and CI maintainer — D2/D4 | Run the owned-supervisor scaffold on hosted Linux x64/arm64 with retained inputs; qualify Windows Vulkan ICD/device and both Mac native sessions. Extend managed Wayland software proof to native desktop/hardware input, not another XCB-only surrogate. Record exact Qt/private/qsb/compiler/OS/provider artifacts and redistribution notices. Only the user-selected pinned/patch-applied libdxfrw submodule may build from source in the persistence lanes; other dependencies stay binary-only. | Cold-cache build and forced-API runtime reports on every required architecture; descendant cleanup, explicit Qt runtime/deployment-floor compatibility, graphical sessions and retained artifacts. Compile-only jobs remain build evidence. |
| Renderer maintainer — D2/D3 | Qualify the implemented terminal-widget batch on missing native providers; retain capability preflight, owned fault/recreation and Linux hidden/zero-size/reparent/delayed-delivery cases. Then extend widget recovery, preserving borrowed/owned distinctions. | Complete window/device-loss and failing-shutdown cases without stale output, borrowed-device misuse or loss overclaims. Existing passing cases remain required; Mac widget cases rerun on an unlocked desktop. |
| Painter and test maintainer — D3 | Complete the drawable/consumer register, path/fill/dash/alpha/font decisions, fixture provenance and selection manifest. Resolve bounded algorithms/dependencies and zero-step/fixed-precision hatch reuse before R2. Separate supported baseline behavior from bugs/stubs to fix; do not generalize the spike into a production painter. | Every method/F-group/consumer has an asserted expected result and implementing stage; CPU geometry and image oracles are independent of old GL output. Unsupported scope requires a decision, not omission. |
| Renderer and performance maintainer — D5 | Pin the workload set/reference hardware and run matched legacy/spike measurements where comparable; instrument missing metrics before judging them. Ratify the section 8 budget worksheet and task-based estimates. | Raw repeatable samples, warm/cold distinctions, approved thresholds and explicit unmeasured items; software numbers never satisfy hardware performance gates. |
| Project and release maintainer — D1/D6 | Accept API policy/private-Qt obligations, deployment support, compatibility release/rollback criteria and actual protected statuses. Reconcile source/package workflows and evidence authority. | D1–D6 each have decision, approver, source/provider identity and evidence link; R0 exit reviewed. No unresolved required platform or budget is waved through. |

For each later stage, use the existing stage items as review-sized patches with
the following entry contract. Keep files/targets private unless a real consumer
requires an interface; share one recording path, executor and smoke validator.

| Stage | Entry and first implementation boundary | Required handoff before its exit |
|---|---|---|
| R1 | Provisional implementation authorized on 2026-10-22; accepted R0 remains required for acceptance. Split CPU/factories and host/test/provider options, then add RHI without changing legacy defaults. | Accepted R0 plus fresh CPU-only/RHI-only/dual/legacy builds, effective options, missing-dependency/OFF negatives, real pixels and source/link audit. |
| R2 | R1 build/test targets. First add CPU path/state/command tests, then triangle stroke/fill/curve execution, then cache/retirement metrics; keep each increment renderable. | F1–F4/F7/F8 case IDs execute; buffer/texture/index limits, invalid/nonfinite inputs, cache revisions, chunking without draw-order changes and numeric/raster oracles pass. |
| R3 | R2 command/asset and lifetime contracts. Implement text/image metrics and retained assets before wrapping `LCADViewer`; then migrate model/paper/MDI and bindings/events. | F5/F6/F9/F11/F12, old public call sites and Python-OFF startup pass; `QOpenGLDebugMessage`/GL callback dependencies are isolated to legacy code. No changes to unrelated scripting ownership. |
| R4 | R2 permits offscreen host/harness preparation alongside R3. Text/image CLI acceptance also needs R3.1 assets; real widget smoke needs R3 integration. Replace CLI GL readback and extend the existing runner, not another harness. | F10, real GTest discovery/execution equality, source/widget/CLI separation and negative evidence fixtures, including actual text/image consumers. Core-only spike/primitive output cannot satisfy complete R4 acceptance. |
| R5 | R3/R4 functional suites and provider manifests. Update existing platform/package scripts one artifact at a time; finish trusted hardware and clean installed-runtime qualification. | All section 7 rows bind exact tested package hashes and source/provider identity; architecture, signing, loader/plugin/assets and source aggregate results are distinct and retained. |
| R6 | Accepted R0–R5 and designated release authority. Change only default selection/support documentation and compatibility policy. | Default app/packages pass forced-policy smoke; recorded rollback trigger and qualified legacy opt-in for the designated release. |
| R7 | Compatibility observation criteria met and R6 accepted. Remove legacy sources/factories/dependencies in audited groups. | RHI-only cold builds/install/runtime and full earlier suites pass; no remaining shipped consumer requires removed resources/licenses. |

A batch is ready only when its prerequisite decisions, provider/session access,
inputs/oracles and exact commands are available. It is complete only when code,
negative tests, actual required runs and same-change receipts agree. Proposed
test names, lint, passing aggregate totals and an implementation checklist alone
do not establish either condition.

### R1 Build boundaries and CI foundation

R1.1 implementation is locally verified, owned/self-reviewed by Codex; independent
acceptance remains pending. `lcviewernoqt` owns CPU/Cairo preparation;
`lcviewer_opengl` owns the legacy implementation, factory and GL settings.
Both remain shared and link one-way. UI/CLI/legacy tests link their directly used
GL/GLEW APIs explicitly. Generated legacy paths and kernel metadata now stay in
the build tree. Optional TinySpline examples default OFF; requesting their GL
probe in strict CPU-only mode is an error. No library functionality is removed.

R1.2 now implements the private static RHI target, trusted embedded SPIR-V/MSL
packages and an owned offscreen factory/readback boundary. Its initial CPU frame
contains bounded prepared pixel-coordinate triangles and premultiplied colors;
it does not pretend to implement the 60-method painter before R2. The current
UI/CLI hosts still require legacy; RHI-only library/test builds explicitly disable
both hosts. Local native tests verify opaque and premultiplied-alpha pixels,
resize, owned snapshot lifetime, invalid-input recovery, owner-thread rejection,
wrong-backend rejection and shader resource/stage/variant/reflection negatives.
Allocation/device-fault recovery is not qualified by these tests. R1.4's initial
exact-ID/XML bindings are locally verified below; required hosted source jobs
remain open.
No no-op painter or silent fallback is introduced. Consumers of the legacy
factory/settings must link `lcviewer_opengl`; public signatures stay unchanged,
but the former export location is not a binary ABI guarantee.

Runnable fresh CPU-only configuration from the repository root:

```sh
cmake -S . -B /absolute/new-cpu-build -DCMAKE_BUILD_TYPE=Release \
  -DWITH_LEGACY_OPENGL_RENDERER=OFF -DWITH_QT_UI=OFF \
  -DWITH_LUACMDINTERFACE=OFF -DWITH_PYTHONSCRIPT=OFF \
  -DWITH_PERSISTENCE=OFF -DWITH_UNITTESTS=ON \
  -DCMAKE_DISABLE_FIND_PACKAGE_Qt6=ON -DCMAKE_DISABLE_FIND_PACKAGE_Qt5=ON \
  -DCMAKE_DISABLE_FIND_PACKAGE_OpenGL=ON -DCMAKE_DISABLE_FIND_PACKAGE_GLEW=ON \
  -DCMAKE_DISABLE_FIND_PACKAGE_glew=ON -DCMAKE_DISABLE_FIND_PACKAGE_glfw3=ON
cmake --build /absolute/new-cpu-build --target lcunittest lcluascript --parallel 4
/absolute/new-cpu-build/bin/lcunittest --gtest_brief=1
```

Use a matched installed Lua SDK. Local Mac proof explicitly selects
`LUA_INCLUDE_DIR=/opt/homebrew/opt/lua/include/lua5.5` and
`LUA_LIBRARY=/opt/homebrew/opt/lua/lib/liblua.dylib`; default lookup mixed 5.3
headers with a 5.5 library and failed. No Lua API or host package was changed.
Native Windows/Intel, static linkage, Python-ON, installed/package and graphics
qualification remain open. These CPU checks cannot accept R0, R1 or a platform.

Local proof is retained at
`build/renderer-evidence/r1-cpu-20261022.InP0i8/index.json`: checked source/Mac and
Linux archives bind actual binaries, configure traces, options, discovery/XML and
dependency audits. This is selected local build evidence, not a complete SDK,
source-release archive, external backup or installed package. Frozen plan copies
precede that historical receipt. These CPU results do not establish RHI or CI
acceptance.

Local provisional R1.2 proof is retained separately at
`build/renderer-evidence/r1-rhi-20261022.dIOpwU/index.json`. Selected source/Mac and
Linux archives bind binaries, offline shader packages, compiler/link inputs,
provider versions/hashes, actual loaded Qt libraries, discovery/XML and logs.
This is local selected proof, not a complete SDK, source release, installed package
or external backup. Final plan copies/checks accompany the archives; the proof
does not depend on rewriting old R0 inputs, helpers or receipts.

On Mac ARM64, Qt 6.12.0/Metal observes Apple M2 Max hardware. On Linux ARM64,
private Qt 6.10.2/Vulkan observes Lavapipe software. Each test binary discovers and
successfully executes the same 13 IDs (six GPU and seven CPU tests), with no skips;
the separate existing CPU suites pass 89/89 each. Linux uses the existing owned
Xvfb supervisor and pinned core/synchronization validation: six instances/devices,
zero validation errors, all owned children/files retired, and all 328 pre-existing
defunct display-process identities unchanged. This is offscreen proof, not XCB
presentation or Wayland/widget qualification. Mac creates no windows or desktop
input. The private boundary's standalone header check compiles in C++14 without
Qt include paths on both hosts; CPU library runtime closures remain Qt/GL-free.
Qt's own GUI provider can still discover/link its OpenGL dependencies; no R6
removal or dependency-free Qt package is claimed.

Installed-provider CMake probes pass a positive baseline and five deliberate
failures: missing GuiPrivate, missing ShaderTools, missing private include path,
disabled Metal capability and Qt 6.8.3 `qsb` against Qt 6.12.0. The last two use
local imported-target/feature overrides, not modified SDK files or qualification
of another native provider. Shader tests mutate only trusted embedded assets;
they do not expose an untrusted drawing-file shader loader. Runtime capabilities
and bounded CPU validation precede allocation/submission; the initial private
limits (16,384 per dimension, 16,777,216 pixels, 1,048,576 vertices/indices, finite
coordinates and premultiplied colors) are implementation guards, not accepted D2/D3
budgets. Owned readback storage outlives device teardown; native resource/submit
failure latches the host instead of returning an accepted snapshot. That failure
branch still needs controlled fault-injection proof.

Fresh RHI-only library/test configuration requires a matching Qt
Gui/GuiPrivate/ShaderTools SDK and conditional CMake 3.21 floor. Unlike the strict
CPU-only recipe, do not disable Qt or the selected Qt provider's own dependencies:

```sh
cmake -S . -B /absolute/new-rhi-build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/absolute/Qt-prefix -DWITH_RHI_RENDERER=ON \
  -DWITH_LEGACY_OPENGL_RENDERER=OFF -DWITH_QT_UI=OFF \
  -DWITH_LUACMDINTERFACE=OFF -DWITH_PYTHONSCRIPT=OFF \
  -DWITH_PERSISTENCE=OFF -DWITH_UNITTESTS=ON
cmake --build /absolute/new-rhi-build \
  --target lcrenderertest lcrenderer_headercheck --parallel 4
```

Use the matched Lua SDK described above. Run `lcrenderertest` with explicit
`LC_RENDERER_EXPECT_API=metal` or `vulkan`, and
`LC_RENDERER_EXPECT_DEVICE=hardware` or `software`; there is no permissive `any`
classification or API fallback. Linux runs must use the owned display supervisor,
not an unmanaged `xvfb-run`. The retained one-use verifier demonstrates this local
recipe; R1.4 must integrate it into the one production manifest/runner rather than
turning the one-use verifier into a second qualification framework.

Minimum-Qt native builds/runs for this new target, Windows/Intel/x64 execution,
static/shared consumer packaging, mixed-provider/installed closure, dual-backend
and every provider branch remain unqualified. R1.3 host/provider flag propagation
is now active; R1.4 exact-ID/source-CI integration follows. No R0/R1 or platform acceptance
is advanced by this checkpoint.

R1.3's local flag/library batch now declares Conan backend/host/test/Python options and selects legacy-only
dependencies conditionally. The generated toolchain records its dependency
selection through `LC_CONAN_WITH_*` variables. The root option declarations apply
those defaults for direct toolchain invocations as well as presets and reject any
explicit/cached disagreement before discovery. This avoids resolving one graph
and building another; changing a selection requires matching Conan options and a
fresh build directory. This uses [Conan toolchain variables](https://docs.conan.io/2/reference/tools/cmake/cmaketoolchain.html#variables),
not cache variables available only through presets/helpers. Windows package defaults still use legacy, Python and unit
tests with the CLI/legacy comparison tests OFF; RHI remains OFF. Qt/ShaderTools and
CPython remain external SDKs rather than new Conan source-built dependencies.

The Windows Conan packaging branch no longer silently resets host/test/Cairo/
libopencad choices and is explicitly Windows-only. The legacy target now links
`glm::glm` explicitly instead of obtaining headers incidentally from another
dependency's prefix. Persistence-OFF root/test paths do not discover libdxfrw;
legacy DXF comparison tests reject that choice because they use persistence.

Two demonstrated flag-boundary failures are repaired without adding Qt to CPU
tests. The Python runtime test uses its already-selected CPython's portable
filesystem/process facilities, retaining all four bounded fresh-process probes
and both GTest IDs. Only the DXF round-trip case/helper is conditional on
`LC_USE_PERSISTENCE`; unrelated Lua/Python callback cases remain enabled. Mac
library-only installs no longer run app-runtime relocation for an unbuilt
`librecad`. They retain the external Python SDK dependency; ordinary app installs
keep their existing relocation path. This is library staging, not a closed runtime
package. CPython stdlib SDK configuration files/symlinks can still refer outside
the prefix and need the later package-closure qualification.

Local proof at `build/renderer-evidence/r1-provider-20261022.nav8bf/index.json`
records four isolated library configurations on each native ARM64 host: legacy,
RHI-only, dual and CPU/Python. Initial configurations were fresh; after fixes all
four reconfigure/build/test against one final source cohort. UI/CLI and legacy
comparison tests are OFF in these library configurations; persistence is OFF and
its package discovery is explicitly forbidden. Python is OFF except in the
CPU/Python case. This is deliberately labeled Conan metadata simulation using
installed SDKs, not a real Conan dependency graph, MSVC or default-app build.

Each host passes 89/89 CPU tests per Python-OFF configuration, seven shader/frame
tests per RHI configuration, and all 187/187 CPU/Python tests including the
runtime probes and 26 Python fixtures. Strict CPU/Python configure traces contain
no Qt/GL/GLEW/GLFW/GLM/libdxfrw package discovery. All eight fresh library staging
prefixes select legacy/Python libraries correctly; CPU runtime dependency closures
remain Qt/GL-free. No new GPU or installed-application execution is claimed by
this batch; R1.2's earlier native pixel proof remains separate.

Linux additionally passes 194/194 full CPU/Python/persistence-ON tests, including
the retained DXF round trip. Mac's corresponding current link is rejected because
discovery selected `/usr/local/lib/libdxfrw.a` for x86_64, not the native ARM64
host. Existing experimental ARM64 archives were inspected but not substituted for
a matched/pinned SDK. That cohort did not qualify the Mac persistence tuple; the
later explicit source-submodule decision and its native checks are recorded below.
No external dependency was built or installed in this cohort; vendored
TinySpline remains part of the ordinary project build.

CPU build/recipe regressions pass 33 tests, covering all 256 boolean combinations,
eight Conan/CMake disagreements, cached changes and independent test defaults.
These mock Conan's API. Full scripts pass Linux 443/443 and Mac 433 plus ten
native-Linux skips. Plan/inventory/worktree checks and renderer/plan workflow lint
pass. A broader lint separately flags the unchanged automatic-release action in
`.github/workflows/main.yml:289` as obsolete; no release action/policy was changed.
Actual Conan/prebuilt-cache, MSVC, minimum providers, full application/host flag
builds and installed/package execution remain open. Continue those R1.3 checks
and R1.4's exact-ID/source-job integration without promoting R0/R1 or any platform.

R1.3's local host-build follow-up is verified. The current UI calls persistence directly, so
UI-ON/persistence-OFF now fails before discovery rather than linking an absent or
incidental `persistence` library. It does not force persistence ON or add dummy
file operations. Persistence-free CLI/library builds remain valid. The new option
regression passes. Native Mac CLI-only and Linux full UI/CLI builds each pass
Python OFF/tests ON and Python ON/tests OFF with both render libraries enabled.
Mac persistence stays OFF and its discovery is forbidden; Linux UI uses persistence
ON. Effective cache values and test-target presence match both requests. All four
CLI help paths preserve exit 1 before document/GLFW initialization. Link audits
observe the legacy factory dependency and no RHI factory symbol in each host;
the application has not silently switched backend. Mac's Python-OFF profile passes
89 CPU and seven shader/frame tests. Linux's UI-enabled Python-OFF test binary
passes exactly the same 89 CPU IDs; no widget fixtures are executed.

Selected source/binary/configure/link/test proof is retained at
`build/renderer-evidence/r1-hosts-20261022.r2OyGT/index.json`. The full scripts pass
Linux 444/444 and Mac 434 plus ten native-Linux skips; option contracts pass 34
tests. Source/history checks preserve 1,064 other files and all prior 19 renderer/
333 Python receipts. This closes local compile/link checks, not default-app startup,
new GPU execution, Mac UI/persistence, minimum/other native providers, Conan/MSVC,
installed/package or phase/platform acceptance. No widget, signing, dependency
installation or CI dispatch was performed. R1.4 may proceed provisionally while
those R1.3/R0 gates remain open.

R1.4's private production frame now supports bounded UInt32 indices. The executor
uploads them and issues an indexed draw; completed snapshots publish actual
issued-command counts. Empty zero-size frames release their target and return no
pixels or commands. An immutable per-instance texture policy, bounded by CPU and
queried device capabilities, rejects excessive dimensions before allocation with
the typed `texture-size-limit` diagnostic. These are initial guards, not accepted
hardware limits or fault-recovery proof.

The canonical tests consume every existing typed input for the 3×2 indexed quad,
zero-size `empty` and injected limit 4/requested-width 5 cases. They compare the
complete normalized RGBA output and report actual draw/target/backend/diagnostic
observations. CMake hashes the existing corpus, tracks it as a configure dependency
and embeds its digest; runtime overrides must contain identical bytes. Missing or
changed fixture data fails rather than silently using a different input. The
nonuniform 65×49 triangle remains a separate orientation regression. Additional
checks cover buffer growth/reuse, index validation/recovery, suspend/resume,
clear-only output and policy recovery. The three `RenderFrameTest` IDs now live
in the ordinary CPU suite instead of the Qt-linked test binary; no test is dropped.
Native Mac ARM64 Metal hardware/Qt 6.12.0 and Linux ARM64 software Vulkan/Qt 6.10.2
each discover and pass exactly 22 RHI tests and 92 CPU tests, including the three
frame guards. Fresh strict CPU configurations also pass those same 92 IDs with
Qt/OpenGL/GLEW/GLFW/GLM discovery forbidden and Qt/GL-free runtime closures.
Eight RHI shader/factory/fixture tests additionally pass without creating a native
device. Independent local audits compare all three canonical observations to the
unchanged manifest/corpus, including all 24 pixel bytes and one indexed draw.

Linux's final run uses the existing owned supervisor and forced Lavapipe ICD:
core/synchronization validation observes 14 instances/devices and zero errors,
all owned children/display files retire, and the same 328 pre-existing defunct
display identities remain. Mac runs are offscreen without windows/input. Both
hosts reject missing/foreign corpus overrides before native device creation;
the private standalone header still compiles in C++14 without Qt include paths.
Selected source/binary/shader/provider/link/test/process proof is retained at
`build/renderer-evidence/r1-contract-20261022.4E1dve/index.json`. Final plan/check
files are separate from the frozen archives. These are selected local inputs,
not a complete SDK, source release, installed package or external backup.
R1.4's shared-runner integration is now locally verified. The existing runner's
explicit `--rhi-contracts` mode selects every R1 requirement for the supplied
manifest configuration; unsupported bindings fail instead of being omitted.
It requires all three current RHI cases, executes every discovered test, and
compares exact discovery/XML identities, completion/skip/failure status and typed
fixture observations. Reports include actual device names, forced API/QPA/native
architecture, revision/dirty flag, effective options, 15 selected RHI/frame source
hashes, four declared private Qt headers, the shader baker/offline packages and
actual loaded Qt image hashes. This is selected build/provider provenance, not
complete dependency closure. Its distinct `r1-rhi-contract-runner` report always
keeps `phase_or_platform_accepted` false.

Native Mac ARM64 Metal/Cocoa and Linux ARM64 software Vulkan/XCB/private headless
Wayland each pass all three required cases and exactly 22 discovered/executed
tests. Linux core/synchronization validation observes 14 instances/devices and
zero errors per QPA; publication follows owned cleanup and a fresh input/artifact
recheck. All 328 old defunct display identities remain unchanged. The existing
CPU suites still pass 92/92 per host. Twenty-nine CPU fixtures cover missing IDs,
skips, altered observations, stale source/header/provider/shader data, poisoned
test filters, timeout/exit failures and tampering before publication; CI executes
that module. Full scripts pass Linux 473/473 and Mac 463 plus ten native-Linux
skips. A real non-RHI binary is rejected for its incompatible discovery prelude;
that is separate from synthetic missing-required-ID proof. A denied Mac native
architecture query also fails closed; neither rejection is a rendered pass.

Selected proof and final plan/check files are retained at
`build/renderer-evidence/r1-runner-20261022.AjPN3c/index.json`, not a complete SDK,
source release, package or external backup. Older R0 helper-bound receipts remain
historical after this runner change. At that checkpoint native source jobs and
CPU/build-case bindings were open; the next local checkpoint below implements the
CPU bindings. Do not promote R0/R1, minimum providers, other
architectures, hardware Linux, widgets/app startup or installed-package gates.

R1.4's CPU/build bindings are now locally implemented in the same runner's
explicit `--build-contracts` mode. It selects every CPU-source R1 requirement;
unsupported bindings, foreign/native-architecture tuples or graphics arguments
fail before a build starts. A separate Qt-free `lcbuildcontracttest` preserves
the three existing `RenderBuildContractTest.CpuBuildOptions*` IDs. Python prepares
fresh ordinary/empty/limit CMake trees; native GTests consume the unchanged
canonical corpus and inspect actual guard-boundary values, expanded configure
traces, client-owned CMake File API target graphs and the rejected host diagnostic.
No XML or native observations are synthesized by the controller. The host guard
now emits `[no-renderer-for-host]` without changing its rejection condition.

The ordinary case requires zero Qt/legacy discovery and the real CPU executable;
the empty case requires no GPU or unit-test targets with tests disabled. The
limit case requires an actual failed host configuration and zero accepted File
API outputs. Fixture-owned flags cannot be supplied as SDK overrides. Each
test records its compiled five-file source manifest, corpus hash and native/build
architecture. Four controlled native rejections alter only owned trace, graph,
diagnostic or configured-corpus artifacts, then restore the original bytes.
Expected-failure XML must contain the exact failed native ID and assertion;
GoogleTest CDATA is accepted while DTD/entities, skips and unrelated errors fail.
These four probes do not inflate the three required manifest IDs.

Native ARM64 Mac and Linux each pass all three cases and four rejections under
both JSON and human expanded traces. Mac additionally exercises real Ninja
multi-config Release artifact discovery. The human adapter preserves the CMake
3.15 source floor, but these runs use current CMake, not a minimum-version binary.
The GTest link adapter supports both pre-3.20 and current imported-target names;
an actual CMake fixture covers both protocols. The new target has its own main,
target-local C++17 and no Qt/GL runtime closure; the CPU library/header floor
remains C++14. See [CMake 3.15's GTest interface](https://cmake.org/cmake/help/v3.15/module/FindGTest.html).

CPU command ownership reuses Linux's fresh CLI subreaper/pidfd supervisor without
claiming display cleanup. Mac pins the unreaped root while retiring its private
group; escaped sessions are not qualified by that mechanism. Windows creates a
suspended root, assigns a kill-on-close Job, then resumes it; assignment failure
never resumes an unowned child. Native success, expected exit 1 and timeout
probes pass on Mac/Linux, including Linux's escaped descendant. Windows Job
ordering/failure/timeout checks are mocks, not native Windows proof. A helper
exception retains an explicitly incomplete cleanup receipt, never a success.

The 18 new CPU parsing/ownership/compatibility fixtures are included in the
renderer contract CI job; that job is still not a native R1 source-build job.
Full scripts pass Linux 491/491 and Mac 481 plus ten native-Linux skips.
Final-source Metal and software Vulkan/XCB/private headless Wayland each pass
22 exact discovered/executed RHI IDs; both existing CPU suites pass 92/92.
Vulkan core/sync validation observes 14 instances/devices and zero errors per
QPA, with complete owned cleanup and all 328 old defunct identities preserved.
Selected source, binary, trace, graph, XML, rejection and process/link proof is
retained at `build/renderer-evidence/r1-buildcases-20261022.MXOvix/index.json`.
It is not a complete SDK, source release, installed package or external backup.
Older helper/source-bound packets remain historical; no old hash is rewritten.
At that checkpoint native source CI was not yet wired; the initial GUI-less
source jobs below now exist. Actual minimum CMake, other native architectures,
real Conan/MSVC, full provider/runtime closure, painter/consumer contracts,
widgets/startup/packages, D1–D6/release and R0/R1 acceptance remain open.

R1.4's initial source jobs are now wired into `renderer.yml` through the local
`renderer-source.yml` reusable workflow. They retain the same caller commit and
read-only permissions; every remote action remains SHA-pinned. The five declared
native tuples are Linux x64/ARM64, Windows x64 and Mac ARM64/Intel. Each runs the
existing CPU build cases, then builds a fresh `rhi-offscreen` source tree through
the same runner's `--rhi-contracts --build-source` mode. This builds the real
RHI/CPU executables and standalone C++14 header target, executes every discovered
CPU ID with exact XML reconciliation, and then applies the unchanged manifest
RHI bindings. A Linux display worker receives the built executable, not a second
recursive build. Foreign, prebuilt/reused-tree and unsupported host/dual/scripted
requests fail instead of silently changing their profile.

Linux runs both XCB and private headless Wayland under the owned supervisor with
forced software ICD/core-sync validation; Windows requests only Vulkan under the
pinned software provider; Mac requests only Metal. Selected graphs, source/tool
identities, caches, logs, XML and process receipts upload even on failure, including
the hidden File API reply directory. SDKs and whole dependency caches are not
uploaded. The initial matrix aggregate fails on a failed/missing/skipped source
job; it is not the final `Renderer Required` report/option/package aggregate.

CI installs distribution binaries, exact restored Mac Qt archives or the declared
Windows Qt SDK. Windows core graphs use [Conan's binary-only policy](https://docs.conan.io/2/reference/commands/install.html)
with fresh per-case toolchains and `--build=never`; ordinary/empty graphs retain
their matching test switches. Mac runtime dependencies are installed in dependency
order with explicit `--force-bottle --ignore-dependencies`, avoiding recursive
source fallback. Package inventories and provider pins are retained, but immutable
full dependency closure/cold restoration and native Windows binaries remain
qualification gates. No project-library installation or CI dispatch was
performed in the local verification below.
The Windows Vulkan-Headers pin is `v1.4.341`'s peeled commit
`b5c8f996196ba4aa6d8f97e52b5d3b6e70f7e4e2`, not its annotated tag object
`0d3f509e57041fbd073a1ee84cd9ccd36d148446`. The retained upstream ref lookup
distinguishes those identities; it is not Windows compile/runtime proof.

The Windows binary-route preflight now checks actual Conan 2.33 graph JSON,
not merely a successful CLI exit. A real metadata query of the original recipe
reported GTest `Missing` while returning zero and `graph.error: null`. The pinned
test dependency is now [GoogleTest 1.18.0](https://github.com/google/googletest/releases/tag/v1.18.0),
with its required GMock/main components. The R1 MSVC194/x64/Release/C++17 static
core graphs resolve as follows; `Download` means available metadata, not an
installed or executed binary:

| Case | Tests / RHI | Original GTest 1.14 | Pinned GTest 1.18 |
|---|---|---|---|
| ordinary | ON / OFF | Missing | Download; seven host binaries available |
| empty | OFF / OFF | Not required | Not required; six host binaries available |
| rhi | ON / ON | Missing | Download; seven host binaries available |

[`validate_renderer_conan_graph.py`](scripts/validate_renderer_conan_graph.py)
requires the exact consumer profile/options/direct requirements, available static
host package IDs/revisions, required GMock/main targets and matching generated
recipe locks. Missing/build-required/invalid/editable packages, unexpected
graphics dependencies, wrong edges/profiles or lock drift fail. Only unused build
tools may have `Skip` status. The source job validates before installation, uses
that exact lock with `--build=never`, then requires the installed graph's normalized
binary identities to match the preflight route. Cache/download state transitions
do not change those identities. Actual binary installation and CMakeDeps/native
MSVC execution are still unqualified, not inferred from this guard's metadata.

The existing Windows packaging command now sets
`gtest/*:compiler.cppstd=17` specifically for the test dependency. A separate real
metadata graph preserves the consumer's C++14 setting and resolves GTest as
C++17; application/library CMake standards and package defaults are unchanged.
Its existing libdxfrw/source-build policy is not expanded or silently replaced
by this renderer change. The original recipe is retained byte-for-byte and its
missing binary is reproducible under the same R1 profile.

Local inspection used a disposable Conan 2.33 CLI under tmp. Per
[Conan graph info's contract](https://docs.conan.io/2/reference/commands/graph/info.html),
only recipe/binary metadata was resolved: all inspected project package folders
remain absent, with no project binary retrieval, dependency build or native
Windows execution. Thirteen new graph fixtures and two recipe/workflow tests
cover the failure and lock/identity ordering. Full scripts pass Linux 516/516 and
Mac 506 plus ten native-Linux skips. Selected metadata/source/tool manifest proof
and final checks are retained at
`build/renderer-evidence/r1-conan-20261022.eDPvCd/index.json`. Earlier CI-source
snapshots remain historical; no new C++/GPU execution is claimed by this batch.
Next cold-install the locked route on native Windows and qualify generated
toolchains, compilation, required IDs and runtime/CRT closure. Full option/host/
provider/package, minimum/missing native and D1–D6/R0/R1/release gates stay open;
legacy remains the default.

The source runner's optional `--library-matrix` now prepares CPU-only, legacy-only
and dual Python-OFF library builds, plus all four renderer modes with Python ON,
before its existing Python-OFF RHI-only build. Each library profile keeps UI,
CLI, persistence and graphical test targets OFF; these are not the canonical
`dual`/`scripted` host configurations or Python-ON GPU acceptance. Python-OFF
modes must execute identical complete CPU IDs. Every Python-ON mode must retain
all those IDs, add a nonempty identical scripting suite and execute both existing
`PythonRuntime` IDs, including four fresh-process configuration probes. Missing
test sources/targets, skipped tests and provider drift fail. Guard-boundary
preflight values and cache values must agree, including Cairo and persistence;
owned Release File API graphs prove target membership and the real legacy
factory's source ownership. Expanded configure traces reject unexpected Qt/GL
discovery in CPU profiles and GLFW/libdxfrw discovery in every library profile.
SDK overrides cannot change fixture-owned flags or select a partial matrix.
Python-OFF library builds forbid Python package discovery and runtime inventories.
Python ON requires all eight existing scripting test sources and the real binding
target. Retained inventory, interpreter/embed-library/header hashes and ABI fields
identify the selected CPython SDK; an explicit `Python_EXECUTABLE` request cannot
silently select another interpreter. Linux job definitions use `/usr/bin/python3`
with the distribution's binary `python3-dev` package. The
[upstream 3.12.10 release manifest](https://raw.githubusercontent.com/actions/python-versions/main/versions-manifest.json)
has no Ubuntu 26.04 artifact; Mac/Windows retain their pinned 3.12.10 setup action.
Custom interpreter shells are selected at job-default scope, where the
[matrix context is supported](https://docs.github.com/en/actions/reference/workflows-and-actions/contexts#context-availability).
Every job passes its actual interpreter as `Python_EXECUTABLE`; local runs use
their declared installed SDKs. The aggregate source-job timeout is 60 minutes
for the additional fresh builds; individual 300-second build limits and contract
deadlines are unchanged. No interpreter installation or Python API/runtime change
was performed locally. Native cold SDK setup remains required.

The five native source-job definitions run this matrix once per architecture,
not again for Linux Wayland. Linux uses distribution GLEW/GLM/FreeType binaries;
Mac adds those bottles to the existing binary-only restore. Windows uses separate
CPU, legacy, dual and four Python-ON Conan toolchains. Real MSVC194/x64 metadata
validates seven/six/seven/fourteen/fourteen available host binaries for ordinary,
empty, RHI, legacy and dual; CPU/RHI Python-ON routes each have seven and
legacy/dual Python-ON routes each have fourteen. Identical unused skipped-tool nodes may repeat
across Conan dependency paths; conflicting skipped metadata and all duplicate
host recipes still fail. System OpenGL/GLU are allowed only in legacy/dual routes.
Generated locks and post-install identity checks remain required; no Windows
project packages were retrieved, installed or compiled during local inspection.

The provisional `--production-builds --platform <native tuple>` runner now
compiles all eight GUI-less renderer/Python combinations with tests OFF. It
rejects GTest discovery and project test targets, verifies actual nonempty owned
library outputs and selected CPython SDK identity, and freezes source/helper and
artifact hashes across the matrix. UI, CLI, persistence and Cairo remain OFF;
these compile-only profiles do not replace the canonical host or runtime matrix.
TinySpline's existing internal targets are not project test qualification.

Mac/Linux ARM64 each pass eight fresh tests-OFF modes and verify 44 library
targets. CPU and binding links remain Qt/GL-free, and the real legacy factory is
owned only by the legacy library. Mac additionally passes all eight fresh
tests-ON modes: unchanged 92 CPU IDs per Python-OFF mode, 190 per Python-ON mode,
40 C++14 header consumers and 22 offscreen Metal IDs. Its CPython fingerprints
match across tests ON/OFF; Linux's four Python-ON compile-only profiles agree
on their provider. CPU build contracts pass three IDs/four controlled rejection
probes per host. Full script suites pass Linux 547/547 and Mac 537 plus ten
native-Linux skips; the 55 focused source/Conan checks pass. Selected proof,
including both verified archives and failed runs, is indexed at
`build/renderer-evidence/r1-production-20261022.rWfCAL/index.json`.

CI now defines ten source jobs: five native architectures crossed with tests
ON/OFF, each retaining compiled libraries, owned graphs, hashes and logs. OFF
jobs omit GTest dependencies. Eight Windows tests-OFF metadata routes validate
six core/RHI or thirteen legacy/dual available locked binaries, without local
project binary retrieval/install/build. Native cold toolchain/MSVC and hosted
execution remain unqualified; the 300-second profile and 60-minute job limits
are unchanged. Missing providers cannot trigger a dependency source build.

The initial parallel Linux tests-ON/OFF builds exceeded 300 seconds. OFF cleanup
reaped every child; ON cleanup failed before sending any signal and left one
recorded `cmake` zombie owned by PID 1. A synthetic clock-jump fixture reproduced
the deadline-before-signal ordering defect without launching processes. These
failures motivated the corrected boundary and full reruns below. Preserve them
and all 328 old display identities; do not restart the shared container or
normalize records. Compilation is not runtime or
source-CI acceptance; canonical host/provider/persistence/Cairo, minimum/missing
native, R0/R1, D1–D6 and release gates remain open, with legacy still the default.

The cleanup follow-up now gives TERM and KILL separate deadlines measured from
their initial signal issuance. A delay before the first signal or between phases
cannot consume the next phase's grace; newly adopted children share its deadline
and never restart it. The configured grace values and command deadlines are
unchanged. Recorded wall time includes scheduling delays and is not a promise of
an absolute five-second teardown under preemption. Failed/timed-out launchers
remain failures even when retirement completes. Existing immediate-child,
start-identity/pidfd and WNOWAIT-before-reap guards remain mandatory; Python's
[wait status semantics](https://docs.python.org/3.14/library/os.html#os.waitid)
and [pidfd signal API](https://docs.python.org/3.14/library/signal.html#signal.pidfd_send_signal)
define those operations. Three clock/ownership fixtures and native Linux
clock-jump/tree-reaping coverage pass. Standalone native probes inject a labeled
seven-second clock offset before/between signals, reap both owned processes per
case and still reject the original timeout. Earlier failed/helper-bound archives
remain unchanged historical proof; exhausted kill grace remains an explicit
failure, not an accepted result.

Mac source CI keeps its pinned Python 3.12 controller. That interpreter lacks
`os.waitid` on Darwin; the installed 3.12.15 SDK reproduces the missing API.
A narrow LP64 Darwin libc adapter now observes WEXITED/WNOHANG/WNOWAIT without
reaping, using the Python API when available. It rejects ABI mismatch, foreign
child identity and syscall errors; final `wait()` still owns the original exit
status. Installed SDK headers validate the 104-byte/8-byte-aligned siginfo layout,
field offsets, argument widths and flags for ARM64 and x86_64 compilation only.
Both Python 3.15 and installed 3.12 execute repeated native observations followed
by the original exit status. Three portable adapter tests and the existing
ownership fixture cover the boundary. Exact CI 3.12.10/cold-host and native Intel
proof remain open; no interpreter pin, runtime implementation or default changes.

The October 23 closeout passes eight fresh tests-ON source modes on each ARM64
host, using installed CPython 3.12.15 on Mac and 3.14.4 on Linux. Each Python-OFF
mode retains all 92 CPU IDs; each Python-ON mode executes the same 190 IDs,
including 98 scripting/runtime IDs, without skips. Each host compiles 40
independent C++14 header consumers. Offscreen Metal and software Vulkan on XCB
and private headless Wayland each pass 22 RHI IDs. Both Linux QPAs report 14
instances/devices, zero core/synchronization validation errors and complete owned
cleanup. CPU build contracts pass three IDs/four controlled rejections per host;
Mac Python 3.12 command success, rejection and timeout probes preserve status.
Full scripts pass Linux 554/554 and Mac 543 plus eleven native-Linux skips under
both installed 3.12 and 3.15. Proof/final checks are indexed at
`build/renderer-evidence/r1-cleanup-20261022.6a9toC/index.json`; the directory names
the batch's October 22 start, not its closeout date. All previous source and
receipt rows remain preserved. R1 host/provider/persistence/Cairo, minimum/missing
native, full painter/consumer, R0/R1, D1–D6 and release gates remain open.

R1.3 now compiles UI sources and Qt-generated code in one private PIC object
target shared by `librecad` and the optional `lcui` test library. Dependency usage
requirements and Python module-generation ordering belong to that compiling
target; final consumers keep their original link dependencies, exports, names,
icon and install rules without redundant autogen targets. Four small CMake
fixtures exercise tests/Python ON and OFF, actual shared-object consumption,
C++17 and compile definitions, generation ordering and executable linkage.

Two fresh Linux ARM64 host profiles pass with UI/CLI and persistence/Cairo.
Legacy/Python-OFF/tests-ON compiles 71 UI units once for both consumers and passes
all 92 CPU, one Cairo and three DXF IDs. Dual/Python-ON/tests-OFF compiles 75 UI
units and the private RHI library with GTest discovery forbidden. Both hosts
retain the legacy factory dependency and no RHI factory symbol. The File API
keeps CPU libraries at C++14 and the UI at C++17. Full scripts pass Linux 555/555
and Mac Python 3.12 with 544 passed plus eleven native-Linux skips; focused build
and plan-checker suites pass 36 and 42 respectively.

Selected source/assets, objects/binaries, provider hashes, graphs, XML, raw logs
and final checks are indexed at
`build/renderer-evidence/r1-uiobjects-20261023.NiZ6oP/index.json`.
The initial empty-autogen inspection was interrupted and reaped; a six-job
Python build lost a compiler process and reaped all children. Its successful
fresh three-job retry keeps the same 300-second command and 1,200-second profile
limits. The wrong-working-directory script failure and corrected root-directory
pass remain separate. No failures were normalized or dependencies installed.
Other host/Python/provider combinations, host source-CI integration, native
Windows/Mac UI, minimum toolchains, widgets, GPU/runtime/package, independent
review and R0/R1/D1–D6/release acceptance stay open; legacy remains default.

The shared runner now has an explicit `--host-builds --host-profile` path for
legacy/dual, Python OFF/ON and `--host-tests ON|OFF`. `--host-kind both|ui|cli`
selects real hosts explicitly; the default remains both with persistence ON.
Only an explicit CLI-only profile may request `--host-no-persistence`; a missing
SDK never changes the requested host or feature. `--host-cairo` additionally
requires Cairo. The existing graph/artifact readers
verify effective options, test-source membership, owned UI objects, C++14/17
boundaries, Python generation and legacy host dependencies. Recorded source,
UI/resource/Python-module bytes, selected SDK files and compiled outputs must
remain unchanged. Every command keeps its maximum 300-second cap and each fresh
host profile has a 1,200-second total budget. Test-enabled profiles compile and
discover the enabled suites and require matching text/JSON list-only metadata.
Every discovered ID is bound to a fingerprinted source/header origin; all
non-widget IDs execute, while UI-source IDs remain unexecuted. This includes
compiled conditional/typed/parameterized cases, not source-text inference.
Missing origins/metadata, mismatched lists, unowned sources and skipped/failed
selected IDs fail. A GoogleTest provider without list-only source metadata is
not qualified by this runner. UI-only profiles forbid CLI CURL/GLFW discovery;
legacy CLI-only profiles forbid Qt, and persistence OFF forbids libdxfrw.
No widget or graphics runtime acceptance follows from this path.

Eight source/host regressions exercise all eight renderer/Python/test modes,
missing/duplicate/foreign objects, compiler requirements, source/SDK drift,
header-defined registrations and explicit CLI scope. The focused source suite
passes 46 fixtures. Three fresh Linux host profiles pass: dual/Python-ON/tests-ON
executes 198 non-widget IDs and compiles/discovers 97 unexecuted widget IDs;
legacy/Python-OFF/tests-OFF sets Cairo OFF and forbids Python/GTest discovery; and
legacy/Python-ON/tests-OFF builds with Cairo and GTest discovery forbidden.
Review corrected the audit's expectation for the Python copy utility, which
also exists in GUI-less profiles. Seven retained native library graphs per ARM64
host pass the corrected reader as metadata compatibility, not new C++ execution.
The earlier two host runs preserve their execution-helper identities; current
source/SDK/graph/object/artifact/XML revalidation passes unchanged, and the third
fresh run uses the final helper. The only helper change between them corrects
the utility predicate outside host mode; it is equivalent when `hosts=True`.
Full scripts pass Linux 563/563 and Mac 552 plus eleven native-Linux skips under
installed Python 3.12/3.15. The 42 plan-checker regressions also pass. Source,
selected SDK files, compiled outputs, process records, metadata-only rejected
Conan route and final validation are retained at
`build/renderer-evidence/r1-hostrunner-20261023.4ZxY5w/index.json`.
This does not close the remaining host/Python/provider, cold CI, minimum/missing
native, widget/GPU/package, independent review or R0/R1/D1–D6/release gates.

Cold host CI cannot yet use a qualified libdxfrw binary route. The installed Linux
SDK has no `dpkg` owner, cached Ubuntu indexes contain no libdxfrw development
package, and [upstream currently publishes no releases](https://github.com/LibreCAD/libdxfrw/releases).
The current project/CodeQL/Mac dependency steps still build libdxfrw from source;
do not copy that fallback into the binary-only renderer lanes. Obtain an approved
prebuilt SDK with platform/source/hash/license identity, or separately authorize
a dependency-binary producer, before enabling cold UI/persistence host jobs. That provider
choice has been requested; existing source jobs and all platform gates remain
unchanged. An available installed SDK is local build proof, not a cold-CI route.

[ConanCenter lists stock libdxfrw binaries](https://conan.io/center/recipes/libdxfrw),
but its inspected recipes use the `1.0.1`/`LC2.2.0` archives, not the required
`f7debf4c5f30734a70c5e828cf05247aabc2d04b` plus application-data patch. A metadata-only
query for Linux ARM64/GCC 15/shared `libdxfrw/1.0.1` resolves recipe
`0e76995cc5eb320b6d3938aaff0e191f` but reports package
`44db788ae3357edadc4486a1a0a0606d52c5f5e5` as `Missing`, despite exit zero and a
null graph error. That query retrieved no dependency binaries and installed or built none. This is
a rejected candidate route, not evidence that no registry publishes any binary;
the required patched-provider and missing-native gates remain open.

Independent host-source verification is now implementing the remaining R1.3
switch combinations. The object audit explicitly checks Python generation
dependencies and nonempty CPU C++14 groups. Three additional regressions cover
32 host/profile/test/persistence protocol combinations, compiled-origin selection
and malformed/missing origin data; the focused suite passes 49 fixtures.
A native list-only probe partitions all 295 existing registrations into 198
non-widget and 97 widget IDs without executing them. Fresh native Mac ARM64
CLI-only/Python-ON/persistence-OFF and Linux ARM64 UI-only/dual/Python-ON/Cairo
builds pass: Mac executes 190 non-widget IDs with no Qt/DXF discovery; Linux
executes 198 and discovers 97 unexecuted widget IDs, with CLI CURL/GLFW discovery
forbidden. Native Mac tests-OFF and the exact Linux CLI CI command also pass;
both ARM64 hosts execute the same 190 CLI IDs. Link/symbol audits find no Qt/DXF
dependency in the CLI project binaries and retain the legacy factory in each
enabled application. Fifteen independent C++14 header objects are verified across
the three tests-ON profiles. The temporary retention script initially resolved
their relative paths against the wrong directory; that copy failure and partial
packets remain preserved, with a corrected new packet byte-verified. These
explicit OFF-path checks do not qualify Mac UI, a missing binary provider,
default-app/graphics startup or any R0/R1/platform/release gate.

Source CI now defines a separate explicit CLI scope: four native Linux/Mac
architectures crossed with tests ON/OFF, using `legacy-python`, UI OFF and
persistence OFF. Linux installs binary CURL/GLFW/core development packages;
Mac uses the existing forced-bottle closure plus GLFW and the selected Python
SDK. Neither CLI scope restores Qt or builds libdxfrw. The ten existing library
definitions retain their original provider/runtime requirements and 60-minute
job cap; the eight CLI definitions keep the runner's 1,200-second profile cap.
All eighteen matrix definitions feed the existing required aggregate, with
scope-specific artifact names and compiled executables retained on failure too.
Windows CLI is explicitly excluded pending its binary/toolchain route; this is
not a substitute for the blocked UI/persistence jobs. Actual shared-step and
dependency-array regressions pass; no hosted execution is claimed. The full
source suite passes 51 fixtures; full scripts pass Linux 568/568 and Mac 557 plus
eleven native-Linux skips under installed Python 3.12/3.15. Plan-checker tests pass
42. Final native/protocol/source/SDK/origin/XML/link/process and CI evidence is
indexed at `build/renderer-evidence/r1-hostkinds-20261023.olASyW/index.json`.
Prior source/receipt history is unchanged; all phase/platform/release gates stay
open and legacy remains the default.

The subsequent provider decision selects a source submodule for libdxfrw only.
`third_party/libdxfrw` is pinned to the existing required
`f7debf4c5f30734a70c5e828cf05247aabc2d04b`; this does not substitute unpatched
upstream master or relax other providers' binary-only policy. The isolated
`scripts/build_libdxfrw.py` builder checks gitlink/HEAD/cleanliness and required
patch revision, copies recorded files into a fresh owned tree, applies the
application-data patch there and builds a PIC static SDK with docs/tools OFF.
It retains source/patch/helper/SDK/header/config/license identities and bounded
supervised configure/build/install commands, leaving the submodule unmodified.
Source CI adds eight explicit persistence-enabled CLI definitions, for 26 total;
the other eight CLI lanes keep persistence OFF and ten library lanes are unchanged.
All CLI lanes now install and relocate using the shared runner's explicit
`--host-install`. CMake installs `luacmdinterface` and gives both Unix hosts
relative project-library search paths. The check builds install prerequisites
(including TinySpline's otherwise unused C archive), verifies the install
manifest/target destinations before and after the move and observes every
project-library load from the moved prefix with ambient loader overrides removed.
This uses the selected system SDKs and only executes CLI help: it does not qualify
standalone packaging, Python initialization, rendering or UI startup. Initial
terminal attempts exposed the missing archive and are retained separately;
current native/provider and script validation is recorded in the newest receipt.
Hosted/Windows CLI/Mac UI/provider/OS/package and R0/R1/D1–D6 acceptance stays open;
legacy is still the default and R2 has not started.

In the prior Python-matrix cohort, Mac/Linux ARM64 each passed eight fresh library builds. All four Python-OFF modes
execute the unchanged 92 CPU IDs; each Python-ON mode executes the same 190 IDs,
retaining those 92 and adding 98 scripting/runtime IDs without skips. The four
provider fingerprints agree per host: CPython 3.14.8 on Mac and 3.14.4 on Linux.
Poisoned-environment startup and all four fresh-process probes pass. Link/symbol
checks keep CPU and binding libraries/executables Qt/GL-free and locate the single
OpenGL factory in each legacy library. Fresh pinned Qt 6.8.3/Metal and Qt
6.10.2/Vulkan XCB/private headless Wayland each pass the exact 22 RHI IDs. Native
CPU build contracts pass three IDs/four controlled rejection probes per host.
Linux core/sync validation records 14 instances/devices, zero errors per QPA and
complete owned cleanup, preserving all 328 historical defunct display identities.
Five additional protocol/workflow tests bring the focused suite to 40; full scripts
pass Linux 532/532 and Mac 522 plus ten native-Linux skips. Selected source,
binaries, graph/trace, XML, SDK, process and link proof is retained at
`build/renderer-evidence/r1-pythonmatrix-20261022.Zvx8et/index.json`.
These are GUI-less library/CPU and Python-OFF offscreen RHI results, not canonical
UI/CLI/scripted/dual GPU, widget, default-app, cold-provider, installed/package or
native Windows/Intel/x64 qualification. Persistence/Cairo/test-OFF combinations,
minimum toolchains, full provider/painter/consumer, R0/R1 and D1–D6/release gates
remain open. Earlier source/helper/CI snapshots, including the Python-OFF matrix
proof, remain historical and unchanged.

R1's public CPU-header boundary now has a test-only object target. It compiles
`lcpainter.h`, `createpainter.h`, `renderframe.h`, `documentcanvas.h` and
`lcdrawoptions.h` independently, each in exact C++14 mode with extensions OFF.
Each generated consumer rejects Qt, OpenGL/GLES, GLM, GLFW, Vulkan, Cairo and
Python/pybind11 SDK macros; `GL_VERSION` covers normal Apple/GLEW headers that
do not define `GL_VERSION_1_0`. MSVC uses its documented
[`_MSVC_LANG` standard value](https://learn.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=msvc-170),
not the legacy default value of `__cplusplus`. This does not imply native MSVC
qualification. The target exists only with unit tests and is never installed or
linked into the application. No production standard/default is raised.

The shared source runner verifies the real Release object target and language
groups, byte-identical configured consumers, source/header hashes and five
nonempty owned object files. Missing, foreign, duplicate/aliased, C++17 or edited
consumers fail. Template/public-header inputs are frozen across the entire source
run, not only within one profile. The existing source CI builds this target in
all eight library/Python modes and retains generated sources and objects even
on failure. These checks establish the named public-header boundary, not every
consumer, the full painter or an installed package.

Fresh Mac/Linux ARM64 matrices each compile all 40 independent consumers across
eight modes, retaining all 92 CPU IDs per Python-OFF mode and 190 per Python-ON
mode. Each native compiler passes the syntax-only positive and rejects 13
controlled cases: ten SDK macro definitions, forced C++17, and actual OpenGL and
CPython header inclusion. Sources and compiled objects remain unchanged by these
syntax-only probes. The first in-flight builds rejected the edited guard inputs;
only the subsequent fresh, stable-input cohort is retained as passing evidence.
Metal and Vulkan/XCB/private headless Wayland each pass the exact 22 RHI IDs;
CPU build contracts pass three IDs/four controlled rejections per host. Linux
per-QPA validation records 14 instances/devices, zero errors and complete owned
cleanup, preserving all 328 historical display identities. Five new parser/input
tests bring the focused suite to 45; full scripts pass Linux 537/537 and Mac 527
plus ten native-Linux skips. Source, object, SDK, diagnostic, process and runtime
proof is indexed at `build/renderer-evidence/r1-cpuheaders-20261022.hboLtz/index.json`.
This is local ARM64 header/build/offscreen proof, not minimum-toolchain, native
Windows/Intel/x64, canonical host, full painter or package qualification. Existing
R0/R1, D1–D6/release gates and legacy defaults remain unchanged.

Native Mac ARM64 now builds and executes production RHI against pinned Qt 6.8.3,
not only the current Homebrew Qt. The first link exposed Qt 6.8/6.9's obsolete AGL
consumer item on the current Apple SDK. R0 and experimental RHI-only Metal builds
now share the existing selected-SDK link-stub check and narrow consumer adapter;
a runtime framework directory is not sufficient evidence of SDK availability.
Legacy/default host links and every Qt binary remain unchanged. The RHI selected
source manifest now includes that adapter (16 files). Both version-range/link
guards and absent/present SDK-stub fixtures pass. This verifies the local minimum
Qt RHI route, not an older OS deployment floor, native Intel or hosted Metal.

Fresh final-source Mac Qt 6.8.3/Metal and Linux Qt 6.10.2/Vulkan XCB/Wayland builds
each execute all 92 CPU and 22 RHI IDs. Current-helper CPU build contracts also
pass three native IDs plus four controlled corruption rejections per host. Linux
core/sync validation observes 14 instances/devices and zero errors per QPA;
owned cleanup preserves all 328 old defunct display identities. Ten new CPU
source/workflow/SDK protocol fixtures cover the actual shared workflow command
construction for all five tuples, but are not native results for the missing
hosts. Full scripts pass Linux 501/501 and Mac 491 plus ten native-Linux skips.
Selected local proof and final checks are retained at
`build/renderer-evidence/r1-sourceci-20261022.IiKIKW/index.json`. Earlier
source/helper-bound packets remain historical. Full option/host/scripting and
provider/OS/runtime/package matrices, hosted/missing native execution, actual
minimum CMake, real Conan/MSVC, painter/consumer contracts, D1–D6/release and
R0/R1 acceptance remain open; legacy is still the default.

After the matching RHI build above, the local Mac ARM64 command is:

```sh
python3 -B scripts/run_renderer_checks.py --rhi-contracts \
  --app /absolute/rhi-build/bin/lcrenderertest --build-dir /absolute/rhi-build \
  --provider-prefix /absolute/Qt-prefix \
  --configuration gui-less-macos/MAC-ARM64/rhi-offscreen/offscreen/cocoa/hardware \
  --backend metal --device hardware --mode offscreen --expected-qpa cocoa \
  --report-dir /absolute/new-evidence-parent --timeout 60
```

Linux ARM64 uses the corresponding `gui-less-linux/LINUX-ARM64/rhi-offscreen/offscreen/xcb/software`
configuration, Vulkan/software/XCB arguments and the existing `--xvfb-run` owned
supervisor. Supply both `--vulkan-validation-manifest`/`--vulkan-validation-library`
and the exactly forced `--icd-manifest`; set SDK library/plugin paths explicitly.
For Wayland replace XCB in the tuple/QPA and pass the qualified private `--weston`
and `--weston-prefix` with `--weston-backend headless`; retain `--xvfb-run` for the
outer owned process/display supervisor even with headless Weston. This is offscreen execution,
not widget presentation or input proof. Never substitute an unmanaged display.

The CPU build variants above now observe real configure traces, targets and
rejected host options, not only compiled constants. A local reproduction is:

```sh
python3 -B scripts/run_renderer_checks.py --build-contracts \
  --configuration cpu-source/MAC-ARM64/cpu/cpu/none/none \
  --report-dir /absolute/new-cpu-evidence-parent --timeout 180 \
  --cmake-option LUA_INCLUDE_DIR=/absolute/matched-lua/include \
  --cmake-option LUA_LIBRARY=/absolute/matched-lua/lib/liblua.dylib
```

Use `cpu-source/LINUX-ARM64/cpu/cpu/none/none` on the native Linux host and its
installed matched packages. `--trace-format human` explicitly tests the human
adapter; `--generator` accepts installed single/multi-config generators. SDK-only
`--cmake-option KEY=VALUE` is common to all cases; use
`--case-cmake-option VARIANT:KEY=VALUE` for independently matched prebuilt Conan
toolchains when tests ON/OFF require different dependency graphs. Do not reuse a
build tree, override fixture-owned flags, install/build dependencies implicitly or
substitute these local checks for native CI. Configure/build/discovery/test and
rejection logs are retained under the fresh `build-*` report directory.
Preserve all ordinary/empty/limit and 60-method deadlines when correcting any
factory-versus-painter binding; R1's private frame is not yet the full painter.
The shared runner now binds the initial exact discovery/XML IDs, selected
source/provider/shader/link identities and owned process retirement. Initial
prebuilt-provider source jobs are wired above; next complete the full option/host
matrix and obtain missing native/cold-provider results without removing minimum,
failure or platform requirements. Keep the
one-use local verifiers as evidence, not a second production test framework.

| Subpatch in order | Files and implementation | Required check before continuing |
|---|---|---|
| R1.1 CPU and factory split | Root/viewer CMake, renderer option declarations, `createpainter.*`, `viewersettings.*` and legacy target. Establish the CPU-only option path first; move concrete factories/generated GL globals out of the CPU viewer and keep Cairo behind its existing option. | Fresh CPU-only configure/build and CPU tests with Qt/OpenGL/GLEW/GLFW discovery disabled; header and link audit, no backend-to-viewer cycle. |
| R1.2 Private RHI and assets | Add the single private RHI target, matching GuiPrivate/ShaderTools, target-local C++17, capabilities and offline `.qsb` resources. Reuse spike lessons, not a dependency on its test executable. | Minimal real offscreen pixel test; missing private headers/qsb/backend and mismatched provider negatives; build does not write source-tree resources. |
| R1.3 Host and provider options | Root/UI/CLI/test/Lua CMake, Conan and install rules. Propagate RHI/legacy/test flags through every provider branch, including MSVC's Lua GLEW path. Keep startup default legacy. | Fresh CPU-only, legacy, dual and RHI-only truth-table builds; explicit/cached conflicting options fail. Separate Python OFF and UI OFF checks; effective flags and direct link dependencies match the request. |
| R1.4 Test and CI boundary | Minimal proposed `lcrenderertest`, CPU contract target, existing selection manifest/runner and source jobs. New tests do not depend on old GDK/GLFW or Python switches. | List/discover/execute exactly the initial required IDs on qualified APIs; a compiled-out/missing suite fails. Retain source/provider/shader and linked-library manifests. |

Keep the split small: one CPU recording/geometry/asset boundary, one private RHI
executor and thin widget/offscreen hosts. Do not create an interface per primitive,
parallel scene graphs or a backend plugin registry. Audit public headers for C++17
types before linking C++14 legacy consumers; target-local QRhi C++17 must not leak
an undocumented standard requirement. Every target compiling Qt 6 headers must
honor that provider's C++ requirements, including UI/CLI/test targets; this is not
permission to compile Qt 6 in C++14 mode. Keep the kernel-only floor separate.
Propose a conditional CMake 3.21 floor for
RHI builds, matching the spike/Windows packaging, and ratify it in D2; do not
silently raise a CPU-only build's floor. Test missing GuiPrivate/ShaderTools/qsb,
unsupported Qt backend and conflicting cached options as configure failures.

**Exit:** default legacy and dual hosts, plus CPU-only and RHI-only libraries/
tests, build with effective options and audited dependency boundaries. An RHI-only
negative-discovery job rejects project-side GLEW/GLFW discovery/direct links.
Qt's own provider closure is audited separately. Required R1
offscreen tests execute real pixels/readback on the declared API/device; source
CI must pass the complete cohort below. RHI-only application widgets and a
migrated offscreen CLI are R3/R4 deliverables, not an unimplemented R1 prerequisite.
Until then, requesting a host without legacy must fail explicitly; do not install
a dummy painter or silently fall back. R0 decisions/prototypes and independent
review remain prerequisites to R1 acceptance even after its source jobs are green.

#### R1 all-platform CI continuation

The current source workflow declares **60 required source jobs**: five native
architectures × tests ON/OFF × `libraries-core`, `libraries-python`, `cli`,
`cli-persistence`, `ui`, `both`. The library split covers the original **50 logical
scopes**; it is scheduling, not ten new platform requirements or reduced coverage.
The protocol job and fail-closed source aggregate bring the expected run to 62
jobs. Current exact-SHA results are in the checkpoint at the top of this plan.

| Scope | Required effective coverage | Not proved by that scope |
|---|---|---|
| libraries-core, tests OFF | CPU/legacy/dual/RHI with Python OFF, compile/graph/artifact contracts. | Runtime GTests or library installation; no claim of a tests-OFF header-check target. |
| libraries-python, tests OFF | The same four modes with Python ON; disjoint union with core is all eight modes. | Installed interpreter initialization or packaged runtime closure. |
| libraries-core, tests ON | CPU, legacy and dual library profiles; CPU contracts plus fresh pure-RHI build/CPU/RHI execution. | Full painter semantics or production viewport rendering. |
| libraries-python, tests ON | Fresh CPU baseline plus CPU/legacy/dual/RHI Python profiles, exact CPU-ID parity, CPU contracts and pure-RHI execution. | Widget interaction and every scripted feature/platform package. |
| cli / cli-persistence | Independent CLI, persistence OFF/ON respectively. Tests ON is dual/Python ON; OFF is legacy/Python OFF. | Migrated RHI CLI rendering; the current CLI still uses legacy. |
| ui / both | UI-only or UI+CLI. Tests ON is dual/Python ON with exact non-widget test execution; OFF is legacy/Python OFF. | Discovered/compiled widget IDs have not executed; dual does not mean the UI uses RHI. |

Both tests-ON library groups retain the 92-ID main CPU suite and 22-ID RHI suite;
both Linux architectures run the RHI offscreen checks under separate XCB and
managed-Wayland configurations. These are offscreen/QPA checks, not presentation
or input qualification. The standalone library-group default remains `all`.
There is no claim of a full host × renderer × Python × provider × build-flavor
cross-product. In particular host Python-OFF/tests-ON, Python-ON/tests-OFF,
minimum/current SDK pairs, Debug/sanitizers and later RHI-only hosts remain
separate checks, not hidden coverage credited to this matrix.

Every persistence-enabled host explicitly builds clean libdxfrw submodule
`f7debf4c5f30734a70c5e828cf05247aabc2d04b` with the three shared required patches
into an isolated PIC static SDK. Retain original/patched source hashes, patch and
license identities, tool/command logs and consumed binary/header manifests.
The original submodule is never patched in place or globally installed. Other
dependencies remain binary-only: Conan `--build=never`, explicit Homebrew bottles
without recursive source fallback, and recorded distribution binaries. A cold
provider miss fails with a diagnostic; it does not authorize a source build,
automatic pin bump or local package installation. The Windows libcurl 8.23.0
route now has selected native source-job results, not merely graph preflight.

The staged SDK builder must prevent parent-repository discovery by `git apply`
and reject an unchanged patched-source manifest. Hosted staging inside the
checkout previously skipped every patch despite successful command exits;
native in-checkout rebuilding now confirms changes to all three required files.
Visual Studio graph inspection binds target IDs, ignores only identified CMake
generator utilities, resolves the exact Release object configuration and retains
the shared UI compilation audit. DLL/import/object outputs stay mandatory;
unemitted optional Release PDBs are not compilation evidence. Unix CI selects
Ninja to avoid repeated Make target traversal. The owner has approved the
host-equivalent 1,200-second library-profile budget, with separate dependency/test
compile stages and the unchanged maximum 300-second command cap. Every profile
records both limits; expiration still fails, cannot restart the deadline and
cannot skip a test. CPU/source preflight totals retain their existing 300-second
limits; only library/host profiles have the approved 1,200-second total. Each
command uses `min(command_cap, remaining_total)` so clock-rounding cannot exceed
the cap or extend the original deadline. The hosted job cap remains 60 minutes.

UI/both jobs select matching Qt Widgets/Svg/UiTools providers. Windows CLI
installation distinguishes DLLs from import archives and observes actual loaded
module paths through an opt-in Toolhelp snapshot before CLI help exits. The
shared installed-prefix checks retain owned manifests, moved bytes and module
reports. Normal startup/defaults and graphics behavior are unchanged. Existing
renderer `.gitattributes` rules belong in the candidate so Windows preserves
contract/fixture bytes; unrelated research/scratch files remain excluded.

The owner approved publishing only the R1 candidate to
`dxli/LibreCAD_3:codex/r1-platform-ci` and running Actions. The direct branch
trigger permits this without a PR or release workflow change. The aggregate
requires both source/protocol checks and every source matrix entry to succeed;
failed, missing or skipped required work is not acceptance. Its current code
checks the two `needs` results; it does not independently count expected tuples
or validate every uploaded payload. The run-metadata/artifact review below is
therefore still required, not an already implemented completeness validator.
Do not close R0/R1, D1–D6, missing runtime/provider/OS or package gates from
workflow definitions, local fixtures or a future green source aggregate.

##### Ordered source-CI closeout work packages

Owner for implementation/self-review: Codex under the existing R1 scope. A
maintainer has approved the bounded source-CI device policy. Technical cohort
execution and Codex self-review below are complete; independent evidence
acceptance is unassigned and must be named before phase/release closeout.
No package/R2/default work is implied.

| Priority / work package | Inputs and bounded implementation | Required validation / stop condition |
|---|---|---|
| P0.1 Intel provider decision — approved | Owner explicitly authorizes verified virtual Metal for Intel Mac R1 source CI on 2026-10-24. Existing adapter JSON/XML identify Apple's paravirtualized provider; hardware acceptance remains mandatory separately. | Decision is recorded, not proof of rendered pixels. Execute native selected-device verification and all 22 tests before source-CI closure; unknown/name-only identity still fails. |
| P0.2A Explicit virtual tuple — implemented and source-verified | Existing diagnostic/validator/runner/GTest/workflow boundaries now bind the selected native Metal registry ID to the exact paravirtual accelerator. Raw Qt type remains unknown/virtual; name-only, contradictory or lost native identity fails closed. Only the Intel R1 source/offscreen tuple is additive. | Current exact-SHA core and Python Intel jobs each pass all 22 original RHI IDs, no skips, with independent system display-ID matching and negative classifier/scope regressions. Hardware/widget/performance/package configurations are unchanged. Preserve this verification on future changes; use route B if it fails. |
| P0.2B Hardware-backed Intel fallback | If virtual source verification fails, provision an authorized native x86_64 Intel Mac runner with supported Metal hardware and retained image/OS/Qt/qsb/device/session provenance. Limit runner-label changes to affected source jobs; keep fork-code isolation and no signing/release credentials. Required hardware qualification remains separate even while route A passes. | Prove actual ISA and `hardware` classification, run both tests-ON library groups and all 22 IDs without skips. A runner label, Rosetta, or an ARM64 result is not proof. No deployment-floor or widget acceptance follows. |
| P1.1 Required-cohort evidence check — self-review complete | Existing matrix/report validation plus one-use read-only retention checks verify all 60 unique tuples/artifacts, exact repository/event/workflow/full SHA/attempt, options/profile unions, source/helper identities, discovered/executed XML IDs, native adapters, File API graphs and process retirement. All payload ZIPs are retained locally with metadata/digests/CRC; expected negative XML is separate. Uploads fail if evidence is missing. | Current cohort passes. Preserve failed history; obtain independent review and durable external retention. Missing uploaded SDK files remain metadata-only, and this packet does not close full provider/license/deployment qualification. No new repository harness or acceptance engine is introduced. |
| P1.2 Fresh full cohort — execution complete | d4e9650e5dd03b8396923e7001dbee1d7c0bd3fb, run 40076688956 attempt 1: 60 source jobs plus protocol and aggregate all pass. All five architectures pass 12/12; nothing is combined across commits/attempts. Superseded failure logs remain retained and no obsolete run is active. | Reuse this exact cohort only for its source scope. A future source/helper/provider/workflow change requires fresh relevant proof and a complete coherent cohort before a renewed all-platform claim. Keep every test and existing budget; diagnose a new failure without weakening requirements. |
| P1.3 Review and handoff — documentation closeout | Synchronize this checkpoint and the Python plan; preserve prior receipts, original index/checkout and unrelated paths. Keep source-CI success distinct from independent/phase/platform/package acceptance. | Repeat plan/checker/worktree/inventory/lint checks after the final receipt. Independent reviewer/durable retention and R0/D1–D6/provider/hardware/session/deployment/package gates remain open. No automatic R2, merge, release or default change. |

Compactness constraint: change the existing diagnostic/tuple/runner boundaries;
do not add a second renderer, provider framework, scene graph, test harness or
acceptance-state engine. Preserve the three staged UI object groups, single
MOC/resource owner and audited consumer exports. Retest these invariants when
touching build orchestration; historical red receipts do not reopen solved
timeout/device checks or invalidate their preserved failure history.

Implemented route A's touchpoints are `lcrenderer/rhi/renderer.{h,cpp}` and private `device_p.h`,
`lcrenderer/rhi/tests/buildtest.cpp`, `unittest/rendering/renderer_cases.json`,
`scripts/{run_renderer_checks,validate_renderer_inventory}.py`, their existing
regression modules and `.github/workflows/renderer-source.yml`. Keep native/Qt
types out of the C++14 public boundary. Recompute inventory/selection counts
from the actual manifest; preserve every existing required hardware tuple and
case instead of hard-coding today's 223 proposed variants as a permanent total.

### R2 Drawing commands and portable rendering core

1. Implement path/state/transform contracts and immutable commands; keep legacy
   source compatibility and independently test CPU recording without a GPU.
2. Implement triangle strokes/dashes, fills/gradients, adaptive curves and local
   origins. Establish blend/coverage conventions and ordering. Run F1–F4/F7 tests
   on each available qualified backend as features land, not only at cutover.
3. Implement persistent geometry, staging and cache invalidation/retirement with
   memory limits. Separate live selection/style from cached geometry where possible.
   Add upload/draw/cache metrics and lifecycle/failure tests.

Use these bounded subpatches after D3 selects the algorithms; each adds its case
IDs to the same selection manifest and keeps the preceding offscreen scene working:

| Subpatch | Concrete implementation and independent acceptance |
|---|---|
| R2.1 CPU path/state | Own paths/subpaths, full style snapshots and double transforms. Test empty/repeated stroke/fill, reset/save/restore, singular/zero scale, nonfinite input and inverse/distance queries against analytic results before GPU creation. Reject invalid transforms without corrupting later valid commands. |
| R2.2 Curves and strokes | Flatten supported spans with the approved screen-space bound, then build ordered triangles for cap/join/miter/dash/coverage. Validate endpoint/cusp/inflection and near-180-degree joins; odd patterns, offsets and closed seams. Check distance/bounds against analytic or high-precision independent geometry, not just sampled matching pixels. Bound recursion, vertex counts and time. |
| R2.3 Fills and hatches | Apply the accepted fill rule, contour orientation/hole handling and bounded CPU tessellator. Test concavity, nesting/disconnected loops, touching/self-intersecting contours under the declared domain, degenerate hatch bounds and zero/negative pattern scale. Verify signed area/point membership and interior/background landmarks. Replace legacy approximation where it fails the contract; no triangle-fan or unbounded hatch expansion. |
| R2.4 Executor and shaders | Consume the same recorded commands on widget/offscreen hosts. Test packing/reflection, distinct aligned per-draw ranges, vertex/index layouts, required SPIR-V/MSL variants and target-compatible pipelines. Check byte-size/index arithmetic before allocation; chunk within device limits without reordering alpha or resetting dash phase. A multi-style same-frame case must catch last-uniform-write corruption. |
| R2.5 Cache and retirement | Exercise document/entity/style/tolerance/device generations, affected and unaffected inserts/views, selection, undo/reinsert, in-flight deletion and bounded eviction. Compare cached/uncached output and count uploads/allocations. Test two separate QRhi owners with colliding local IDs/generations; no cross-device resource reuse or stale-handle aliasing is allowed. |

Set gradient stop ordering, duplicate/out-of-range offsets, interpolation/alpha and
zero-length-axis behavior in D3; test them in R2.3. Pin the initial SDR contract to
the spike's explicit `SDR-unorm-no-sRGB-transfer` only if D3 accepts it. Do not
silently add hardware sRGB conversion, premultiply twice or imply color-managed
composition from correct interior RGBA values.

**Exit:** numerical/path tests plus actual offscreen rendering pass the core
feature matrix, including cache invalidation and large-coordinate/DPI cases;
validation-layer/resource-lifetime errors are blocking.

### R3 Text images and application integration

| Subpatch in order | Implementation boundary | Required acceptance |
|---|---|---|
| R3.1 CPU assets and metrics | Font selection/UTF-8/metrics, decoded image/pattern snapshots, bounded opaque handles and atlas policy; no widget required for metrics. | Licensed real-font/image fixtures, metrics plus rendered placement, invalid/stale handles and bounded resource-limit errors; F5/F6 oracles below. |
| R3.2 Surface and consumers | `LCADViewer` wrapper and private RHI surface, model/paper/proxy/MDI callers, promoted widgets and bindings. Keep event/document APIs CPU-facing; move GPU work into legal callbacks. | Every F9 consumer and all drawable families execute, including dimensions/hatch/images/inserts. Test focus/menus/drag/preview/undo, shared top-level devices, cross-window reparent and never-initialized teardown. |
| R3.3 Selection and failure UX | `main.cpp`, About diagnostics, API choice before hierarchy construction, bounded loss/recreate/close behavior. Isolate `QOpenGLDebugMessage`, GLEW and context sharing to legacy. | Forced unsupported/compiled-out choices fail; no hidden GL surface or silent fallback. Preserve unsaved documents/save/undo and callbacks through failure; report actual API/device. |
| R3.4 Scripting and startup regression | Existing Lua/Python custom entities, bindings and startup smokes, with renderer selection shared by app and tests. Do not change scripting ownership as part of GPU work. | F11 draw/edit/preview/commit/cancel/undo and repeated shutdown; Python OFF GUI/install and lazy no-GPU `--python-smoke`/CPU scripting remain supported. |

Implement CPU font selection/UTF-8 decoding/metrics before atlas upload. Reuse
FreeType preparation where suitable, not `GL_Font` ownership or its fixed first-256
glyph population. Cover supplementary code points independently of `wchar_t` width,
malformed UTF-8, missing glyphs/fallback, whitespace/newlines, bearings/advances,
font reload and bounded atlas growth. Pin a redistributable real font; the spike's
bitmap glyph is not this oracle. Check metrics and rendered placement together in
dimensions/MText, with the shaping domain explicitly accepted in D3.

For images/atlases test odd-width row stride, transparent edge filtering, mirrored
affine transforms, missing/oversized assets and texture-page eviction. Keep decode
and handle lifetime CPU-owned; GUI delivery cannot access a deleted document.
Validate dimensions, stride/product arithmetic, aggregate decoded bytes and device
texture limits before allocation/upload; compressed file size is not a decoded
memory bound. If using Qt image plugins, retain an application-side asset budget
as well as the provider's [image allocation limit](https://doc.qt.io/qt-6.8/qimagereader.html#setAllocationLimit),
which has an environment override. Test oversized headers, truncated data, failed
decode and cache pressure without exhausting real memory. Preserve document asset
resolution policy; the renderer must not introduce implicit network downloads.
The viewport wrapper must request redraw on document/style/input changes without
a perpetual timer; validate hide/minimize, screen/DPR change, reparent, sleep/resume
and available remote-session transitions separately from physical device loss.

**Exit:** all F1–F9/F11/F12 source-build scenarios pass on supported native platforms;
new startup/smoke paths also work with Python disabled. Missing fonts/drivers and
device recreation produce explicit errors, not empty successful output.

### R4 CLI offscreen output and regression harness

| Subpatch | Implementation boundary and tests |
|---|---|
| R4.1 Offscreen host and CLI | Replace GLFW/GL setup/readback in supported CLI builds with the same executor and typed CPU snapshot. Preserve Lua/Python dispatch, size/orientation/exit codes and Cairo/vector paths. Test nondefault/odd dimensions, alpha/stride/origin, missing driver/assets, write failure and UI OFF without Widgets. Windows CLI remains out of current provider scope unless separately accepted; Windows offscreen renderer tests are still mandatory. |
| R4.2 Production smoke and records | Implement the section 6 app smoke/report flags and distinct production evidence kinds. Widget mode uses the real viewport/event loop; installed smoke uses embedded provenance without checkout/Git/SDK access. Negative tests reject spike substitution, stale/mismatched identity, partial output and false backend claims. |
| R4.3 Required test selection | Extend the existing R1 renderer exact-ID/XML path and the single manifest to the remaining application/CLI cases; keep legacy `run_required_gtests.py` count/filter consumers aligned without creating a parallel framework. Select the reviewed stage/configuration before discovery and compare expected, filtered-discovered and executed IDs, including parameterized names. Test missing/disabled/compiled-out suites, wrong filters, missing XML, skips, duplicates and unexpected selected IDs. Unrelated unselected suites are allowed; one present suite cannot hide another missing suite. |
| R4.4 Cross-backend regressions | Port existing fixtures and add F10/component/image/input/lifecycle cases with independent geometry/raster oracles. Separate processes per backend; CPU suites require no GPU, offscreen no Widgets, widget suites their declared actual sessions. Preserve exact failures as well as successes. |

Split development freshness from installed validation: the R0 validator reads the
source tree, but production/package smoke must work without Git, SDKs or checkout
paths. Embed reviewed build/input/asset identities and let the external runner bind
them to the candidate/package. Release source archives carry their provenance
manifest so building an archive does not require a `.git` directory; unknown or
unreviewed provenance cannot pass a release gate. Fresh source builds still check
actual inputs, not just a manually supplied revision string.

**Exit:** actual render/readback works without a visible window where supported;
CLI and viewport tests cover F10; the harness rejects missing/skipped/partial runs,
backend fallback, empty output, timeout and driver errors.

### R5 CI scripts and installed package qualification

Implement every path/job in section 6, including clean installed-artifact smoke,
script regression tests, evidence upload and the required aggregate gate. Use the
same qualified per-platform build/toolchain/shader pins for source, package and
runtime tests; different architectures/providers have separate manifests.
Run native hardware/widget tests in addition to software offscreen tests. Retain
all existing Python-provider, parity, persistence and package-isolation gates.

Deliver this in four compact steps, reusing the platform scripts rather than
introducing per-package renderers or smoke implementations:

| Subpatch | Implementation and fail-closed result |
|---|---|
| R5.1 Reproducible source CI | Finish all source/native tuples, minimum/current providers and option negatives; run CodeQL against enabled RHI code. Cold restore pinned prebuilt inputs, execute required IDs, upload evidence on failure and audit descendant cleanup. |
| R5.2 Staged install and Linux packages | First prove each architecture's clean staged install, then x64 AppImage payload/launcher and actual amd64 Snap. Update deployment tools, base/GPU/display content and assets as needed; hash and test exact outputs, not just build-tree executables. Keep FUSE/confinement caveats explicit. |
| R5.3 Native Windows and Mac artifacts | NSIS install with sanitized PATH; both native Mac bundles installed/relocated with closure/rpaths and final signatures. Test offline shaders/fonts/plugins and Python ON/OFF. Never call a cross-build, ad-hoc signature or source smoke final package acceptance. |
| R5.4 Trusted hardware and release binding | Run approved hardware/performance/session cases on candidate packages; join trusted source/package/hardware records by exact source/provider/artifact identity. Establish protected status names and reviewed release validator before R6, without giving PR jobs write/signing credentials. |

**Exit:** the full platform/evidence matrix in section 7 passes at the candidate
source/provider pins. Syntax lint, build success or the existing Python smoke alone
does not satisfy an installed-render or hardware gate.

### R6 Default migration and compatibility release

Switch `auto` to Vulkan/Metal only after R0–R5 pass their evidence review. Retain
an explicit legacy option during one designated compatibility release and publish
driver/Qt requirements, known limitations, fallback/recovery behavior and diagnostics.
Set the designated release and observation criteria at R0 rather than waiting an
undefined number of releases. Required CI must force the new backend even while
an end-user compatibility option exists. No known required feature may be deferred
as a hidden cutover exception.

**Exit:** default source and final package runs use and identify the intended API;
no unresolved correctness, stability, supported-platform or mandatory performance
regressions remain. A rollback changes the default, not document formats or geometry.

### R7 Legacy retirement

Remove legacy GL source/factories/resources, `glewsupport.*`, context-sharing flags,
GL diagnostics, GLFW-based tests/CLI paths, direct dependency discovery/linking and
obsolete Conan/package requirements. Remove resources/licenses only when no shipped
consumer still needs them; GLM/FreeType or Qt's own OpenGL support are not automatically
obsolete. Update documentation, CI filters and artifacts together. Keep an optional
tested RHI-OpenGL fallback only if explicitly retained in the support policy.

**Exit:** a clean RHI-only checkout builds/tests/packages without application GL,
GLEW or GLFW discovery; dependency/compiled-source audits agree; all earlier gates
still pass and the designated compatibility-release criteria are met.

## 6. CI and script implementation details

### Shared smoke and evidence contract

The shared `scripts/run_renderer_checks.py` and `scripts/validate_renderer_inventory.py`
implement separate explicit R0 `--spike` and provisional R1 `--rhi-contracts`
paths, with focused tests under `scripts/tests/`. R1 performs exact ID/XML and
observation checks here; the existing count-based `run_required_gtests.py` remains
unchanged for its current consumers. Extend the shared orchestration for later
app/package contracts; neither current evidence kind certifies those artifacts.
Reuse package helpers and avoid a second runner or plan-management framework.

The runner selects an explicit backend, executes each required suite, launches the
real app/CLI smoke with a timeout, validates outputs and returns a nonzero status
on failure. A strict request cannot succeed through `auto`, Null, a different API,
or a missing/skipped test. Required software runs identify a software device;
required hardware runs reject software devices. Never silently substitute one.
Define the expected suite set per configuration: Python-OFF legitimately excludes
Python-only suites, but cannot suppress native renderer/contract suites. Verify
each expected suite separately so another matching suite cannot hide its absence.

Widget smoke must use the requested real QPA plugin/session, show and expose the
top-level window, observe a render callback and submitted frame after exposure,
and check the viewer plus its composition with ordinary widgets/overlays. Calling
`grabFramebuffer` on an unshown QRhiWidget can succeed offscreen and is insufficient.
Report platform-plugin name, exposure state and frame identity; reject `offscreen`
or `minimal` plugins in presentation jobs. Offscreen smoke is a separate mode with
its own expectations and can use a virtual display where the provider needs one.

Use a versioned report schema with these required common fields: source revision
and dirty status, effective build flags, build/runtime Qt, shader/qsb identity,
OS/architecture, requested/actual API, initialized device name, execution mode,
pixel dimensions/DPR, selected format/sample count, fixture/test identity and
counts, validation result and output-image hashes. Read actual API/device values
from the initialized renderer, not environment variables or process exit alone.

R0 uses schema version **3**, `r0-spike`/`r0-spike-runner` success kinds and separate
`r0-spike-failure`/`r0-spike-expected-failure` terminal kinds. Preserve that boundary.
Specify the production/runner/package schema before R4 integration with explicit field names/types and negative
fixtures; do not label the future records below as already implemented or overload
spike totals as GTest results. Version incompatible changes and reject old reports
for changed inputs/scenarios rather than silently migrating their claims.

| Record | Producer and required content |
|---|---|
| `build` | Configure/build: actual checked-out full SHA, source-input diff identity, effective options, architecture/compiler, Qt/qsb and shader manifest. Generated resources stay in the build tree; release inputs must be clean or carry an explicitly reviewed patch manifest. |
| `runtime` | Initialized app/test renderer: requested/actual API, device name/classification, execution/QPA mode, exposure/frame identity for widget mode, target format/sample count/DPR and supported capability subset. |
| `checks` | Runner: expected/discovered/executed test IDs and skipped/failed counts from actual GTest output/XML, plus smoke scenario results. The app does not self-certify GTest execution or reuse counts from another configuration. |
| `outputs` | Runner/app: fixture identity and normalized image metadata/hash, validation log identity and completed readback frame. No successful report before its asynchronous outputs complete. |
| `package` | Trusted workflow's external manifest: artifact SHA256 and associated executable/assets/report digests, source/provider and trusted run identity. It is not embedded self-referential package metadata. |

Reject unsupported schema versions, duplicate/ill-typed required fields and missing
records for the requested evidence kind. An app-only render report is not yet the
runner's test report or a trusted package manifest; validators compose those records
rather than requiring every process to invent information it cannot observe.
Lifecycle records name the operation (target resize, consumer invalidation,
surface replacement, owned-device teardown or injected loss/recovery), generation,
pending/completed state and result. Synthetic Qt events with observed focus are
not physical-input evidence; a completion before destruction is not pending-GPU
destruction. Evidence consumers must compare the requested scenario, not just a
generic `lifecycle: pass` flag.

Keep availability-qualified metadata separate. Qt 6.8 does not expose a universal
driver version, vendor ID or device ID: accept documented backend-specific zero/
unavailable fields with a reason, not invented values or a blanket schema failure.
Use `QRhiDriverInfo`/qualified native queries for device classification; an unknown
classification cannot pass a required software/hardware gate. Record software ICD
identity/digest in software jobs; driver and GPU cohort details are required where
performance comparability depends on them. [Driver metadata](https://doc.qt.io/qt-6.8/qrhidriverinfo.html)

#### Expected failure evidence

Normal runner mode rejects nonzero process exit. Additive
`--expect-failure=render|readback|shutdown` now validates only standard-widget proofs,
with app `r0-spike-failure` and runner `r0-spike-expected-failure` records. These new
kinds retain schema 3 without changing existing success fields. They require exact
exit 1 and validated failure diagnostics, not stderr matching or inverted exit status.

The implemented failure validator requires:

- Requested/reached injection point, full build/provider/source identity, observed
  API/device/class/architecture/QPA, exposed render state and capture/device generation.
  The selected injection must execute; unavailable sessions/drivers cannot stand in.
- Process exit exactly 1, a persistent terminal diagnostic, actual versus effective
  injected status and completion/pending observations. Unexpected additional errors,
  signal/crash, timeout, missing/malformed diagnostics or exit 0 fail the test.
- Zero accepted outputs and no success `app.json`, PNG or RGBA artifacts. Bind binary,
  helper and failure-report hashes and reject changes during the run, as in success
  mode. Keep failed-run logs in the fresh evidence directory.
- For shutdown, retained-result count and last-owner release after the qualified
  teardown boundary; reject inconsistent ordering/counts. Mark physical device loss
  false for substitutions, especially a drain failure substituted after real completion.
  Readback/shutdown cases must observe capture completion at widget release; a
  missing or genuinely failed native drain cannot satisfy the controlled failure.
- Dedicated expected-failure case IDs, never image-landmark/GTest successes. Test
  unrelated startup failure, false reached-point claims, wrong backend/class, stale
  identity, secondary error, success artifacts and late-success-overwrites-error
  as validator/runner negatives, plus real C++ lifetime/latch execution.

Success and expected-failure contracts share identity checks, not fabricated
successful views; their outcomes remain separate. Missing failure evidence fails the job.

#### Package evidence and harness validation

Package binding is external: produce a manifest mapping the final package SHA256,
tested executable/assets and report hashes to the source/provider identity. Do not
embed a package's own hash inside that package or trust an arbitrary PR report as
release authority. The validator checks this binding plus required report fields,
backend/mode/fixture agreement and compatible build/runtime providers.

Report tests must include malformed/missing inventory, wrong API/device/architecture,
stale shader/provider data, zero or skipped tests, missing expected suites, partial
execution, null/blank output, timeout and incomplete package closure. Do not reject
all legitimate blank drawings globally: the smoke fixture deliberately contains
known colored coverage/geometry landmarks so its output cannot be blank.
Add fixtures for legitimate zero/unavailable driver metadata, same source with a
different package/provider hash, wrong workflow/event/SHA in release evidence,
unexposed widget capture, and a shared target accidentally cleared between layers.

Example commands below are future interfaces, not runnable commands today:

```sh
python3 scripts/run_renderer_checks.py --tests build/bin/lcrenderertest \
  --app build/bin/librecad --backend vulkan --device software \
  --mode offscreen --report-dir build/renderer-evidence
build/bin/librecad --renderer=vulkan --render-smoke=widget \
  --renderer-report=build/renderer-evidence/widget.json \
  --render-output=build/renderer-evidence/widget.png
```

Use the matching native Python command on Windows and the installed executable
for package tests. A test-control backend option/environment hook must be private
to the harness or use the same checked selection code as the app.

### Existing paths that must be updated

| Existing path | Required change and verification |
|---|---|
| [main.yml](.github/workflows/main.yml) | Add explicit RHI configure/build/runtime checks to Linux x64, native Linux arm64 and Windows; keep required scripting suites. Run final AppImage/Snap/NSIS renderer smoke. Upload per-architecture render reports, comparisons and package identity. Do not publish release artifacts from untrusted PR execution. |
| [python-binding-macos.yml](.github/workflows/python-binding-macos.yml) | Keep both native architectures and scripting gates; add separate new renderer target/suites despite the legacy rendering switch being OFF. Test Metal offscreen and Cocoa widget presentation, source/install/relocated `.app`, with matching provider tools. The Python offscreen smoke remains a separate test. |
| [codeql.yml](.github/workflows/codeql.yml) | Trace new C++ renderer sources in an RHI-enabled build. Align the Qt provider/image with the qualified floor instead of relying blindly on Ubuntu 24.04's packages; stage dependencies before tracing. Keep Python analysis and do not disable new code merely to make analysis configure. |
| [Ubuntu build script](scripts/ubuntu-install/installDependenciesAndBuildRepo.sh) | Provision matching Qt private development headers, Shader Tools/qsb, Vulkan headers/loader, validation layers and qualified software ICD by architecture; name the RHI options. Add dedicated blocking renderer runs, not the informational non-core tail. Respect the selected build directory and existing Python inventory/isolation checks. |
| [AppImage script](scripts/ubuntu-install/createAppImage.sh) | Make errors/missing assets fail; pin/checksum deployment tools for reproducibility. Package shader/font/image assets and required runtime libraries/plugins; audit loader/host-ICD discovery without bundling arbitrary host GPU drivers. Emit artifact digest and run both extracted-payload and actual-launcher render smoke. |
| [Snap spec](snapcraft.yaml) and [Snap helper](scripts/ubuntu-install/createSnap.sh) | Add matching build/runtime render packages and flags; verify the existing `gpu` extension and display/GPU plugs with Vulkan and actual base-provided drivers. Preserve explicit `snapcraft pack`, output-success dependency and current devmode scope; a devmode pass is not strict-confinement proof. Run `snap run librecad` for render smoke in the installed snap. |
| [Windows dependency script](scripts/windows-install/createConanDirAndInstallDependencies.bat) and [Conan recipe](conanfile.py) | Make GLEW/GLFW conditional during coexistence, then remove only unused requirements. Provision one qualified Qt/private-header/qsb provider and pinned software ICD where needed; avoid mixing Conan, SDK and system Qt ABIs. Fail each install/configure command and record provider identity. |
| [Windows build/package script](scripts/windows-install/buildLibrecadAndCreatePackage.bat) | Build/run `lcrenderertest` independently of the forced-off legacy switch; propagate every test error. Keep existing Python/Lua suite discovery checks. Deploy Qt runtime/plugins/assets and explicit loader dependencies; install NSIS output and run strict renderer smoke after removing SDK/build paths from PATH. |
| [macOS bundle helper](scripts/macos-install/createBundle.sh) and [dependency walker](scripts/macos-install/stagePythonDependencies.py) | Stage new renderer library/assets, audit all non-system loads, fix install names/rpaths, and sign only after final changes. Do not require MoltenVK for the Metal policy. Keep relocation/Python closure tests; make Python-specific staging conditional if a Python-OFF bundle is selected rather than allowing graphics startup to depend on CPython. |
| Root, [viewer](lcviewernoqt/CMakeLists.txt), [UI](lcUI/CMakeLists.txt), [CLI](luacmdinterface/CMakeLists.txt) and [test CMake](unittest/CMakeLists.txt) | Implement target-scoped options/dependencies, correct install/resource paths and independent CPU/new-renderer/legacy test targets. Review CPack runtime sets and all provider branches, not just the default Unix branch. |
| [Lua build](lcadluascript/CMakeLists.txt) and viewer settings | Remove/conditionalize the MSVC-only GLEW dependency and source-generated GL font/shader globals when splitting targets. Verify Lua/custom-entity CPU paths in an RHI-only and CPU-only build; do not leave these dependencies hidden in a non-renderer library. |
| [Python plan-sync workflow](.github/workflows/python-binding-plan.yml) and [plan](PYTHON_BINDING_IMPLEMENTATION_PLAN.md) | Preserve fail-closed synchronization; register new exact/prefix paths with appropriate gates and update same-change receipts when implementation touches scripting/build/package inputs. Do not bypass the checker with broad exemptions or mutate CI evidence automatically. |

The R0 spike registered `lcrenderer/` with S1, S2, S6, S7 and ALL_PLATFORMS in
the existing Python plan map. Preserve it and add gates identified by subsequent
review. Existing `scripts/`, workflow and test rules already cover the helpers.
Documentation-only renderer-plan edits map to DOC and PLAN-SYNC; that classification
must not be reused to exempt renderer implementation or shader changes.

### Required CI configurations and responsibilities

Expand `.github/workflows/renderer.yml` from its R0 scaffold and initial reusable
R1 source jobs to the complete shared
renderer/configuration matrix, with native package qualification remaining in the existing platform workflows.
Do not duplicate full dependency builds in every job: consume distribution/prebuilt
SDKs or verified provider caches keyed by OS, architecture, compiler, exact Qt,
options, shader tools and lock/manifest digests. A cold-cache path must work; never
download unpinned executable tools or run one platform's cached binaries on another.
Versioned apt package requests and hosted runner labels are not immutable images:
retain resolved package artifacts/checksums and transitive package inventory, record
the actual runner image version, and test cache-miss restoration. Record qsb host
architecture separately from target architecture for any deliberate cross-build;
cross-compilation is never native runtime acceptance. Verify runner-label and
graphical-session availability against the [hosted-runner reference](https://docs.github.com/en/actions/reference/runners/github-hosted-runners)
when provisioning, not from the OS name alone.
The user-selected libdxfrw source submodule is the explicit exception: build its
recorded pin and required patch in an owned SDK prefix, retain original/patched
source and outputs and consume that SDK explicitly. Other dependencies retain
their binary-only policy; a missing provider never authorizes a source fallback.

For each required architecture, D2/D4 must provide one retained record containing
build-host and deployment-floor OS, host/target/tool architecture, compiler/CRT,
resolved Qt module/private-header/qsb packages, loader/ICD/driver identity, plugin
and session setup, configure/build/run commands and artifact hashes/notices.
Keep minimum and current-provider records separate. The verified Mac 6.8.3
Base/ShaderTools pin is only a standalone subset; the application's Svg/UiTools and
other enabled modules need matching provider/closure qualification too. Hosted
labels in the YAML request an OS/architecture; their availability and usable
GPU/session access must be verified when provisioned. Neither the label nor a
deployment compiler flag establishes deployment-floor runtime compatibility.

| Required configuration | Gate and execution policy |
|---|---|
| CPU-only, no RHI/legacy, UI/CLI disabled | Build and run CPU painter/geometry/kernel tests without Qt/GL discovery. Include Python OFF; retain existing Python-enabled kernel/scripting jobs separately. |
| RHI-only, Qt UI ON, legacy OFF — R3 onward | After R3 surface integration, compile shaders/backend/UI and run requested native API; discovery/link audits reject direct legacy dependencies. Required on all five architectures. R1 intentionally rejects hosts without legacy, rather than faking this path. |
| RHI offscreen, UI OFF | R1 executes its private test host with Qt Gui, not Widgets. R4 must migrate the real CLI/output path and scripting dispatch; the now-buildable native Windows legacy CLI is not already RHI output qualification. |
| Dual-backend during coexistence | Run each backend in a separate process; compare approved fixtures and exercise explicit legacy startup. A dual build cannot substitute for the RHI-only job. |
| Optional combinations | At least one Python-OFF GUI/install test per OS, a Linux Cairo configuration, Debug plus Release lifetime coverage, minimum qualified Qt and current supported Qt minor. Run full native architecture coverage on Release. |
| Linux software Vulkan | Force a qualified Lavapipe ICD, verify report device/API and run both native architectures. Retain separate XCB/Xvfb and managed-Wayland widget matrices. An actual Wayland client on nested Weston qualifies that software path, not DRM/hardware or physical input; `QT_QPA_PLATFORM=offscreen` alone is not presentation proof. |
| Windows software Vulkan | Qualify a pinned software ICD at R0 (for example SwiftShader), verify its real Vulkan execution and package notice/ABI. If unavailable, provision a trusted GPU runner; compile-only is an open gate, not a replacement. |
| Native macOS Metal | Execute offscreen suites on both architectures plus Cocoa/MDI widget smoke in a graphical session. A Qt offscreen plugin/Python pass does not establish Metal presentation. |
| Trusted real-GPU qualification | Windows/Linux representative Intel/AMD/NVIDIA and native supported Mac GPUs; exercise presentation, DPI, device recreation, performance and final packages. Verify hardware device classification. Record explicit run/artifact/source/provider identity. |

Assign cases to valid coverage layers rather than multiplying every dimension:

- Source Release qualification covers all five architectures, minimum/current
  qualified Qt, RHI-only and real offscreen/widget paths. Linux has separate XCB
  and Wayland software requirements; Windows uses the qualified software ICD or
  an explicitly provisioned trusted hardware path from D4, not an assumed ICD.
  Hardware/session qualification remains separately required on supported tuples.
- UI-OFF tests never require Widgets, widget tests never use a headless QPA, and
  Python-OFF does not select scripting cases. Retain separate Python-ON dispatch
  tests, optional Linux Cairo and supported non-Windows CLI requirements. Dual
  builds execute RHI and legacy in separate processes; a legacy result must not
  carry a Vulkan/Metal identity.
- Installed/package smoke exercises the shipped viewer/offscreen path, assets,
  scripted/unscripted startup and errors under the same capability policy. It is
  not every source GTest repeated inside an installer. Hardware/vendor, Debug,
  performance and sanitizer cohorts are declared explicitly in D2/D4/D5; they do
  not require every vendor on every architecture or all Debug/package permutations.

The manifest must retain the mandatory coverage of each layer, with explicit
reason/decision for inapplicable combinations. A missing device/session is pending
execution, not inapplicability. Changing a mandatory row needs the same support
decision as changing the completion contract; this layering cannot waive a gate.

PR jobs use read-only permissions, timeouts and pinned actions/tools; no publishing
tokens, signing secrets or persistent self-hosted GPU machines for unreviewed fork
code. Use disposable hardware runners or reviewed trusted push/manual qualification.
If GPU capacity/session support is unavailable, keep that acceptance gate pending.
Protected signing/notarization and releases use separate trusted environments.

For managed Wayland retain compositor version/backend/renderer, selected input
hashes, private socket/session isolation, app QPA/API and DISPLAY removal, exposure,
input acknowledgment and independent child exit status. Version-specific Weston
options must be tested against the selected binary, not assumed from current
documentation. Never send automated input to a user's real desktop to satisfy CI.
Add bounded process-tree retirement assertions: logs, a closed socket or a clean
Weston exit do not prove that Xvfb/helper descendants were reaped. Cleanup failures
fail the harness gate even when rendering pixels pass.

### Workflow graph and required statuses

GitHub [`needs`](https://docs.github.com/en/actions/reference/workflows-and-actions/workflow-syntax#jobsjob_idneeds)
cannot join jobs in unrelated workflow runs. Keep the source-render
aggregate inside `renderer.yml` and require the existing platform/package statuses
separately in branch protection. Do not imply that a green source aggregate proves
that AppImage, Snap, NSIS or macOS packages in other workflows passed.

| Proposed job or workflow | Dependencies and acceptance |
|---|---|
| Current `r1-source` call in `renderer.yml` / `renderer-source.yml` | 60 native source jobs + protocol + fail-closed source aggregate use the shared runner. Latest cohort is 58/60 source successes; Intel library device policy is unresolved. Initial UI/CLI source/non-widget checks and RHI offscreen checks are not the full option/widget/provider/package matrix. Selected uploads are not yet a payload-complete acceptance archive. |
| `renderer-contracts` in `renderer.yml` | New helper/selection-manifest/method-register failure tests, shaders/configuration metadata checks and workflow lint. No GPU required. |
| `renderer-cpu` in `renderer.yml` | Fresh CPU-only configure/build/test with Qt/GL discovery disabled; Python OFF and no GUI/CLI. |
| `renderer-linux` matrix in `renderer.yml` | Native x64/arm64 RHI-only Vulkan software offscreen; declared X11 and Wayland widget cases, with separate mode reports. |
| `renderer-windows` in `renderer.yml` | Native x64 RHI-only Vulkan offscreen under the qualified ICD/device; no accidental D3D/OpenGL success. |
| `renderer-macos` matrix in `renderer.yml` | Native arm64/x86_64 RHI-only Metal, declared offscreen/widget cases. Prove graphical-session support at R0; no skipped runtime substituted by compilation. |
| `renderer-required` in `renderer.yml` | `if: always()`, explicit `needs` on all expected source/contract jobs; verify every matrix leg, counts and reports, not only aggregate exit status. Expose stable `Renderer Required` status. |
| Existing main/macOS workflows | Retain source/scripting gates and add artifact-specific renderer smoke; keep their package/native-architecture statuses required independently. |
| Proposed trusted `renderer-hardware.yml` | Reviewed trusted push/manual execution on qualified real devices; produce hardware/performance and candidate-package reports, not an untrusted-fork job requiring secrets. |
| Trusted release/default-cutover validation | Validate same full source SHA, effective per-platform provider/configuration, exact tested package hashes and successful trusted source/package/hardware runs; reject stale or missing reports. Gate `UpdateAssets`/release promotion on this check. |

Missing/skipped/cancelled required jobs fail the source aggregate. Required statuses
must run on every applicable PR without top-level path filters leaving them absent;
any optional work must be outside the required set. Document exact protected status
names after R0 runner/job provisioning; branch-protection configuration is a required
external action and must not be claimed changed by a YAML edit.
If a merge queue is enabled, include its required trigger/configuration and validate
the actual checked-out merge-group SHA. PR head, synthetic PR merge and trusted
post-merge/release commits are different evidence identities; do not mix their reports.

Release/default-cutover validation is not a cross-workflow `needs` shortcut: retrieve
trusted run/artifact metadata through an appropriately permissioned validator or a
reviewed release manifest, and verify repository, event, conclusion, full SHA,
workflow identity and hashes. PR software runs alone cannot authorize R6. Existing
`UpdateAssets` must explicitly exclude PR execution and cannot publish a candidate
merely because the old Linux/Windows build jobs passed. Never add write credentials
to PR renderer jobs to implement this coordination.

Upload evidence with `if: always()` where safe: test XML/logs, expected/actual/diff
images, runtime reports, validation output, provider/shader manifests, package
hashes and performance samples. Missing failure evidence cannot convert a failure
to a pass. Keep secrets out of reports and configure retention for regression triage.

## 7. Platform and package acceptance

| Deliverable | Required execution and clean-runtime proof |
|---|---|
| Linux x86_64 install | Native Vulkan offscreen and X11/Wayland widget checks; dependencies resolve outside the build tree. Python ON/OFF install scenarios are distinct from rendering. |
| Linux arm64 install | Native architecture and software-Vulkan tests, then supported-hardware qualification; identify emulation separately. Do not claim ARM64 AppImage/Snap distribution from this row. |
| x86_64 AppImage | Smoke exact staged install, extracted payload and actual AppImage launcher. Record FUSE-mounted versus extract-and-run evidence separately; default-launcher acceptance remains open if mounting was unavailable. Verify shaders/assets/loader and both X11/Wayland behavior. |
| amd64 Snap | Pack/install actual output and render through `snap run`; verify display/driver content/GPU access under the advertised current devmode policy. Do not run outside confinement and call it a package pass. |
| Windows x64 NSIS | Install exact installer into a path with spaces/non-ASCII text; render with build/SDK paths removed and system loader/driver policy honored. Run software qualification and separate real-GPU/window tests. |
| macOS arm64 and x86_64 bundles | Native Metal offscreen/widget tests, exact installed then relocated `.app`, final dependency closure and signature verification. Ad-hoc signing is not protected release/notarization evidence; qualify each architecture, not a Rosetta substitute. |

Keep graphics-runtime contamination tests separate from driver selection: remove
build/SDK Qt/plugin/shader/font paths without poisoning a deliberately selected
system/software ICD. Test missing/broken ICDs in dedicated expected-failure cases.
The Python-poisoning smoke stays unchanged and runs in addition to graphics smoke.

GPU/presentation failures must not overwrite a document, silently drop custom
entities, modify persisted geometry, or make save/undo depend on renderer state.
Check multiple windows/documents, block edits, layer visibility/style changes,
font/image reload and repeated open/close for stale output or unbounded resources.
For each package, exercise both installed offscreen output and the real widget
launcher; record display/session access separately. Loader/plugin/assets failures
must fail without relying on build-tree environment paths, and user-generated
output/cache files must go to writable destinations outside read-only installs.
Keep final signing/notarization after payload changes and bind evidence to that
final artifact; replacing a signed/tested payload invalidates its package receipt.

## 8. Validation budgets and maintenance

Use numeric/geometry assertions and small licensed deterministic fixtures alongside
raster comparisons. Pin fonts, image data, viewport, DPI, sample count and relevant
driver/provider identity. Compare edges/coverage, landmark placement, dash phase,
glyph metrics, alpha and fill topology with explicit thresholds; never approve
new baselines merely because output differs. The old GL renderer is a baseline,
not an independent correctness oracle.

R0 fixes reference workloads, machines and budgets for cold-open, warm pan/zoom,
single-entity edit, previews, text/hatch-heavy drawings, repeated lifecycle and large
control nets. Measure CPU preparation, GPU time where available, p95 interaction,
draw/upload/allocation counts, memory growth and idle redraw/power behavior. Do not
run continuous frames for an idle CAD view or claim a Vulkan speedup without data.
Software drivers validate correctness, not hardware performance budgets.

### R0 workload and budget worksheet

D5 must replace these **candidate** limits with accepted values and named hardware
before R1; none is a measured production result. Keep fixtures/generators and sample
metadata in the same selection manifest, not a separate benchmark framework.
R0 pins the inputs, measures legacy baselines and comparable spike primitives,
and accepts the limits. Production workload compliance is measured as R2–R5
implements each feature; R0 does not need to build the full renderer to set budgets.

| Workload | Repeatable input and operation | Candidate acceptance to ratify |
|---|---|---|
| Mixed and dense geometry | Seeded 10k and 100k drawable scenes; fixed viewport/zoom/DPR, 600 warm pan/zoom samples after 60 warmups, 30 independent cold starts. Pin an ordinary real CAD drawing too. | Per-workload p95 event-entry-to-submit ≤ 1.10× qualified legacy and ≤ 100 ms on reference hardware; cold-open p95 ≤ 1.20× legacy. Agree absolute cold-start budget in D5. |
| Text, images, hatches and nested curves/blocks | Pin counts/assets for 5k text items, 50 images, dense hatch loops and transformed nested blocks/splines. Time CPU preparation separately and exercise one-entity edits/selection. | No missing topology/glyphs/layers; candidate curve error ≤ 0.25 physical pixel. Set per-case metrics/edge/coverage tolerances; the spike's interior channel tolerance 3/255 is not a whole-image acceptance rule. |
| Long lifecycle and idle | 200 open/edit/undo/close cycles with two views and assets, including resize/suspend/recreate; sample process/GPU/cache bytes after warmup and after equivalent settled states. Observe an untouched view for 30 s after draining scheduled work. | Zero lifecycle/validation errors or monotonic retained-resource growth; candidate settled memory ≤ 1.10× post-warmup baseline. Zero unsolicited view submissions while idle; OS expose/resize events are recorded separately. |

Record sample counts, p50/p95, raw distributions, units, reference commit,
driver/provider, power mode, viewport and warmup rules. Compare only matched
correct output/workloads; legacy bugs or unsupported features are not a meaningful
performance comparator. Set absolute decoded-asset/geometry/readback/cache limits
from hardware capability and workload peaks, including bounded chunking/eviction
and a tested explicit resource-limit error. A percentage memory budget alone
cannot justify unbounded allocations. Review threshold changes as changes to the
acceptance contract, not automatic baseline refreshes.

Control run order and driver/OS cache state explicitly; repeated in-process opens
are not independent cold starts. Retain raw samples and report the uncertainty of
small p95 samples instead of promoting a single timing. D5 names the blocking
metric for each workload and its comparison cohort; unavailable GPU timestamps
cannot become invented zeros or excuse a missing CPU/interaction measurement.
Keep measurement helpers inside the existing runner/test instrumentation, not a
second benchmark service or permanent telemetry subsystem.

Specify each seeded workload completely before calling it reproducible: generator
revision, PRNG/integer arithmetic, entity ordering/counts, geometry/style formulas,
asset/font identities and viewport/camera/DPR. Pin a generated-scene digest or
independently checked totals. Real CAD drawings are separate matched baselines;
they do not silently change a synthetic scene's count. Distinguish one lifecycle
cycle per sample from an inner loop of 200 cycles, and identify which cold/warm
variants each metric measures. Define p95 calculation and retain all raw samples;
ratio comparisons require the same correct scene and an explicit nonzero baseline.

### Validation and plan maintenance

Measure interaction as GUI-event entry to frame submission unless a separately
qualified presentation instrument is used; do not label it photon/display latency.
GPU timestamps may be unavailable or incomparable across APIs: mark unavailable
values with a reason and use matched-backend/hardware samples, not invented zeros.
Raster thresholds must protect topology and interior coverage, not hide missing
thin entities in a high whole-image similarity score. Test DPR 1/1.25/1.5/2,
near-zero segments, sharp joins, dash seams, mirrored/skewed images/blocks, and
small detail near large world coordinates. Verify the tessellation bound for each
curve family rather than relying only on a midpoint or sparse sampling oracle.

For error paths use deterministic injected allocation/device-loss statuses plus
real close/recreate tests. Do not force driver resets or exhaust a shared runner's
GPU memory. An injected-failure pass validates the recovery path, not a claim of
observed physical device loss. Resource budgets/limits produce explicit diagnostics;
they cannot silently omit drawable content or downgrade a requested backend.

Lint changed YAML with `actionlint`, shell with `bash -n`/appropriate shell checks,
PowerShell/batch with their native tooling and execution tests, and new helpers
with failure fixtures. Parse Snap/build manifests and run actual CMake configure
matrices; text checks alone cannot prove that a forced-OFF option remains OFF.
Run sanitizers/leak checks on supported builds and validation/debug layers in the
qualified development matrix; do not ship development layers as required runtime.

Before accepting an implementation batch, run the exact configuration's suite
manifest through the actual binary, compare discovered/executed IDs and reject
skips; lint/contract fixtures cannot replace that step. Every R-stage receipt names
its affected F-groups, changed paths, effective options, provider/run/package IDs,
remaining decisions and the next runnable command. A local documentation check
never advances R0–R7 to implemented or validated.

Update this roadmap in the same change as phase state, APIs/options, CI filters,
providers, packages, support decisions or external results. Keep a compact receipt
with exact source/provider/artifact identity, command/environment, selected/executed
counts, native/emulated/software/hardware classification, result and next gate.
Mark old evidence stale when its inputs change; preserve historical receipts.
CI validates and reports but never auto-edits semantic state or commits the plan.

For a hosted result, first bind exact repository, event/workflow, full SHA, run
attempt and every expected matrix tuple; then read payloads and separate ordinary
results from deliberately failing negative probes. Record whether a packet has
full payloads or only artifact metadata, its retention deadline/location, and
which identities were actually revalidated. A green job/aggregate or artifact
name is not evidence of complete, current payload contents. Add a new receipt
when a run finishes; don't keep yesterday's queued state as the current blocker.

The next read-only resume checks are:

```sh
gh run view 40076688956 --repo dxli/LibreCAD_3 --json status,conclusion,headSha,url
gh api 'repos/dxli/LibreCAD_3/actions/runs/40076688956/jobs?per_page=100'
gh api 'repos/dxli/LibreCAD_3/actions/runs/40076688956/artifacts?per_page=100'
```

For this run, require exactly the 60 source tuples listed in R1 plus protocol and
aggregate; follow pagination rather than treating a truncated API page as the
whole run. Resume from the current checkpoint, not an older queued-run note.
There is no persistent agent repair automation. The existing authorized R1 CI
workflow is not permission for a scheduled agent to edit/push/cancel indefinitely;
an implementation session still obeys its current scope and pending owner choices.

Use readiness as a checklist, not a second machine-readable state enum. A work
package is ready only with prerequisite decisions, owner/reviewer, inputs, access
and exact acceptance commands; implemented only with code and negative tests;
accepted only with required execution and reviewed retained evidence. Preserve the
existing stage/gate states and their checker. A missing native session stays open,
not a passing skip. Keep failure-proof and successful-rendering results separate;
a documentation-only review cannot advance a work package.

Refresh in this order: reconcile actual source/provider inputs → select required
cases/configurations → execute and retain evidence → review outputs → update the
checkpoint/receipt and next gate. A documentation-only edit does not invalidate
compiled inputs; changes to shaders/fixtures/compiled sources, helpers, providers
or packages invalidate their corresponding evidence. Preserve earlier receipts
as history, never combine counts from incompatible sources/fixtures to close a gate.
Use the existing plan checker plus renderer helper regressions; extend those
checks with new schema/test-selection failures instead of adding a new plan engine.

Use the existing Python plan's mapping/receipts for overlapping implementation
paths. Keep this renderer plan registered in that CI map; preserve the separate
SMLib roadmap and unrelated worktree files. Before handoff, check every painter
method, feature ID, script/job, architecture and package row against tests and
actual evidence. No required row can disappear simply because a runner is missing.

Require one reviewed trace from every feature/consumer requirement to its case,
native producer, selected tuple and retained result. M2 validates data, M3 validates
selection, R1–R4 execute features and R5 binds final artifacts; none can self-certify
the next layer. Invalidate only the affected identities: code/shaders/compiled
fixtures need rebuilds, helpers need new runner evidence, providers need their own
qualification and changed final payloads need package retests. Preserve a valid
unaffected cohort without relabeling stale cohorts as current. Do not add another
ledger or automate acceptance decisions to implement this traceability.

Current CPU-only review commands are runnable without a GPU or graphical session:

```sh
python3 -B scripts/validate_python_binding_plan.py --full
python3 -B -m unittest discover -s scripts/tests
actionlint .github/workflows/renderer.yml
actionlint .github/workflows/renderer-source.yml
git diff --check
```

Also run the existing complete-worktree checker against the actual base, citing
every explicit unrelated-path exclusion in the newest Python receipt. Inspect
untracked renderer files for whitespace/conflict markers separately because
`git diff --check` covers tracked changes only. After replacing the draft receipt
with actual validation, repeat these checks and the path/history checks. The plan
checker validates structure/synchronization, not QRhi behavior, D1–D6 acceptance,
or R0–R7 completion. Future manifest checks complement, not replace, manual review
of required contracts and actual native/package evidence.

## 9. Direct Vulkan alternative if required by R0

The feature, CLI, scripting, package and evidence gates above still apply. Replace
the QRhi target with a private native Vulkan target and qualify the Qt integration
before choosing `QVulkanWindow`/native-window embedding or another composition path.
Prove MDI stacking, focus, menus, overlays and screenshots; window embedding has
documented limitations and is not a drop-in QWidget replacement.

Add explicit instance/device/queue selection, surface/swapchain recreation, render
passes/pipelines/descriptors, shader compilation to SPIR-V, staging/allocation
(prefer a pinned established allocator), barriers/fences/frame resources, deferred
destruction, pipeline-cache compatibility and device-loss recovery. Choose the
minimum capability profile from required hardware; do not require the latest
Vulkan version or optional geometry/wide-line features just for 2D strokes.

For macOS add pinned native MoltenVK artifacts for arm64/x86_64, portability
enumeration/subset handling, loader/ICD or direct-link policy, shader-conversion and
feature checks, bundle relocation/rpaths, dependency notices and signing. Add these
to all relevant source/install/package CI paths; a Metal-only test cannot validate
this alternative's Vulkan claims. Do not build the entire Vulkan SDK in each app
job; consume verified prebuilt tools/libraries or a separate pinned provider build.

Do not schedule from the earlier 12–20 engineer-week RHI or 20–32 direct-Vulkan
figures; neither covers the now-explicit missing painter/provider/qualification
work with a task-based estimate. Before R1, estimate the bounded subpatches above,
state assumptions and uncertainty, distinguish engineering effort from runner/
approval lead time, and identify the dependency-critical path. Re-estimate after
D1–D6 close; no delivery date is implied by the current prototype.

## 10. References and review history

The platform choice follows [QRhiWidget](https://doc.qt.io/qt-6.8/qrhiwidget.html).
Its underlying [QRhi API](https://doc.qt.io/qt-6.8/qrhi.html) requires version-specific
qualification; [qsb](https://doc.qt.io/qt-6.8/qtshadertools-qsb.html) supplies shader
packaging. [Mesa](https://docs.mesa3d.org/sourcetree.html) documents Lavapipe's software
Vulkan role. For the alternative, consult
[QVulkanWindow](https://doc.qt.io/qt-6/qvulkanwindow.html),
[window-container limits](https://doc.qt.io/qt-6/qwidget.html#createWindowContainer),
[MoltenVK integration](https://github.com/KhronosGroup/MoltenVK), and
[Vulkan synchronization](https://docs.vulkan.org/spec/latest/chapters/synchronization.html).

Frame/readback and metadata details additionally follow
[QRhi command buffers](https://doc.qt.io/qt-6.8/qrhicommandbuffer.html),
[resource lifetime](https://doc.qt.io/qt-6.8/qrhiresource.html),
[readback results](https://doc.qt.io/qt-6.8/qrhireadbackresult.html), and
[driver information](https://doc.qt.io/qt-6.8/qrhidriverinfo.html).
For exact teardown ordering use the pinned
[Qt 6.8.3 QRhi implementation](https://github.com/qt/qtbase/blob/v6.8.3/src/gui/rhi/qrhi.cpp#L7753),
not a cleanup-callback notification as an inferred completion barrier.
The Linux supervisor follows [subreaper adoption and wait semantics](https://man7.org/linux/man-pages/man2/PR_SET_CHILD_SUBREAPER.2const.html)
and Python's [pidfd interface](https://docs.python.org/3/library/signal.html#signal.pidfd_send_signal).
Native fixtures qualify those operations on the actual kernel/Python provider;
they do not add a process supervisor to the production CAD application.
The [Weston backend and renderer model](https://wayland.pages.freedesktop.org/weston/toc/running-weston.html)
distinguishes nested X11, headless and standalone DRM/KMS sessions. The retained
14.0.2 binary/help and runtime logs establish the exact tested version's flags;
the moving documentation is not a version-14 provider pin.

| Date | State | Scope and next action |
|---|---|---|
| 2026-10-24 | Wayland popup race fixed; full Vulkan and repeated native proof pass | Codex implementation/self-review: final clean source965e4465b7a55c85f433499bf3344508fe75357b passes all60 existing Vulkan tuples, per QPA24 positives/1384 checks and six controlled failures/18 assertions. Fifty consecutive unretried Wayland DPR1.25 lifecycle cases each pass34 original checks; traces show held F12 before popup grab, no early release/popup_done. Zero failed/skipped ordinary checks or core/synchronization errors; complete owned cleanup. Readiness plus one native keydown and exposure-only acknowledgment fix the unchanged-source reproduced release-before-grab race. Key release/app reaping cover success, timeout and tool failures; new artifact remains limited to input sessions. Six added regressions; final Linux622/622, Mac611 plus11 Linux-only skips pass. Keep initial missing-offline-fixture regression failure, original popup failure, first-fix and final proofs separate. No provider/case/assertion/deadline relaxation, dependency build, R2/default/release change. Evidence: build/renderer-evidence/r1-popup-fix-20261024.fDteaO/. All110 retained reports independently revalidate PNG/RGBA pixels, exact IDs, native app/report/helper hashes, core/synchronization logs and owned cleanup; all50 native traces verify press/grab ordering and successful key release. Full-plan, checker42/42, complete-worktree mapping116 paths/16 gates/no stale or unmapped evidence, 190-configuration inventory, both workflow lints, git diff --check, 50 local links, all prior receipts/gate states and1076 unrelated files pass preservation review. Repeat these checks on this finalized receipt before documentation-only closeout commit and authorized push. Preserve prior43 renderer/357 Python receipts, original checkout/index/staging and unrelated files. New hosted cohort is pending, not old d4 success rebased onto new code. Independent/physical-hardware/production/widget/provider/deployment/package and R0/R1/D1–D6 gates stay open. |
| 2026-10-24 | Wayland serial-race fix; fresh validation pending | Codex implementation/self-review reproduces the unchanged DPR1.25 popup failure with protocol tracing: native release precedes the client's popup grab, followed by popup_done before exposure. Add focused-widget readiness, one held native keydown, acknowledgment only after the existing visible/active/exposed predicate, and key release plus app cleanup on all paths. Keep every existing assertion, required tuple, provider pin and deadline. Six new CPU regressions cover readiness/ack separation, delayed acknowledgment, app failure, partial keydown failure, timeout and failed release cleanup. Fresh clean-source full60 Vulkan cohort, repeated fractional-DPR proof, regression/plan/inventory/workflow/preservation validation are pending; no passing claim or publication yet. Evidence: build/renderer-evidence/r1-popup-fix-20261024.fDteaO/. Preserve old failures and all42/356 prior receipts; source-CI success below remains bound to d4e9650e. Hardware/widget/production/phase/package and independent acceptance stay open. |
| 2026-10-24 | Committed-source Vulkan proof passes; intermittent Wayland popup remains open | Commit 2b876010ea0e76e02afa566caf0810f900257ec8 records the R1 closeout plans locally on codex/r1-platform-ci without touching the original checkout/index. Its clean native-ISA Linux ARM64 snapshot runs actual Vulkan/Lavapipe with Qt/private/qsb 6.10.2, Widgets-ON and Gui-only builds. Fresh cohort passes all 60 existing XCB/Wayland tuples: per QPA 24 positives / 1,384 checks and six separate controlled failures / 18 assertions, zero skipped/ordinary failed cases or Vulkan core/synchronization errors, complete owned cleanup. This is standalone feasibility rendering, not the application's legacy viewport, hardware/performance or full painter acceptance. Retain the first missing-private-header configure, the first cohort's DPR1.25 Wayland popup-exposure timeout, unchanged passing diagnostic, fresh complete cohort and corrected post-run summary readback separately. No rendering/test/helper/provider pin or budget change; only local Make/private binary paths differ from hosted setup. Evidence: build/renderer-evidence/r1-vulkan-20261024.L6azJJ/. Validation passes: all 60 retained reports independently revalidated, including PNG/RGBA pixels, exact check IDs, committed source/helper/app hashes, validation logs and owned cleanup; full-plan, checker 42/42, complete-worktree mapping (116 paths / 16 mapped gates / four cited exclusion rules / no stale or unmapped evidence), 190-configuration inventory, both workflow lints, git diff --check, 50 local links and prior-receipt/source/index preservation. All checks repeat on this final receipt. Prior 41 renderer/355 Python receipts remain unchanged. Known popup flakiness, independent review/durable retention, full providers/deployment/hardware/widget/package and R0/R1/D1–D6 gates remain open; no automatic R2, source fix, merge, push, release/default change. |
| 2026-10-24 | All-platform R1 source CI green; independent/phase acceptance open | Codex implementation/self-review: d4e9650e5dd03b8396923e7001dbee1d7c0bd3fb / run 40076688956 attempt 1 passes all 60 source tuples plus protocol/aggregate, 62/62 jobs. Every platform passes 12/12; all 60 artifact ZIPs are retained/reviewed with exact source/helper/profile/XML/native-device/graph/cleanup binding. Supplementary readback checks 98 ordinary passing XML reports, 52 separate expected-negative reports and all 30 pinned source-submodule SDK records. Hosted regressions pass 616/616 without skips. Both Intel library suites execute all 22 RHI IDs with independent native/display registry-ID agreement; Intel UI and combined hosts execute 197 non-widget IDs each, retain 97 unexecuted widget IDs and all 75 UI units with one generator owner. Original caps, legacy defaults, submodule pin/patches and all hardware configurations remain unchanged. Historical failures and prior 40 renderer/354 Python receipts remain preserved; the original checkout/index/staging and 1,076 unrelated baseline files remain unchanged. These are local post-dispatch documentation receipts, not new tested code or an external durable backup. Evidence: build/renderer-evidence/r1-ui-partition-20261024.kJGirf/hosted/ and closeout.Y6aatK/. Validation: full-plan, checker 42/42, complete worktree against HEAD (116 included paths, 16 mapped gates, 72 exclusions under the four cited rules, no unmapped paths or stale evidence), inventory (56 cases / 223 variants / 60 signatures / 75 consumers / 190 configurations), both workflow actionlint, git diff --check, 50 local links, whitespace/conflict and receipt/source/index/submodule preservation checks pass. Repeat all checks on this final receipt. Independent review/durable retention, R0/R1/D1–D6, hardware/session/deployment/provider/license/package/signing/release gates remain open. No R2, merge, new PR or release/default change. |
| 2026-10-24 | Three-group UI candidate published; full replacement cohort pending | d4e9650e5dd03b8396923e7001dbee1d7c0bd3fb / run 40076688956 contains nine changed paths over ded3b55a, with 116 intended candidate paths and unchanged original checkout/index/staging. Source matrix remains all 60 tuples plus protocol/aggregate; all 22 RHI IDs, 190 configurations and other cases remain intact. Local Linux scripts 616/616, Mac 605 plus 11 Linux-only skips, focused120/120, checker42/42; real matched Qt6.12 Mac host executes197 non-widget IDs and proves the exact75-unit three-group union, while fresh Qt6.8.3 RHI22 and CPU92 plus Intel syntax checks pass. The local pinned UI conflict is retained as a provider limitation, not hidden or claimed qualified. Superseded ded3 run is completed/cancelled with no active jobs; its both-Intel virtual successes and ordinary-command failure remain archived. Evidence at build/renderer-evidence/r1-ui-partition-20261024.kJGirf/index.json. This post-dispatch documentation receipt changes no compiled input; repeat plan/checker/worktree/lint/history validation, then observe and retain the full exact-SHA cohort without weakening budgets/tests or closing hardware/phase/package gates. |
| 2026-10-24 | Intel virtual Metal verified; bounded UI command follow-up active | On ded3b55a / run 40071386912, both native Intel library/tests-ON jobs pass all 22 RHI IDs with no skips and exact selected-device AppleParavirtAccelerator registry attestation; core registry ID 4294968061 independently matches system_profiler 1000002fd. A separate combined-host/tests-ON job 120805379513 reaches 93/98 ordinary UI edges, then its 300-second command expires with five units left (two dialogs, two managers, resource). No compiler error; owned command group retires correctly. Preserve the failure artifact 12552112799 and successful Metal proof under build/renderer-evidence/r1-virtual-metal-20261024.AirkWi/hosted/. Codex follow-up: partition the existing ordinary sources into root and widget object groups, with one MOC/UIC/resource owner and exact shared consumer union, keeping 300/1200/60-minute limits, all tests, flags and behavior. Also release the native IOKit reference before string allocation and reject contradictory paravirtual/native hardware typing. Validation: Linux scripts 616/616, Mac 605 passed plus 11 Linux-only skips, focused build/source 120/120. Fresh actual Mac ARM64 Qt6.12 combined host builds and executes 197 non-widget IDs, zero skips; 97 widget IDs are compiled/discovered, not executed. File API proves root28/widgets44/bindings3 is exactly the prior 75-unit union, unique per consumer, C++17 and one MOC/resource owner. Tiny real CMake builds cover Python/tests ON/OFF, inherited usage and generated-header dependency. Fresh Qt6.8.3 RHI executes 22/22 plus CPU92/92; updated Intel-only code syntax passes. The initial local Qt6.8.3 UI attempt fails before the new widget stage on conflicting existing Homebrew QtCore headers; it is retained and not minimum-UI qualification. No global SDK is changed; hosted CI keeps the pinned clean provider. Final plan/checker/worktree/inventory/lint/history checks repeat before publication. Evidence: build/renderer-evidence/r1-ui-partition-20261024.kJGirf/. Next publish a fresh complete cohort. Hardware/phase/package gates remain open. |
| 2026-10-24 | Verified virtual-Metal R1 candidate published; full cohort pending | Published ded3b55ab23a7fb13894eb61a25490b1c812f084 to the authorized dxli/LibreCAD_3:codex/r1-platform-ci branch; run 40071386912 dispatches all 60 source jobs plus protocol/aggregate. Separate-index publication contains 116 intended paths / 14 changed paths; all 22 GTest IDs, 189 pre-existing configurations and unrelated cases are byte/structure-checked unchanged. Source archive/publication/local proof retained at build/renderer-evidence/r1-virtual-metal-20261024.AirkWi/index.json. Local Linux scripts 615/615, Mac 604 plus 11 Linux-only skips, final focus 154/154, checker 42/42, pinned Qt6.8.3 native ARM64 RHI 22/22 + CPU 92/92 and Intel syntax-only check pass; original branch/HEAD/index/staged files and unrelated paths remain untouched. This post-dispatch documentation receipt changes no compiled input. Plan/checker/worktree/lint/history checks repeat before continuation. Next observe both Intel native registry attestations and the entire exact-SHA cohort, retain/review all required payloads; no CI-success or hardware/phase/package acceptance is claimed yet. |
| 2026-10-24 | Authorized verified Intel virtual-Metal source CI; implementation active | Owner explicitly permits only verified Intel virtual Metal for source CI, keeping hardware acceptance gates open. Codex owns implementation/self-review; independent acceptance remains pending. Implement a native selected-device registry identity check, an R1-only manifest tuple and strict runner/XML attestation, preserving all 22 rendering/shader/factory IDs, unknown/mismatch rejection, unchanged budgets and all existing hardware tuples. Local validation: Linux ARM64 scripts 615/615; Mac ARM64 scripts 604 passed plus 11 explicitly Linux-only skips; focused final runner/case/workflow checks 154/154 (broader first successful focus 260/260). Real pinned Qt 6.8.3 Mac ARM64 executes all 22 RHI IDs and 92 CPU IDs, no skips; Intel-only IOKit/Objective-C scalar-call path cross-compiles with actual pinned-Qt build flags, not native Intel execution. The initial local command compiled 22 passing tests against unintended Homebrew Qt 6.12, and the provider audit correctly rejected that run; the explicit Qt6_DIR/CMAKE_PREFIX_PATH rerun passes. Wrong-scope virtual CLI exits 2. Full-plan, checker/worktree/inventory/actionlint/diff checks are repeated after this receipt. Manifest has 190 configurations (one additive virtual tuple), 56 cases, 223 variants, 60 signatures and 75 consumers; all 189 prior configurations, other cases and 22 original GTest IDs are retained. Evidence: build/renderer-evidence/r1-virtual-metal-20261024.AirkWi/. Prior 36 renderer and 350 Python receipts and original checkout/index/unrelated paths are preserved. Next publish the bounded R1 candidate and require a fresh complete five-platform cohort. No R2/default/release or hardware acceptance is authorized. |
| 2026-10-24 | Deep readiness review; R1 source CI 58/60, device-policy decision open | Documentation-only reconciliation of cb91cf0160ac5e0c4d6e3c16ca98c8a058f79b72 / run 40021957419: four platforms pass 12/12; Intel passes 10/12. Both Intel library/tests-ON artifacts identify Apple's paravirtualized Metal device and execute 22 RHI IDs with eight passes, 14 device-setup failures, zero skips. Windows core/tests-ON executes 22/22; Intel UI/tests-ON executes 197/197 non-widget IDs. All 60 tuple/artifact names verified; four selected payloads retained at build/renderer-evidence/r1-plan-review-20261024.P1Sh1I/index.json, remaining 56 metadata-only in this packet. Corrected 60 jobs versus 50 logical scopes, scope/option limits, R1 versus R3/R4 host gates, historical provider claims and aggregate/payload completeness gap. Added ordered owner-decision, conditional virtual/hardware routes, exact-cohort retention/review and bounded handoff sub-plans using existing helpers. No policy approval, runtime/CI/default edits, dispatch, push or gate promotion. Validation: Mac ARM64 scripts 611 run, 600 passed / 11 explicitly Linux-only skips; checker regressions 42/42. Full-plan, complete-worktree against HEAD (115 included paths, 16 mapped gates, 72 paths excluded by the four cited rules, no unmapped paths/stale validated evidence), renderer inventory (56 cases / 223 variants / 60 signatures / 75 consumers), both renderer workflow actionlint and git diff --check pass. The final receipt repeats full-plan/checker/worktree/inventory/lint/diff checks. Fifty local plan links and both plans' whitespace/conflict checks pass; prior 35 renderer and 349 Python receipts remain byte-for-byte/newest-first, all 1,082 other baseline files plus original HEAD/branch/index/staging unchanged. Next obtain maintainer source-CI device decision; keep R0/R1, D1–D6, hardware/widget/deployment/package/release acceptance open. |
| 2026-10-23 | R1 bounded CI groups, deadline arithmetic and early UI scheduling. | The two-object-group candidate `a9bb098f` reaches 45 passing source configurations in [run 40004590514](https://github.com/dxli/LibreCAD_3/actions/runs/40004590514), including Windows and Intel Mac combined hosts/tests-ON. Intel Mac UI-only/tests-ON still times out at 95/98 ordinary-group build edges: only dialogs/textdialog.cpp, managers/uisettings.cpp and managers/contextmenumanager.cpp remain unfinished. The follow-up schedules those three template-heavy files first within lcui_objects, before Python helper and ordinary widget units; Lua GUI, Python GUI (when enabled) and clipboard serialization stay in separately staged lcui_bindings_objects. A completed Linux x64 UI log separately shows 184 MB taking 41 minutes from the Azure mirror; only that runner's Azure mirror entry changes to the official Ubuntu HTTPS archive, retaining signed suites, exact pins, priorities and all other providers. A real sed fixture checks exact replacement and untouched ARM/vendor/lookalike URLs. Windows tests-OFF libraries pass in 52 minutes, while GitHub explicitly reports the tests-ON library job exceeded its 60-minute maximum. The complete library workload now has two required CI groups (core and Python), producing 60 source jobs covering the original 50 logical scopes; tests-OFF partitions all eight modes without overlap, tests-ON preserves every existing profile and repeats a fresh CPU baseline in Python jobs for exact ID parity. Both groups retain native CPU/RHI/QPA checks and all source/provider/artifact/cleanup validation. The standalone default remains the whole matrix. Group forwarding, exhaustive union/baseline/failure/unknown-group and no-recursive-worker regressions validate selection; progress names each starting profile. The grouped `22f1cb1e` candidate passes hosted Linux core/tests-ON in 14 minutes and cuts the x64 UI dependency restore from 41 minutes to 91 seconds. An early Windows CPU-preflight guard then rejects a deadline before any CMake command: same coarse ticks can round (212.007 + 300) - 212.007 to 300.00000000000006. CPU and RHI source steps now clamp the remaining timeout to the configured cap, matching existing host/library behavior, without resetting the original total deadline. Deterministic CPU/source fixtures prove both the exact cap under rounding and rejection after expiry; full scripts including additive diagnostics pass 611/611 on native Linux ARM64 and 600 plus eleven Linux-only skips on Mac, and the real Mac CPU suite passes three IDs with controlled rejections. Linux's initial full-run I/O stall is retained, not misreported as a failure. Intel Mac then builds every selected library/CPU profile but fails all 14 device-backed RHI tests because its created Metal adapter is classified unknown rather than the required hardware; all eight factory/shader/contract tests pass. Qt 6.8.3 maps Metal type only from device location. Tests now identify the adapter in the unchanged classification assertion, and CI records native Intel display/Metal metadata. No unknown/virtual adapter is promoted to hardware, no rendering test is skipped, and no virtual-CI tuple is accepted without owner approval. A separate verified virtual-Metal source-CI case versus a hardware-backed Intel runner is an explicit pending owner decision; physical hardware and all other acceptance gates remain open. No new CMake target, binding implementation, optimization level, test ID, cleanup exemption or budget expansion. Both groups retain PIC, explicit export ownership, C++17/Python usage and module-generation dependencies; only the ordinary group owns Qt code generation, and both application/test-library consumers reuse every object once. The real CMake-list regression checks early ordering and no duplicates; native Mac File API verifies 75 unique units with the three late units first. Full Linux/Mac suites and plan/inventory/workflow/preservation checks are rerun and retained with this candidate. Prior candidate proof has 604/604 scripts on Linux and 593 plus eleven native-Linux skips on Mac; Windows combined/tests-ON has 75 unique UI compilations, 197 executed non-widget IDs, 97 compiled/discovered but unexecuted widget IDs and both object groups hashed. Earlier Linux incremental proof compiles/hashes both groups; the missing pinned Qt runtime path attempt remains separately rejected. Prior fixes retain DLL-owned DocumentList/Lua custom-entity singletons, exact manifest-bound relocated installs, Windows CPython ABI, ID-bound Qt/VS utilities, verified pre-exec Mac ownership and four UI data import/export owners. libdxfrw remains the clean source-submodule revision f7debf4c5f30734a70c5e828cf05247aabc2d04b with all three patches; all other dependencies stay binary-only. The original checkout/index, 115 candidate paths and prior receipts are preserved. Excluded paths remain `.agnes/`, `SMLIB_2D_CAD_IMPLEMENTATION_PLAN.md`, `libdxfrw.cpp.o` and `t.diff`. Caps stay 300 seconds per command, 1,200 seconds per profile and 60 minutes per job. Legacy remains default; full source CI, independent acceptance and R0/R1/D1–D6/runtime/package/release gates remain open. Owner/self-review Codex; independent review pending. |
| 2026-10-23 | R1 source-submodule provider and CLI installs locally verified | Owner/self-review Codex; independent acceptance pending. User selected libdxfrw source submodule only: clean recorded f7debf4 plus required patch now builds into isolated PIC static SDKs on both ARM64 hosts, with source/patch/SDK/license/command identities retained and the original checkout unmodified. Both native CLI profiles pass the identical 197 non-widget IDs and relocated installs with seven project libraries actually loaded from the moved prefix; Mac Python/persistence/tests-OFF also passes with five libraries. Ten independent C++14 header objects pass. CMake now installs the CLI and relative Unix loader paths; staging omits Python development/configuration links and host customization. The archive prerequisite precedes binary snapshots/tests, and canonical loader paths remain contained. Initial native failures, helper-invalidated attempts, fixture/retainer corrections and the concurrent-load Linux compiler timeout remain retained separately; the quiet fresh retry passes unchanged 300-second command/1,200-second profile caps. Source regressions pass 59, provider fixtures five, full scripts Linux 582/582 and Mac 571 plus eleven native-Linux skips under 3.12/3.15; checker regressions pass 42, plan/worktree/inventory/actionlint/diff checks pass. Byte-verified proof and final audit: `build/renderer-evidence/r1-submodule-cliinstall-20261023.qUTDZv/index.json`. Audit preserves 1,070 other files and prior 33 renderer/347 Python receipts; only new submodule metadata is staged, with no commit/push/hosted execution or global install. CI declares 26 definitions: ten library, eight persistence-OFF CLI and eight explicit source-provider CLI. Missing native/hosted/full-option/UI/provider/toolchain/runtime/package, independent review and R0/R1/D1–D6/release gates remain open. Legacy/defaults and Python APIs remain unchanged; no R2 or desktop/input. |
| 2026-10-23 | Provisional R1 independent hosts and initial CLI CI locally verified; no acceptance change | Owner/self-review Codex; independent acceptance pending. Explicit UI-only/CLI-only/both and CLI-only persistence OFF now preserve host/dependency intent. Compiled GTest text/JSON discovery binds every ID to an owned fingerprinted source/header, including conditional/parameterized cases; all non-widget IDs execute and UI IDs remain unexecuted. Python-generation and nonempty CPU C++14 groups are checked. Five new regressions bring the focused source suite to 51 over 32 host combinations plus actual CI command/dependency branches. Fresh Mac CLI tests ON/OFF and Linux CLI/UI-only builds pass; both ARM64 CLI binaries execute identical 190 IDs, Linux UI executes 198 and discovers 97 unexecuted widget IDs. Read-only links/symbols retain legacy and keep CLI project binaries Qt/DXF-free; 15 C++14 header objects pass. CI defines eight binary-only Linux/Mac persistence-free CLI lanes alongside ten library lanes, with unique artifacts and executables/SDK inventories/qsb retained; no hosted run or Windows CLI claim. Full scripts pass Linux 568/568 and Mac 557 plus eleven native-Linux skips under 3.12/3.15; checker tests pass 42. Proof/final checks: `build/renderer-evidence/r1-hostkinds-20261023.olASyW/index.json`. Initial temporary retention-path failures and partial packets remain separate; corrected final archives byte-verify. Audit preserves 1,075 other files and prior 32 renderer/346 Python receipts. Next qualify hosted/missing native/full option/provider/toolchain/package and R0 cases, and resolve the requested patched SDK/producer choice for cold UI/persistence CI. Legacy/defaults, all 16 accumulated-work gates and R0/R1/D1–D6/release acceptance stay unchanged; no external dependency installation/source build, R2 or desktop/input. |
| 2026-10-23 | Provisional shared R1 host runner locally verified; cold provider gate open | Owner/self-review Codex; independent acceptance pending. The existing runner now checks explicit real UI/CLI profiles using shared option/target/artifact readers, owned shared UI objects, source/assets/SDK bytes and exact non-widget IDs including header registrations; widget suites remain compiled/discovered but unexecuted. Eight regressions cover all eight renderer/Python/test modes and the GUI-less Python copy utility; focused source tests pass 46. Fresh Linux dual/Python-ON/tests-ON passes 198 selected IDs with 97 widget IDs unexecuted; legacy/Python-OFF/tests-OFF and legacy/Python-ON/tests-OFF also pass. Current reader revalidates initial host source/SDK/graph/objects/artifacts/XML without rewriting original helper identities; the third build uses the final helper. Seven retained native library graphs per ARM64 host pass metadata compatibility only. Full scripts pass Linux 563/563 and Mac 552 plus eleven native-Linux skips under installed 3.12/3.15; checker tests pass 42. Proof/final checks: `build/renderer-evidence/r1-hostrunner-20261023.4ZxY5w/index.json`. Audit preserves 1,076 other files and prior 31 renderer/345 Python receipts. No Ubuntu/upstream release route is qualified; Conan stock recipes differ from the required pinned patch and Linux ARM64/GCC 15/shared 1.0.1 reports Missing despite exit zero/null graph error. The Conan check retrieved no dependency binaries; no dependency installation or external source build was performed. Next resolve the requested SDK/producer choice before cold host CI, then complete remaining host/provider/native/toolchain/package and R0 checks. Legacy/defaults, all 16 accumulated-work gates and R0/R1/D1–D6/release acceptance stay unchanged; no R2, hosted dispatch or desktop/input. |
| 2026-10-23 | Provisional R1 UI object sharing locally verified; no acceptance change | Owner/self-review Codex; independent acceptance pending. One private PIC UI target compiles Qt-generated/application code once, carries dependency/Python-generation requirements and feeds unchanged application/test-library links without empty consumer autogen targets. Four tests/Python CMake fixture combinations and 36 focused tests pass. Fresh Linux ARM64 UI/CLI with persistence/Cairo passes legacy/Python-OFF/tests-ON (71 UI units, shared objects, exactly 96 CPU/Cairo/DXF IDs) and dual/Python-ON/tests-OFF (75 UI units, private RHI library, GTest discovery forbidden). Both hosts retain the legacy factory; actual CPU/UI language groups remain C++14/17. Full scripts pass Linux 555/555 and Mac Python 3.12 with 544 passed plus eleven native-Linux skips; plan checker tests pass 42. Selected proof/final checks: `build/renderer-evidence/r1-uiobjects-20261023.NiZ6oP/index.json`. Interrupted autogen inspection, six-job compiler-killed attempt and wrong-working-directory script failure remain archived; all owned build children retired. Successful fresh three-job retry retains 300-second command/1,200-second profile limits. Audit preserves 1,076 other files and prior 30 renderer/344 Python receipts. Next full host/Python/provider and source-CI host integration, minimum/missing native, installed/package and required R0-case checks. Legacy/defaults, all 16 accumulated-work gates and R0/R1/D1–D6/release acceptance stay unchanged; no R2, external dependency installation/source build, hosted dispatch or desktop/input. |
| 2026-10-23 | Provisional R1 cleanup and Mac Python 3.12 controller locally verified; no acceptance change | Owner/self-review Codex; independent acceptance pending. Corrected TERM/KILL phase origins and bounded adoption without changing configured graces or command failure/timeout status. Added a Darwin LP64 non-reaping adapter for the existing Python 3.12 CI family; no interpreter-pin, Python runtime-implementation or default change. Seven new tests and adapted ownership fixture pass; full scripts pass Linux 554/554 and Mac 543 plus eleven native-Linux skips under installed 3.12/3.15. SDK ABI syntax/owned compiler checks pass ARM64/x86_64 without native Intel execution. Native labeled seven-second clock jumps reap both owned processes per case and still reject timeout. Fresh source matrices pass eight modes per ARM64 host: all 92 CPU IDs per Python-OFF mode, identical 190 per ON mode and 40 C++14 header objects each. Mac CPython 3.12.15 and Linux 3.14.4 provider fingerprints agree within all four ON modes. Metal and Vulkan XCB/private headless Wayland each pass 22 RHI IDs; Linux per-QPA validation has 14 instances/devices, zero errors and complete owned cleanup. CPU contracts pass three IDs/four controlled rejections per host; Mac 3.12 command success/rejection/timeout probes preserve status. Selected proof/final checks: `build/renderer-evidence/r1-cleanup-20261022.6a9toC/index.json`. Audit preserves 1,075 other files and prior 29 renderer/343 Python receipts. Earlier failures and the PID-1-owned cmake zombie remain archived/untouched; all 328 old display identities remain. Next R1 host/provider/persistence/Cairo, minimum/missing native and required R0-case refreshes; legacy/defaults, all 16 accumulated-work gates and R0/R1/D1–D6/release acceptance stay unchanged. No R2 work, external dependency installation/build, hosted dispatch, container restart or desktop/input. |
| 2026-10-22 | Provisional R1 tests-OFF matrix locally verified; Linux tests-ON acceptance open | Owner/self-review Codex; independent acceptance pending. Eight compile-only GUI-less renderer/Python modes reject GTest/project tests and verify owned outputs/source/SDK hashes. Both ARM64 hosts pass eight tests-OFF modes/44 library targets each. Mac also passes eight fresh tests-ON modes, 92 CPU IDs per Python-OFF mode/190 per ON mode, 40 header consumers and 22 Metal IDs. CPU contracts pass three IDs/four controlled rejections per host; full scripts pass Linux 547/547 and Mac 537 plus ten native-Linux skips; focused source/Conan checks pass 55. Ten native source CI definitions and eight available locked Windows OFF metadata routes are wired, not hosted/native Windows qualification. Initial Linux OFF/ON deadline failures remain unchanged; OFF cleanup completed, ON left one recorded PID-1-owned cmake zombie. Synthetic clock-jump reproduction confirms deadline-before-signal ordering; next fix that boundary and repeat full Linux tests ON. Selected proof/final checks: `build/renderer-evidence/r1-production-20261022.rWfCAL/index.json`. Audit preserves 1,073 other files and prior 28 renderer/342 Python receipts. Legacy/defaults, all 16 accumulated-work gates, canonical host/provider/persistence/Cairo, minimum/missing native, R0/R1 and D1–D6/release acceptance stay unchanged. No R2 work, external dependency installation/build, hosted dispatch, container restart or desktop/input. |
| 2026-10-22 | Provisional R1 CPU public-header guards locally verified; no acceptance change | Owner/self-review Codex; independent acceptance pending. Five independently generated CPU-facing consumers enforce exact C++14 and reject graphics/Python SDK leakage; the test-only object target is not installed or linked into applications. Source runner verifies actual Release target/language groups, configured bytes, source hashes and five owned nonempty objects, freezing six header/template inputs across the whole run. Current SDK inspection corrected the GL_VERSION guard; earlier changing-input runs were rejected, not counted as passing. Each ARM64 host passes eight fresh modes/40 compiled consumers, unchanged 92 CPU IDs per OFF mode and 190 per ON mode, plus a positive syntax probe/13 expected native compiler rejections including actual OpenGL/Python headers. Current-helper Metal and Vulkan XCB/private headless Wayland each pass 22 RHI IDs; CPU contracts pass three IDs/four controlled rejections per host. Linux per-QPA validation records 14 instances/devices, zero errors and complete cleanup with all 328 old display identities preserved. Five new tests; focused 45 and full Linux 537/537/Mac 527 plus ten native-Linux skips pass. Selected proof/final checks are indexed above; audit preserves 1,073 other files and prior 27 renderer/341 Python receipts. Legacy/defaults unchanged; no external dependency installation/build, hosted dispatch, desktop/input, API/runtime or R2 change. Next canonical hosts, persistence/Cairo/test-OFF, full provider/runtime/package, minimum/missing native and cold Windows/MSVC qualification. R0/R1 and D1–D6/release gates remain open. |
| 2026-10-22 | Provisional R1.3/R1.4 Python library matrix locally verified; no acceptance change | Owner/self-review Codex; independent acceptance pending. The shared matrix now covers all four renderer modes with Python OFF/ON. Required target/test-source membership, full CPU ID retention, identical Python suites and selected interpreter/header/embed-library/ABI receipts fail closed. Each ARM64 host passes eight fresh builds: 92 CPU IDs per OFF mode and identical 190 per ON mode, including 98 scripting/runtime IDs and poisoned/fresh-process probes. Mac CPython 3.14.8/Linux 3.14.4 fingerprints agree across their four ON profiles; CPU/binding links remain Qt/GL-free with explicit legacy factory ownership. Current-helper Metal and Vulkan XCB/private headless Wayland each pass 22 RHI IDs; CPU contracts pass three IDs/four controlled rejections per host. Linux per-QPA validation records 14 instances/devices, zero errors, complete cleanup and all 328 old display identities preserved. Nine Windows metadata-only routes validate available locked binaries without project package retrieval/install/build. Linux CI now pairs its system interpreter with binary dev headers, avoiding an absent 3.12.10 Ubuntu 26.04 release artifact; Mac/Windows keep that pinned SDK setup. Job-default shell selection passes workflow lint. Five new tests; focused 40 and full Linux 532/532/Mac 522 plus ten native-Linux skips pass. Selected proof/final checks are indexed above; audit preserves 1,072 other files and prior 26 renderer/340 Python receipts. No external dependency installation/build, hosted dispatch, desktop/input, Python API/runtime or default change. Next canonical host/scripted, persistence/Cairo/test-OFF, full provider/runtime/package, minimum/missing native and cold Windows toolchain/MSVC qualification. R0/R1 and D1–D6/release gates remain open; R2 is not authorized. |
| 2026-10-22 | Provisional R1.3/R1.4 library-option matrix locally verified; no acceptance change | Owner/self-review Codex; independent acceptance pending. Shared source runner and five native CI definitions add fresh CPU/legacy/dual library builds before existing RHI-only execution, once per architecture. Effective flags, actual File API factory ownership, discovery and complete CPU ID parity fail closed. Mac/Linux ARM64 pass the same 92 CPU IDs in all four modes; link/symbol checks keep CPU Qt/GL-free and legacy factory ownership explicit. Pinned Mac Qt 6.8.3/Metal and Linux Qt 6.10.2/Vulkan XCB/private headless Wayland each pass 22 RHI IDs; native CPU contracts pass three IDs/four controlled rejections per host. Per Linux QPA validation records 14 instances/devices, zero errors and complete cleanup with all 328 old display identities preserved. All five Windows metadata-only routes validate available host binaries, including 14 each legacy/dual; identical unused skipped-tool duplicates are allowed, conflicting/host duplicates and CPU OpenGL/GLU rejected. Eleven new script tests; focused 35 pass, full Linux 527/527 and Mac 517 plus ten native-Linux skips. Selected proof/final checks are indexed above; audit preserves 1,071 other files and prior 25 renderer/339 Python receipts. No project dependency install/source build, hosted dispatch or desktop/input. GUI-less Python-OFF library builds do not replace canonical host/scripted/widget or dual GPU acceptance. Next full host/Python/provider/runtime/package, native cold Windows/toolchain/MSVC, minimum/missing architectures and painter/consumer qualification; preserve legacy defaults, R0/R1 and D1–D6/release gates. |
| 2026-10-22 | Provisional R1.4 Windows binary-route metadata locally verified; native acceptance open | Owner/self-review Codex; independent acceptance pending. Byte-verified original recipe reproduces missing GTest 1.14 in ordinary/RHI graphs despite CLI exit zero and null graph error. Pinned test-only GTest 1.18 resolves with required GMock/main; all current ordinary/empty/RHI routes validate seven/six/seven available host binaries and generated recipe locks. Source CI inspects every graph before binary-only installation, locks that install and validates the same post-install binary identities. Existing Windows packaging uses dependency-specific C++17 for GTest, preserving the consumer C++14 setting in real metadata and leaving application standards/defaults unchanged. Disposable Conan 2.33 CLI tool only; no project binary retrieval/install, dependency build, native C++/GPU execution, CI dispatch or desktop/input. Thirteen graph plus two recipe/workflow tests added; focused 59 pass. Full scripts pass Linux 516/516, Mac 506 plus ten native-Linux skips. Selected proof/final checks are indexed above; audit preserves 1,069 other files and prior 24 renderer/338 Python receipts. Earlier CI-source snapshots remain historical. Next native Windows cold-install/toolchain/MSVC/required-test/runtime closure; full option/host/provider/package, missing native/minimum and R0/R1/D1–D6/release gates remain open. |
| 2026-10-22 | Provisional R1.4 initial native source-CI wiring locally verified; hosted and R0/R1 unaccepted | Owner/self-review Codex; independent acceptance pending. Root renderer workflow calls same-commit reusable source jobs for five native tuples. Shared runner builds fresh GUI-less RHI/CPU/header targets using installed providers, executes every CPU ID, then retains exact existing RHI bindings; recursive Linux worker builds and unsupported profile substitution are rejected. Binary-only apt/Brew/Conan setup, matched Windows per-case toolchains and failure artifact retention are wired, not installed/dispatched locally. Reused selected-SDK AGL-stub check for R0 and experimental Metal-only hosts; legacy/default links and Qt binaries remain unchanged. Mac ARM64 pinned Qt 6.8.3/Metal and Linux ARM64 Qt 6.10.2/Vulkan XCB/private headless Wayland each build fresh and pass 92 CPU/22 RHI IDs. Current-helper CPU contracts pass three IDs/four native corruptions per host. Linux validation observes 14 instances/devices and zero errors per QPA; all owned cleanup passes and all 328 old defunct identities remain. Ten new protocol fixtures plus final full scripts pass Linux 501/501, Mac 491 plus ten native-Linux skips. Both workflows' remote actions remain SHA-pinned; only the exact same-commit local workflow is exempt from a remote-action SHA suffix. Selected proof/final checks are indexed above; preservation audit retains 1,067 other files and prior 23 renderer/337 Python receipts. Old source/helper-bound evidence stays historical. Full option/host/scripting/provider/runtime/package matrices, actual minimum CMake, hosted/other native platforms, real Conan/MSVC, painter/consumers, R0/R1 and D1–D6/release acceptance remain open. No dependency installation/source build, CI dispatch, desktop/input or default/acceptance change. |
| 2026-10-22 | Provisional R1.4 CPU/build bindings locally verified; R0/R1 unaccepted | Owner/self-review Codex; independent acceptance pending. Shared runner prepares real fresh configurations and a separate Qt-free native GTest inspects actual preflight values, traces, File API graphs and stable host rejection. Both ARM64 hosts pass the exact three existing CPU IDs and four controlled native corruptions per JSON/human route; Mac also uses real Ninja multi-config. Current/minimum GTest target-name protocols pass, not an actual CMake 3.15 run. Native Mac/Linux success/exit-1/timeout and Linux escaped-child cleanup pass; Windows suspended-Job ordering/failure/timeout are mocks only. Eighteen new CPU fixtures are wired into CI. Refreshed Metal and Vulkan/XCB/private headless Wayland each pass exact 22 RHI IDs; both CPU suites pass 92/92. Per Linux QPA validation observes 14 instances/devices, zero errors and complete owned cleanup, preserving all 328 old defunct identities. Full scripts pass Linux 491/491, Mac 481 plus ten native-Linux skips. Selected proof/final checks are indexed above; source/history audit preserves 1,065 other files and prior 22 renderer/336 Python receipts. Earlier helper/source-bound packets stay historical. Legacy remains default; no dependency install/source build, native CI dispatch, desktop/input, default, package or platform acceptance change. Next native prebuilt-provider source jobs, minimum/other architectures and real Conan/MSVC; preserve full provider/painter/consumer, R0/R1 and D1–D6/release gates. |
| 2026-10-22 | Provisional R1.4 shared-runner integration locally verified; R0/R1 unaccepted | Owner/self-review Codex; independent acceptance pending. Added manifest-driven source/offscreen execution to the existing runner, embedded selected-source/revision/dirty/options/provider receipts and actual Qt image hashes. Native Mac Metal and Linux Vulkan/XCB/private headless Wayland each select all three required cases and pass exactly 22 discovered/executed IDs; existing CPU suites pass 92/92 each. Per Linux QPA core/sync validation observes 14 instances/devices, zero errors and complete owned cleanup, preserving 328 old defunct identities. Twenty-nine rejection fixtures are wired into CI; full scripts pass Linux 473/473 and Mac 463 plus ten native-Linux skips. Non-RHI discovery and denied native-query failures are retained as rejections, not rendered success. Plan/inventory/worktree, renderer/plan workflow lint, tracked diff and source/history checks pass. Preserved 1,063 other files and prior 21 renderer/335 Python receipts; selected proof and verified archives are indexed above. Older helper-bound R0 receipts stay historical. No native source-CI dispatch/build-job completion, external dependency installation/source build, Mac desktop/input, default, package, platform, D1–D6/release or R0/R1 acceptance. Next implement real CPU/build-case bindings and full prebuilt-provider source jobs; preserve all painter/consumer deadlines and missing native/minimum/Conan/MSVC gates. |
| 2026-10-22 | Provisional R1.4 contract handling locally verified; R0/R1 unaccepted | Owner/self-review Codex; independent acceptance pending. Added actual indexed submission, zero-target suspension, immutable bounded texture policy and typed rejection. Canonical tests consume unchanged existing fixture bytes and publish matching actual observations. Moved all three frame-validation IDs into the CPU suite without removing tests. Mac ARM64 Metal hardware and Linux ARM64 software Vulkan each pass exact discovery/XML 22 RHI and 92 CPU IDs; fresh Qt/GL-disabled CPU builds pass the same 92, and eight RHI CPU checks pass without native creation. Final Vulkan core/sync validation observes 14 instances/devices, zero errors and complete owned cleanup preserving all 328 old defunct display identities. C++14 header, provider/shader/source/link audits and both missing/foreign fixture negatives pass. Full scripts pass Linux 444/444 and Mac 434 plus ten native-Linux skips; plan/inventory/worktree, renderer/plan workflow lint, tracked diff and source/history checks pass. Preserved 1,059 other files and prior 20 renderer/334 Python receipts; selected proof is indexed above. Legacy stays default; native minimum/other architectures, real Conan/MSVC, widgets/application startup, packages, D1–D6/release and R0/R1 acceptance remain open. Next integrate exact IDs/observations and identities into the existing runner/source CI, without dropping CPU or painter requirements. |
| 2026-10-22 | Provisional R1.3 local host builds verified; R0/R1 unaccepted | Owner/self-review Codex; independent acceptance pending. Added early UI/persistence consistency rejection without forcing flags or dummy I/O; 34 option contracts pass. Native Mac CLI-only and Linux full UI/CLI each build/link both Python-OFF/tests-ON and Python-ON/tests-OFF dual-library profiles, with correct effective flags/test targets and preserved CLI help exit 1 before graphics. Link audits observe the legacy factory and no RHI factory reference. Mac passes 89 CPU/seven shader-frame tests; Linux's UI-enabled binary passes exactly the same 89 CPU IDs without widget fixtures. Full scripts pass Linux 444/444 and Mac 434 plus ten native-Linux skips. Preserved 1,064 other files and prior 19 renderer/333 Python receipts; selected proof and final checks are indexed above. Recorded real manifest/native-smoke mismatches and R1.4 input/observation/runner work without removing requirements. No widget/new-GPU/default-app-startup, signing, dependency installation, CI dispatch or native/provider/package/D1–D6/phase acceptance. Legacy remains default. Next implement R1.4's actual fixture/production handling and exact-ID source integration; Mac matched SDK, minimum/other native, real Conan/MSVC and remaining R1.3 gates stay open. |
| 2026-10-22 | Provisional R1.3 local flag/library batch verified; R0/R1 unaccepted | Owner/self-review Codex; independent acceptance pending. Conditional Conan requirements, eight direct-toolchain defaults/cache disagreement guards and explicit legacy GLM replace implicit dependencies/silent provider overrides. Fixed demonstrated Qt-only Python runtime tests, persistence-OFF DXF case/discovery and Mac library-only app-relocation failures. Thirty-three CPU contracts pass, including 256 boolean combinations. Final-source native Mac/Linux ARM64 legacy/RHI/dual/CPU-Python library builds pass 89 CPU tests per Python-OFF mode, seven shader/frame tests per RHI mode and 187 CPU/Python tests each; all eight fresh staging prefixes and Qt/GL-free CPU closures pass. Linux persistence-ON passes 194/194. Mac persistence-ON rejects the selected x86_64 SDK; experimental ARM64 archives are not accepted substitutes. Full scripts pass Linux 443/443 and Mac 433 plus ten native-Linux skips. Preserved 1,057 other files and all prior 18 renderer/332 Python receipts; local proof/final checks are indexed above. Required renderer/plan workflow lints pass; broader main-workflow lint flags its unchanged obsolete release action. No real Conan/MSVC/default-app/minimum-provider/GPU/package/phase acceptance, external dependency installation/source build, release change or CI dispatch. Legacy stays default. Next finish R1.3 actual provider/host builds and R1.4 exact-ID/source CI with all R0/D1–D6/release gates open. |
| 2026-10-22 | Provisional R1.2 locally verified; R0/R1 unaccepted | Owner/self-review Codex; independent acceptance pending. Added one private static RHI executor/owned offscreen factory, CPU-only bounded prepared-frame data, target-local Qt/C++17 and embedded offline shader validation. Final native ARM64 Metal hardware (Qt 6.12.0) and ARM64 software Vulkan (Qt 6.10.2) each discover/execute exactly 13 successful IDs without skips; actual opaque/alpha/resize/lifetime pixels and input/thread/backend/shader negatives pass. Linux core/sync validation observes six instances/devices, zero errors and verified owned process/file cleanup with all 328 old defunct identities preserved. Both existing CPU suites pass 89/89; headers compile C++14 without Qt paths and CPU runtime closures remain Qt/GL-free. Installed-provider baseline plus five deliberate configure failures pass. Helpers pass Linux 426/426 and Mac 416 plus ten native-Linux skips. Source audit preserves 1,052 other files and all prior 17 renderer/331 Python receipts. Local checked proof and final checks are indexed above. Legacy stays default; no full painter, viewport, minimum/missing-platform, hosted/package, controlled resource-fault, D1–D6 or release acceptance. No dependency installation/source build, external write or CI dispatch. Next R1.3 provider/host flags and R1.4 production exact-ID/source CI, with R0 gates open. |
| 2026-10-22 | Provisional R1.1 locally verified; R0 and R1 unaccepted | User approved provisional build/factory/test integration with legacy default and all native/D1–D6/release gates open. Owner/self-review Codex; independent acceptance pending. Split CPU/Cairo and legacy targets/factories/settings, scoped discovery/links and fixed demonstrated TinySpline example probing, source-tree kernel metadata writes and Python-OFF CLI include/link gaps. Added 15 CPU regressions and a native Cairo factory test. Linux and matched-Lua Mac CPU builds each pass 89/89 with discovery/XML equality, no skipped IDs and no Qt/GL package discovery or runtime links; Linux legacy GUI/CLI compile, legacy CPU tests pass 89/89 and Cairo passes 90/90. CLI help retains its existing exit 1; no help-status change or graphics run is claimed. Helpers pass Linux 425/425 and Mac 415 plus ten native-Linux skips. Source audit preserves 1,041 other files and all prior 16 renderer/330 Python receipts. Local proof is indexed above; final plan/lint/readback checks accompany this receipt. No RHI pixels, graphical launch, dependency installation, missing-platform/hosted/package/default or phase acceptance. Next implement R1.2 private RHI/pixels, then finish R1.3/R1.4 while R0 decisions remain open. |
| 2026-10-21 | M2 implementation resumed; R0 active | Owner and self-review Codex; independent acceptance pending. Finish the real contract-input corpus and fixture bindings, add isolated mutation and independent analytic/font/image tests, and consolidate input parsing. Exact acceptance: `python3 -B -m unittest scripts.tests.test_renderer_cases`, then full scripts/plan/path/lint checks. No production/default or new provider/GPU qualification. Final validation pending. |
| 2026-10-21 | Deep plan review; R0 active and M2/M3 incomplete | Owner and self-review Codex; independent acceptance pending. Documentation-only review reconciles partial M2 helper/notice work with the absent manifest/corpus/tests; identifies M3's duplicate schemas, Cartesian quotas, mixed-profile/legacy conflicts and absent CLI. Added bounded M2/M3 corrections, proposed command/output contracts, native observations and independent oracles, endcap/path defects, total screen-error and decoded-asset/handle limits, R3/R4 prerequisites, provider/flag cohorts, reproducible workloads and evidence traceability. Retains all F1–F12, five source architectures and seven deliverables, with one manifest/recorder/executor/runner and no new framework. Validation on macOS 27.0.1 ARM64 / Python 3.15.0: full script suite 229 total, 219 passed and ten explicitly native-Linux skips; focused plan/GTest-runner regressions 51/51. Full-plan, explicit two-document and complete-worktree-against-HEAD checks pass all 16 accumulated mapped gates without unmapped paths/stale validated ledger entries; both workflow lints, tracked diff check, 51 local links and 30 candidate text-file whitespace/conflict checks pass. All prior 14 renderer and 309 Python receipts retain identical hashes/order; 959 non-plan files retain aggregate SHA256 `e4f713d8df9ed34154e8c27312e705e5b406643ea379569967b5ea1aa1df61a5`. Header/source audit confirms 60 signatures/75 consumers; missing manifest is rejected. Compiled input remains `a151d043776423225289e6a588e321d9d101fc6ae934803208e6d5b2999350bd`; unchanged draft validator is `83f5183b3fd0ce4be0773c7286d39039aaf7e5e3be4fa05e826e13c6c90a013b`. Existing helper-bound GPU evidence stays historical. No implementation, provider installation, new Linux/GPU/hosted/package run, default or gate-state change. Next finish M2 real corpus/negative/oracle tests, repair M3 selection/CLI, then M4 stable-helper execution; D1–D6 and missing native/session/hardware/budget approvals remain open before R1. |
| 2026-10-21 | M2 inputs in progress; R0 active | Owner and self-review Codex; independent acceptance pending. Implementing concrete typed inputs/assertions, complete source/case mappings and pinned real-font/drawing identities with matching notices. Configuration selection remains M3 work; no production renderer, new GPU run or threshold acceptance is claimed. Final validation pending. |
| 2026-10-21 | M1 source-register component implemented and locally verified; R0 active | Owner and self-review Codex; independent acceptance pending. Hardened the existing inventory helper to consume the entire bounded public painter interface, distinguish 60 signatures/overloads/qualifiers, reject unfamiliar/duplicate declarations, and hash/parse one header byte snapshot. Added source/header discovery and immutable expected roles for 75 current consumers, a conditional-branch lexical caller guard, bounded/path-safe input checks and exact schema/type/role drift rejection. Fixed qualifier normalization and Windows newline/absolute-path fixture issues during self-review. Added 34 CPU mutation/compatibility tests and nonempty/no-skip CI wiring in existing Linux and minimum-Qt Windows/Mac jobs; old report CLI and app/runner schemas remain unchanged. Final validator SHA256 `806997d45e1dd5ca163b3a08376ff36a0764ea18bf5e1ba46586953e52ac5406`; unchanged compiled spike input `a151d043776423225289e6a588e321d9d101fc6ae934803208e6d5b2999350bd` at HEAD `132265a0573253980451facf9109678f8bffe743`. Native-ISA Linux ARM64 Docker VM / Python 3.14.4 full scripts pass 229/229 without skips; macOS 27.0.1 ARM64 / Python 3.15.0 runs 229 with 219 passed and ten explicitly native-Linux skips. M1's exact CI Python check runs 34/34 without skips locally. Full-plan, explicit-batch and complete-worktree-against-HEAD checks pass all 14 mapped gates without unmapped paths/stale validated ledger entries; renderer/plan-workflow actionlint, tracked diff check, 51 local links and 28 candidate text-file whitespace/conflict checks pass. Prior 12 renderer and 307 Python receipts remain byte-for-byte/newest-first. No production/default, dependency/provider installation, new GPU/hosted/package execution or platform acceptance. Remaining configuration/case logic is draft; next implement M2 real inputs and M3 valid selection, then M4 stable-helper GPU evidence. Existing helper-bound GPU records remain historical; D1–D6 and R1–R7 gates stay open. |
| 2026-10-21 | Deep review complete for the plan; R0 active | Owner and self-review Codex; independent acceptance pending. Documentation-only update: identified the unfinished signature/case-validator draft and absent manifest/corpus/tests/CLI; added M1–M4 subpatches, concrete D3 decision inputs, stage-aware exact-ID selection, per-device resource isolation, per-draw dynamic-buffer ranges, conservative culling, borrowed-device recovery policy, valid platform/provider/package layers and controlled benchmark sampling. Preserved all support rows and compact single-recorder/executor/manifest/runner boundaries. Corrected retained versus current-helper evidence and historical cleanup wording. No product code/default/CI or provider installation changed in this review. Validation: Mac scripts run 195, with 185 passed and ten explicitly native-Linux skips; focused plan/GTest-runner regressions 51/51. Full-plan, explicit two-document and complete-worktree-against-HEAD checks pass all 14 mapped gates without unmapped paths/stale validated ledger entries; renderer/plan-workflow actionlint, tracked diff check, 51 local roadmap/README links and 27 candidate text-file whitespace/conflict checks pass. Prior 11 renderer and 306 Python receipts remain byte-for-byte/newest-first; the digest of 956 non-plan source/config files is unchanged. Compiled input remains `a151d043776423225289e6a588e321d9d101fc6ae934803208e6d5b2999350bd`. Read-only validation checks 12 retained Metal app reports / 512 assertions per Qt 6.8.3 and Qt 6.12.0 provider; all 24 runner bindings are stale against draft validator `51c9bdaee6bd486e6ac834f3439a115e9bcbdc40879b09809935ccde9dbb779c`. No new GPU/hosted/package execution or manifest acceptance is claimed. Next finish M1–M3, refresh stable-helper evidence in M4, retain providers and close native/session/hardware/deployment/workload/package and D1–D6 decisions; R1–R7 remain proposed and not ready. |
| 2026-10-21 | R0 active; Linux process cleanup locally verified | Owner and self-review Codex; independent phase acceptance pending. Implemented one private Linux supervisor in the existing runner: fresh single-threaded/child-free/default-SIGCHLD guards, subreaper adoption, identity-checked pidfd TERM/KILL, exact wait/reap and restored subreaper state, private Xauthority and owned lock/socket retirement, staged output plus post-cleanup identity/pixel/failure revalidation. CI uses the owned launcher and requires native cleanup tests without skips. Fixed the required CMake source module being ignored by adding a single-file exception and Git-visibility regression; registered .gitignore in the existing plan map. Unchanged compiled input `a151d043776423225289e6a588e321d9d101fc6ae934803208e6d5b2999350bd`, process helper `4dd0c5755a067a77ab893de6bdd9cd80ed9fe11e36d9a5ac30a9e887dad318ef`, final `supervised-v4-*` cohort: each Linux QPA passes 24 positives / 1,384 IDs plus six expected failures / 18 separately; both native ARM64 Metal providers each pass 12 offscreen / 512, C++-unchanged CTest 6/6 each. All 60 process scopes and output/helper/binary/report hashes revalidate; 120 adopted display/compositor children reaped, zero new or live display/compositor remnants/private runtime directories, all 328 pre-existing defunct identities preserved. Linux process contracts pass 19/19 and full scripts 195/195 without skips; Mac scripts are 185 passed / ten explicitly native-Linux skips. Full-plan, explicit-document and complete-worktree checks pass all 14 mapped gates without unmapped paths/stale validated evidence; renderer actionlint, tracked diff check, 51 local roadmap/README links and 27 owned-file whitespace/conflict checks pass. Prior ten renderer and 305 Python receipts remain byte-for-byte/newest-first. No production/default, host dependency installation/source build, hosted execution or release-policy change. Next implement the single case manifest and close missing provider/native/session/hardware/deployment/package/workload and D1–D6 gates; R1–R7 remain proposed. |
| 2026-10-21 | Deep review; R0 active and production not ready | Owner and self-review Codex; independent phase acceptance pending. Reconciled existing unreceipted managed-Wayland code/CI and current `a151d043776423225289e6a588e321d9d101fc6ae934803208e6d5b2999350bd` evidence without new GPU execution. Added per-tuple readiness, precise Wayland virtual-input/desktop scope, a newly identified descendant-cleanup P0 batch, hidden Lua/viewer GL dependencies, ordered R1/R3/R4/R5 subpatches and final validation/retention requirements. Current report revalidation gives each Linux QPA 24 positive / 1,384 and six expected failures / 18 separately; each native ARM64 Metal provider has 12 offscreen / 512. Mac widget/native Intel/Windows/Linux x64, hardware, deployment/package, workload and D1–D6 gates stay open; passing pixels do not qualify cleanup. Validation: script regressions 175/175 (renderer/Wayland modules 83/83); full-plan, explicit-document and complete-worktree-against-HEAD checks pass all 14 mapped gates without unmapped paths/stale validated evidence. Renderer actionlint, tracked diff check, 47 local roadmap/README links and 24 owned-file whitespace/conflict checks pass. Prior nine renderer and 304 Python receipts remain byte-for-byte and newest-first. Compiled inputs are unchanged by this documentation review. No production/default, provider installation, CI dispatch or release-policy change. Next implement isolated process cleanup, prepare the single case manifest and obtain missing provider/decision evidence. |
| 2026-10-21 | R0 active; terminal widget failures implemented and Linux-verified | Owner and self-review Codex; independent phase acceptance pending. Added persistent fatal state, output/render/delivery guards, post-Qt output finalization, fixed two-slot CPU retirement through teardown, three controlled terminal failures and additive schema-3 failure records/strict runner negatives. Final input digest `0590fd38fab3e1ba1b4a7da85e3d59029a8b30b8cfc2ab1ddac522a1f09d7d62`: Linux ARM64 software Vulkan passes 24 successes / 1,384 IDs plus six expected failures / 18 separate IDs; both native ARM64 Metal Qt providers each pass 12 offscreen / 512 and CTest 6/6. Minimum-Qt Intel targets rebuild only. Actual shutdown drain completes before failure substitution; retained storage expires after QApplication, no failed-run app/image artifacts. Script tests pass 163/163, renderer module 71. Both Mac widget preflights remain locked, without success reports. CI gains six expected-failure definitions, not hosted execution; production/defaults unchanged. Full-plan/worktree checks pass all 14 mapped gates without unmapped paths or stale validated evidence; renderer actionlint, 40 local links and owned-file whitespace/conflict checks pass; prior receipts remain byte-for-byte. Next qualify missing native/widget/providers and complete D1–D6, workload and recovery gates; R1–R7 remain proposed. |
| 2026-10-21 | Deep plan review; R0 active, production stages not ready | Documentation-only update: identified widget terminal-result and failed-drain ownership risks; ordered a bounded P0 batch with distinct expected-failure evidence. Added readiness priorities, manifest/algorithm decisions, R2 geometry/hatch/shader/cache subpatches, UTF-8/font/image/redraw contracts and full-provider/deployment/package/source-archive requirements. Unchanged source digest `4978755ef2ae2ec10a3a63d81739736e38fec05c41ce511fc86eb4187c8ddb3e`: existing Qt 6.8.3 and Qt 6.12.0 ARM64 Metal offscreen artifacts each revalidate 12 reports / 512 checks, not new GPU execution or widget teardown acceptance. Script regressions 154/154, full-plan/explicit-document/complete-worktree checks, renderer actionlint, 40 local links and whitespace/conflict checks pass. Worktree maps all 14 gates with no unmapped paths/stale validated evidence; prior receipts are unchanged. No renderer code, CI, policy/default or platform acceptance changed; D1–D6 and native/session/hardware/Wayland/package/budget gates remain open. Next implement and validate local P0 hardening with owner/reviewer before extending widget acceptance. |
| 2026-10-21 | R0 active; minimum-Mac Qt provider proof and SDK link fix verified | Implementation owner Codex; phase acceptance pending. Retrieved and verified official prebuilt Qt 6.8.3 Base/ShaderTools into `/tmp`, pinned archive/qsb/reference-file identities, fixed only old Qt's obsolete consumer AGL item when the selected Apple SDK lacks a link stub, recorded the effective flag and included CMake modules in source freshness. Current input SHA256 `4978755ef2ae2ec10a3a63d81739736e38fec05c41ce511fc86eb4187c8ddb3e`: minimum Qt 6.8.3 and current Qt 6.12.0 each pass 12 native ARM64 Metal offscreen invocations / 512 checks and CTest 6/6; Linux ARM64 software Vulkan rerun passes 24 / 1,384, zero failed/skipped. Both Intel targets cross-build but are not executed. Mac widget preflight remains locked for both providers. Script regressions 154/154 and actionlint pass; Mac CI now checks/retains the minimum Qt/qsb input pin, but no hosted run is claimed. SDKs stayed unmodified, no dependency sources were built or host packages installed; target 12.0 is not OS 12 runtime evidence. Native Intel/Windows/Linux minimum providers, GUI sessions, license/deployment/package/hardware/Wayland/budgets and D1–D6 remain open. Local full-plan/worktree checks pass all 14 mapped gates with no unmapped paths/stale validated evidence; 39 roadmap/README links and owned-file whitespace/conflict checks pass. Next qualify missing native providers/window failure paths and obtain pending policy decisions before R1. |
| 2026-10-21 | R0 active; widget suspend/reparent batch locally verified | Implementation owner Codex; phase acceptance remains pending. Added controlled completed-readback invalidation, automatic resize snapshot invalidation/current-target readiness, observed hidden/visible-zero-size no-draw intervals, and cross-top-level Qt device release/rebind/return with four-stage pixels for both the moved and surviving view. Current compiled-input digest `8f50661eb1671c4d9e7fe380738b696294be0f847ab845e6393d68b1fde8b8de`: 24 Linux ARM64 software-Vulkan invocations / 1,384 checks and 12 native Metal offscreen / 512, zero failed/skipped; Metal CTest 6/6; script regressions 151/151; actionlint passes. Expanded R0 CI definitions, not hosted execution. Mac widget preflight still reports locked console, no success report. Preserve D1–D6, native providers/hardware/Wayland, widget failure/shutdown and workload/budget gates; R1–R7 remain proposed. Local full-plan/worktree validation passes all 14 mapped gates with no unmapped paths/stale validated evidence; 37 roadmap/README links and owned-file whitespace/conflict checks pass. Next qualify missing providers and widget failure/teardown, obtain the pending D1 policy decision and measure R0 workloads. |
| 2026-10-21 | R0 active; owned failure/recreation batch implemented and locally verified | Implementation owner: Codex; phase acceptance/reviewer assignment remains pending. Added actual capability preflight, four deterministic owned-host failure points with per-attempt API/device-class validation, bounded cleanup and real recovered-device pixels, readback ownership through device teardown, schema-3 validation and rejection of unreported failed-frame output. R0 fixture v2 remains unchanged; current compiled-input digest is `620cb9023f2d62f22fd414d1c14dc99fc16701192e9827ad782a40223f089609`. Native Metal offscreen passes 12 invocations / 512 checks and CTest 6/6; ARM64 software Vulkan passes 19 / 744; helper regressions pass 143/143 and workflow actionlint passes. Current Mac widget preflight still fails on a locked console, with no success report. Updated CI definitions and README; hosted, missing native architectures/hardware/Wayland, widget reparent/failure cases, budgets and D1–D6 stay open. D1 policy question has been requested; no answer or acceptance is assumed. Local full-plan/worktree checks pass all 14 mapped gates, all 37 roadmap/README links resolve, and whitespace/conflict checks pass. Next extend widget suspend/reparent/recovery and provider qualification while preserving the R1 gate. |
| 2026-10-21 | Deep review updated; R0 active, R1–R7 not ready | Reconciled current schema-2/fixture-v2 source and retained evidence; added drawable/consumer coverage, explicit host failure/readback state, pre/post-cutover `auto` behavior, exact private-Qt/provider obligations, ordered R0 closeout and stage-entry/handoff criteria, candidate workloads/budgets and input-sensitive plan maintenance. Read back 15 Linux reports / 480 checks and 8 Metal offscreen reports / 248 checks at compiled-input digest `bc3804d8ccddb3e8429c4c64e0d66b464be5db2066a1a0c1177acd8109d92095`; representative Metal/Linux lifecycle outputs revalidate at 47/34 checks. Local script regressions pass 133/133, full-plan and complete-worktree checks pass all 14 mapped gates with no unmapped paths/stale validated evidence, 37 local links resolve, and renderer actionlint/owned-file whitespace checks pass. These are documentation/helper checks and existing local spike results, not new production/hosted/package acceptance; current Mac GUI remains unqualified because the recorded console was locked. Historical receipts remain unchanged. Next execute R0 closeout in the order above and accept D1–D6 before R1. |
| 2026-10-21 | R0 active; first standalone batch locally verified | Added isolated Qt RHI offscreen/MDI targets, original bitmap/image fixtures, shader/build/source manifests, strict evidence runner/PNG validator, painter-inventory drift and failure fixtures, and R0 CI scaffolding. Current compiled-input digest is `ab3f3d4cf85a5441ca0ad6b0b4a0de8187c81697d9a553d2e2c4205b762b22a8`. Metal hardware and ARM64 software Vulkan each pass 124 landmark/composition checks across four DPR widget runs plus two offscreen configurations; script regressions pass 122/122. Hosted/minimum-Qt/native Windows+Intel/Wayland/hardware/package and remaining R0 decisions stay open; R1–R7 are not advanced. The R0 checkpoint above records exact providers, commands, evidence roots and next cases. |
| 2026-10-21 | Deep review complete for documentation; R0 unimplemented | Added exhaustive 60-method coverage, frame/pass and factory ownership, retained asset/readback semantics, conditional metadata validation, option defaults/truth table, cross-workflow status/release boundaries and the first R0 batches with D1–D6 decisions. No GPU implementation/runtime/CI result is claimed; the next action remains the standalone R0 spike and its evidence/decision record. |
| 2026-10-21 | All R0–R7 proposed | Roadmap grounded in the pinned checkout, existing workflows/platform scripts, painter interface and official API documentation. No renderer or CI implementation/build/runtime result is claimed. First implementation task: R0's cross-platform widget/offscreen stroke-and-readback spike, feature inventory, provider/runner qualification and explicit API policy decision. |
