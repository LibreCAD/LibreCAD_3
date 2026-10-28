# Renderer contract inputs

[renderer_cases.json](../renderer_cases.json) is the single requirements manifest.
[renderer_contracts.json](renderer_contracts.json) supplies concrete operands and
expected observations for its cases. These are proposed production bindings, not
rendering results, discovered GTests, accepted graphics decisions or qualified
configurations. D1–D6 and the migration plan's native/package gates remain open.

M2 validates the API/source register, fixture identities, typed data, assertion
units, case bindings and workloads. M3 validates logical configurations, coverage
deadlines and stage selection. Run the CPU checks or inventory-only CLI from the
repository root:

```sh
python3 -B -m unittest scripts.tests.test_renderer_cases
python3 -B scripts/validate_renderer_inventory.py --cases unittest/rendering/renderer_cases.json
python3 -B scripts/validate_renderer_inventory.py --cases unittest/rendering/renderer_cases.json \
  --stage R2 --configuration source-linux/LINUX-X64/rhi-only/offscreen/xcb/software
```

The real-repository positive test uses `validate_case_inputs()`; isolated mutation
tests mock only the independently tested M1 source projection. No test here
executes a fixture script or the future renderer. Independent geometry/font/image
checks protect the authored expectations; they do not prove a renderer's curve
error bound, event routing, GPU lifetime or performance.

## Configuration and stage selection

Requirements schema 2 adds explicit case `requires` flags and separates Lua-only
and Python-enabled CLI profiles. Corpus schema 1 and R0 report schema 3 are
unchanged. Old requirements schema 1 is rejected, rather than inferring missing
prerequisites. The current inventory has 56 cases/223 proposed variants and 189
logical configurations; these are not native executions or accepted providers.

| Profile | RHI | Legacy | Widgets | CLI | Python | Cairo |
|---|---|---|---|---|---|---|
| cpu | OFF | OFF | OFF | OFF | OFF | OFF |
| cairo | OFF | OFF | OFF | OFF | OFF | ON |
| rhi-only | ON | OFF | ON | OFF | OFF | OFF |
| dual | ON | ON | ON | OFF | OFF | OFF |
| scripted | ON | OFF | ON | OFF | ON | OFF |
| rhi-offscreen | ON | OFF | OFF | OFF | OFF | OFF |
| rhi-cli | ON | OFF | OFF | ON | OFF | OFF |
| rhi-cli-python | ON | OFF | OFF | ON | ON | OFF |
| dual-legacy | ON | ON | ON | OFF | OFF | OFF |

These are reviewed source-build intents, not observed CMake flags. Unit tests are
enabled; new/old rendering-test flags follow the enabled RHI/legacy targets.
Installed/package smoke uses runner checks, never requires a shipped GTest binary,
and binds separately to source qualification from that build. `dual-legacy` uses
the dual build in a separate forced legacy process with device class `any`; that
cannot qualify hardware, software or performance. Lua remains a core dependency.
These bounded profiles are coverage layers, not every legal CMake permutation.
Additional D2/D4/D6 cohorts, including any retained CLI compatibility route, need
explicit declarations and qualification before use.

Successful Python execution requires Python ON. Python-OFF CLI requests instead
have explicit `python-unavailable` rejection cases, before renderer creation,
including empty/over-budget inputs. Negative-only cases cannot satisfy successful
method/feature/ordinary-consumer coverage or legacy compatibility.

Templates join only valid mode/profile relations. Every requested field must be
reachable; duplicate IDs/logical tuples, unsupported APIs/QPAs/package architectures
and missing mandatory layers fail. Source requirements retain Linux software,
native hardware candidates, both Linux QPAs, CPU-only, UI-OFF, Python-OFF/ON CLI,
dual/legacy, Cairo and all installed/package rows. Packages do not repeat every
source device cross-product. Windows' software-or-trusted-hardware route remains
a D4 decision: inventory success neither selects a fallback nor accepts a route.
Missing access is pending qualification, not inapplicability.

Selection takes both an exact expanded key and R1–R7, includes earlier owning
stages, and returns sorted proposed IDs with input/oracle identities and expected
options. R1/R2 use source CPU/offscreen requirements; widget selection starts at
R3, installed/package selection at R5, and R7 excludes legacy/dual builds. Cairo
and legacy first have cases at R4. Empty/unknown/premature requests exit nonzero
without JSON success output. R6/R7 cases never enter R5 selections.

The inventory and selection JSON have distinct `renderer-case-inventory` and
`renderer-case-selection` kinds, `execution_evidence: false`, and a digest of the
same manifest byte snapshot that was parsed. They cannot satisfy R0 pixel records,
GTest discovery/XML equality, provider acceptance, package trust or phase readiness.
No build, native program or fixture script is launched. Minimum/current Qt,
Debug/Release and hardware/vendor cohorts still require separate actual build/run
records; sharing a logical key or test ID cannot merge their evidence.

## Data and assertion contract

Every dataset has ordinary, empty and limit variants, with additional edge cases
where needed. Each input/expected value has exactly `type`, `unit` and `value`.
Numbers are finite and bounded; vector/array sizes are bounded, matrices are
row-major affine 3×3, codepoints exclude surrogate values, and byte values are
integers in 0–255. A `nonfinite` input names NaN/Infinity as text for a negative
test; JSON nonfinite literals are forbidden. `asset` is an input reference to a
declared hashed fixture. `configuration` is expected-only and refers to the
selected tuple field, not permission for an app to self-certify its API.

Units distinguish model distance/area, radians, font design units, physical and
logical pixels, normalized ratios, 8-bit channels, bytes, counts and seconds.
Unqualified arrays are not implicitly pixels or coordinates. Model coordinates
are Cartesian; device/readback orientation is specified by each host case. The
new host snapshot is top-left straight RGBA, with stride explicit. CPU font
metrics do not depend on a GPU frame.

Assertions name an `observed` value, its typed `expected` value and an oracle.
Native tests must produce those observations from actual painter/consumer calls.
The corpus is not a scene language, callable script, generated "pass" record or
generic assertion interpreter. Prefer shared C++ fixtures and parameterized tests
over a class/executable for every case. Empty/limit variants must exercise their
real native operation/guard, not merely return the expected count/diagnostic.

Categorical `contracts` assertions require exact equality (zero violations).
Geometry/metric oracles bound absolute error in their stated units; curve error
bounds maximum deviation over the supported interval, not only test midpoints.
Pixel tolerance applies to named landmarks, never a whole-image similarity score
that could hide lost topology. Decoded original PNG bytes are exact. A diagnostic
variant requires its exact reason and no accepted output. Numeric limits and
domains remain D3/D5 candidates until reviewed.

Performance oracles are upper bounds on the named measured statistic or ratio,
not absolute differences against an expected geometry value. They reference
successful workload variants on hardware; software runs cannot qualify them.
Ratios need matched correct legacy output and a nonzero baseline. The candidate
100 ms p95 event-entry-to-submit limit is not photon/display latency. The absolute
cold-start budget, reference machines and fixed byte/cache budgets remain D5 work.

## Asset cache contracts

R2's `cache` case covers geometry. The existing R3 `text` and `image` cases also
carry F8/F12, the cache methods and DocumentCanvas consumers. Their retained,
reload, eviction and owner-recreation variants require native cache/asset counters,
completed pixels and cached/uncached comparison; the metadata checks do not supply
these observations. Seven new bindings extend those cases; the image-retained
binding is strengthened rather than duplicated. All are source GTests at R3,
never R2 asset requirements or GTests shipped inside packages.

Already-recorded snapshots retain old assets/style across public-image-handle
destruction or an asset revision. Future cache lookup must use the new revision.
The font reload uses the same pinned font with an injected revision and sizes
10/20: H's model advances are 7.15/14.3. It tests the revision boundary, not a
chosen filesystem timestamp/digest detector or a public font-destroy API.
The image reload overwrites the same temporary filename with another original
3×2 PNG: its first opaque texel changes red to blue; queued old output stays red.
Native probes sample that texel's center through the supplied affine transform,
not transparent RGB or an unspecified whole-image similarity score.

Eviction injects small renderer-policy limits: two one-glyph atlas pages for H,
é and visible glyph zero, or 48 decoded bytes for three 24-byte images. Hold the
first cached asset, drain/release the second, then admit the third by evicting only
the unreferenced asset. The incoming image's first texel is green. Assert bounded
native counters and correct pinned/incoming pixels; do not turn these expectations
into measured results or exhaust actual memory. Decoded bytes are not GPU bytes.

Owner recreation starts two distinct devices with equal local generations and
the same entity ID. Drain both, close A, verify B survives, recreate A at its next
generation and check rebuilt versus uncached output on A plus unchanged B. Settle
all resources after closing both. This is normal owner lifetime, not physically
pending GPU teardown or observed device loss. Implement these as ordinary shared
native fixtures, not an interpreter for the operation strings. D3 accepts the
production domain/policy before implementation; R3 later proves it.

## Fixture provenance

| Input | Provenance and notice |
|---|---|
| Contract corpus and embedded 3×2 PNG | Original project test data under the root GPL-3.0-or-later [notice](../../../LICENSE). The corpus hash pins PNG base64 and RGBA expectations; no downloaded image or old GL output. |
| Existing ABeeZee Regular OTF | Exact existing Version 1.002 binary in the legacy font directory, retained in place. Its embedded copyright names Anja Meiners (2011), Reserved Font Name ABeeZee; the embedded license declares OFL 1.1. [ABeeZee-OFL.txt](ABeeZee-OFL.txt) combines that copyright with the standard OFL 1.1 body. It is not a claim that this binary equals a newer upstream font. |
| `res/1.dxf` and `res/2.dxf` | Existing project regression drawings, root GPL notice. Their ordinary top-level entity inventories are respectively LINE and ARC/CIRCLE/ELLIPSE/LWPOLYLINE/SPLINE. They are baseline inputs, not independent geometric or rational-spline oracles. |
| Python and disabled Lua Rectangle plugins | Existing project operation sources, root GPL notice. Future adapters explicitly load them; metadata validation neither runs them nor enables the disabled Lua operation in normal startup. |

Each manifest fixture pins raw content and notice SHA256, kind, license and
provenance. Narrow `.gitattributes` rules preserve those bytes across Windows/Unix
checkouts, without changing the existing global DXF binary policy. No binary asset
is copied or replaced. Redistributing a final package still requires its complete
dependency/license audit.

The font checks read the actual cmap/hmtx design units independently of the future
font engine. H maps to glyph 61 with advance 715 and left bearing 90 at 1000 units
per em; é maps to glyph 189 with advance 544. The supplementary fixture decodes
U+1F642 and explicitly requests the missing glyph (index 0, advance 600), not a new
shaping engine or guaranteed emoji support. Raw design units are not hinted pixel
extents. See the OpenType [character mapping](https://learn.microsoft.com/en-us/typography/opentype/spec/cmap)
and [horizontal metrics](https://learn.microsoft.com/en-us/typography/opentype/spec/hmtx)
contracts. Missing fonts and malformed UTF-8 have separate diagnostics.

The original PNG SHA256 is
`f47327f4736508c64ad289abb041f3dad0e7a33977b496600c2441bacdf5d7ef`.
Its top row is red/half-alpha green/transparent blue and its bottom row is
yellow/cyan/magenta. Native image tests materialize these bytes in their own
temporary directory, including a Unicode/space filename. Padded readback and
affine/mirrored corners have independent expectations. Antialias on/off captures
start from separate transparent clears, so one result cannot contaminate the other.
The padded-readback case normalizes an explicitly straight CPU payload; it does
not recover hidden RGB lost by GPU premultiplication at alpha zero. Rendered
fully transparent RGB follows the separately accepted D3 normalization rule.

## Workload generation and sampling

The corpus pins LCG32 revision 1: unsigned state starts at 73 and advances by
`(1664525 * state + 1013904223) mod 2^32`. Each entity consumes two states;
`x = state1 % 1024`, `y = state2 % 1024`, with model step 1. The first positions
are (276,355), (870,909), (904,327). No language's default PRNG is interchangeable.

Mixed scenes contain 10k or 100k top-level entities, cycling line/circle/CCW
quarter-arc/closed triangle/cubic. The corpus pins deltas, radii, angles, control
points, line width and color. Asset-heavy scenes contain contiguous groups of
5k text items, 50 images, 100 hole hatches, 1000 nested inserts and 500 splines.
Text is H at model size 10; images use the original PNG/affine basis, hatches the
declared even-odd contours, inserts the canonical triangle with depth 2, inner
90-degree rotation and outer scale 2; splines use the pinned three Bézier spans.
Shared block definitions are not extra top-level entities. Place geometry at each
generated position, in the declared order, through the real DocumentCanvas.

Viewport 1024×1024, camera origin, zoom, DPR and deterministic pan/zoom sequence
are explicit inputs. Asset-heavy edit/undo/redo occurs after each 60 interactions
and settles before the next sample. Real drawings run as separate matched
baseline cases, never as hidden additions to a synthetic entity count. The
manifest binds successful variants, 600 warm samples/60 warmups and 30 independent
cold processes. Lifecycle uses one cycle per sample (200 measured plus 10 warmup
samples), not 200×200 cycles. Idle observes 30 seconds after draining scheduled
work; OS expose/resize events are recorded separately.

Future native adapters must retain raw samples, p50/p95 calculation, cohort and
operation order. Limit variants inject small production budgets rather than
exhausting memory or resetting a GPU. Runtime selection, stage execution, actual
capability probing and final artifact binding remain their own implementation gates.
