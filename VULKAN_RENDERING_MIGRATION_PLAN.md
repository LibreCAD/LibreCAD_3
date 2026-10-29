# Vulkan rendering migration plan

Status: modular/API refinement active 2026-10-29; R0 remains active and unaccepted.
Provisional R1, R2.1, R2.2 and R2.3 implementation are authorized; R2.4–R7 remain proposed.
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
that R1 exception does not authorize R2, cutover, retirement or desktop interaction.
R2.1 and R2.2 have their own later explicit provisional approvals recorded below;
neither exception authorizes subsequent stages or acceptance.
The branch/push/Actions authorization covers R1 and, through the later explicit
“Allow R2.1 commit, push and CI” reply, the locally validated provisional R2.1
candidate in `dxli/LibreCAD_3:codex/r1-platform-ci`. It does not authorize a merge,
new PR, release workflow change or acceptance-policy change. Independent review
remains pending.
The later human reply “Allow R2.2 commit, push and CI” also authorizes the
provisional CPU curve/stroke candidate on that same fork/branch, conditional on
passing local R2.2 checks and resolving the point-size/continuity choices first.
Once those conditions are satisfied, commit/push and the complete five-platform
source cohort may proceed without another publication question. This is not
permission to merge, change release workflows/defaults or promote acceptance.
The owner has now selected fixture diameter and corrected continuity, and
explicitly requested local validation followed by commit, push and five-platform
source CI. Local verification passed; candidate17fdc878 is now published on the
authorized branch. Exact-SHA run40146772555 is green:62/62 latest checks, with
all60 source tuples (12 per architecture). This is latest-per-tuple proof across
attempts1/2:59 initial successes plus one unchanged-budget Intel CLI retry, not
sixty fresh second-attempt executions or pristine first-attempt success.

### Current PR #453 correction boundary — 2026-10-29

On 2026-10-29 the owner requested a compact modular/API refinement. The
`cf77a989` baseline completed all60 source profiles across five platforms;
one Intel host profile passed an unchanged-budget retry after timing out. Keep
that source evidence separate from the new candidate. Consolidate target
lifetime management, preserve certified Region bounds and shared work accounting
through the preparation API, make temporary recorder results owned, and report
both device texture limits on rejection. Validate locally and require a new
all60-profile source cohort for the changed candidate.
Recorder accessor member-pointer types become ref-qualified; overloaded Region
preparation requires explicit types for function pointers. RecordedRegion is a
mutable snapshot: callers must keep its boundary, certificate and work together.
Legacy remains the default, all acceptance gates remain open, and upstream
reconciliation remains separate.

Parent `4ab8c386` already contains master `6c331bdb`; its source run40603224689
fails39/60 native profiles, so pre-merge green evidence does not qualify it.
The owner authorizes fixing all ten review findings, regression tests, local
validation, commit/push and a fresh complete five-platform source cohort.
Reconcile the actual Python ABI field, backend-owned test inventory, Conan pins
and Windows Lua dependencies. Reuse one exact-sum helper for affine transforms,
tile/Region placement and fill orientation; remove the duplicate fill adder.
Preserve pattern cell periods independently of ink bounds and certify Region
joins before camera placement. Fill preparation uses local curve certificates,
bounded topology refinement and sweep/active-edge processing within the original
work cap, including adjacent-curve crossing rejection. Regression and pre-fix
negative controls precede publication. Frozen Mac ARM64 validation passes554
CPU/Python/persistence IDs and22 offscreen Metal IDs; full scripts pass874 with12
expected platform skips. All139 path/geometry cases pass production ASan/UBSan
(fixture ASan-only, dependencies and Mac leaks unqualified); new regression
families fail preserved pre-fix implementations. Final plan/inventory/lint checks
repeat before publication. All60 exact-SHA source jobs remain required; partial
passes and earlier green commits are not qualification.
The failed5cec106 cohort exposed installed-runtime and feature-membership gaps:
publish only validated shared Lua DLL directories on Windows, select the
available GTest1.18/C++17 binary, guard Lua persistence tests with both features,
build/register the installed Mac CLI, and stage its GLFW/GnuTLS curl providers
in Snap. Reuse existing target-owned header caches through one default-OFF
cold-source opt-in, preserving300/1200 limits. Capture native Metal identity on
both Mac architectures and preserve every Intel contradictory-typing assertion.
The owner approved a distinct ARM virtual Metal source/offscreen route on
2026-10-28: require native ARM Metal, an ID-matched AppleParavirtGPU entry and
raw Qt type1. Retain the Intel route separately; neither route qualifies
hardware, widgets, packages or a later stage. Unknown/missing native identity
still fails closed. Windows library-source lanes use the same selected MSVC
provider through Ninja to overlap compilation; host lanes retain Visual Studio.
Every target/test and the300/1200-second budgets stay unchanged.
The separate46-case Hatch/Region/Tiler/Catalog subset also passes ASan/UBSan
with production and test TUs instrumented; dependency internals and Mac leaks
remain unqualified. Durable local proof is retained in
`build/renderer-evidence/pr453-ten-fixes-20261028.55Ne2l/`.
Use the merged C++17/in-tree libdxfrw contract. Keep legacy default,0.25px and
300/1200-second limits, every assertion and all acceptance gates unchanged.
No new upstream reconciliation, R2.4+, hardware, application, package or release
acceptance is authorized. Historical checkpoints below retain their old scope.

### Previous exact-source closeout and implementation boundary — 2026-10-26

Candidate `7720f239a675f5f1136404e3e4af2ca39f1ff633` has completed
run40264405249 attempt1:62/62 source checks, all60 profiles/12 per platform.
The full strict archive audit passes, including all seven new regression IDs;
all40 host artifacts/20 CLI installs pass independent retained-byte/loader
readback. Final closeout binds six immutable seals, the original checkout/index/
staging and all metadata. Retain its initial self-comparison plan-check failure
and corrected actual-parent six-path validation, not a waived assertion.
Evidence: `build/renderer-evidence/pr453-pch-chain-20261025.wlqdWF/hosted/final-closeout-v2.json`.
This is source proof only; verified virtual Intel Metal is not hardware acceptance.

The full migration remains incomplete. Continue existing provisional R2.3 by
validating an ellipse's major axis as well as its center: a nonzero/nonfinite Z
must not become planar ink through radius/angle projection. Preserve planar
ellipse geometry, both directions/seams, atomic failure and recovery. Fresh Mac
checks pass266 CPU IDs preserving264 and22 Metal IDs; both new regressions
fail against preserved pre-fix libraries. Focused Mac ASan/UBSan passes all27
recorder IDs with adapter/tests instrumented, other dependencies and leak
acceptance unqualified. Linux fresh266 CPU/22 Vulkan checks and retained-object
readback pass, including four own caches/167 ordinary units/eight raw C++14
headers and14 validation instances/devices with zero core/synchronization errors.
Full666 scripts pass on both hosts (Mac11 expected Linux skips/Linux0). Initial
ownership-count failures (Region15 vs16/hatch10 vs11) are retained; updated
guards require the two exact new IDs as well as the counts. Linux binds a
single test-guard-only native/protocol transition, with separate complete
freezes and no compiled-input/helper/provider drift. Preserve all local
preflight/evidence-harness false starts, including the Mac receipt-format retry.
All45 unchanged reader models, additional exact-inventory rejection probes and
both fresh native readbacks pass. Final plan validation, six-path publication
and a fresh all60 exact-SHA source cohort are next; old source CI is not proof
for this new correction. Evidence remains in `build/renderer-evidence/r2-3-ellipse-plane-20261026.egZvFY/`.
The R2.4 readiness audit identifies ordered coverage-domain ranges, owned paint,
bounded double-to-float packing and unique aligned per-draw uniforms as the next
executor boundary; reuse the existing Frame/Executor, never feed overlapping
stroke triangles directly to OVER or apply the camera/DPR twice. Provisional
R2.4 implementation/publication was requested, not approved; later-stage gates
and all D1–D6, default/cutover/hardware/package/release prerequisites remain open.
Legacy stays default and upstream reconciliation remains separate.

### PR #453 review corrections — 2026-10-25

The human requests fixes, compact implementation, regression tests, local and
five-platform source validation, then commit/push. Correct near-reversal miter
certification, whole-spline dash/cap continuity and installed Lua module lookup.
Retain the0.25-physical-pixel bound, all assertions and300-second command /
1200-second profile limits. Shared build/provider preparation may be simplified;
legacy remains default and every acceptance gate remains open. Fresh proof:
`build/renderer-evidence/pr453-fixes-20261025.QEEQlr/`.

The prior Region source cohort failed its Intel Mac libraries/Python/OFF cold
kernel command; other old passes do not qualify this correction. The owner
explicitly keeps this correction focused and leaves the86-conflict upstream
reconciliation for a separate update. Its isolated trial merge is aborted.
Fresh Mac/Linux ARM64 core checks pass264 IDs, preserving257; Mac persistence
passes270. Both hosts pass seven independent C++14 headers, strict CPU builds
and SDK-free viewer links. Linux ASan/UBSan passes172 geometry/Lua IDs; Mac
geometry sanitizers pass170, with kernel/provider instrumentation and Mac leak
acceptance explicitly unqualified. Linux maintenance passes648 scripts without
skips; Mac runs648 with11 expected native-Linux skips, plus full-plan/checker/worktree/inventory/lints.
Fresh five-platform source CI remains required. No merge or new
phase/GPU/widget/hardware/package/release acceptance is inferred.

Candidate4c5b9e97 is pushed. Its delayed automatic run40238006451 superseded the
manual dispatch; partial passing jobs are not a full-cohort result. Strict actual
artifact readback finds missing discovery.txt and vk_layer_settings.txt uploads.
Fix and regression-test retention rather than waive byte checks. C++/Lua code
and local native proof are unchanged; fresh follow-up evidence belongs in
`build/renderer-evidence/pr453-retention-20261025.k3Z7iF/`.
The two uploader globs and two actual-glob regressions pass locally:650 scripts
per ARM64 host (Linux0 skips/Mac11 expected Linux-only skips),264 native per
host,270 Mac persistence and172 Linux sanitizer IDs. These binaries are reused
with902 unchanged compiled/test inputs and original hash bindings; no fresh
compilation is claimed. Publish only the four follow-up paths and require a
fresh full source cohort; cancelled or incomplete proof does not qualify it.

Retention candidate590a79e8's run40240293620 exposes a Windows cold final RHI
compile timeout, not a compiler error. Its four-target command invokes MSBuild
four times and regenerates/relinks shared kernel/viewer dependencies three
times. One opt-in, non-default aggregate target must retain the same four
checks, with actual File API dependency validation. Keep the source profile's
300-second TOTAL deadline, per-command cap, tests, independent headers and
assertions unchanged. Fresh local/regression and all-five source validation
remain required; partial prior passes do not qualify this graph correction.
Evidence: `build/renderer-evidence/pr453-graph-20261025.irkjee/`.
Legacy defaults, upstream deferral and all acceptance gates remain unchanged.

Fresh ARM64 checks now pass264 CPU and22 offscreen RHI IDs per Mac/Linux host.
Linux Vulkan validation reports14 instances/14 devices and zero errors. Seven
CPU C++14 headers and the RHI header pass; CPU-only/RHI-test-OFF profiles omit
the aggregate. Full658 scripts pass on Linux with0 skips and Mac with11 expected
Linux-only skips. Original902 inputs have only root CMake changed, with901
unchanged; no C++ implementation or new sanitizer qualification is claimed.
The superseded590a79e8 cohort retains52 successful source jobs, one timeout and
seven cancellations, not a full pass. Publish the five-path graph correction
and require fresh all60 exact-SHA source execution and retained-artifact readback.

Candidate6c880cf9's run40244434797 passes the corrected Windows graph, but the
Intel combined/Python tests-ON host fails its final immutability comparison
after all372 non-widget tests pass. This is not a timeout or a native test
failure, and the missing initial snapshots prevent retrospective attribution.
Retain that failure unchanged; keep the same five fatal comparisons and add
failure-only before/after component diagnostics, with orchestration regressions
and a fresh local host reproduction. Evidence:
`build/renderer-evidence/pr453-hoststate-20261025.SQGiCD/`.
Do not waive drift checks or infer qualification from the other passing jobs.
Fresh exact-SHA all60 source validation remains required after any correction;
legacy defaults, upstream deferral, all budgets and every acceptance gate stay
unchanged. This diagnostic follow-up changes no C++ implementation.

Local follow-up validation passes659 script checks on Mac/Linux ARM64 (Mac11
expected Linux-only skips, Linux0), including each individual and simultaneous
guard drift. A fresh coherent Qt6.12.0 Mac both/Python host passes372 required
non-widget IDs, with97 widget IDs discovered but unexecuted; three repeated
372-ID runs leave all five state maps unchanged. The Intel mutation is not
reproduced or attributed. Retain the initial mixed-local-Qt provider failure;
it is not the hosted Intel failure. Publish only the runner, its regressions
and both plan updates, then require fresh all60 exact-SHA source jobs and byte
readback. Prior CPU264/RHI22/sanitizer proof is unchanged, not newly executed
by this diagnostic follow-up. All budgets, assertions and acceptance gates stay
open and unchanged.

Candidate644afb32/run40252124285 passes the previously failing Intel host-state
guard, but three final fresh RHI builds exhaust the unchanged300-second TOTAL
deadline (Windows core/Python and Intel core). All preceding matrix profiles
pass; these are cold compilation timeouts, not compiler or runtime failures.
Keep them distinct from the unattributed earlier guard drift. Retain all old
proof and add a private Eigen/Dense PCH only when WITH_RENDERER_TESTS is enabled.
Remove the final Intel kernel-prefix drain so one aggregate schedules the whole
cold graph; ordinary library/host prefixes and PCH-OFF defaults stay unchanged.
Bind the generated cache inputs, compiled hash, C++14/private metadata and
independent header consumers, retaining tiny generated inputs but not the large
cache binary. Evidence: `build/renderer-evidence/pr453-pch-20261025.imFiPD/`.
Fresh ARM64 Mac/Linux CPU264 and RHI22 runs pass within the same limits; complete
local scripts/reader checks and fresh exact-SHA all60 source CI remain required.
No optimization level, test, assertion,0.25px bound, budget or acceptance gate
is relaxed. Upstream reconciliation remains separate; legacy stays default.

The private-cache candidate now passes fresh264 CPU/22 RHI IDs on both ARM64
hosts, with all84 original kernel units plus one cache-creation unit and eight
independent PCH-free C++14 header objects. Mac source+CPU takes56.187s; Linux
source+CPU+Vulkan takes106.867s with14 validation instances/devices and zero
core/synchronization errors. These are observations, not a controlled speedup
claim. Full665 scripts pass (Mac11 expected Linux skips/Linux0), and40 reader
models plus actual Clang/GNU cache readbacks pass. Linux's632-file compact proof
is supplemented by only eight tiny header objects (8112 bytes), not large cache
binaries. Preserve initial local-reader assumption failures and corrected
provenance. Publish only six cache/helper/test/uploader/plan paths, then require
fresh all60 exact-SHA source jobs and retained-byte readback. Actual Windows PCH
schema/timing remains unqualified until that run; all acceptance gates stay open.

The published cache candidateed7ca043's independent review identifies a Visual
Studio layout error: CMake3.31.6 keeps generated inputs and compiled PCH in
different private kernel directories. Select exact Release File API inputs,
not every generated configuration header, and require one owned nonempty cache
across those two target directories. Regression coverage includes the actual
VS layout and rejects duplicate/foreign caches; native compiler flag/hash
bindings remain mandatory. Retain superseded proof, then repeat fresh local
666 scripts/264CPU/22RHI,40 reader models, maintenance and the full exact-SHA
all60 source cohort. No time limit or assertion changes. Evidence:
build/renderer-evidence/pr453-pch-20261025.imFiPD/vs-followup/.
Corrected fresh Mac/Linux ARM64 each pass264CPU/22RHI and666 scripts (Mac11
expected Linux skips/Linux0). Mac source+CPU64.556s; Linux source+CPU+Vulkan
122.763s with14instances/devices and zero core/sync errors, under unchanged300.
Keep original proof and configure-only multi-config fixture; require actual
Windows retained cache flags/bytes and the fresh all60 hosted cohort.
Legacy defaults, upstream deferral and all acceptance gates stay unchanged.

Kernel-only caching is insufficient on b4dd957/run40259321120: the Windows core
final fresh compile exhausts300 TOTAL with no compiler error and verified
cleanup. Actual VS cache layout/flags are correct; the remaining viewer/Lua/unit
dependency chain starts tests late. Extend only opt-in WITH_RENDERER_TESTS
PRIVATE caches to those targets, each owning its cache and existing standard
(three C++14 libraries, C++17 tests), never REUSE_FROM across DLL definitions.
Eigen remains common; Lua also caches Kaguya, tests also cache GTest. Defaults,
ordinary host/library builds and seven CPU/one RHI independent C++14 headers
stay PCH-free. Static444-input macro/include-order review is clean. Fresh Mac
264/Metal22 and Linux264/Vulkan22 pass55.404s/115.467s, observations not a
controlled speedup claim. Full666 scripts,45 reader models/native readback,
maintenance and a new complete all60 exact-SHA source cohort remain required.
Full666 local scripts now pass (Mac11 expected Linux skips/Linux0);45 reader
models and both fresh strict native readbacks pass, binding eight generated
inputs/four cache hashes/167 ordinary+four creation units and seven raw headers.
Both audit entrypoints bind exactly666; retain the prior parser-only negative
control and original models. Linux exports645 compact files with eight tiny
raw header objects, not large caches. Final maintenance/six-path publication
and fresh complete all60 exact-SHA hosted validation remain mandatory.
Evidence: build/renderer-evidence/pr453-pch-chain-20261025.wlqdWF/.
Retain the failed cohort, all five seals and original checkout/index/staging;
legacy default,300/1200 budgets,0.25px bound, upstream deferral and all gates stay.

### Provisional R2.3 entry — 2026-10-25

The actual CAD Region adapter now records owned physical boundaries using the
shared curve provider. First-edge winding, unique edge reorder/reversal and
roundoff-only joins have native regressions; certified join error participates
in topology clearance and hatch bounds BEFORE preparation. Recording and
preparation share work/storage caps. Real gaps, branches and multiple cycles
inside one Loop reject rather than create connector ink. Boundary polyline widths
are stroke decoration, not an invented region offset or new pattern-width support.
Mac/Linux ARM64 each pass257 core/263 persistence/165 focused sanitizer tests,
including an independently authored raw DXF's two imported loops/area300/hole,
all63 finite pattern tiles, seven C++14 headers and source/SDK-free link checks.
Six source mutations reject;13 Intel objects are compile-only. Fresh source
publication is next;8a5e21f0's immutable green cohort does not qualify this code.
Broader providers/domains and all later-stage authority/acceptance obligations
remain. Evidence:
build/renderer-evidence/r2-3-region-20261025.oGhvya/.

The actual HatchTile candidate now prepares bounded repeated finite CAD geometry,
including affine placement, inherited physical/model dash phase, diameter points,
curve seams and full-width coverage alongside an owned fill mask. Per-call base
meshes avoid repeated subdivision; shared input/work/vertex/cell/instance caps
and compensated large-world rebasing fail atomically. Independent mask/ink
coverage tests and all63 real bundled provider tiles pass locally. This returns
union-before-OVER ink and a separate intersection mask: it does not implement GPU
masking, painter/application execution or acceptance. Published8a5e21f0's exact
run40199328429 attempt1 passes62/62: all60 five-platform source tuples are
retained/read back,242 CPU IDs per architecture preserve229,40 library profiles
execute242/340 and20 tests-on hosts execute340 CLI/349 others. All63 provider
tiles prepare on each persistence host. Diagnostic triangle counts are51198 on
ARM64/Linux x64 and51196 on Intel Mac/Windows, not a bit-identical mesh contract;
native coverage/error assertions and all timeouts remain unchanged.51/51 selected
CPU R2 bindings and14 RHI runs/22 IDs pass; two virtual Intel Metal attestations
remain source-only. Actual Region conversion and broader domains are next.
Evidence: build/renderer-evidence/r2-3-tiling-20261025.XLpiA3/.
Hosted evidence: build/renderer-evidence/r2-3-tiles-publication-20261025.0Lf7J6/.

Current continuation audits the real CAD provider, not just the passing subset.
The63 bundled DXF patterns contain11588 LINE,40 ARC, one LWPOLYLINE and two POINT
records. The provider loads finite drawing tiles; infinite solid-line families
cannot represent that corpus. New `recordHatchTile` captures owned pattern-local
commands, inherited style and a separate outer transform; it does not execute or
repeat the tile. Native Mac/Linux ARM64 each pass229 core IDs preserving221 and
233 persistence-profile IDs, including actual provider loads of all63 patterns/
11631 entities/two point marks without drops.137 sanitizer cases pass per host
(five consumers plus new fixture ASan/UBSan; old RapidJSON fixture ASan-only,
kernel/provider library uninstrumented, Linux leaks pass/Mac leaks unqualified).
Seven C++14 headers, four strict standalone CPU sources and the kernel-dependent
adapter with existing project flags pass;12 Intel objects are compile-only.
Clean Linux scripts632/632 and fresh three build IDs/five corruption negatives
per host pass. The model's full-ellipse classification now records an explicit
closed seam; retain the pre-fix regression and corrected directional tolerance
test. Published4b36697c's proof remains immutable and is not a pass for new code.
Evidence: build/renderer-evidence/r2-3-pattern-20261025.yDUe2g/.

Remaining R2.3 boundaries are implementation obligations, not a reduced exit:

| Boundary | Current proof | Next required work |
|---|---|---|
| Real pattern provider | All63 bundled DXFs load/snapshot and prepare actual repeated finite ink at a documented normalized64-pixel scale on both ARM64 hosts (51198 ink triangles each).11588 lines/40 arcs/one polyline/two points are preserved. Independent finite-gap, mask/hole/concave/disconnected, width/cap/point, curve/bulge/seam, affine/dash and binary-multiprecision large-origin cases pass. | Qualify broader scale/numeric/provider domains and implement the mask/ink coverage executor under later-stage authority. Separate CPU masks plus unioned ink are not pixel, application or GPU clipping proof. |
| CAD boundary regions | Native actual Region loops preserve unique reordered/reversed lines, arcs, full ellipses, bulges and current polynomial spline spans, winding, affine/large-origin coordinates and owned paint/gradients. Roundoff-only joins propagate into topology/culling; gaps/branches/multiple cycles fail. Independent raw DXF import proves two loops, area300 and the hole on both ARM64 hosts. | Publish and source-qualify this candidate on all five platforms; retain broader provider/width/plane/topology/numeric obligations. No legacy drawable, GPU mask executor or application integration is claimed. |
| Broader geometry/domain | Unknown/null/nonplanar/variable-width/rational/periodic providers reject explicitly rather than disappear. Catalog absence does not prove those requirements irrelevant. | Resolve supported provider/domain decisions under D3, implement required width/plane/periodic/rational behavior without inventing a new document provider, and qualify broader touching/intersection/numeric domains. Keep unsupported scope visible. |
| Cross-platform/executor acceptance | Published8a5e21f0/run40199328429 passes62/62 attempt1 with all60 exact-source artifacts read back; two-host local native/sanitizer/header/link/build proof is immutable. | Keep source proof separate from application/executor acceptance; new Region work needs fresh local/source validation. R2.4+ still needs its authority/prerequisites. No GPU/painter/default/package/hardware/phase/full-goal acceptance follows. |

Published candidate4b36697c249d4201b8ad9932bd0554a760f411f5 on the authorized fork
branch starts [run40163765536](https://github.com/dxli/LibreCAD_3/actions/runs/40163765536)
once by push:60 source tuples across five architectures, plus protocol/aggregate.
Attempt1 passes62/62, with12/12 source tuples per architecture. All60 artifacts
are retained/read back against the exact source, helpers, headers, File API,
XML/native IDs, process scopes and pinned SDK metadata. The5720-file local proof
is immutable. This is source execution/self-review, not acceptance. Evidence:
build/renderer-evidence/r2-3-publication-20261025.8wIw2W/.

The human explicitly authorizes continuing fills/hatches, local validation,
commit/push and full five-platform source CI on the existing fork branch.
Legacy remains default; every acceptance gate remains open. Parent is the local
documentation closeout4e8bb205, whose published source parent17fdc878 and complete
five-platform evidence remain immutable. R2.3 changes do not inherit those passes.
Evidence: build/renderer-evidence/r2-3-cpu-20261025.W4jcrE/.

Private provisional policy: implicitly closed simple contours; even-odd by fixture
default and explicit nonzero winding. Nested/disconnected/reversed loops are
supported; crossing, touching, retraced nondegenerate or numerically unresolved
boundaries fail closed under a declared bounded domain. Horizontal-band
decomposition replaces fixed-step solid-hatch approximation in CPU preparation,
without modifying legacy drawables. Input, topology comparisons, sorting, band
work and output are capped atomically. Physical points stay independent of fills.
Pattern candidate is positive-scale affine line families with physical widths;
finite-width stroke coverage must be clipped against the owned fill mask before
one OVER. Arbitrary CAD-entity/dashed pattern adapters and production painter
integration remain explicit obligations, not silently successful empty output.
Gradient snapshots use bounded stable insertion sorting, right-continuous duplicate
offsets, endpoint padding and straight-channel SDR interpolation; empty/invalid
stops or degenerate axes reject, without transfer functions or GPU alpha claims.
All six existing CPU fill bindings now execute real geometry. Mac/Linux ARM64
each pass221 CPU IDs (200 preserved),129 focused sanitizers, seven C++14 headers,
four strict sources, SDK-free links and separate three-ID/five-negative build
contracts. Linux leak checks pass; fixture ASan-only RapidJSON and Mac leak
qualification limits stay explicit. Eleven Intel objects are compile-only.
Clean exact-source Linux scripts pass631/631. Six copied fixture mutations reject
and restore; independent multiprecision winding/area/interior-overlap grids,
curved landmarks, finite-width hatch masks, affine/large-origin/numeric/topology
and aggregate limits are ordinary tests. All five hosted source architectures
execute221 CPU IDs preserving200, and bind51/51 selected CPU R2 IDs (48 geometry
plus three separate build IDs). Forty library profiles execute221 or319 with
Python; twenty tests-on hosts execute319 CLI /326 others. Seven headers and
14 RHI runs of22 IDs pass; two independently matched virtual Intel Metal source
attestations do not close hardware gates.97 widget IDs per UI host are discovered,
not executed. Full-plan/checker/worktree/inventory/lint/diff checks repeat after
the finalized receipt. GPU R2's76 IDs and broader hatch/domain obligations remain.
No D3 ratification, GPU/phase/platform/hardware/widget/package/release/full-goal
acceptance or R2.4+ authority follows from this provisional implementation.

### Previous R2.2 checkpoint — 2026-10-24

Owner-authorized continuation: implement independent diameter-sized points
(physical or model units; zero size has no ink), visible inner round joins and
round intra-curve cusp continuity. Preserve explicit outer joins/caps, bounded
atomic ordered preparation, the 0.25-physical-pixel budget and legacy default.
Fresh local evidence passes: Mac/Linux ARM64 each200 native CPU IDs (all190
preserved),108 sanitizer cases, seven C++14 headers/three strict sources and
Qt/GPU/Python/Cairo-free links. Three separate build IDs and five corruption
negatives pass per host; ten Intel objects are compile-only. Exact-code clean
Linux scripts pass630/630. Six unchanged corrected-policy regressions fail
against pre-fix geometry; three copied point-fixture mutations fail and restore.
Cairo-only linear comparison retains three intended inner-join differences;
the corrected analytic sector oracle passes648 tuples/693002 stable samples
(12670 separate at unchanged0.09), explicitly checks those three landmarks, and
curved comparison passes108/114954 (2658 separate at unchanged0.25).
CPU membership is not GPU pixels. Fixture ASan-only RapidJSON and Mac leak
qualification limits remain. Local maintenance checks pass and repeat after the
receipt below. Candidate17fdc878 is committed/pushed; five-platform source
[run40146772555](https://github.com/dxli/LibreCAD_3/actions/runs/40146772555) passes
all62 latest checks. All60 source artifacts are retained/read back:200 CPU IDs
per architecture (190 preserved),40 library test profiles (200 or298 with
Python),20 tests-on hosts (298 CLI /305 persistence-UI-combined), seven C++14
headers and14 RHI source runs of22 tests. Intel's verified virtual Metal stays
source-only;97 widget IDs per UI host are discovered but not executed.
All-five CPU R2 ownership is45/51 (42 geometry plus3 separate build IDs), with
six explicit unimplemented R2.3 fill proposals; RHI R2's76 IDs are not certified
by CPU or the22-ID source checks. D3 defaults/hairlines and every acceptance
gate remain open. Local evidence: build/renderer-evidence/r2-2-continuity-20261024.fjsO85/.
Hosted evidence: build/renderer-evidence/r2-2-publication-20261024.gGE6mU/.

Hosted attempt1 has one cold Intel CLI tests-OFF timeout: unchanged `lckernel`
reaches86/91 build steps at the unchanged300-second command cap, without a
compiler error; R2.2 viewer sources have not been compiled in that failed step.
The failed log/ZIP and complete initial cohort (73 indexed files) are retained.
After all original jobs finished, only that source tuple and the dependent
aggregate were retried at the same SHA. The retry passes with unchanged source,
assertions and300-second command/1200-second profile limits. GitHub creates new
attempt2 IDs for carried successes; the audit binds all60 carried job aliases
to identical original execution times, runner and complete step records instead
of calling them new executions. Full readback verifies60 source maps,2356
process records,98 ordinary XML files,52 expected corruption rejections,13162
retained file hashes and30 source-submodule SDK records. Original timeout
history is not erased; no unqualified promise of future cold-host reliability,
hardware/widget/package/release, phase or full-goal acceptance follows.

#### Prior readiness checkpoint (source-bound history)

Readiness audit: CPU R2 selects48 manifest IDs. Exact-source `lcunittest` evidence
executes39 of those; fresh native `lcbuildcontracttest` execution adds the three
build-option IDs on each ARM64 host, with all five existing corruption checks.
The union is42/48, not48/48: six `CpuFillTest.CpuHatchFill*` IDs are unimplemented
R2.3 proposals. Point has no CPU binding/preparation yet, so42/48 is not complete
F1, R2.2 or R2 coverage. RHI R2 selects76 IDs; CPU execution is not their proof.
The detailed readiness matrix in section5 names each owner and next gate.
No implementation changed in this audit; the190/98/629 and Intel/Cairo results
below retain their exact-source packet, not newly executed results.

Fresh Cairo1.18.6 library-state/membership observations plus constructor/source
inspection identify additional D3 default mismatches: candidate initial width1
versus fresh Cairo width2, and candidate zero-effective-width1-pixel hairline
versus Cairo no ink. Default butt/miter/limit10 agree. Cairo's point code uses
size as radius, while the proposed shared fixture uses diameter. These are not
project-factory or GPU pixel tests and do not silently choose a new policy.
Keep candidate/legacy behavior unchanged pending point/continuity choices and D3
review. R2.2 commit/push/five-platform source CI is now conditionally authorized;
publication waits for passing local checks and the two policy answers. Further fills,
GPU execution, text/application work, packages, cutover and retirement still need
their original prerequisites/authority. Evidence:
build/renderer-evidence/r2-2-readiness-20261024.fxBwcx/.
Full-plan/checker42/worktree122 paths/16 gates/inventory190/lints/diff checks pass
and repeat after the finalized receipt. This audit does not convert source-bound
historical tests into new implementation or platform acceptance.

#### Prior monotone-domain checkpoint (source-bound history)

The current provisional R2.2 kernel accepts provably monotone collinear cubics
that the broad near-critical root guard previously rejected. Coordinate equality
certifies axis-aligned lines without demanding exact global-offset subtraction;
ordered control polygons need no stationary roots. For other exact derivative
coefficients, strict rounded-product/FMA-residual ordering certifies d0*d2>d1²
without production multiprecision, a long-double assumption or an epsilon sign.
Power-of-two normalization must round-trip before that exact-sign route is used.
Ties and truly ambiguous reversals keep the existing fail-closed path; point,
inner-join/intra-cusp and dash-boundary policies have not changed.
The initial three regressions fail before the fix; the final FMA regression adds
96 axis/sign/scale/product-neighbor tuples with independent multiprecision signs,
positive coverage and adjacent genuinely reversing-input rejection. Fresh Mac /
Linux ARM64 each pass190 CPU IDs (187 preserved),98 sanitizer cases, seven C++14
headers/three strict sources/Qt-GPU-Python-Cairo-free links. Linux leak detection
passes; fixture ASan-only RapidJSON/Mac leak limits remain. Ten verified Intel
Mach-O x86_64 objects provide compile-only proof, not native runtime qualification.
Exact-code clean Linux scripts pass629/629 without skips. Fresh Cairo remains
648 linear tuples/693005 stable samples (12667 separate at0.09) and108 curved /
114954 stable samples (2658 separate at0.25), not GPU pixels. Full-plan/checker42 /
worktree122 paths/16 gates/inventory190/lints/diff checks pass and repeat after
this finalized receipt. Evidence:
build/renderer-evidence/r2-2-monotone-20261024.3z7TMb/.
This expands a proved numerical domain; it does not declare all valid curve
domains, R2.2, source architectures or any acceptance gate complete. Publication
and later-stage authority remain separate. R2.2 publication/five-platform source
CI now has a nonblocking owner authorization question, not assumed permission.

#### Prior work-cap checkpoint (source-bound history)

The R2.2 correctness audit now charges every flattening input element as well as
subdivision nodes against one work cap. Input size is rejected before scanning
or allocating subpaths. The unchanged new regression first reproduced a closed
line succeeding at work2 with visited0; it now fails atomically at2 and passes
at3 with visited3. Move-only/empty and mixed-curve exact-cap/recovery checks pass.
Fresh Mac/Linux ARM64 each pass187 CPU IDs (185 preserved),95 sanitizer cases,
seven C++14 header checks, three strict production sources and SDK-free links.
Linux leak detection passes; prior RapidJSON fixture/Mac leak limits remain.
Seven CPU headers and all three production sources additionally compile to ten
verified Mach-O x86_64 objects using installed Apple Clang/SDK; headers inherit
the existing configured compatibility flag, production sources remain strict.
This is compile-only evidence, not Intel native runtime/deployment/hardware CI.
Fresh exact-code clean Linux scripts pass628/628 without skips. Cairo comparison
remains648 linear tuples/693005 stable samples (12667 separate at0.09) and108
curved tuples/114954 stable samples (2658 separate at0.25); CPU membership is not
GPU pixels. Full-plan/checker42/worktree122 paths/16 gates/inventory190/lints/diff
checks pass and repeat after this finalized receipt. Evidence:
build/renderer-evidence/r2-2-workcaps-20261024.xnpj5S/.
Point size-as-diameter versus legacy radius now has an explicit nonblocking
owner question alongside continuity policy; neither behavior changes here.
No R2.2 commit/push/hosted CI or later-stage/acceptance authority is inferred.

#### Prior shared-binding checkpoint (source-bound history)

The current local candidate adds `prepareStrokes`: one owned result per command
in order, with shared input path/dash storage, work and output-vertex caps and
atomic failure. Distinct commands retain separate union-before-OVER domains;
the CPU ordered-alpha binding checks snapshots/order, not pixel blending.
Six CPU-only cases bind23 existing line/arc/circle/ellipse/polyline/stroke corpus
variants, preserving all GPU cases and the unchanged corpus/oracle limits.
Model lengths/areas are measured from owned conics by independent refined
integration (1e-9); flattened display deviation retains the0.25-pixel oracle.
Three batch regressions exercise ownership/empty entries, aggregate budgets and
late invalid-command recovery. Fresh Mac/Linux ARM64 each pass185 native CPU IDs,
preserving159, and93 recorder/curve/stroke sanitizer cases with the prior scoped
RapidJSON/Mac leak limitations. Seven strict C++14 headers, three strict sources
and Qt/GPU/Python/Cairo-free links pass. Six copied-fixture mutations and two
production mutants (reset budgets/reverse order) fail the unchanged assertions as
required, then the copied fixture is restored. Exact-code clean Linux scripts
pass627/627 without skips; inventory validates62 cases/246 proposed variants in
190 configurations. Fresh Mac Cairo checks retain648 linear tuples/693005
stable samples (12667 separate at0.09) and108 curved tuples/114954 stable samples
(2658 separate at0.25), without combining stencils or claiming GPU pixels.
Full-plan/checker42/worktree122 paths and16 gates/inventory/lints/diff checks
pass and repeat after this finalized receipt. Evidence:
build/renderer-evidence/r2-2-bindings-20261024.hay7LM/.
No R2.2 publication/new hosted CI is authorized. Point semantics, continuity
policy, broader numerical domains and new-code five-platform proof remain open;
R2.3 fill cases are still proposed, not implemented or authorized by this work.

#### Prior curved-stroke checkpoint (source-bound history)

Provisional R2.2 now prepares polynomial and positive-conic curved strokes in the
same CPU kernel/preparer. Stroke-space tangent cones, shared exact endpoint
tangents and parametric physical/dash-space displacement bounds produce strips /
caps/joins without treating every sample as a new user join. Dash-length
uncertainty is bounded in physical or stroke-time user units as selected. Full
ellipses weld to their owned start; collinear stationary spans use scalar extrema
with power-of-two scaling/exactness guards rather than unbounded subdivision.
Keep the single0.25-pixel budget:1/16 center/dash displacement,1/32 normal/cap
variation,1/32 phase uncertainty and1/16 round geometry; later float packing is
not implemented and must share the remaining budget.

Fresh Mac/Linux ARM64 each pass159 CPU IDs, preserving142 and adding17 curve
regressions. Seven independent C++14 headers, strict three-source compilation and
Qt/GPU/Python/Cairo-free CPU links pass. Both hosts pass67 recorder/curve/stroke
sanitizer cases; Linux leak detection passes, while the existing fixture-adapter
ASan-only RapidJSON limitation and unqualified Mac leak detection remain explicit.
Final exact-code Linux scripts pass626/626 without skips. On Mac, the unchanged
linear Cairo comparison passes648 tuples/693005 stable samples with12667 boundary
samples separate at0.09; a separate curved comparison passes108 scoped tuples /
114954 stable samples with2658 boundary samples separate at the total0.25 budget.
Do not combine these distinct stencils or call CPU membership rendered pixels.
Analytic annulus/filled-circle, normal-sweep, ellipse, dash-length, shear, large
origin/reflection, stationary/cusp and sharp miter-threshold checks supplement
Cairo. Underflow/near-critical failures are retained before their fixes; winding
and empty/partial-mesh guards reject uncertifiable geometry atomically.

Near-stationary scalar ambiguity, unresolved tangents/widths, and phase uncertainty
straddling a sharp corner/end return `precision-limit`, not a partial/fallback
mesh. These are explicit numerical-domain restrictions, not proof that every
valid near-cusp/topology case is complete. Visible inner-join and intra-curve cusp
continuity remain a D3 policy input: the retained short-L and collinear-cusp probes
show the current candidate and Cairo both omit the selected landmarks. The
optional corrected-continuity behavior is proposed in
[Nehab's stroke-to-fill research](https://w3.impa.br/~diego/publications/Neh20.pdf),
not an accepted project contract. The nonblocking owner choice—correct
provisionally or preserve Cairo behavior—is pending; no silent semantic change.
Next finish the selected policy, remaining CPU case/feature bindings and new-code
source validation on the other three architectures. Publication/CI needs its
separate authority; D3, GPU union/blend, hardware and all later-stage/acceptance
gates remain open. This does not redefine complete R2.2 around passing inputs.
Evidence:
`build/renderer-evidence/r2-2-curved-stroke-20261024.nYHxD2/`.

Linear-path stroke preparation is implemented provisionally in one additional
C++14 source: physical/model widths, compensation, candidate hairlines, odd /
zero dash normalization, signed offsets, subpath phase/closed seams and butt /
round/square caps plus miter/round/bevel joins. Fresh Mac/Linux ARM64 builds each
pass142 CPU IDs, preserving121 and adding21 stroke regressions; seven independent
headers, strict three-source C++14 and Qt/GPU/Python/Cairo-free CPU links pass.
Each host passes50 recorder/curve/stroke ASan/UBSan cases, Linux leak detection
enabled; the fixture-adapter ASan-only and Mac leak-detection limitations remain.
An exact-code clean Linux checkout passes625/625 script tests without skips.

Independent Cairo1.18.6 membership comparison passes648 affine/cap/join/dash/path
tuples and693005 non-boundary samples;12667 boundary-ambiguous samples are recorded
separately using the same0.09-pixel stability stencil, not counted as tests passed.
This is CPU geometry comparison, not rendered-pixel/GPU acceptance. Retain earlier
failures: full round joins/caps overpaint short neighbors, zero gaps create false
caps, trigonometric endpoint recomputation cracks shared edges, zero-on square
dashes are suppressed and internal zero arrays differ from legacy normalization.
Use exact shared normals, outer sectors/semicaps, continuous zero gaps and legacy
canonicalization while preserving original state. Harden the independent oracle
with adaptive multiprecision signs, removing fixed-epsilon thin-triangle false
positives; the required geometry/error budgets and stability stencil are unchanged.

The new boundary explicitly requires per-command triangle UNION before one OVER;
it is not a Frame ready for independently blended triangles. That linear snapshot
did not certify curves; the newer batch above adds bounded curve preparation and
still fails closed outside its qualified numerical domain. The
candidate zero-effective-width hairline is one physical pixel, not qualified
legacy zero-width parity; the Cairo comparison uses nonzero widths. Reserve1/16
pixel for linear round geometry and1/32 physical pixel phase uncertainty inside
the single0.25 total budget, alongside the earlier curve reserve. Next integrate
curve endpoint/tangent/length bounds, remaining feature/case bindings and native
platform evidence; preserve exact geometry at sharp joins, cusps and wide caps.
This temporary subpatch does not reduce complete R2.2 or qualify GPU union/blend.
Evidence: `build/renderer-evidence/r2-2-stroke-20261024.Ir0gEd/`.
Keep all publication, default, D1–D6 and later-stage/acceptance gates unchanged.

The human explicitly answered “Authorize provisional R2.2”. This permits CPU
curve flattening/stroke preparation using candidate contracts and a total screen
error at most0.25 physical pixel, not D3 ratification or later-stage acceptance.
The first implemented batch owns polynomial spans and exact positive quadratic
conics for arcs/ellipses and adaptively subdivides them with finite work/depth /
vertex limits. Reserve1/16 pixel for centerline flattening and1/32 pixel total path
length uncertainty; stroke expansion and later float packing must fit the
remaining total budget, not each reuse0.25. Zoom/DPR are included in the recorded
physical-coordinate transform. Keep prior109 IDs and existing manifest cases;
add native CPU spline bindings and independent analytic/dense-geometry checks.
Fresh Mac/Linux ARM64 builds each pass121 CPU IDs, preserving109 and adding all
seven existing CPU spline cases plus five conic/bulge/transform/recovery checks.
Both pass strict C++14, seven independent headers, Qt/GPU/Python-free linkage and
29 recorder/curve ASan/UBSan cases; Linux leak detection passes. The fixture
adapter retains the earlier RapidJSON1.1.0 UBSan limitation and uses ASan only;
Mac leak detection is not qualified. A clean exact-code Linux clone passes624/624
script tests without skips. Retain the tiny-bulge failing regression: endpoint /
half-angle conics replace angle-plus-tiny-sweep cancellation that dropped paths.
Initial validation wrappers missed the CMake File API query after tests passed;
fresh corrected builds provide the final hash-bound header/link/test evidence.
That source-bound curve-only checkpoint did not implement stroke meshes. The
linear batch above adds them, but curved strokes/integrated GPU coverage,
remaining bindings and the other three native platforms remain required.
Centerline tests alone do not prove wide-stroke offset error.
Evidence: `build/renderer-evidence/r2-2-cpu-20261024.wU7bFK/`. No source-CI dispatch,
commit/push, production factory/default, hardware or release change is inferred.
All D1–D6 and acceptance gates stay open; R2.3–R7 keep their prerequisites.

The human explicitly approved R2.1 commit, push and full five-platform source CI.
Commit `d28c41d376cb571ae84dfd37a297b1a8800bc1af` is pushed to the existing
dedicated branch, containing exactly nine intended implementation/build/test/plan
paths. Original checkout, index, staging and1086 unrelated files are preserved.
That publication's seven source hashes match its109-ID/17-sanitizer growth-fix proof;
the three2166/1292/104-file local proof packets remain unchanged. Publication,
exact-SHA CI and complete artifact readback are retained in
`build/renderer-evidence/r2-1-publication-20261024.Nlf2Gb/`.
The push completes [run40110565811](https://github.com/dxli/LibreCAD_3/actions/runs/40110565811),
attempt1, successfully: **62/62 jobs**, including all60 source tuples and12/12 on
each of Linux x64/ARM64, Windows x64 and both Mac architectures. All60 unique
artifacts pass source/helper/profile/test/header/device and supplemental payload
readback, not merely the aggregate. The audit verifies60 source maps,2356 process
records,98 ordinary passing XML reports,52 separate controlled-negative reports,
13162 retained hashes and30 exact f7debf4 libdxfrw SDKs with all three patches.

Hosted protocols pass623/623 without skips. All109 CPU IDs preserve92 prior IDs
and add17 recorder cases; all40 library test profiles retain109 CPU or207
CPU/Python IDs and six independently compiled C++14 headers. CLI hosts execute207
non-widget IDs; persistence/UI/combined hosts execute214. Each UI/combined host
still compiles/discovers97 unexecuted widget IDs. All14 RHI runs execute22 IDs;
Intel's two library lanes use the approved source-only virtual Metal route,
with selected registry IDs4294968047/4294968071 independently matching system_profiler.
Growth observations are13/13 on Linux/macOS and21/21 on Windows, within the same32
bound. No test, assertion, provider pin, SDK patch or300/1200-second/job cap changes.

R2.1's documentation-only closeout is retained locally; the remote remains the
exact green source commit. Its source/helper proof stays bound to that candidate;
changed R2.2 inputs do not inherit it. Separate full Vulkan/Metal proofs retain
their original commits/providers.
Repeat maintenance checks on the finalized receipt before its local commit.
Legacy defaults, D1–D6, hardware, independent/phase/platform/package acceptance
and R2.3–R7 prerequisites remain unchanged and open. R2.2 has only the separate
provisional implementation exception above, not accepted algorithms or rollout.

Compiler-only R2.1 portability follow-up: the unchanged recorder and six
independent CPU-facing headers compile to seven verified Mach-O x86_64 objects
using the installed Apple Clang21/SDK. Recorder compilation retains strict
C++14 warnings; header compilation uses the existing CMake flags with only
arm64 changed to x86_64. An earlier extra-strict header probe omitted the existing
nano-signal-slot compatibility flag and failed; retain it separately. Restoring
the already-configured flag changes no repository suppression or dependency.
Evidence: `build/renderer-evidence/r2-1-intel-compile-20261024.VShgni/`.

This is cross-compilation from ARM64, not a native Intel host, runtime test,
minimum-deployment qualification, full SDK attestation, CI cohort or hardware
acceptance. The exact unchanged-code109/17 native ARM64 proofs remain valid in
their original packets; that compile-only packet alone supplies no five-platform
CI claim. The separate native cohort above now supplies source-CI execution.
Linux x64 and
Windows cross compilers were not available in the existing local container;
no toolchain or dependency was installed. This portability proof predates the
now-published R2.1 candidate; it is not native Intel execution. The new explicit
approval resolves the earlier publication blocker. R2.2 now has its own explicit
provisional exception; R2.3–R7 do not inherit it.

R2.1 follow-up: review found exact-size reserve on each line append, making
open-path recording quadratic despite a bounded element count. A deterministic
4096-append capacity-growth regression fails against the prior code (4096 growths,
limit32). Replace exact reserve with amortized vector append and a single atomic
range insertion for close/restart. The unchanged regression also checks closed
subpaths, element counts/endpoints and path consumption; no wall-time assertion
is substituted. Fresh Mac/Linux ARM64 builds each pass109 CPU IDs, preserving all
108 prior IDs, and17 recorder ASan/UBSan cases (Linux leak detection enabled).
Open growths fall4096 to13; closed/restarted growths fall24 to13, within the same32
bound. Strict C++14 and all six independent header consumers pass; source binding,
full-plan/checker42, worktree119 paths/16 gates, inventory190, workflow lints and
diff checks pass and repeat after the finalized receipt. Evidence is at
`build/renderer-evidence/r2-1-growth-20261024.WmWNfZ/`. Keep earlier108-ID/16-sanitizer
and623-script results bound to their exact sources, not promoted to new-code proof.
Recorder changes are limited to amortized append and the regression; helpers,
defaults, limits, assertions and300-second caps remain unchanged. Candidate code
is now committed/pushed at d28c41d3 with the successful source cohort above;
broader phase/platform acceptance is not inferred.

#### Authorized provisional R2.1 CPU recording

The human explicitly answered “Authorize provisional R2.1” after the completed
R1 source cohort. This permits CPU path/state/transform recording using candidate
contracts, not D3 ratification, later subpatches or a production painter/default
change. Implementation is in `lcviewernoqt/painters/pathrecorder.{h,cpp}` beside
the existing prepared-frame boundary. It is Qt/QRhi/OpenGL/Python-free C++14;
the existing source/library/host runner executes its native tests. Full support
and phase acceptance remain open. Initial candidate validation is retained under
`build/renderer-evidence/r2-1-cpu-20261024.lM6CiQ/`.

Candidate semantics are explicit: no implicit Y flip; column-vector affine
postmultiplication after local-origin subtraction; double coordinates;
append-time path capture; save/restore of all implemented style fields and CTM,
but not the path; consuming stroke/fill; rotation-invariant unit-X scale query;
recoverable invalid input with prior valid state/path/commands retained. Recorded
snapshots own color, width/compensation, dash/offset/scaled flag, antialias and
transform. Font/asset/pattern state and curve/stroke/fill preparation remain
later work, not fake no-op LcPainter methods. Path/command/dash/stack limits are
bounded, and no GPU is created by these CPU tests. Native Mac ARM64 and Linux
ARM64 each execute108 CPU IDs without skips, preserving92 prior IDs and adding
the nine fixture cases plus seven regressions. Six independent C++14 headers,
strict C++14 compilation and CPU no-Qt/GPU/Python linkage checks pass. Both hosts
pass16 recorder ASan/UBSan tests; Linux also passes leak detection. The fixture
adapter is ASan-only because installed RapidJSON1.1.0 reports null-pointer
arithmetic under UBSan; this limitation and unsupported Mac leak detection remain
retained, not suppressed in recorder code or counted as passes. Three deliberately
corrupted fixtures fail as expected, with the copied fixture restored afterward.

All623 scripts pass on Linux in an exact-code-hash local-filesystem clone
(121.333 seconds, no skips); Mac's disjoint253/159/211-ID groups cover the same
623 IDs,612 pass and11 existing Linux-only skips. Preserve the earlier300-second
whole/combined-group timeouts and malformed AppleDouble source-archive failure;
the corrected archive contains exactly nine files. No command cap, assertion,
test, source requirement or acceptance gate is relaxed. Full-plan/checker42,
worktree119 paths/16 mapped gates, inventory190 and both workflow lints pass;
repeat these checks after the final receipt. This initial proof predates the
growth correction and the subsequently approved R2.1 publication/source-CI run.
Old acbab6cd CI does not validate the new candidate on five architectures.

#### Independent D3 Cairo reference (before authorization)

The source-CI cohort below is complete, not a live job to restart. One bounded independent R0
CPU reference check uses the existing native ARM64 Cairo1.18.6 image-surface API,
not a new recorder or the project's whole Cairo wrapper. Actual loaded-library,
header and unchanged project-source identities are retained at
`build/renderer-evidence/r0-cairo-reference-20261024.ICOCI3/`.

Five observation groups pass, with the two expected error contexts isolated:
path coordinates capture the current transform when appended; save/restore
restores the Cairo style/CTM but retains intervening path edits; stroke/fill
consume paths and repeated empty stroke leaves pixels unchanged; analytic point,
distance and inverse queries match the candidate matrix; unmatched restore and
singular scale produce sticky status2/5 errors, not recovery after later valid
calls. At a 90-degree rotation with uniform scale 2, matrix.yy is approximately 0,
while the basis norm is 2. The current wrapper returns matrix.yy as scale(); do
not adopt that expression as the new rotation-invariant query oracle.

These successful-domain observations support D3 discussion, not acceptance of
all Cairo behavior. The new recorder must still satisfy the plan's recoverable
invalid-input policy, bounded work and full owned style state. Bare Cairo state
does not cover wrapper-side fields such as line-width compensation. Coordinate
sign conventions and capture timing must be decided explicitly, not copied from
inconsistent legacy implementations. Source defaults/factories and all required
R2–R7 cases remain untouched. No provider is installed, compiled or modified, no
desktop or GPU is used, and both owned30-second probe scopes retire.

Independent native-report/library/header/source/process readback, full-plan,
checker42/42, worktree116 paths/16 gates, inventory190, both workflow lints, diff
checks,50 local links and prior receipts/gate states/1,083 unchanged files pass.
Repeat maintenance checks on the finalized receipt before handoff.
This reference predates the subsequently granted provisional R2.1 authority;
D1–D6, independent review, missing native/hardware/widget/provider/deployment /
package and phase acceptance remain open. Do not substitute another reference
probe for that approval or declare the full migration complete.

#### Latest exact-source CI closeout and next implementation boundary

Pushed `acbab6cd851a2c1de5d7c5d8ae2ec291cf5a5dc0` completes
[run40093455633](https://github.com/dxli/LibreCAD_3/actions/runs/40093455633),
attempt1, successfully: all60 source jobs plus protocol and aggregate, **62/62**.
Linux x64/ARM64, Windows x64 and both Mac architectures each pass12/12. All60
unique artifacts are retained and independently read back against this full SHA,
attempt, profiles/options, source/helpers, actual XML execution, native devices,
File API graphs, staged submodule patches and owned process retirement. No test,
required tuple, assertion, provider pin or command/profile/job cap is relaxed.

Supplementary payload audit verifies60 source maps,2,356 process records,98
ordinary passing XML reports,52 separate controlled-negative XML reports,13,162
retained hashes and30 pinned libdxfrw SDK records. The two Intel library/tests-ON
jobs each execute22 RHI IDs with zero skips; raw Qt type remains0, and selected
native `AppleParavirtAccelerator` IDs4294968044 and4294968035 independently match
system_profiler. This is the explicitly authorized source-only virtual route,
not hardware or widget qualification. Hosted protocols pass622/622 without skips.
Current source evidence is
`build/renderer-evidence/r1-popup-fix-20261024.fDteaO/hosted/`.

The full Vulkan60/50-repetition proof and Mac Metal24/1,024-check refresh below
retain their own exact tested commits/providers. Source-CI technical execution /
self-review is complete; independent acceptance, external durable retention,
full providers/deployment/hardware/widgets/packages and R0/R1/D1–D6 gates stay open.
Do not treat this as completed R2–R7 or as validation of the legacy application's
not-yet-implemented RHI painter/default.

R2.1 is the next code boundary, not another R1 rerun. A read-only readiness audit
binds the existing four CPU path/state and five CPU transform case IDs, required
typed observations and source identities in
`build/renderer-evidence/r1-cohort-followup-20261024.wDPfZt/r2-1-readiness.json`.
The earlier audit found no direct C++ definitions; the authorized candidate now
implements these nine IDs plus semantic/recovery regressions. Lexical presence
is not execution proof. Existing GL new-path/subpath stubs, CTM-only save/restore
and unchecked empty restore must not become golden behavior. Cairo and GL also
differ in coordinate/transform handling. D3 must clarify transform capture when
the CTM changes within one path, alongside consumed paths, complete style restore,
finite/singular rejection and preserved large-origin precision.

The explicit R2.1 approval supersedes the historical pending-authority receipts,
without accepting D3 or changing defaults/releases. Keep all nine case IDs and
all later requirements. Extend the existing Qt-free C++14 CPU boundary and shared
discovery/XML runner, with independent analytic observations; do not create a
second painter framework or call discovery alone execution. The prior full source
CI remains bound to acbab6cd, not the new candidate. Repeat relevant native and
maintenance checks; old all-platform green is not proof of this implementation.

#### Historical Metal refresh and then-live source-CI snapshot

Exact pushed source `acbab6cd851a2c1de5d7c5d8ae2ec291cf5a5dc0` now has fresh
native ARM64 offscreen Metal proof on the existing Apple M2 Max / macOS 27.0.1
host, with Rosetta translation observed false. Both Qt/private/qsb 6.8.3 and
6.12.0 build Widgets-ON and separate Gui-only targets from a clean clone. Each
provider passes 12 strict hardware/Metal/Cocoa invocations / 512 original checks;
the four builds also execute their exact six CTest IDs each, without skips.
All 38 owned command groups retire. This refresh changes no source/helper,
assertion, provider binary, dependency, timeout or production default.

Independent readback verifies pixels, app/report/helper identities, actual loaded
Qt libraries and Cocoa plugin (five minimum-provider modules, four current-provider
modules per run), 22 pinned SDK reference files and all four raw Qt 6.8.3 archives.
The archives are copied into retained evidence; the loaded minimum QtDBus bytes
also match the raw archive. The current Homebrew umbrella resolves to its declared
6.12.0 component modules, not the minimum private SDK. Deployment target 12.0 is
only a compiler target, not macOS 12 runtime qualification. Evidence:
`build/renderer-evidence/r0-metal-refresh-20261024.SQfPML/`.

Keep the one-use setup/readback diagnostics: initial current-Qt plugin layout
assumption, then CTest/strict-directory overlap in the collector. No native test
failed; the final fresh evidence namespace passes the unchanged complete matrix.
No desktop interaction, unlocking, widget acceptance, general hardware/performance
qualification or R0/R1/D1–D6 acceptance is implied.

At this earlier continuation snapshot, hosted
[source run 40093455633](https://github.com/dxli/LibreCAD_3/actions/runs/40093455633),
attempt1, was live at the exact same pushed SHA. That retained snapshot had
45/60 source jobs successful and all 45 matching artifacts independently reviewed;
the remaining jobs running/queued, not failed or missing. Protocol regressions
pass 622/622 without skips. Do not restart or cancel this live cohort just because
the run-level status remains queued. Preserve one coherent cohort and collect the
remaining payloads before a renewed all-platform claim. The earlier d4e9650e green
run remains historical; current partial results are not merged with it.

The M1–M3 CPU inventory/selector implementation below is now reconciled with actual
code and 110 focused regressions, rather than repeatedly treating its old draft
checklist as unfinished code. M4, missing native/provider/session/hardware proof,
D3 review, D1–D6 decisions and the remaining production stages stay open. Full-plan,
checker42/42, worktree116 paths/16 mapped gates/no stale or unmapped evidence,
inventory190, workflow lint, whitespace, 50 local links, all prior receipts/gate
states and original checkout/index/staging/1,083 other files pass preservation
review. That update stayed local rather than cancel the then-live cohort with a
new push. The latest closeout above supersedes its pending-CI state, not its
preserved native proof or historical receipt.

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
62 cases / 246 proposed variants / 60 signatures / 75 consumers. Earlier M2/M3
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

R1 implements `WITH_RHI_RENDERER`, `WITH_LEGACY_OPENGL_RENDERER` and
`WITH_RENDERER_TESTS` in `lcviewernoqt/cmake/RendererOptions.cmake`, keeping the
old default until R6. Fresh defaults
are RHI OFF and legacy ON; new renderer tests default ON only with unit tests and
RHI enabled, and old rendering tests default ON only with unit tests and legacy
enabled. Conflicting explicit/cached requests are errors, not forced OFF options.
These build options and their conflict guards are implemented; the runtime
selection flags below remain proposed. Separate old/new test switches ensure
`WITH_RENDERING_UNITTESTS=OFF` cannot disable requested RHI tests.
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

R0 is **active**; R1 and the explicitly authorized R2.1/R2.2 implementations are
**provisional**, without acceptance; remaining R2 work and R3–R7 are **proposed**.
Acceptance order is R0 → R1 → R2 → R3/R4 → R5 → R6 →
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
The manifest now pins the existing ABeeZee Version1.002 fixture bytes and its
corresponding redistribution notice; typed fixture validation checks the embedded
copyright/license, cmap and metrics. This is not a pinned production font/runtime
qualification or a claim that the bytes equal a newer upstream font.
`unittest/rendering/fixtures/README.md` records this completed input binding.
Distinguish raw font design-unit advances/bearings from
hinted/scaled raster metrics and supported shaping; licensing alone establishes
none of those rendering behaviors. Independently check the pinned binary's cmap
and metrics, with explicit replacement-glyph and malformed-UTF-8 expectations.

Audit existing consumer defects before fixing baseline expectations. In
`drawitems/endcaps.h`, `TICK` is enum value 7, but `EndCaps` does not register it
and `render()` uses `map::at`. Closed caps also call `fill()` followed by `stroke()`
on the same path. D3 must specify intended tick geometry and filled/outlined caps;
R2/R3 must test/fix these callers under the accepted path-consumption rule, not
preserve a throw or a missing outline as golden behavior. These production caller
fixes remain open; the CPU recorder/preparer does not fix the drawables.

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
| macOS arm64 / Metal / Cocoa | Historical R1 source CI passes12/12. Current clean acbab6cd R0 minimum/current offscreen proof passes12 strict runs/512 checks per Qt6.8.3 and6.12.0, four CTest builds each6 IDs, exact loaded providers and38 retired command groups. Widgets-ON targets compile; widget execution remains unqualified. | Review retained raw minimum archives and current loader/source/helper proof; obtain an authorized usable console for widget/failure/reparent/input/DPR on both providers, then qualify full providers, deployment floor and final relocated/signed bundle. Never unlock/bypass a session; current-OS execution does not qualify target macOS12. |
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
| P1 D3 geometry and assets | The shared on-disk manifest/corpus and inventory CLI validate 62 cases, 246 proposed variants, 60 signatures and 75 consumers. Initial R1 source/offscreen bindings execute; structural coverage is not full painter, geometry or asset correctness. Choices/oracles are not ratified. | Extend the existing typed inputs, independent oracles and stage-appropriate bindings without dropping required cases. Resolve endcap/path-consumption defects in the authorized stage; retain the D3 decision packet and missing feature/consumer gates. |
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

Batch 1 has current ARM64 controller/source proof and the current Linux Vulkan /
Mac Metal R0 refreshes above; missing native/provider bindings still need
qualification. Batch 2's single CPU manifest and selector are implemented; native
production bindings and D3 ratification remain stage-specific work. Batch 3's
retention/provider qualification remains independent. The current subpatch status is:

| Subpatch | Files and required acceptance |
|---|---|
| M1 Schema and source register | Implemented in `validate_source_register()` and the shared `validate_case_inputs()` path. Current inventory binds all60 painter signatures/defaults and75 consumer roles. The focused case module passes110/110, and d28c41d3's hosted protocol suite passes623/623 without skips. Keep that proof source-bound as R2.2 code changes. Old report CLI behavior is preserved. This is CPU/source coverage, not all painter execution or phase acceptance. |
| M2 Inputs and assertions | CPU implementation exists in the single schema2 manifest/corpus and `validate_case_inputs()`: bounded bytes, pinned asset/notice identities, typed ordinary/empty/limit observations and independent analytic/font/PNG oracles. Current positive/mutation regressions pass; inventory has62 cases/246 variants. D3 review and native production execution remain open; metadata validation is not painter correctness or platform acceptance. |
| M3 Valid selection | Implemented in `case_configurations()`, `validate_case_manifest()`, `select_case_requirements()` and the inventory-only CLI. Tests cover all declared stage/configuration relations, invalid/unreachable requests, CPU/RHI/legacy/Python/UI/CLI and source/install/package distinctions. Current190 expanded configurations include the bounded Intel source-only virtual tuple. Actual R1 runner bindings discover/execute22 RHI IDs; later producer IDs remain required, not claimed implemented. |
| M4 Stable-helper qualification | Partially executed, not complete: current final Linux ARM64 full60 Vulkan plus50 DPR1.25 cases, and Mac ARM64 minimum/current offscreen24 strict runs/1024 checks retain separate source/provider bindings and failed history. The d28c41d3 source cohort is completed62/62, not live; all60 artifacts are reviewed. New R2.2 changes require fresh relevant source proof. Missing native R0/widget/providers/hardware/deployment and independent acceptance stay open. Manifest success does not ratify D3 or complete R0. |

M1–M3's CPU implementation is verified within its stated scope; keep their
regressions and requirements intact while implementing missing native bindings in
their authorized stages. Next finish M4/provider/session evidence and D3 decision
review, not another manifest or a second selection validator. Before writing code,
assign the batch owner/reviewer and run section8 checks. Use the spike README's
implemented runtime commands; section6's unimplemented production flags remain
requirements, not commands that can pass today. Derive affected Python-plan gates
from actual paths rather than copying an earlier batch's count.

#### M2 and M3 retained regression checklist

These earlier draft inconsistencies now have CPU implementations/regressions in
the shared path. Preserve each invariant during later changes; native rendering,
D3 and phase acceptance still require their separate proof. Do not add a third
validator or reintroduce incompatible schemas.

| Earlier draft defect / preserved invariant | Required behavior and regression |
|---|---|
| No on-disk manifest/corpus or M2-specific positive test | Add the real files and require successful repository-wide `validate_case_inputs()` before mutation tests. Hash and parse each fixture/notice from the same bounded byte snapshot. Check Git visibility and LF/binary attributes on native Windows as well as Unix; preserve existing DXF binary policy. |
| `validate_case_manifest()` reparses fixtures/oracles and rejects newer `script-source`/workload-variant fields | Consume M2's already validated structures once; mutate each field through that shared path. Fixture scripts are inert input/provenance here, never executed by the metadata validator. |
| `case_configurations()` requires software × hardware across source/install/packages | Implement section 6's coverage layers. D4's explicitly qualified Windows software-or-trusted-hardware route is a declared alternative; separate hardware qualification stays mandatory. Missing access remains pending, never an automatic alternative/waiver. |
| Mixed profile/mode predicates conflict; legacy selections are checked as native Vulkan/Metal | Validate supported relations, not every cross-pair of a case's mode/profile lists. Give the legacy process its real backend and separate IDs; `any` device cannot satisfy software/hardware or performance proof. Reject genuinely unreachable requested rows rather than silently filtering them. |
| Data types cannot establish semantic or numerical correctness | Independently recompute analytic lengths/areas/transforms/curve bounds, decode the odd-sized image and check actual font metrics. Separate categorical equality, dimensional error and performance ratios/absolute limits; malformed UTF-8, huge integers, NaN, overflow and oracle-unit swaps must fail safely. |
| No inventory CLI or CI invocation of the real corpus/selector | Add the proposed mode below, preserve old report CLI tests and run every valid stage/tuple in CPU contract CI. Proposed IDs remain requirements until a later native producer discovers and executes them. |

Implemented M3 inventory/selection CLI, **not native execution or acceptance**:

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
| P0.2A Explicit virtual tuple — implemented and source-verified | Existing diagnostic/validator/runner/GTest/workflow boundaries now bind the selected native Metal registry ID to the exact paravirtual accelerator. Raw Qt type remains unknown/virtual; name-only, contradictory or lost native identity fails closed. The Intel tuple stays distinct; the later ARM authorization is scoped separately in P0.2C. | Current exact-SHA core and Python Intel jobs each pass all 22 original RHI IDs, no skips, with independent system display-ID matching and negative classifier/scope regressions. Hardware/widget/performance/package configurations are unchanged. Preserve this verification on future changes; use route B if it fails. |
| P0.2C ARM virtual source route — approved 2026-10-28 | Owner explicitly allows verified ARM virtual Metal for source CI. The selected-device native probe binds nonzero registryID4294967706 to actual AppleParavirtGPU and Qt type1. Add only r1-source-virtual-metal-arm64/MAC-ARM64/rhi-offscreen/offscreen/cocoa/virtual; preserve Intel class/type restrictions and all hardware templates. | Require exact native ID/class/type/architecture and22 unchanged RHI IDs without skips. Reject cross-class/type/ISA, missing/overflow IDs, cross-case mismatch, widgets/packages and R2–R7. This decision does not accept hardware, R0/R1, D1–D6 or release gates. |
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
case instead of hard-coding today's246 proposed variants as a permanent total.

### R2 Drawing commands and portable rendering core

Current entry status: provisional R2.1 implementation explicitly authorized by
the human after the successful acbab6cd source cohort. The CPU recorder uses the
candidate contracts above, with nine existing native case IDs and additional
recovery/ownership/resource/append-time tests. It does not ratify D3 or change a
factory/default. The later explicit R2.2 exception permits provisional CPU curve
and stroke preparation, not D3 ratification or GPU execution. The human's later
R2.3 exception permits provisional fills/hatches and local-check-conditioned
publication/source CI. R2.4–R2.5 and every phase/release acceptance retain their
original prerequisites.

1. Implement path/state/transform contracts and immutable commands; keep legacy
   source compatibility and independently test CPU recording without a GPU.
2. Implement triangle strokes/dashes, fills/gradients, adaptive curves and local
   origins. Establish blend/coverage conventions and ordering. Run F1–F4/F7 tests
   on each available qualified backend as features land, not only at cutover.
3. Implement persistent geometry, staging and cache invalidation/retirement with
   memory limits. Separate live selection/style from cached geometry where possible.
   Add upload/draw/cache metrics and lifecycle/failure tests.

Use these bounded subpatches after D3 selects the algorithms, except the explicitly
authorized provisional R2.1/R2.2/R2.3 candidate work; each adds its case
IDs to the same selection manifest and keeps the preceding offscreen scene working:

| Subpatch | Concrete implementation and independent acceptance |
|---|---|
| R2.1 CPU path/state | Own paths/subpaths, full style snapshots and double transforms. Test empty/repeated stroke/fill, reset/save/restore, singular/zero scale, nonfinite input and inverse/distance queries against analytic results before GPU creation. Reject invalid transforms without corrupting later valid commands. |
| R2.2 Curves and strokes | Flatten supported spans with the approved screen-space bound, then build ordered triangles for cap/join/miter/dash/coverage. Validate endpoint/cusp/inflection and near-180-degree joins; odd patterns, offsets and closed seams. Check distance/bounds against analytic or high-precision independent geometry, not just sampled matching pixels. Bound recursion, vertex counts and time. |
| R2.3 Fills and hatches | Apply the accepted fill rule, contour orientation/hole handling and bounded CPU tessellator. Test concavity, nesting/disconnected loops, touching/self-intersecting contours under the declared domain, degenerate hatch bounds and zero/negative pattern scale. Verify signed area/point membership and interior/background landmarks. Replace legacy approximation where it fails the contract; no triangle-fan or unbounded hatch expansion. |
| R2.4 Executor and shaders | Consume the same recorded commands on widget/offscreen hosts. Test packing/reflection, distinct aligned per-draw ranges, vertex/index layouts, required SPIR-V/MSL variants and target-compatible pipelines. Check byte-size/index arithmetic before allocation; chunk within device limits without reordering alpha or resetting dash phase. A multi-style same-frame case must catch last-uniform-write corruption. |
| R2.5 Cache and retirement | Exercise document/entity/style/tolerance/device generations, affected and unaffected inserts/views, selection, undo/reinsert, in-flight deletion and bounded eviction. Compare cached/uncached output and count uploads/allocations. Test two separate QRhi owners with colliding local IDs/generations; no cross-device resource reuse or stale-handle aliasing is allowed. |

#### R2 readiness audit — requirements retain their full scope

The published221-ID native CPU run preserves all200 R2.2 IDs and adds21
fill/hatch/gradient regressions. All48 geometry IDs plus three separate build
IDs cover51/51 selected CPU R2 IDs on both ARM64 hosts, including all six prior
fill proposals. This is CPU selection ownership, not complete F4/GPU acceptance.
Counts from different executables describe disjoint ownership, not a combined run.
The working CAD-pattern adapter adds eight ordinary CPU regressions (229 core
IDs) and one persistence catalog test (233 persistence-profile IDs) on each ARM64
host.137 focused sanitizers preserve129 prior cases. These new source hashes are
not published or five-platform-qualified yet; recording does not implement tile
repetition or actual painter execution.

| Required boundary | Current evidence/status | Remaining implementation or authoritative gate |
|---|---|---|
| Path/state/transforms and polynomial spans | Preserved recorder/curve/stroke ownership42 selected geometry IDs plus six fill IDs in the native221-ID suite; supported document `LCVSpline` consumes2/3/4-control polynomial spans. Rational/higher-order document providers are not invented here. | D3 path/default/units ratification; actual drawable/painter integration belongs to later execution/integration stages. |
| Build-option ownership | Fresh three-ID `lcbuildcontracttest` on all five source architectures; ordinary/empty/limit, five corruption negatives (six on Windows), hashes/traces/File API/XML verified. | Build evidence is source-only; these three are distinct from geometry GTests and do not close deployment/hardware/package gates. |
| Curves, caps/joins/dashes, numeric/resource limits | CPU preparation retains analytic/multiprecision/Cairo membership, monotone-domain, aggregate-work/vertex/state and atomic-recovery evidence; the current129 focused sanitizer cases per ARM64 host preserve all108 prior cases. All original error/time/size limits stay unchanged. | Ambiguous near-stationary reversals, unresolved tangents/widths and sharp endpoint/corner dash uncertainty remain fail-closed, not declared fully qualified domains. |
| Point and stroke defaults | Owner-selected diameter marks preserve the path, snapshots/order, physical circles or affine model ellipses, zero no-ink, bounded physical major axis and shared resource caps. Three CPU fixture variants retain the GPU case. Candidate default width1/one-pixel hairline still differs from Cairo initial width2/zero-width no ink. | D3 default/hairline review; point authority does not change legacy or ratify stroke defaults. |
| Inner-join/intra-cusp continuity | Owner-selected inner round fans and internal curve-cusp round continuity preserve explicit outer join/cap decorations. Independent analytic landmarks, six pre-fix regression rejections and corrected sector/Cairo comparisons bind the selected policy. | D3 acceptance and broader qualified numerical domains remain separate. Do not claim unchanged Cairo parity for the three intentional corrected inner-join landmarks. |
| Fills/hatches/gradients | Provisional bounded band tessellation implements all six CPU fixtures, both fill rules, concavity/nesting/disconnection/reversal, implicit closure and degenerate no-ink; ambiguous/touching/crossing topology rejects. Owned masked affine solid-line families include physical-radius padding; bounded owned gradients have explicit stop/alpha/axis policy. Native221/129 sanitizer evidence per ARM64 host and independent winding/area/overlap/numeric/resource regressions pass. All five source architectures bind51/51 CPU R2 IDs. | D3 domain/fill/color ratification; arbitrary CAD-entity/dashed pattern adapters, broader numerical/topology qualification and actual painter/executor integration remain open. CPU51/51 does not prove GPU fill/gradient pixels, phase or full-plan completion. Retain exact4b36697c source proof and refresh after future code changes. |
| GPU coverage/ordering and caches | PreparedStroke requires a separate union-before-OVER domain per command. No production painter consumes these meshes; CPU snapshots do not prove alpha pixels or cache invalidation. RHI R2 selects76 IDs. | R2.4 executor/shader/packing/resource proof and R2.5 cache/retirement work under their prerequisites; do not feed overlapping stroke triangles directly to Frame OVER. |
| Native platform/publication/whole-plan closure | Published4b36697c/run40163765536 passes62/62 on attempt1:60 source tuples/five architectures plus protocol/aggregate, all60 artifacts read back. Native221 IDs per architecture preserve200; local129 sanitizers per ARM64 host, Intel virtual Metal source-only. Prior R2.2 evidence and its initial timeout remain immutable history. R0/D1–D6 and later gates remain open. | Independent review, broader numerical/topology/pattern domains, R2.4–R2.5 and actual application/export/package/default/cutover/retirement work remain required under their own authority; source CI is not platform/hardware/release acceptance. |

Current provisional R2.2 implements odd/zero dash normalization, signed offsets,
physical versus stroke-time user-space metrics, affine widths, closed seams and
bounded curve/stroke preparation. Keep their ordinary suite/manifest regressions
and aggregate input/state/work/output limits. Owner-selected diameter and
corrected continuity are now implemented provisionally. Prove remaining numerical
domains and retain the now-green exact-source checks on every supported
architecture. Refresh them after future code changes. Rejecting
unresolved valid cases is not completion.
Use private preparation policy, not new public LcPainter setters; legacy default
and fixture overrides, including compensation/hairlines/degenerates/near-180
joins, remain distinct. Width/tangent/dash/offset and future packing share one
total0.25-pixel budget, not a fresh budget at every stage.

Keep one ordered stroke command's coverage as a union before its single OVER
application. Overlapping segment/join triangles must not double-blend translucent
paint. CPU preparation/oracles must make that obligation explicit; actual GPU
coverage resolution belongs to R2.4 and is not proven by a CPU mesh or sampling.
Use analytic dash intervals, cap/join landmarks, independent coverage membership
and bounded-work negatives in the existing suite/manifest, retaining all prior
IDs. Finish these obligations before claiming R2.2 complete; publication/new CI
must satisfy the recorded local-check and point/continuity conditions first.

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
| 2026-10-29 | Modular/API refinement locally validated | Consolidate target teardown, report the full texture interval, return owned recorder snapshots from temporaries, and carry Region certificates and work into preparation. The `cf77a989` baseline passed all60 source profiles after one unchanged-budget Intel retry; its proof does not qualify the new candidate. Fresh Mac ARM64 passes556 CPU/Python/persistence IDs,22 Metal IDs and874 scripts (12 expected skips); standalone C++14 sanitizers reproduce the old recorder use-after-free and pass the new code. Evidence: `build/renderer-evidence/pr453-ten-fixes-20261028.55Ne2l/local-modular-refinement/` and `local-recorder-accessors/`. Run final plan checks and a fresh all60-profile source cohort. Keep legacy default, existing assertions/budgets and all gates; upstream reconciliation remains separate. Exemption: .gitignore retains the generated build-r1 exclusion and two maintained renderer CMake inclusions. |
| 2026-10-28 | Approved ARM source route and bounded Windows scheduling | Owner explicitly authorizes the distinct ARM virtual Metal R1 source route. Require actual ARM Metal plus selected-ID-matched AppleParavirtGPU/type1/nonzero uint64 identity; retain every Intel/default/contradictory-host assertion and all hardware templates. Only the exact new source/offscreen tuple is additive; R2–R7, widgets and packages reject it. Diagnose bb3707's two Windows initial-source300s timeouts as VS project serialization; library lanes use Ninja with the Conan-selected MSVC194/x64 environment, while host lanes retain Visual Studio. Admit only the exact owned Ninja/MSVC PCH filename; every target/test and300/1200 limits remain. Frozen Mac554 CPU/Python/persistence +22 Metal and874 scripts complete (12 expected Mac skips);191 configurations/252 requirements, focused rejection/activation/cache tests and generation-only Conan/CMake probes pass. Evidence: build/renderer-evidence/pr453-ten-fixes-20261028.55Ne2l/local-arm-ninja-final/ and hosted/windows-ninja-*-generation/. Retain bb3707's57-success/5-failure cohort as unqualified; fresh exact-SHA all60 CI and final maintenance remain required. Legacy default, all acceptance gates and separate upstream reconciliation remain. Exemption: .gitignore retains the merged generated build-r1 exclusion and two hand-maintained renderer CMake inclusions. |
| 2026-10-28 | Route the pinned Qt installer log to owned evidence | Keep the pinned aqt3.3.0/action and active stock logging; supported LOG_CFG sends its file output to the absolute owned build-r1 log, with INI/log retained. No new ignores or source cleanup. The actual pinned wheel CLI proves default root dirt, clean configured execution, append behavior and untouched unrelated dirt still rejected by the guard. All867 scripts complete (12 expected Mac skips);19 workflow tests pass. Native554 CPU/Python/persistence +22 Metal evidence is reused after every source/helper digest compares unchanged. Evidence: build/renderer-evidence/pr453-ten-fixes-20261028.55Ne2l/local-aqt-log-followup/ and hosted/aqt330-clean-source/. Retain0911 source-guard failures as unqualified; require final maintenance and fresh exact-SHA all60 CI. Intel virtual source scope stays; the ARM virtual route or physical runner awaits the owner decision. Existing targets/assertions,300/1200 limits, compiler flags, legacy default and all acceptance gates remain; upstream reconciliation stays separate. Exemption: .gitignore retains the merged generated build-r1 exclusion and two hand-maintained renderer CMake inclusions. |
| 2026-10-28 | Compact build follow-up and native Metal identity | Share the Mac/Linux install-path target loops (eight fewer production lines); the existing test evaluates actual CMake properties across all/partial/no targets on all three platforms. Freeze metadata per owned profile, stage persistence before Lua, and keep Windows SDK/presets out of source without new ignores or cleanup. Real Git/Conan/metadata regressions preserve dirt detection and invalidation. Retain the selected Metal device's ID-matched actual registry class; reject native virtual classes claimed as hardware. The hosted ARM probe proves AppleParavirtGPU/type1, explaining the old Intel-class filter failure. New native assertions pass; preserved old collector/classifier/reader controls fail. Frozen Mac554 CPU/Python/persistence +22 Metal and866 scripts complete (12 expected Mac skips). Evidence: build/renderer-evidence/pr453-ten-fixes-20261028.55Ne2l/local-compact-refactor-final/, local-metal-identity-followup-native/, local-metal-reader-followup/ and hosted/metal-probe-hosted/. Intel virtual source scope stays unchanged; an ARM virtual source route or physical runner requires the pending owner decision. Five retained cap failures and dirty Windows RHI success remain unqualified; fresh exact-SHA all60 CI and final maintenance remain required. Original checkout/index, every existing target/assertion,300/1200 limits, compiler flags, legacy default and all gates remain; upstream reconciliation stays separate. Exemption: .gitignore retains the merged generated build-r1 exclusion and two hand-maintained renderer CMake inclusions. |
| 2026-10-28 | Attest the exact owned kernel cache edge | Six-line mapper correction accepts only the owned generated PCH source and exact .pch/.gch output; ordinary .o, ownership, escape, duplicate and complete-bijection checks remain. Both native f239 Mac artifacts replay88 unique inputs/outputs; old mapper fails both new regressions. Actual tiny Ninja graph compiles every mapped output;123 source regressions pass. Frozen Mac554 CPU/Python/persistence +22 Metal and862 scripts pass (12 expected platform skips). Evidence: build/renderer-evidence/pr453-ten-fixes-20261028.55Ne2l/local-kernel-pch-followup/ and hosted/run-40753992954/kernel-pch-mapping-summary.json. Fresh all60 exact-SHA CI remains mandatory. Current upstream58f3a8e0 adds the separate AppImage floor update; full-PR plan comparison passes, reconciliation remains separate. Final maintenance precedes commit/push. Original checkout/index, legacy default, assertions,0.25px/300/1200 limits and all gates remain. Exemption: .gitignore retains the merged generated build-r1 exclusion and two hand-maintained renderer CMake inclusions. |
| 2026-10-28 | Compact shared helpers and platform-CI follow-up locally validated | Remove14 duplicate fill-adder lines; reuse the strict private-cache graph for default-OFF cold-source PCH. Repair real GTest/C++17, Windows Lua DLL, feature-guard, Mac install/origin and Snap runtime failures. Native Metal capture covers both architectures; Intel-only virtual policy/assertions stay. Frozen Mac554 CPU/Python/persistence +22 Metal and860 scripts pass (12 expected platform skips);139 geometry cases pass production ASan/UBSan, fixture ASan-only, dependencies/Mac leaks unqualified. Actual persistence-OFF447 pass; causal negatives remain. Evidence: build/renderer-evidence/pr453-ten-fixes-20261028.55Ne2l/local-ci-followup-final/. Failed5cec source run40694421566 is unqualified (34 passed,20 failed,8 ARM jobs never acquired runners); require a fresh exact-SHA all60 cohort. Final plan31/inventory64/lints/worktree/full-PR/diff checks precede commit/push. Original checkout/index, legacy default,0.25px/300/1200 bounds and all gates remain; no R2.4+ or upstream reconciliation. Exemption: .gitignore retains the merged generated build-r1 exclusion and two hand-maintained renderer CMake inclusions. |
| 2026-10-28 | Ten PR453 corrections locally validated; exact-source publication next | Four CI contracts, shared compensated affine math, explicit pattern periods, pan-independent Region certificates and locally certified bounded fill sweeps/refinement have regressions and preserved pre-fix failures. Frozen Mac553 CPU/Python/persistence and22 Metal IDs pass;851 scripts pass with12 expected platform skips. All138 path/geometry cases pass production ASan/UBSan (fixture ASan-only; dependencies/Mac leaks unqualified). Final plan/inventory/lint/diff checks repeat before commit/push, then require all60 exact-SHA source jobs. Evidence: /tmp/pr453-ten-fixes-evidence-20261028.iGl01n/ plus referenced focused controls. Original checkout/index preserved. Legacy default,0.25px/300/1200 bounds and every acceptance gate stay unchanged; no R2.4+ or new upstream reconciliation. Exemption: .gitignore carries the existing generated build-r1 exclusion and the two hand-maintained renderer CMake source inclusions from the master merge; these are not new feature or gate changes. |
| 2026-10-26 | Provisional R2.3 ellipse-plane correction locally validated; source publication next | Frozen six-path correction adds one existing coordinate validation, two native regression IDs and exact ownership guards; no new provider/plane policy. Fresh Mac/Linux ARM64 each266 CPU preserves264 and22 Metal/Vulkan passes; Linux core/sync14instances14devices0errors. Both new IDs fail against preserved pre-fix libraries; focused Mac ASan/UBSan passes all27 recorder IDs, with other dependencies/Mac leaks unqualified. Full666 scripts pass (Mac11 expected Linux skips/Linux0); exact one-file test-guard transition preserves separate Linux native/protocol freezes and unchanged compiled inputs/helpers/providers. Four own caches/167 ordinary units/eight raw C++14 headers pass. All45 unchanged reader models,12 inventory rejection probes and both fresh native readbacks pass;672 compact Linux files retain no large executables/caches. Preserve all initial source-count, preflight and evidence-harness failures, six prior seals and original checkout/index/staging. Pre-final full-plan/checker42/worktree6paths16mapped gates/inventory190/lints/diff PASS; repeat after this final receipt, then publish six paths to the existing authorized fork branch and require fresh exact-SHA all60 source jobs/artifact audits. Evidence: build/renderer-evidence/r2-3-ellipse-plane-20261026.egZvFY/. Legacy default,0.25px/300/1200 bounds, upstream deferral and all acceptance gates stay; full goal and R2.4 authority remain open. |
| 2026-10-26 | R2.3 plane-validation correction implemented; protocol ownership update pending | Reuse point(majorP) before radius/angle derivation; no planar projection or new provider policy. Two new actual native IDs cover11 invalid axes/full or partial/both directions, atomicity/recovery and signed-zero rotated planar controls. Mac fresh266 CPU+22 Metal pass; immutable pre-fix libraries fail both new IDs. Focused Mac ASan/UBSan passes all27 recorder IDs without skips, instrumenting adapter/tests only; other dependencies/Mac leaks stay unqualified. Linux native CPU/Vulkan execution passes with additional byte/object readback still pending. Full Mac666 initially fails the two obsolete source-count guards (Region15 vs16/hatch10 vs11), not native tests; preserve that failure, bind the new IDs/counts and rerun complete protocols/maintenance before source publication/full five-platform CI. Initial preflight/helper failures remain retained in build/renderer-evidence/r2-3-ellipse-plane-20261026.egZvFY/. All prior264 IDs, source bounds/assertions/defaults, upstream deferral and acceptance gates remain. R2.4 is requested, not approved; this is not full R2.3/full-goal completion. |
| 2026-10-26 | Exact PR453 source cohort closed; remaining R2.3 implementation active | Candidate7720f239/run40264405249 attempt1 passes62/62 checks/all60 tuples (12 per platform); full archive readback verifies1753 process scopes and seven new regressions. Complete40-host/20-CLI audit verifies19984 files/120 project-library loads; final-closeout-v2 binds six seals and preserved original checkout/index/staging. Preserve the initial plan self-comparison failure and the corrected actual-parent six-path proof. Evidence: build/renderer-evidence/pr453-pch-chain-20261025.wlqdWF/hosted/final-closeout-v2.json. Existing local666 scripts/264CPU/22RHI source proof stays exact-source, not acceptance. Next enforce the declared planar-provider boundary for ellipse major axes, with pre-fix negative controls, full/partial/both-direction/atomic/recovery regressions and fresh native validation in build/renderer-evidence/r2-3-ellipse-plane-20261026.egZvFY/. R2.4 executor readiness is audited, but its authorization remains pending. Prose-only sync does not stale product evidence; full plan/checker/worktree/inventory/lints/diff repeat. No default, upstream reconciliation,0.25px/time-limit or acceptance changes; full goal remains incomplete. |
| 2026-10-25 | Cache the remaining cold source-check dependency chain privately | Candidateb4dd957/run40259321120 still exhausts300 TOTAL in Windows core final compilation; actual VS layout is correct, one kernel/viewer/Lua link each, no compiler error, children retired. Preserve failure/partial passes unqualified. Extend only WITH_RENDERER_TESTS PRIVATE caches to four targets with own C++14/14/14/17/export definitions; defaults/host/library PCH-OFF and eight independent C++14 headers stay. Static444-input review is clean. Fresh Mac264/Metal22 and Linux264/Vulkan22 pass55.404s/115.467s; full666 scripts pass (Mac11 expected Linux skips/Linux0),45 reader models and two fresh strict four-cache readbacks pass. Eight generated inputs/four hash-only cache outputs/167+4 units bind;645 compact Linux files include eight tiny raw objects, no large caches. Both audit entrypoints require666 and retain the old parser-only negative control. Final plan/checker42/worktree/inventory190/lints/diff precede six-path publication and mandatory fresh all60 exact-SHA source/artifact validation. Evidence: build/renderer-evidence/pr453-pch-chain-20261025.wlqdWF/. Preserve five seals, failed/superseded proof, original checkout/index/staging,300/1200 budgets,0.25px bound, legacy default and all gates. Upstream reconciliation stays separate; no new sanitizer, merge or phase/platform/GPU/hardware/widget/package/release/full-goal acceptance. |
| 2026-10-25 | Correct Visual Studio private-cache ownership and exact Release inputs | Official CMake3.31.6 source and real configure-only multi-config fixture confirm distinct MSVC roots/other-config headers. One new regression covers exact VS layout and duplicate/foreign rejection. Fresh Mac/Linux ARM64 each264CPU/22RHI pass; source+CPU64.556s/source+CPU+Vulkan122.763s,14instances/devices0core-syncerrors. Full666 scripts pass (Mac11 expected Linux skips/Linux0);40 strict reader models/fresh native readbacks and final maintenance precede four-path publication and mandatory all60 exact-SHA source/artifact validation. Retain superseded ed7ca043/run40258405546 and all prior proof/seals; original checkout/index/staging stays untouched. Evidence: build/renderer-evidence/pr453-pch-20261025.imFiPD/vs-followup/. Eigen compile inputs/uploader/aggregate/C++ implementation,300/1200 budgets,0.25px bound, legacy default and all acceptance gates stay unchanged. Upstream reconciliation remains separate; no new sanitizer, merge or phase/platform/GPU/hardware/widget/package/release/full-goal acceptance. |
| 2026-10-25 | Private-cache candidate locally validated; six-path publication next | Fresh Mac/Linux ARM64 each264CPU/22RHI pass, preserving all IDs;84 ordinary kernel units+one cache creation and8independent PCH-free C++14 headers verify. Mac source+CPU56.187s; Linux source+CPU+Vulkan106.867s/core-sync14instances14devices0errors, not a controlled speedup claim. Full665 scripts pass (Mac11 expected Linux skips/Linux0);40 reader models and actual Clang/GNU readbacks pass. Preserve632original Linux files plus8tiny objects/8112bytes and separate failed-preflight/metadata-reader provenance; no large cache binary export. Evidence: build/renderer-evidence/pr453-pch-20261025.imFiPD/. Repeat plan/checker42/worktree/inventory190/lints/diff after receipt, commit/push only six intended paths, and require fresh all60 exact-SHA source/artifact validation. Windows cache schema/timing remains pending native proof. Preserve all prior seals/receipts, original checkout/index/staging,300/1200 budgets,0.25px bound, legacy default, upstream deferral and every gate. No new sanitizer, merge or D1–D6/R2.4+/independent/GPU/painter/phase/platform/hardware/widget/package/release/full-goal acceptance. |
| 2026-10-25 | Private Eigen cache cold-build correction active | Candidate644afb32/run40252124285 passes Intel host-state checks but Windows core/Python and Intel core final RHI compilation exhaust300 TOTAL; preceding matrix profiles pass. Actual graphs retain one aggregate and no duplicate TUs; kernel throughput varies substantially. Cache only Eigen/Dense privately on the opt-in RHI-test kernel and remove only final-source prefix draining; defaults/library/host profiles stay PCH-free and retain old prefixes. Bind C++14/private metadata, generated inputs and one compiled cache hash, reject malformed/leaked caches, and retain three narrow text-glob variants. Fresh Mac264/Metal22 and Linux264/Vulkan22/core-sync0errors pass; full665 scripts,40 reader models, maintenance and fresh all60 exact-SHA source/artifact validation remain required. Evidence: build/renderer-evidence/pr453-pch-20261025.imFiPD/. Preserve1553+112+1209+146 sealed files, all prior receipts, original checkout/index/staging,300/1200 budgets,0.25px bound and every gate. No new sanitizer, merge, D1–D6/R2.4+/independent/GPU/painter/phase/platform/hardware/widget/package/release/full-goal acceptance. Legacy default and upstream deferral remain. |
| 2026-10-25 | Host diagnostic follow-up locally validated; fresh source cohort next | Full659 scripts pass on ARM64 Mac/Linux (11 expected native-Linux skips/0 skips),108 focused source tests and the real SDK mutation reject as required; independent review confirms the same five fatal operands and existing always-upload JSON retention. Fresh coherent Qt6.12.0 Mac both/dual-python host builds and passes372 non-widget IDs with97 widget IDs unexecuted; three subsequent372-ID executions preserve every state map. Intel drift is not reproduced or attributed, and no transient classification or assertion waiver is inferred. Preserve the initial local mixed-Qt header failure separately. Evidence: build/renderer-evidence/pr453-hoststate-20261025.SQGiCD/. Prior264 CPU/22 RHI/sanitizer proof is unchanged/source-equivalent, not fresh execution by this follow-up. Final plan/checker42/worktree/inventory190/lints/diff repeat after receipt. Commit/push only runner, source regressions and both plans; require fresh all60 exact-SHA source jobs and strict retained-byte readback. Preserve1553+112+1209 sealed files, all prior receipts, original checkout/index/staging,300/1200 budgets,0.25px bound, legacy default and upstream deferral. No C++ implementation, merge or D1–D6/R2.4+/independent/GPU/painter/phase/platform/hardware/widget/package/release/full-goal acceptance change. |
| 2026-10-25 | Retain strict host-state diagnostics after Intel guard failure | Candidate6c880cf9/run40244434797's Intel both/Python tests-ON host builds successfully and passes all372 required non-widget IDs, then fails the existing final immutability comparison. All20 command receipts have expected exits, no timeout and retired children; there is no retained before-state to identify the term. Preserve this failed attempt and every passing artifact separately. Keep all five comparisons fatal; write before/after hashes/maps only for changed components and regression-test each component, simultaneous changes and clean success. Fresh native host reproduction, full local scripts/maintenance and a fresh exact-SHA all60 source cohort are required; do not assume a transient or relax assertions,300/1200 budgets or0.25px bound. Evidence: build/renderer-evidence/pr453-hoststate-20261025.SQGiCD/. Preserve1553+112+1209 sealed predecessor files, all prior receipts, original checkout/index/staging, legacy default and upstream deferral. No C++ implementation, merge or D1–D6/R2.4+/independent/GPU/painter/phase/platform/hardware/widget/package/release/full-goal acceptance change. |
| 2026-10-25 | Single aggregate graph locally validated under unchanged source deadline | Fresh Mac/Linux ARM64 each pass264 CPU/22 offscreen RHI IDs; Linux Vulkan core/sync observes14 instances/14 devices with zero errors. Seven independent CPU C++14 headers plus one RHI header pass; CPU-only/RHI-test-OFF profiles omit the aggregate. Full658 scripts pass (Linux0 skips/Mac11 expected Linux-only skips),107 source tests and28 offline reader/driver/install tests pass. Native receipts prove all four exact direct dependencies and one aggregate compile; missing/forged/drifting edges and deadline resets reject. Mac cold proof72.402s and Linux about185s remaining after compile preserve300-second TOTAL/per-command and1200-second library/host limits. Original902-input subset has901 unchanged and root CMake-only graph delta;973 Linux snapshot entries freeze. No C++ implementation/new sanitizer acceptance changes. Evidence: build/renderer-evidence/pr453-graph-20261025.irkjee/; preserve1553+112 predecessor sealed files and all prior receipts. Final plan/checker42/worktree/inventory190/lints/diff repeat after receipt. Superseded40240293620 retains52 source successes, one timeout and seven cancellations, not qualification. Publish only five intended graph/test/plan paths and validate/read back fresh all60 exact-SHA source tuples. Preserve0.25px bound, original checkout/index/staging, legacy default, upstream deferral and all gates. No merge or D1–D6/R2.4+/independent/GPU/painter/phase/platform/hardware/widget/package/release/full-goal acceptance. |
| 2026-10-25 | Compact Windows source-check graph correction active | Candidate590a79e8 run40240293620's Windows core/tests-ON final RHI build times out, not a compiler/link failure: four MSBuild invocations rebuild shared dependencies three times. Add one opt-in non-default aggregate for the same four native/header targets and verify its actual File API dependency closure before/after execution. Missing-dependency/deadline regressions and fresh native/all-five source validation are required. Preserve final source300-second TOTAL/per-command deadline,1200-second library/host profiles, all native IDs/header objects/assertions and0.25px bound; no prebuild escapes the timer. Evidence: build/renderer-evidence/pr453-graph-20261025.irkjee/. Partial prior passes are not qualification; preserve immutable predecessors and all prior receipts, original checkout/index/staging, legacy default and every acceptance gate. Upstream reconciliation stays separate; no merge or D1–D6/R2.4+/independent/GPU/painter/phase/platform/hardware/widget/package/release/full-goal acceptance. |
| 2026-10-25 | Native discovery/Vulkan settings retention fix locally validated | Two new upload globs and two actual-file/glob regressions prove native discovery/settings bytes are retained and the old filter fails; unchanged R0 directory uploads already suffice. Focused99/actionlint/full650 scripts pass per ARM64 host (Linux0 skips/Mac11 expected native-Linux skips). Warm source-equivalent264 native per host/270 Mac persistence/172 Linux sanitizer IDs pass using original hash-bound binaries, not new compilation. Original902 compiled/test inputs,869 production/host inputs,13 helpers and1553 indexed proof files remain unchanged. Final plan/checker42/worktree/inventory190/lints/diff repeat after receipt. Evidence: build/renderer-evidence/pr453-retention-20261025.k3Z7iF/. Retain40238006451's28 successful/32 cancelled source jobs and cancellation-dependent aggregate failure as incomplete, separate from the earlier cancelled manual run. Keep strict missing-byte failures; waive no assertions/evidence,0.25px bound or300/1200 budgets. Commit/push four workflow/test/plan paths on the authorized branch, wait for automatic push CI and read back all60 exact-SHA source tuples. Preserve all prior receipts, original checkout/index/staging, legacy default and every acceptance gate; upstream86-conflict reconciliation stays deferred. No merge or D1–D6/R2.4+/independent/GPU/painter/phase/platform/hardware/widget/package/release/full-goal acceptance. |
| 2026-10-25 | Correct source-CI native discovery and Vulkan settings retention | Committed/pushed4c5b9e97; delayed push40238006451 supersedes cancelled manual40237422163, whose partial evidence remains separate. Local native264/270, sanitizer and648-script proof stays immutable and CPU/Lua code unchanged. Strict artifact readback exposes omitted discovery.txt and vk_layer_settings.txt; narrow upload-filter correction plus actual-retainer regression tests and fresh local/protocol/full60 source validation are required. Preserve missing-byte failures and original indexed readers; source-derived raw-DXF inventory/Windows path-reader corrections remain diagnostic, not native assertion changes. No0.25px bound,300/1200 budget or byte check is waived. Evidence: build/renderer-evidence/pr453-retention-20261025.k3Z7iF/; predecessor build/renderer-evidence/pr453-fixes-20261025.QEEQlr/. Preserve all prior receipts, original checkout/index/staging, legacy default and every gate; upstream86-conflict reconciliation stays explicitly deferred. No merge or D1–D6/R2.4+/independent/GPU/painter/phase/platform/hardware/widget/package/release/full-goal acceptance. |
| 2026-10-25 | PR453 focused correction locally validated; five-platform source cohort next | Mac/Linux ARM64 each264 core IDs preserve257; Mac270 persistence IDs pass with fresh pinned source SDK. Seven C++14 headers, five strict CPU sources/adapter C++14, SDK-free links and independent20-path review pass. Linux172 ASan/UBSan geometry/Lua IDs/leaks and Mac170 geometry sanitizer IDs pass; kernel/provider instrumentation and Mac leaks stay unqualified. Four old-geometry failures/two missing-bootstrap controls reject; sorted gradient1000 allocations become0 with checksum unchanged. Linux648 scripts pass without skips, Mac648 runs with11 expected native-Linux skips; plan/checker42/worktree/inventory190/lints/diff repeat after receipt. Retain temporary harness setup/cwd/File API failures and superseded pre-format proof; final902-file Linux freeze is exact. Evidence: build/renderer-evidence/pr453-fixes-20261025.QEEQlr/. Publish only20 correction paths on the authorized fork branch, then execute/read back all60 source tuples at the exact candidate SHA. Shared CI preparation keeps assertions,0.25px bound and300/1200 budgets unchanged. Preserve all prior receipts, original checkout/index/staging, legacy default and every acceptance gate. Upstream86-conflict reconciliation remains explicitly separate; no merge or D1–D6/R2.4+/independent/GPU/painter/phase/platform/hardware/widget/package/release/full-goal promotion. |
| 2026-10-25 | Focused PR453 fixes locally validating; upstream integration deferred | Human explicitly leaves86-conflict upstream reconciliation separate. Trial merge aborted in isolated checkout; no upstream edits enter candidate. Mac core264/264 preserves257 native IDs, adding two Python-OFF/non-widget Lua cases and five geometry/gradient cases; seven C++14 headers, strict source builds and SDK-free viewer links pass. Geometry/native/sanitizer and missing-bootstrap controls retain pre-fix failures. Evidence: build/renderer-evidence/pr453-fixes-20261025.QEEQlr/. Linux/full local maintenance and fresh all-five source CI pending. Preserve prior receipts, original checkout/index/staging, legacy default,300/1200 budgets and every acceptance gate. No upstream merge, D1–D6/R2.4+/phase/platform/hardware/widget/package/release/full-goal promotion. |
| 2026-10-25 | PR453 review corrections active; acceptance unchanged | Human authorizes fixes/regressions/local checks/commit/push/all-five source CI. Correct unstable miter certificates, connected-spline dashes and installed Lua require paths; keep independent regressions and Python-OFF/non-widget coverage. Compact shared CI/provider preparation without relaxing assertions,300-second command/1200-second profile budgets or0.25px bound. Preserve all prior rows, original checkout/index/staging and legacy default. Evidence: build/renderer-evidence/pr453-fixes-20261025.QEEQlr/. The old Region cohort has one Intel cold-kernel failure; fresh local/hosted proof is pending. Isolated86-conflict upstream reconciliation awaits scope choice. No D1–D6/R2.4+/phase/platform/hardware/widget/package/release/full-goal acceptance. |
| 2026-10-25 | Actual CAD Region candidate locally validated; source cohort next | Shared bounded CAD recorder produces owned physical Region boundaries with first-edge winding, unique graph reorder/reversal, exact curve control reversal and roundoff-only joins. Error certificates feed fill topology clearances and tile bounds before preparation; combined recording/preparation work/storage caps fail atomically. Native Mac/Linux ARM64 each257 core IDs preserve242,263 persistence IDs include all63 provider tiles plus independent raw DXF two-loop/area300/hole import,165 focused sanitizer cases, seven C++14 headers/five strict CPU sources/adapter C++14 project flags/SDK-free links, three build IDs/five corruption negatives pass. Six curve-reversal/gap-bridge/topology-tube/certificate/work/camera mutants reject;13 Intel objects are compile-only. Six production sources/new fixtures ASan+UBSan; old RapidJSON fixture ASan-only, kernel/provider uninstrumented, Linux leaks pass/Mac leaks unqualified. Clean exact23-file-overlay Linux scripts634/634 and full-plan/checker42/worktree/inventory190/lints/diff repeat after receipt. Preserve84 renderer/398 Python prior rows,1097 baseline files/1088 unrelated files, original checkout/index/staging and sealed109-file hosted/10246-file local predecessor packets. Initial native reports preceded the added import regression and Linux stale-source checks rejected reuse; final source-closed21-hash reruns pass. Retain one negative-harness executable/directory name-collision failure; corrected harness compiles/links all six controlled mutants and rejects each without relaxing native assertions/timeouts. Evidence: build/renderer-evidence/r2-3-region-20261025.oGhvya/. Commit/push only this whitelist to the authorized fork branch and run/read back fresh five-platform source CI. Broader provider/plane/pattern-width/topology/numeric domains remain; no R2.4+ authority, legacy-default or D1–D6/independent/GPU/painter/phase/platform/hardware/widget/package/release/full-goal acceptance changes. |
| 2026-10-25 | Actual CAD Region continuation started after verified tile source CI | Previous candidate8a5e21f0/run40199328429 passes62/62 with all60 source artifacts read back; local documentation closeoutf36b0534 stays unpushed to preserve exact green remote source. Verify sealed109-file hosted packet/20 source hashes and1097-file baseline, original checkout/index/staging and83 renderer/397 Python prior rows/all gates. Implement owned Region recording using shared curve capture, deterministic first-edge winding and unique endpoint graph stitching with bounded roundoff joins, curve reversal and shared work/storage caps. Propagate certified input error into topology clearance and tile culling, not only output annotations. Native real-Region ownership/order/hole/curve/bulge/spline/affine/large-origin/dash/gradient/budget/invalid-input tests and fresh source validation are next. Evidence: build/renderer-evidence/r2-3-region-20261025.oGhvya/. Preserve all previous proofs; no R2.4+ authority, legacy-default or D1–D6/GPU/painter/phase/platform/hardware/widget/package/release/full-goal acceptance changes. Broader provider/width/plane/topology/numeric obligations remain visible. |
| 2026-10-25 | Actual CAD tile candidate five-platform source CI closed out; Region next | Published8a5e21f0160f91bd990a40e52d3d0c4dcae53709/run40199328429 attempt1 passes62/62 with60 source tuples/12 per architecture. All60 ZIPs/CRCs/digests/source/helpers/headers/File API/XML/native IDs/process/provider records are retained/read back:242 CPU IDs preserve229;40 library profiles242/340,20 tests-on hosts340 CLI/349 others; all63 bundled finite provider tiles execute on persistence hosts. Diagnostic51198 triangles on ARM64/Linux x64 versus51196 Intel Mac/Windows is not a bit-identical topology contract; preserve the initial overstrict artifact-reader failure and exact platform diagnostic correction, with no native assertion/error-bound/timeout changes. Seven C++14 headers,14 RHI runs/22 IDs, two independently matched virtual Intel Metal source attestations and51/51 selected CPU R2 bindings pass;76 GPU R2 IDs and97 discovered/unexecuted widget IDs per UI host remain separate. Preserve immutable10246-file local proof,82 renderer/396 Python prior rows,1097 publication-baseline files/original checkout/index/staging and every gate. Full-plan/checker42/worktree/inventory190/lints/diff repeat after receipt. Evidence: build/renderer-evidence/r2-3-tiles-publication-20261025.0Lf7J6/. Keep remote at exact tested8a5e21f0, commit documentation closeout locally without duplicate CI/skip directives, then implement/validate actual CAD Region recording and bounded numeric joins. Broader provider/width/plane/topology/numeric obligations remain; no R2.4+ authority, legacy-default, D1–D6/independent/GPU/painter/phase/platform/hardware/widget/package/release/full-goal acceptance changes. |
| 2026-10-25 | R2.3 actual finite CAD tile preparation locally validated; source publication next | Bounded CPU prepareTiledHatch repeats actual owned primitives at extent periods with affine model-before-camera placement, compensated fractional large-origin phase, whole-primitive physical/model dashes, diameter points, curves/seams and finite-width mask/ink coverage. Per-call base meshes, shared input/state/work/output/cell/instance caps and atomic diagnostics retain union-before-OVER semantics. Mac/Linux ARM64 each242 core IDs preserving229,247 persistence IDs including all63 normalized provider tiles/51198 finite ink triangles,150 focused sanitizer cases, seven C++14 headers/five strict CPU sources/adapter project-flags C++14/SDK-free links, three build IDs/five corruption negatives pass;13 Intel objects are compile-only. Six production consumers/new fixtures ASan+UBSan; old RapidJSON fixture ASan-only, kernel/provider uninstrumented; Linux leaks pass/Mac leaks unqualified. Clean isolated scripts633/633 and five source mutations (period, FMA phase, dash restart, transform order, cell cap) are checked; final source-frozen native/sanitizer/script/maintenance reruns bind20 source hashes after strengthening the independent phase oracle. Retain fixture/setup failures, initial double-oracle seam miss, insufficiently amplified mutation oracle and stale-source detection; no assertion/timeout relaxation. Preserve81 renderer/395 Python prior rows,1095-file baseline/1088 unrelated files, original checkout/index/staging and immutable prior packets. Evidence: build/renderer-evidence/r2-3-tiling-20261025.XLpiA3/. Commit/push the explicit candidate whitelist to the authorized fork branch, then run/read back the complete five-platform cohort; new source cannot inherit4b36697c's pass. Real Region conversion and broader provider/topology/numeric domains remain required. Legacy default and all gates stay open; no R2.4+ authority, GPU/painter/independent/phase/platform/hardware/widget/package/release/full-goal acceptance. |
| 2026-10-25 | R2.3 actual tiled geometry continuation started | Previous turn is progress: owned CAD snapshots and model-correct full-ellipse closure,229/233 native/137 sanitizer per ARM64 host, real63-pattern/11631-entity/two-point provider proof and632 clean scripts. Revalidate7314 files/18 source hashes and1095 baseline files; preserve original branch/HEAD/index/staging,80 renderer/394 Python rows and all gates. Implement bounded repeated tile geometry with inherited dash phase, affine placement, large-origin rebasing, shared resource caps and finite-width mask coverage; independent analytic/precise coverage and real catalog preparation checks are next. Evidence: build/renderer-evidence/r2-3-tiling-20261025.XLpiA3/. No new source publication or inherited all-five pass; actual Region conversion, broader domains, R2.4+ authority/prerequisites and full-plan obligations remain. Legacy default and all acceptance gates stay open. |
| 2026-10-25 | R2.3 real CAD tile input locally validated; whole goal incomplete | New owned HatchTile recording preserves model-local curves/bulges/diameter points, inherited hatch style and outer transform, with aggregate recorder caps and explicit unsupported-input diagnostics. Borrowed const spline-span access bounds provider reads; no geometry/kernel rewrite. Mac/Linux ARM64 each229 core IDs preserving221,233 persistence-profile IDs including real63-pattern/11631-entity/two-point loads,137 sanitizer cases (five consumers/new fixture ASan+UBSan; old RapidJSON fixture ASan-only; kernel/provider uninstrumented; Linux leaks pass/Mac leaks unqualified), seven C++14 headers/four strict CPU sources/adapter project-flags C++14/SDK-free links, three build IDs/five corruption negatives pass.12 Intel objects are compile-only; clean exact-source Linux scripts632/632 pass. Retain initial constructor/budget-test setup failures, full-ellipse seam regression failing pre-fix, and directional tolerance-test correction; final frozen model-qualified reruns pass. Full-plan/checker42/worktree/inventory190/lints/diff repeat after receipt. Evidence: build/renderer-evidence/r2-3-pattern-20261025.yDUe2g/. Preserve79 renderer/393 Python prior rows, original checkout/index/staging and immutable127-file R2.3 hosted packet/5720-file local packet; unrelated baseline/source/assets stay unchanged. No commit/push/new hosted CI for this incomplete recording-only batch: next bounded tile expansion/clipping/dashes and actual Region conversion; broader provider/topology/numeric domains remain required. Legacy default and all gates stay open; no R2.4+ authority, GPU/painter/Python ownership/release/phase/platform/hardware/widget/package/full-goal promotion. |
| 2026-10-25 | R2.3 real-provider completeness continuation started | Previous goal turn is progress: code4b36697c and62/62 source CI with60 verified artifacts, local documentationc78bc8a9. Revalidate127 publication files,13 source hashes,1092 baseline files and original checkout/index/staging. Structured inventory of63 bundled DXFs finds11588 LINE/40 ARC/one LWPOLYLINE/two POINT records; this is record inventory, not provider-load/render proof. Current line-family candidate cannot reproduce finite tile geometry. Implement bounded owned CAD Region/Pattern recording to feed real tiled preparation, without dropping points/curves or silently narrowing F4. Next prove native geometry/style/ownership/budget/invalid-provider behavior, then actual63-provider loads and tile expansion/dashes; R2.4+ retains its authority/prerequisites. Evidence: build/renderer-evidence/r2-3-pattern-20261025.yDUe2g/. Preserve78 renderer/392 Python prior rows, immutable source packets and all gates; legacy default unchanged. Fresh code cannot inherit old source-CI passes; no acceptance/full-goal promotion. |
| 2026-10-25 | Provisional R2.3 five-platform source CI closed out; acceptance open | Published4b36697c249d4201b8ad9932bd0554a760f411f5/run40163765536 attempt1 passes62/62:60 source tuples,12 per architecture. All60 ZIPs/CRCs/digests/source/helpers/headers/File API/XML/native IDs/process/provider records are retained/read back:221 CPU IDs per architecture preserving200;40 library profiles221/319,20 tests-on hosts319 CLI/326 others; seven C++14 headers,14 RHI source runs/22 IDs, two independently matched virtual Intel Metal source attestations. All-five CPU R2 binding is51/51 (48 geometry+3 build);76 GPU R2 IDs remain unproved. UI97 widget IDs per UI host are discovered, not executed. Payload audit verifies60 source maps,2356 process records,98 ordinary XML/52 expected negatives/13162 retained hashes/30 pinned f7debf4c source SDKs with unchanged three patches. Preserve immutable5720-file local proof,77 renderer/391 Python prior receipts,1090 other publication-baseline files, original checkout/index/staging and every gate. Full-plan/checker42/worktree123 paths/16 gates/inventory190/lints/diff repeat after this receipt. Evidence: build/renderer-evidence/r2-3-publication-20261025.8wIw2W/. Keep remote at exact green source4b36697c; commit documentation-only closeout locally without duplicate CI/skip directives. Legacy stays default; no source/assertion/timeout/provider/dependency/Python ownership, merge/new PR/release-workflow/R2.4+ or D1–D6/independent/GPU/phase/platform/hardware/widget/deployment/package/release/full-goal promotion. Broader numeric/topology domains, arbitrary CAD/dashed hatch adapters and actual painter/executor integration remain open. |
| 2026-10-25 | Locally validated R2.3 candidate published; hosted source CI started | Separate-index commit4b36697c249d4201b8ad9932bd0554a760f411f5 changes exactly seven intended paths, parent4e8bb205; original fix/msvc-eigen-include HEAD/index/staging and unrelated files remain intact. Non-force push to dxli/LibreCAD_3:codex/r1-platform-ci automatically starts run40163765536 once:60 source tuples/five architectures plus protocol/aggregate. No duplicate dispatch, merge, new PR or release-workflow change. The immutable5720-file local packet binds221 native/129 sanitizer per ARM64 host, all51 CPU R2 IDs,631 clean scripts, seven headers/four sources,11 Intel compile-only objects and six fixture negatives; GPU pixels are not proved. Preserve76 renderer/390 Python prior receipts and all gates. Evidence: build/renderer-evidence/r2-3-publication-20261025.8wIw2W/. Finalized maintenance repeats; hosted completion/full artifact audit pending. Legacy stays default, Intel verified virtual Metal remains source-only; D1–D6/R2.4+/independent/GPU/phase/platform/hardware/widget/package/release/full-goal gates remain open. |
| 2026-10-25 | Provisional R2.3 candidate locally validated; source publication next | Mac/Linux ARM64 each pass221 CPU IDs preserving200,129 focused sanitizers, seven independent C++14 headers/four strict sources/SDK-free links and separate three build IDs/five corruption negatives; Linux leaks pass, fixture ASan-only RapidJSON/Mac leak limits stay explicit. All51 CPU R2 IDs bind48 geometry+3 build, including six fill proposals;76 GPU IDs remain separate. Eleven Intel objects are compile-only. Exact-source clean Linux scripts631/631; six copied fixture mutants reject then restore. Independent multiprecision winding/area/overlap grids, analytic curved landmarks, finite-width hatch mask and affine/numeric/topology/resource/gradient ownership tests pass. Retain initial conic failure, exclusion-citation rejection and two source-guard rejections after a comment-only header edit; final frozen-source audited reruns pass. Replace library-dependent gradient sort budget with explicit bounded stable insertion sorting before final reruns. Full-plan/checker42/worktree123 paths/16 gates/inventory190/lints/diff repeat after this receipt. Evidence: build/renderer-evidence/r2-3-cpu-20261025.W4jcrE/. Preserve75 renderer/389 Python prior receipts,1085 other baseline files, original checkout/index/staging and all prior proof/gate states. Publish only intended paths via separate index to authorized fork branch, then full five-platform source CI. No default/assertion/timeout/provider/dependency/Python ownership, merge/release-workflow/R2.4+ or D1–D6/independent/GPU/phase/platform/hardware/widget/package/release/full-goal promotion. |
| 2026-10-25 | R2.3 local validation active; numeric and coverage boundaries explicit | Independent fixtures and winding/grid/area/overlap, finite-width hatch coverage and owned gradient tests exercise provisional preparation. Initial conic scan-level failure retained; roundoff-scale level canonicalization is charged to the same position budget and cannot erase real area. Exact bounded-double orientation signs replace epsilon winding decisions. Full-plan/checker42 pass; initial worktree exclusion-citation failure is retained and corrected in synchronized newest receipt. Source-bound native/sanitizer/header/link/build/Intel/clean-script/final maintenance checks and full hosted cohort remain pending. Evidence: build/renderer-evidence/r2-3-cpu-20261025.W4jcrE/. Preserve74 renderer/388 Python prior rows and immutable R2.2 proof; legacy stays default and all acceptance gates stay open. No production painter, D3/R2.4+ or GPU/platform/hardware/widget/package/release/full-goal acceptance. |
| 2026-10-25 | Provisional R2.3 explicitly authorized; implementation active | Human requests fills/hatches, local validation, commit/push and five-platform source CI on the existing fork branch. Preserve legacy default and all acceptance gates. Verify237 hosted/3291 local R2.2 evidence files,1091 baseline files and original checkout/index/staging; parent4e8bb205 remains local until locally validated source publication. Private even-odd/nonzero simple-contour band decomposition and bounded affine solid-line hatch families are candidate policy, not D3 ratification. Crossing/touching/retraced/uncertain domains fail closed; arbitrary CAD/dashed adapters and production executor remain obligations. Six CPU bindings, independent geometry, gradients, native/sanitizer/header/link/Intel/script/maintenance checks and hosted CI pending. Evidence: build/renderer-evidence/r2-3-cpu-20261025.W4jcrE/. Preserve73 renderer/387 Python prior receipts and all gate ledger states; no R2.4–R7, GPU/phase/platform/hardware/widget/package/release/full-goal promotion. |
| 2026-10-25 | Provisional R2.2 five-platform source CI closed out; acceptance open | Published17fdc87893da5c4f962c0cdadd8b86e933cf8818 has run40146772555 attempt2 green62/62 latest checks:60 source tuples/12 per platform, initial59 successes plus one unchanged-source/budget Intel CLI/OFF retry and dependent aggregate. Preserve all73 indexed first-attempt files including timeout; bind60 carried job aliases to identical original execution timestamps/runner/steps, not new executions. All60 ZIPs/CRCs/digests/source/helper/header/File API/XML/native IDs/process/provider records read back:200 CPU IDs per architecture (190 preserved),40 library test profiles (200/298),20 tests-on hosts (298 CLI/305 others), seven C++14 headers,14 RHI runs/22 IDs, two independently matched virtual Intel Metal source attestations,98 ordinary XML/52 expected negatives/2356 process records/13162 retained hashes/30 f7debf4c source SDKs with unchanged three patches. Widgets97 per UI host remain unexecuted. All-five CPU R2 ownership45/51 retains six R2.3 fill proposals;76 GPU R2 IDs are not proved. Full-plan/checker42/worktree122 paths/16 gates/inventory190/lints/diff repeat after this receipt. Evidence: build/renderer-evidence/r2-2-publication-20261024.gGE6mU/. Preserve72 renderer/386 Python prior rows, immutable3291-file local proof,1089 other publication-baseline files, original checkout/index/staging and all gate states. Keep remote at exact green source17fdc878; commit the documentation-only closeout locally as in R2.1, without duplicate CI/skip directives. No assertions/timeouts/provider/default/source changes, merge/new PR/release-workflow/R2.3–R7/D1–D6/independent/phase/platform/hardware/widget/deployment/package/release/full-goal promotion. Legacy remains default; broader numerical domains and original later-stage prerequisites remain open. |
| 2026-10-25 | Hosted Intel cold-kernel timeout retained; cohort continues | Attempt1 run40146772555 has one failed cli/MAC-X64/tests-OFF job121029691082: library-lckernel times out at300 seconds after86/91 Ninja steps, no compiler error, before R2.2 viewer compilation. Retain failed job log, artifact12582393078/ZIP digest2428cb1accd2b39a08579edf5b707c770894a10bfd5d6f5d0b16b1e8e2125900 and all successful same-SHA evidence. Exact source remains17fdc878; other jobs continue. GitHub rejects the early job rerun; wait for cohort completion and archive all records before a bounded same-SHA failed-job retry. No time/assertion/provider/default/source changes, cancellation of live jobs or success claim. Evidence: build/renderer-evidence/r2-2-publication-20261024.gGE6mU/failure-attempt1/. Preserve71 renderer/385 Python prior rows, local3291-file proof and all gates; repeat final maintenance after resolved cohort. Legacy remains default; full source and acceptance closure remain pending. |
| 2026-10-24 | Locally validated R2.2 candidate published; hosted source CI started | Commit17fdc87893da5c4f962c0cdadd8b86e933cf8818 adds13 intended candidate paths via a separate index; original fix/msvc-eigen-include HEAD/index/staging remain intact. Non-force push to dxli/LibreCAD_3:codex/r1-platform-ci triggers run40146772555 once, with60 source tuples across Linux x64/ARM64, Windows x64 and Mac ARM64/Intel plus protocol/aggregate jobs. No duplicate dispatch, merge, new PR or release-workflow change. Local3291-file proof packet remains immutable/source-bound:200 native/108 sanitizer per ARM64 host,630 clean Linux scripts, seven headers/three sources, Intel compile-only, distinct corrected-Cairo geometry and negative checks. Preserve70 renderer/384 Python prior receipts and all gates. Evidence: build/renderer-evidence/r2-2-publication-20261024.gGE6mU/. Hosted completion/artifact audit pending; legacy stays default, Intel verified virtual Metal is source-only, hardware/D1–D6/R2.3–R7/widget/package/release/full-goal gates stay open. Repeat finalized maintenance checks; no hosted success inferred from push. |
| 2026-10-24 | Selected diameter/continuity candidate locally validated | Owner-selected independent diameter points preserve current path/state/order, physical/model geometry, zero no-ink and bounded physical major axis. Inner normal fans and internal-cusp continuity preserve explicit outer styles/dash caps. Fresh Mac/Linux ARM64 each pass200 native IDs (190 preserved),108 sanitizers, seven C++14 headers/three strict sources/SDK-free links, separate three build IDs/five corruption negatives; Linux leaks pass, fixture ASan-only RapidJSON/Mac leak limits stay explicit. Ten Intel objects are compile-only. Exact-code clean Linux scripts630/630 without skips. Six corrected regressions reject pre-fix geometry; three point fixture mutations reject then restore. Cairo-only648 comparison retains three intended differences; independent sector-corrected648/693002 stable plus12670 separate at0.09 explicitly checks those three, curved108/114954 plus2658 separate at0.25 unchanged. No GPU pixels. Full-plan/checker42/worktree122 paths/16 gates/inventory190/lints/diff pass and repeat after receipt. Evidence: build/renderer-evidence/r2-2-continuity-20261024.fjsO85/. Preserve69 renderer/383 Python prior rows,1080 other baseline files, original checkout/index/staging and all indexed proof/gate states. Publish using separate index to authorized dxli/LibreCAD_3:codex/r1-platform-ci, then full five-platform source CI; no merge/release-workflow or D1–D6/R2.3–R7/independent/phase/platform/hardware/widget/package/release/full-goal promotion. Legacy stays default. |
| 2026-10-24 | Owner-selected point/continuity implementation started | Human selects fixture diameter and corrected inner-join/intra-curve-cusp continuity; requests local validation, then commit/push and full five-platform source CI on dxli/LibreCAD_3:codex/r1-platform-ci. Preserve explicit caps/outer joins, aggregate atomic caps, 0.25px budget and legacy default. No merge/release-workflow changes or acceptance promotion. Prior32 publication-authority files verify; snapshot1091 baseline files/11 intended paths,68 renderer/382 Python prior receipts and original checkout/index/staging. Evidence: build/renderer-evidence/r2-2-continuity-20261024.fjsO85/. Fresh independent geometry, native/sanitizer/header/link/Intel/protocol checks and hosted cohort pending. D3 defaults/hairline, D1–D6, R2.3–R7, hardware/widget/package/release/full-goal gates remain open. |
| 2026-10-24 | Conditional R2.2 publication and full source CI approved | Human reply “Allow R2.2 commit, push and CI (Recommended)” authorizes the provisional CPU curve/stroke candidate in dxli/LibreCAD_3:codex/r1-platform-ci after local R2.2 checks pass and point-size/continuity choices are resolved. Record the approval once; do not ask for publication permission again within that scope. The two policy answers are still absent, so no commit/push/dispatch occurs yet. Legacy stays default, all acceptance gates stay open, and merge/release-workflow changes are not authorized. Only these two plans change; all1731 readiness files/12 source hashes and original branch/HEAD/index/staging are verified. Preserve67 renderer/381 Python rows,1089 other baseline files and all prior proof/gate states. Evidence: build/renderer-evidence/r2-2-publication-approval-20261024.9URggv/. Repeat full-plan/checker42/worktree122 paths/16 gates/inventory190/lints/diff and history/source-binding checks after this finalized receipt. No fresh source tests or all-five success is claimed;190/98/629/Intel/Cairo/build-contract evidence stays in its source-bound packets. No policy/default/dependency/assertion/timeout/provider/Python ownership/R2.3–R7/D1–D6/independent/phase/platform/hardware/widget/deployment/package/release/full-goal promotion. |
| 2026-10-24 | Full-scope readiness audit and separate build bindings verified | Codex implementation/self-review: fresh existing three-ID build-contract runner passes per Mac/Linux ARM64 host, including five controlled corruption rejections, source/helper/fixture/trace/File API/native XML/binary/artifact binding. The unchanged native190-ID proof contains39 selected CPU R2 bindings; disjoint fresh build IDs add3, totaling42/48, with six explicit unimplemented R2.3 fill IDs. Point has no CPU preparation/binding; RHI R2 selects76 IDs that CPU proof does not certify. Add a requirement/owner/evidence/next-gate matrix retaining numerical-domain, point/continuity/default/hairline, fill/executor/cache, all-five/platform/package/application/cutover/retirement obligations. Fresh Cairo1.18.6 state/membership plus source inspection confirms candidate initial width1 versus Cairo2 and one-pixel zero-effective-width versus no ink; butt/miter/limit10 agree, radius/diameter differs. Scope is library/source observations, not project-factory or GPU pixels; no behavior changed. Correct stale section5 provisional-stage and pinned-font-input prose without promoting D3/font runtime acceptance. Retain initial sandbox-denied Mac native-ISA guard and probe header-path compile failure; corrected setup reruns preserve the exact guard/assertions/budgets. Full-plan/checker42/worktree122 paths/16 gates/inventory190/lints/diff pass and repeat after finalized receipt. Evidence: build/renderer-evidence/r2-2-readiness-20261024.fxBwcx/. Preserve66 renderer/380 Python prior rows,1089 other baseline files, original branch/HEAD/index/staging and all1607/1583/1601/4521/3021/2677/163/2166/1292/104 prior proof files/gates. Only two plans change;190/98/629/Intel/Cairo source results retain their original packet, not fresh execution. Point/continuity/publication choices are unanswered; further implementation requires those choices and original stage prerequisites. No source/dependency/default/assertion/timeout/provider/Python ownership, commit/push/hosted CI/R2.3–R7/D1–D6/independent/phase/platform/hardware/widget/provider/deployment/package/release/full-goal promotion. |
| 2026-10-24 | R2.2 completion/readiness audit started | Previous turn is progress; all1607 monotone-packet files and12 current source hashes verify. Derive remaining obligations from the complete plan, not the passing CPU subset. CPU R2 selection requires48 IDs:39 appear in lcunittest, three build-option IDs belong to separate lcbuildcontracttest, and six fill IDs are unimplemented R2.3 proposals. Refresh the existing three-case native build-contract runner on Mac/Linux ARM64 and bind the union without calling a CPU result GPU acceptance. Audit point/continuity/numerical-domain/publication/native-platform and R0–R7 gates explicitly; no authority for unanswered questions is inferred. Preserve65 renderer/379 Python prior rows,1091 baseline files, original checkout/index/staging and all gates. Evidence: build/renderer-evidence/r2-2-readiness-20261024.fxBwcx/. Only the two plans change; current190/98/629/Intel/Cairo proofs remain exact-source history, not new execution. Fresh build-contract and finalized maintenance audits are pending. No implementation/default/dependency/assertion/timeout/provider/publication/hosted-CI/later-stage or acceptance promotion. |
| 2026-10-24 | Proven monotone R2.2 domain expanded and locally verified | Codex implementation/self-review: exact coordinate equality admits axis-aligned lines without exact global-offset subtraction; ordered control polygons avoid needless roots; exact normalized Bernstein derivative products use strict rounded-product/FMA-residual ordering to prove no reversal. Power-of-two normalization round-trips, ties prove nothing, and ambiguous reversals/phase topology still fail closed. No production multiprecision/long-double assumption/dependency or epsilon sign. Initial three ordinary regressions fail pre-fix; final FMA regression expands to96 axis/sign/scale/neighbor tuples with independent multiprecision positivity, coverage and adjacent true-reversal rejection. Retain initial/expanded fast source identities; only final native/sanitizer/clone hashes certify current code. Fresh Mac/Linux ARM64 each pass190 CPU IDs (187 preserved),98 sanitizer cases, seven C++14 headers/three strict sources/Qt-GPU-Python-Cairo-free links; Linux leaks pass, fixture ASan-only RapidJSON/Mac leak limits remain. Ten verified x86_64 Mach-O objects give Intel compile-only proof, not runtime/deployment/hardware. Exact-code Linux scripts629/629 without skips. Fresh Cairo unchanged linear648/693005 stable plus12667 separate at0.09; curved108/114954 stable plus2658 separate at0.25; CPU membership is not GPU pixels. Full-plan/checker42/worktree122 paths/16 gates/inventory190/lints/diff pass and repeat after finalized receipt. Twelve source hashes bind native/sanitizer/clone/current files. Evidence: build/renderer-evidence/r2-2-monotone-20261024.3z7TMb/. Preserve64 renderer/378 Python prior rows,1086 other baseline files, original index/checkout/staging and all1583/1601/4521/3021/2677/163/2166/1292/104 prior proof files/gates. Point/continuity and now separate publication/source-CI authorization remain pending owner choices. No default/GPU/public painter API/Python ownership, work/vertex/geometry/assertion/timeout/provider, publication/new hosted CI/R2.3–R7/D1–D6/independent/phase/platform/hardware/widget/provider/deployment/package/release/full-goal promotion. Broader valid domains and all-five source proof remain required; this is not narrower R2.2 completion. |
| 2026-10-24 | R2.2 monotone numerical-domain continuation started | Previous turn is progress; all1583 work-cap packet files verify. Audit identifies valid monotone collinear cubics rejected by the conservative near-critical discriminant guard. Reproduce using ordinary tests and independent multiprecision derivatives, then certify monotone control polygons or exact normalized derivative products via strict rounded-product/FMA-residual ordering, without extended precision/new production dependencies. Axis-aligned collinearity can be proved from coordinates even when an offset subtraction rounds. Keep truly ambiguous reversals/phase topology rejected, same work/vertex/0.25-pixel limits and pending point/continuity choices. Preserve63 renderer/377 Python prior rows,1091 baseline files, original checkout/index/staging and gates. Evidence: build/renderer-evidence/r2-2-monotone-20261024.3z7TMb/. Fresh pre-fix/native/sanitizer/independent-oracle/source/header/link/Intel-compile/script/finalized maintenance proof is pending. No GPU/default/publication/hosted CI/R2.3–R7 or acceptance promotion. |
| 2026-10-24 | R2.2 work cap corrected and locally verified | Codex implementation/self-review: flatten now preflights input size and charges every move/line/close/curve element plus subdivision nodes against the same existing work cap. Retain unchanged new regression failing pre-fix (closed line accepts work2/visited0); corrected code rejects at2 and passes at3/visited3. Move-only/empty and mixed-curve exact-cap/atomic recovery cases pass. Fresh Mac/Linux ARM64 each pass187 native CPU IDs (185 preserved),95 sanitizer cases, seven C++14 headers/three strict sources/Qt-GPU-Python-Cairo-free links. Linux leaks pass; fixture ASan-only RapidJSON/Mac leak limitations remain. Seven headers/three sources additionally produce ten verified Mach-O x86_64 objects using installed Apple Clang/SDK, inheriting only existing configured header compatibility flags; production remains strict. Compile-only is not Intel native/runtime/deployment/hardware/hosted-CI acceptance. Exact-code clean Linux scripts628/628 without skips. Fresh Cairo retains648/693005 stable linear plus12667 separate at0.09 and108/114954 curved plus2658 separate at0.25; separate stencils/CPU membership are not GPU pixels. Full-plan/checker42/worktree122 paths/16 gates/inventory190/lints/diff checks pass and repeat after finalized receipt. Twelve source hashes bind native/sanitizer/clone/current files. Evidence: build/renderer-evidence/r2-2-workcaps-20261024.xnpj5S/. Preserve62 renderer/376 Python prior rows,1085 other baseline files, original branch/HEAD/index/staging and all1601/4521/3021/2677/163/2166/1292/104 prior proof files/gates. Point diameter/radius and continuity remain explicit pending owner choices, not silent changes. No dependency/framework/public painter API, Python ownership, geometry/assertion/timeout/provider/default, publication/new hosted CI, R2.3–R7/D1–D6/independent/phase/platform/hardware/widget/provider/deployment/package/release/full-goal promotion. Remaining numerical domains and all-five native source validation still need proof and separate publication authority. |
| 2026-10-24 | R2.2 bounded-work audit continuation started | Previous turn is progress:185 native/93 sanitizer cases per ARM64 host,627 Linux scripts and all1601 indexed binding-packet files verify. Audit finds flatten's recursive counter does not charge move/line/close elements. Add pre-fix regressions, preflight input size against work, and charge each input element plus subdivision nodes, retaining atomic output/recovery and unchanged geometry/0.25-pixel/assertion/timeout limits. Preserve61 renderer/375 Python prior rows,1091 baseline files, original index/checkout/staging and all gates. Evidence: build/renderer-evidence/r2-2-workcaps-20261024.xnpj5S/. Fresh native/header/link/sanitizer/negative/script/finalized maintenance proof is pending. Point diameter policy now has a separate nonblocking owner question; existing point/continuity behavior stays unchanged. No dependency/framework/public painter API, GPU/default, publication/new hosted CI, R2.3–R7 or acceptance promotion. |
| 2026-10-24 | Ordered aggregate CPU strokes and23 shared bindings locally verified | Codex implementation/self-review: compact prepareStrokes retains command order/owned straight-alpha snapshots/empty entries and per-command union domains, with aggregate path/dash/work/output caps and atomic failure. Six CPU cases bind23 unchanged corpus variants without replacing GPU cases; model/area1e-9 uses independent refined integration of owned conics, display remains0.25 and old assertions stay strict. Fresh Mac/Linux ARM64 each pass185 CPU IDs, preserving159, seven C++14 headers/strict three-source builds/Qt-GPU-Python-Cairo-free links and93 sanitizer cases. Linux leaks pass; existing fixture ASan-only RapidJSON/Mac leak limitations remain. Six copied-fixture negatives and two reset-budget/reverse-order production mutants fail ordinary assertions, then copied bytes restore. Retain initial invalid-bulges adapter and mistaken off-dash sample failures; corrections change only the new tests. Exact-code clean Linux scripts627/627 without skips; inventory62 cases/246 variants/190 configurations. Fresh Cairo linear648 tuples/693005 stable with12667 separate at0.09 and curved108/114954 stable with2658 separate at0.25; CPU membership is not GPU pixels. Pending continuity behavior remains unchanged. Full-plan/checker42/worktree122 paths and16 gates/inventory/lints/diff checks pass and repeat after this finalized receipt. Twelve source hashes bind native/sanitizer/clone/current files. Evidence: build/renderer-evidence/r2-2-bindings-20261024.hay7LM/. Preserve60 renderer/374 Python prior rows,1083 other baseline files, original checkout/index/staging and all4521/3021/2677/163/2166/1292/104 prior proof files/gates. No dependency/framework/public painter API, assertion/timeout/provider/default, Python ownership, publication/source-CI or R2.3–R7/D1–D6/independent/phase/platform/hardware/widget/provider/deployment/package/release/full-goal promotion. Point semantics, continuity choice, broader numerical-domain proof, separate publication authority and all-five new-code source checks remain required. |
| 2026-10-24 | Authorized R2.2 aggregate stroke and CPU binding continuation started | Prior turn is progress; all4521 curved-checkpoint files revalidate. Add a compact ordered CPU batch with shared input/work/output caps and atomic failure, then bind the existing line/arc/circle/ellipse/polyline/stroke corpus without replacing GPU cases. Independent geometry and fixture-mutation negatives must exercise actual candidate output; CPU order is not GPU alpha blending. Preserve current continuity policy pending the owner's response, single0.25-pixel budget,59 renderer/373 Python receipts,1091 baseline files, original branch/HEAD/index/staging and all gate states. Evidence: build/renderer-evidence/r2-2-bindings-20261024.hay7LM/. Fresh native/header/link/sanitizer/script and finalized maintenance checks are pending. No dependency/framework/public painter API, GPU/default, assertion/timeout, publication/source-CI, R2.3–R7 or acceptance promotion; point/fill/native-platform and broader numerical-domain obligations remain open. |
| 2026-10-24 | Width-aware curved stroke preparation locally verified; full R2.2 remains open | Codex implementation/self-review: existing CPU kernel/preparer adds parametric physical/dash displacement, stroke-space tangent cones/shared normals, bounded normal-sweep strips and exact sharp-boundary tangents. Scalar stationary spans use guarded extrema; full ellipses weld their endpoints. Fresh Mac/Linux ARM64 each pass159 CPU IDs, preserving142 and adding17, strict three-source C++14/seven headers/Qt-GPU-Python-Cairo-free links and67 recorder/curve/stroke sanitizer cases. Linux leak detection passes; fixture ASan-only RapidJSON and Mac leak limitations stay explicit. Final exact-code Linux scripts626/626 without skips. Mac comparison: unchanged linear648 tuples/693005 stable samples with12667 separate at0.09; separate curved108 scoped tuples/114954 stable samples with2658 separate at0.25. Analytic normal/annulus/folded-circle/ellipse/dash/shear/large-origin/stationary/cusp/miter-threshold and atomic limit tests supplement Cairo, not GPU pixels. Preserve pre-fix underflow/near-critical failures, area-oracle/source-compile failures and earlier154/159 cohorts as exact historical evidence, not final-source passes. Reject uncertain roots/tangents/phase topology and empty or partial unresolved meshes; broader valid numerical domains remain unqualified. Inner-join/intra-cusp continuity is an explicit pending owner choice, not silently changed or accepted. Full-plan/checker42/worktree122 paths and16 gates/inventory190/lints/diff checks pass and repeat after this finalized receipt. Nine source hashes bind final native/sanitizer/clone/current code. Evidence: build/renderer-evidence/r2-2-curved-stroke-20261024.nYHxD2/. Preserve58 renderer/372 Python prior rows,1083 other baseline files, original branch/HEAD/index/staging and all3021/2677/163/2166/1292/104 prior proof files/gate states. No new dependency/framework/public painter setters, assertion/timeout/provider/default, Python ownership, publication/source-CI or D1–D6/later-stage/independent/phase/platform/hardware/widget/provider/deployment/package/release/full-goal promotion. Next finish policy and remaining case/feature bindings, then separately authorize publication and qualify new code on all five source architectures; R2.3–R7 retain their prerequisites. |
| 2026-10-24 | Authorized R2.2 curved-stroke continuation started | Prior goal turn is progress: bounded linear preparation142/50 per ARM64 host,625 Linux scripts and648 Cairo tuples/693005 stable samples; all3021 proof files revalidate. Begin width-aware curve integration using the existing kernel, explicit tangent/parametric/dash-metric bounds and shared endpoint tangents; keep the single total0.25-pixel budget and finite depth/work/vertices. Independently test polynomial/exact-conic/wide/closed/dashed/affine/stationary cases and failure recovery. Cusp/near-cusp and visible inner-join policy/proof are not replaced by Cairo agreement or simple offset contours. Preserve57 renderer/371 Python prior rows,1091 baseline files, original branch/HEAD/index/staging and all gate states. Evidence: build/renderer-evidence/r2-2-curved-stroke-20261024.nYHxD2/. Fresh native/header/link/sanitizer/oracle/limit/script and finalized maintenance checks are pending for changed code. No new dependency/framework/public painter setters, GPU/factory/default, publication/source-CI, later-stage or acceptance promotion; full R2.2 and R2.3–R7 prerequisites remain intact. |
| 2026-10-24 | Provisional linear R2.2 stroke batch locally verified; curved strokes remain required | Codex implementation/self-review: one additional C++14 source prepares bounded linear widths/dashes/caps/joins and per-command union-before-OVER meshes, not GPU execution. Fresh Mac/Linux ARM64 each pass142 CPU IDs, preserving121 and adding21; strict three-source C++14/seven headers/Qt-GPU-Python-Cairo-free links and50 recorder/curve/stroke sanitizer cases per host pass, Linux leak detection enabled. Retain fixture ASan-only RapidJSON limitation and unqualified Mac leak detection. Independent Cairo1.18.6 comparison passes648 tuples/693005 stable samples;12667 boundary-ambiguous samples remain separate under unchanged0.09 stencil. Preserve all earlier failing probes, fixed-epsilon/oracle failures and setup-only recipe failure; adaptive multiprecision removes numerical misclassification without relaxing geometry/stencil/budgets. Fix outer join sectors/semicaps, shared endpoint welding, continuous zero gaps, zero-on dots and legacy internal-zero canonicalization; keep original snapshots. Exact-code Linux scripts625/625 without skips. Nine code/build/helper/test hashes bind final native/sanitizer/clone sources; earlier141/49/625 packets remain source-bound history, not final proof. Full-plan/checker42/worktree122 paths and16 gates/inventory190/lints/diff checks pass and repeat after this finalized receipt. Evidence: build/renderer-evidence/r2-2-stroke-20261024.Ir0gEd/. Preserve56 renderer/370 Python prior rows,1084 unrelated baseline files, original checkout/index/staging,2677-file curve proof and all old indexed packets/gate states. Candidate one-pixel hairline is not legacy zero-width qualification. Curved width/tangent/offset/dash preparation, remaining bindings/native platforms and GPU union resolution remain incomplete; next continue the authorized R2.2 curve integration within the total0.25-pixel budget. No new dependency, assertion/timeout/provider/default, Python ownership, commit/push/source-CI, D1–D6/independent/phase/hardware/widget/provider/deployment/package/release/full-goal acceptance change; R2.3–R7 keep their prerequisites. |
| 2026-10-24 | Authorized R2.2 CPU stroke continuation started | Prior goal turn is progress: first curve batch121 native IDs/29 sanitizer cases per Mac/Linux ARM64,624 Linux scripts and2677 indexed files verify unchanged. Continue existing provisional R2.2 authority with bounded linear stroke/dash/cap/join preparation and explicit per-command union-before-OVER semantics. Independently test physical/model widths, compensation/hairlines, odd arrays/signed offsets/zeros/closed seams, near-180 joins and bounded recovery. Curved stroke preparation remains fail-closed until width/tangent/offset error is bounded; this temporary subpatch does not replace complete R2.2. No public painter setters/new framework, dependency/GPU/factory/default, source-CI/publication or acceptance change. Entry verifies1090-file baseline and original checkout/index/staging; preserve55 renderer/369 Python receipts and all gate states. Evidence: build/renderer-evidence/r2-2-stroke-20261024.Ir0gEd/. Native strict/header/link/geometry/negative/sanitizer checks, scripts and finalized plan/checker/worktree/inventory/lints remain pending for new code. R2.3–R7 and all original acceptance/platform/package obligations stay open. |
| 2026-10-24 | Provisional R2.2 curve batch locally verified; strokes remain incomplete | Codex implementation/self-review: owned polynomial/exact positive conic recording plus bounded double subdivision; no new dependency, GPU or painter stub. Fresh Mac/Linux ARM64 each pass121 CPU IDs, preserving109 and binding all seven existing CPU spline cases; independent Bernstein/dense-distance/circle/ellipse-integral oracles, cusp/large-origin/limits/recovery tests pass. Seven independent C++14 headers, strict two-source compilation and Qt/GPU/Python-free CPU links pass; both hosts pass29 recorder/curve ASan/UBSan cases, Linux leak detection enabled. Fixture-adapter ASan-only RapidJSON1.1.0 UBSan limitation and unqualified Mac leak detection remain explicit. The unchanged tiny-bulge regression fails against angle-cancellation code, then passes with endpoint/half-angle conics. Retain early120-ID evidence, the failed negative, mistaken focused unittest class selection and post-test missing File API query wrapper failures; fresh corrected121-ID builds are final source-bound proof, not an erased failure. Exact-code clean Linux scripts pass624/624 without skips; all eight code/build/helper/test hashes agree across the native builds, clone and current source. Full-plan/checker42/worktree121 paths and16 gates/inventory190/both lints/diff checks pass and repeat after this finalized receipt. Evidence: build/renderer-evidence/r2-2-cpu-20261024.wU7bFK/. Preserve54 renderer/368 Python prior rows, original branch/HEAD/index/staging,1080 other baseline files and all prior indexed proof/gate states. Reserve1/16 pixel centerline and1/32 pixel length uncertainty inside the single0.25 total budget; wide-stroke offset, dash/cap/join/single-coverage preparation and other native platforms remain mandatory. No assertion/timeout/provider/default, commit/push/source-CI, D1–D6/independent/phase/platform/hardware/widget/provider/deployment/package/release/full-goal promotion. Next implement the remaining authorized CPU stroke batch; R2.3–R7 retain their prerequisites. |
| 2026-10-24 | Provisional R2.2 explicitly authorized; CPU curve/stroke work started | Human reply “Authorize provisional R2.2” permits candidate CPU curve flattening and stroke preparation, caps/joins/dashes, independent geometry tests and bounded work under the total0.25-physical-pixel budget. Start owned polynomial/exact conic recording and adaptive subdivision, retaining all109 prior native IDs and all existing required cases. Reserve1/16 pixel position and1/32 pixel path-length uncertainty; later stroke/packing must share the remaining budget. No new dependency, GPU/factory/default, source-CI dispatch, commit/push, D1–D6 or phase/hardware/release acceptance is inferred. Stroke/coverage and remaining feature/platform checks remain incomplete, not replaced by curve-only tests. Reconcile stale live-CI/622 text with the completed d28c41d3 cohort, preserving its exact-source62/62/60-artifact proof and163-file packet. Entry guards verify original checkout/index/staging and1088-file baseline. Preserve53 renderer/367 Python prior receipts and all gate states. Evidence: build/renderer-evidence/r2-2-cpu-20261024.wU7bFK/. Run native CPU/header/link/limit/recovery/independent-oracle checks, scripts and finalized plan/checker/worktree/inventory/lints; qualify supported platforms before any new all-platform claim. R2.3–R7 and all acceptance/prerequisite gates remain open. |
| 2026-10-24 | Provisional R2.1 source cohort green on all five architectures; artifacts verified | Implementation/self-review: d28c41d376cb571ae84dfd37a297b1a8800bc1af / run40110565811 attempt1 completes62/62 jobs, all60 source tuples and12/12 per platform. All60 unique artifacts are retained/read back against source/helpers/options, native IDs/XML, six C++14 header consumers, actual graphs, provider patches/devices and process retirement. Supplemental audit verifies60 source maps/2356 process records/98 ordinary XML/52 controlled-negative XML/13162 retained hashes/30 pinned f7debf4 SDKs and three patches each. Protocol623/623 without skips; CPU109 preserves92 and adds17 recorder cases;40 library test profiles execute109 or207 IDs; CLI207 and persistence/UI/combined214 non-widget IDs. UI/combined still compile/discover97 unexecuted widgets each. All14 RHI runs execute22 IDs; Intel virtual Metal source-only IDs4294968047/4294968071 match independent system_profiler. Growth13/13 Linux/macOS and21/21 Windows stays within unchanged32; tests/assertions/provider pins/SDK patches/300-command/1200-profile/60-minute-job caps are not relaxed. Evidence: build/renderer-evidence/r2-1-publication-20261024.Nlf2Gb/. Preserve52 renderer/366 Python prior rows,2166/1292/104-file prior packets, original branch/HEAD/index/staging,1086 unrelated files and every gate state. Repeat source-binding/full-plan/checker42/worktree119 paths and16 gates/inventory190/lints/diff checks on this finalized receipt; verify preservation before retaining a local documentation-only commit, without another push/dispatch or skip directive. Remote remains the exact green source candidate; separate Vulkan/Metal proofs remain source/provider-bound. R2.1 is provisional execution/self-review, not D3 ratification, a production RHI painter/default, R0/R1/R2.2–R7/independent/phase/platform/physical-hardware/widget/provider/deployment/package/release acceptance or full-goal completion. |
| 2026-10-24 | R2.1 committed/pushed; exact-SHA five-platform source cohort running | d28c41d376cb571ae84dfd37a297b1a8800bc1af, tree9e65591eb7878c3ae5bcc8df5629cb4a550f99c3, updates the approved existing fork branch non-forcibly from acbab6cd. Exactly nine intended paths differ from local parent806f3269; original branch/HEAD/index/staging and1086 unrelated files remain preserved. Prepublication source binding/full-plan/checker42/worktree119 paths and16 gates/inventory190/both lints/diff checks pass; all50/364 prior receipts and2166/1292/104 prior proof files are unchanged. Run40110565811 attempt1 is a push-triggered full60-source-job cohort plus protocol/aggregate, not a duplicate dispatch. New hosted results/payload reviews are pending; do not infer success from prior acbab6cd. Retain publication/source/run identities at build/renderer-evidence/r2-1-publication-20261024.Nlf2Gb/. This local documentation-only dispatch follow-up preserves51 renderer/365 Python rows and all source/helpers/defaults/assertions/provider/SDK patches/budgets/gate states. Repeat maintenance checks after this finalized receipt, monitor the same exact-SHA cohort, and review all60 artifacts. No merge/new PR/release/R2.2–R7/hardware/widget/phase/platform/full-goal acceptance. |
| 2026-10-24 | R2.1 publication and full source CI explicitly authorized | Human reply “Allow R2.1 commit, push and CI” permits the locally validated provisional recorder on dxli/LibreCAD_3:codex/r1-platform-ci, not a merge/new PR/release/default change or R2.2–R7. Publish exactly nine source/build/test/plan paths using a separate index; bind the resulting full SHA and all60 source tuples plus protocol/aggregate to build/renderer-evidence/r2-1-publication-20261024.Nlf2Gb/. Seven source hashes match the unchanged native109-ID/17-sanitizer Mac/Linux ARM64 proof; Intel seven-object evidence remains compile-only. Verify all2166/1292/104 prior proof files and all1086 unrelated baseline files, preserve50 renderer/364 Python prior receipts and original branch/HEAD/index/staging/gate states. Full-plan/checker42/worktree119 paths and16 gates/inventory190/lints/diff checks must pass on this finalized receipt before commit. The new exact-SHA hosted cohort is pending, not a pass; prior acbab6cd62/62 is historical. Keep all tests/assertions/provider pins/submodule SDK patches and command/profile/job budgets unchanged; independently read all60 payloads. D1–D6/R0/R1/R2.2–R7/independent/phase/platform/hardware/widget/provider/deployment/package/release/full-objective acceptance remain open. |
| 2026-10-24 | Intel C++14 cross-compilation verified; owner continuation required | Codex implementation/self-review: unchanged R2.1 recorder and all six independently configured CPU headers produce seven Mach-O x86_64 objects with installed Apple Clang21/SDK. Recorder uses strict C++14 warnings; headers retain exact CMake flags except target architecture. The extra-strict documentcanvas probe fails on nano-signal-slot's existing syntax compatibility; retain it, then use the already-configured flag without changing project suppression, source, helpers, SDK, dependencies, assertions or300-second caps. This is compile-only ARM64-host evidence, not native Intel runtime/CI/hardware/deployment-floor/full-provider acceptance; no Linux/Windows cross compiler was installed. Evidence: build/renderer-evidence/r2-1-intel-compile-20261024.VShgni/. Existing109/17 native ARM64 and older source-bound623-script proofs remain in their original packets. Full-plan/checker42/worktree119 paths and16 gates/inventory190/both lints/diff pass; repeat these checks after the finalized receipt, then verify preservation and the retained objects. Preserve49 renderer/363 Python prior rows, prior1292/2166-file packets, original index/checkout/staging and all gate states. Separate R2.1 publication authority remains unanswered for three consecutive goal turns; further hosted validation requires it, and R2.2–R7 need their original decisions/prerequisites. No phase/platform/release/default or full-goal completion is claimed. |
| 2026-10-24 | R2.1 bounded-work correction validated locally | Codex implementation/self-review reproduces4096 capacity increases for4096 open-path appends versus an unchanged32 bound; retain the failing native XML. Replace exact-size reserve with amortized append and atomic close/restart range insertion. Fresh Mac/Linux ARM64 each pass109 CPU IDs, preserving all108 prior IDs/nine fixture cases, and17 recorder ASan/UBSan cases with the same scoped fixture-adapter limitation; Linux leak detection passes. Both providers observe13 open/13 closed growths versus prior4096/24, with unchanged element/command/style limits, recovery and consuming semantics. Strict C++14/six independent headers, source binding, full-plan/checker42/worktree119 paths and16 gates/inventory190/lints/diff checks pass; repeat after finalized receipt. Evidence: build/renderer-evidence/r2-1-growth-20261024.WmWNfZ/. Preserve all48 renderer/362 Python prior rows, prior2166-file proof, original index/checkout/staging and unrelated files. Earlier108/16/623 evidence stays source-bound, not fresh new-code full-platform CI. Adds one regression; no helper/provider/dependency/default change, dropped or relaxed existing test/assertion, or expanded timeout or broader gate promotion. Candidate remains local/uncommitted pending publication authority; R2.2–R7/D1–D6/independent/phase/hardware/widget/provider/deployment/package/release/full goal remain open. |
| 2026-10-24 | Provisional R2.1 implemented and locally validated; publication pending | Codex implementation/self-review: explicit human approval permits candidate CPU path/state/transform recording only. One compact C++14 recorder owns paths and all implemented color/width/compensation/dash/antialias/transform state; no LcPainter stub, factory/default or GPU change. Both native ARM64 hosts pass108 CPU IDs, preserving92 and adding nine shared-fixture cases plus seven ownership/append-time/recovery/limit regressions. Six independent C++14 headers, strict compiler and no-Qt/GPU/Python linkage checks pass; both hosts pass16 recorder ASan/UBSan cases and Linux leak detection passes. RapidJSON1.1.0 fixture-adapter UBSan arithmetic and unsupported Mac leak detection remain separately retained, not suppressed in recorder code or counted as successful full-fixture UBSan. Three altered-fixture negatives fail as expected and restore the copied fixture. Mac disjoint253/159/211 groups cover623 script IDs,612 pass plus11 existing Linux-only skips; exact-code-hash Linux clone passes623/623 in121.333 seconds. Retain earlier300-second broad/group timeouts and malformed AppleDouble archive; verified replacement has exactly nine files. No test/assertion/timeout/provider/dependency/workflow change. Full-plan/checker42/worktree119 paths and16 gates/inventory190/lints/diff pass; repeat after final receipt. Evidence: build/renderer-evidence/r2-1-cpu-20261024.lM6CiQ/. Preserve47 renderer/361 Python prior rows,1079 unrelated files, original index/checkout/staging and all gate states. R2.1 is local/uncommitted; separate commit/push/CI authority awaits an answer. Prior acbab6cd source62/62 remains historical for this candidate. R2.2–R7, D1–D6, R0/R1, independent/physical-hardware/widget/provider/deployment/package/release and full goal remain open. |
| 2026-10-24 | D3 Cairo reference observations verified; R2.1 authority still pending | Codex implementation/self-review runs a bounded native ARM64 CPU reference against existing Cairo1.18.6, observed loaded binary/header/project-source identities retained at build/renderer-evidence/r0-cairo-reference-20261024.ICOCI3/. Five observation groups pass: path-append CTM capture, restored Cairo style/CTM with path edits retained, consuming stroke/fill and unchanged pixels on repeated empty stroke, independent point/distance/inverse queries, and isolated expected sticky errors2/5 for unmatched restore/singular scale. Rotation90/scale2 yields matrix.yy approximately0 but basis norm2; current wrapper's yy scale expression is not an independent rotation-invariant oracle. This tests bare Cairo, not the whole wrapper, production recorder, Windows/Linux or GPU/hardware behavior. Both30-second owned scopes retire; no dependency installation/build, source/helper/default/timeout/CI change or new stage begins. Keep current62/62 source-CI and separate rendering evidence bound to their original commits; no live jobs remain to restart. R2.1 provisional scope is requested with no answer assumed; D3 must decide capture/sign conventions, full wrapper-owned state and recoverable error behavior, not clone known legacy defects. Independent native-report/module/hash/process readback, full-plan, checker42/42, worktree116 paths/16 mapped gates/no stale or unmapped evidence, inventory190, both workflow lints, diff checks,50 local links, all prior rows/gate states and1083 unchanged files pass; repeat maintenance checks on this finalized receipt and retain the documentation-only update locally. Preserve46 renderer/360 Python prior rows, original index/checkout/staging/unrelated files and every S/platform/D1–D6/phase/hardware/widget/provider/deployment/package/independent gate. Full goal remains incomplete; further implementation requires the requested scope or accepted prerequisites. |
| 2026-10-24 | Popup-fix source cohort green on all five architectures; R2.1 scope requested | Codex implementation/self-review: acbab6cd851a2c1de5d7c5d8ae2ec291cf5a5dc0 / run40093455633 attempt1 completes62/62 jobs, all60 source tuples and12/12 per architecture. All60 unique artifacts are retained/reviewed; payload audit verifies60 source maps/2356 process records/98 ordinary XML/52 separate controlled negatives/13162 retained hashes/30 exact f7debf4 SDK records and three required patches each. Both Intel library/tests-ON runs execute22 RHI IDs without skips, native AppleParavirtAccelerator IDs4294968044 and4294968035 match independent system_profiler, raw Qt type0 retained. Protocol622/622; all assertions, required tests/tuples, provider pins and300/1200/60-minute caps unchanged. Original popup failures and current Linux Vulkan60 plus50 DPR1.25 and Mac Metal24 strict proofs remain source/provider-bound, not combined into full platform acceptance. Read-only R2.1 audit binds nine existing path/state/transform IDs and baseline semantic differences; no direct implementations are present. Explicit provisional scope is requested, not granted or inferred; D3 includes unresolved path/transform/style decisions. Reconcile implemented build options versus still-proposed runtime flags without source changes. Evidence: build/renderer-evidence/r1-popup-fix-20261024.fDteaO/hosted/ and r1-cohort-followup-20261024.wDPfZt/. Validation passes: full-plan, checker42/42, build-option regressions41/41, complete-worktree116 paths/16 mapped gates/no stale or unmapped evidence, inventory56 cases/223 variants/60 signatures/75 consumers/190 configurations, both workflow lints, diff checks,50 local links, prior receipts/gate states and1083 unchanged files; repeat maintenance checks on this finalized receipt; preserve all45 renderer/359 Python prior receipts and original index/checkout/staging/unrelated files. R0/R1/D1–D6, missing native R0/widgets/physical hardware/provider/deployment/packages, independent review and R2–R7 acceptance remain open; do not declare the full goal complete or begin unapproved code. |
| 2026-10-24 | Current-helper Mac Metal refresh complete; replacement source CI live | Codex implementation/self-review: clean acbab6cd851a2c1de5d7c5d8ae2ec291cf5a5dc0 builds main/Gui-only standalone targets for Qt6.8.3 and6.12.0 on native AppleM2Max ARM64/macOS27.0.1, Rosetta false. Per provider12 strict Metal/hardware/Cocoa runs/512 checks pass; each of four builds executes6 exact CTest IDs, zero skips/failures and38 owned groups retire. Independent readback binds pixel/PNG/RGBA, app/report/helper hashes, actual loaded Qt/Cocoa modules,22 pinned SDK references and four raw minimum archives now retained; loaded QtDBus also matches archived bytes. Keep initial one-use plugin-path and CTest/strict-directory collector diagnostics; final fresh namespace has no native test failure or retry. No source/helper/provider/default/assertion/timeout change, desktop interaction or dependency build. Evidence: build/renderer-evidence/r0-metal-refresh-20261024.SQfPML/. Reconcile implemented CPU M1–M3 inventory/selection against110 focused tests and real CLI output, preserving all56 cases/223 variants/60 signatures/75 consumers/190 configurations and required future native bindings. Source run40093455633 attempt1 remains live at the same pushed SHA; retained snapshot45/60 source successes with45 matching reviewed payloads and protocol622/622, no skip. Do not combine with old d4 success, cancel live work or claim a full cohort. Full-plan/checker42/42/worktree116 paths and16 gates/no stale or unmapped evidence/inventory190/both workflow lints/diff checks/50 local links, prior receipts/gate states and1083 unchanged files pass preservation review; repeat maintenance checks on this finalized receipt; no new push while the current cohort is active. Preserve all44 renderer/358 Python prior receipts and original checkout/index/unrelated files. Independent/D1–D6, wider native/widget/physical-hardware/provider/deployment/production/package and R0/R1 acceptance remain open. |
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
