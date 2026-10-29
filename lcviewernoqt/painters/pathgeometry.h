#pragma once
#include "pathrecorder.h"

namespace lc { namespace objects { struct Pattern; } }
namespace lc { namespace geo { class Region; } }
namespace lc { namespace viewer { namespace render {
// Coordinates already include zoom and DPR, in physical target pixels. This
// reserves most of the total 0.25px budget for stroke expansion/float packing.
struct CurveLimits {
    double positionError = 1.0/16, lengthError = 1.0/32;
    // Work charges every input element plus recursive/subdivision nodes; the
    // input-size preflight is bounded before walking or allocating subpaths.
    std::size_t vertices = MaxFrameElements, work = 4*MaxFrameElements;
    unsigned depth = 32;
};
struct CurveTangents {
    Point start{}, end{}; // unit stroke-space directions
    bool internal = false; // start belongs inside ONE input curve, not a decorative join
};
struct CurveMetrics {
    // Linear maps only: physical pixels -> stroke-width / dash-length spaces.
    Transform tangent, length;
    double physicalRadius = 0, normalError = 1.0/32, lengthScale = 1;
    bool dashed = false; // also bound parametric displacement in dash space
};
struct Polyline {
    std::vector<Point> points;
    bool closed = false;
    // Present only for width-aware preparation; one entry per consecutive edge.
    std::vector<CurveTangents> tangents;
    struct Edge {
        std::array<Point,4> controls{}; // positive convex curve hull, including endpoints
        unsigned degree = 1;
        double positionBound = 0;
    };
    // Fill-only certificates, one per consecutive edge. Local bounds avoid
    // applying a distant high-curvature span's error to every topology check.
    std::vector<Edge> edges;
};
struct FlattenedPath {
    std::vector<Polyline> subpaths;
    // Lengths use metrics.length units; multiplying their gap by lengthScale
    // bounds physical phase uncertainty. Position/normal bounds are physical.
    double lengthLower = 0, lengthUpper = 0, positionBound = 0, normalBound = 0;
    std::size_t visited = 0;
};
// Bounded de Casteljau subdivision, never fixed sample counts. Positive conics
// are exact ellipse pieces; they do not enable a rational document provider.
// Returns owned double data atomically, leaving commands untouched on failure.
FlattenedPath flatten(const PathCommand& command, CurveLimits limits = {}, CurveMetrics metrics = {});

// Private CPU preparation policy; these are not new LcPainter setters.
enum class StrokeCap { Butt, Round, Square };
enum class StrokeJoin { Miter, Round, Bevel };
enum class WidthUnits { Physical, Model };
struct StrokeStyle {
    StrokeCap cap = StrokeCap::Butt;
    StrokeJoin join = StrokeJoin::Miter;
    WidthUnits widthUnits = WidthUnits::Physical;
    double miterLimit = 10;
};
struct StrokeLimits {
    // Round-geometry component, not an independent total curve/packing budget.
    double positionError = 1.0/16;
    double pointDiameter = 64; // maximum physical diameter (major axis for model marks)
    std::size_t vertices = MaxFrameElements, work = 4*MaxFrameElements;
};
struct DashPattern {
    // Odd arrays repeated; internal zero intervals merged as in legacy Cairo.
    // Leading/trailing zero-on dots remain. Original caller values stay in state.
    std::vector<double> values;
    double period = 0, phase = 0;
};
DashPattern normalizeDash(const PaintState& state);
enum class StrokeCoverage { UnionBeforeOver };
struct PreparedStroke {
    PaintState state;
    std::vector<std::array<Point,3>> triangles; // owned physical-pixel doubles
    // Union all triangles for this command before applying its color ONCE.
    // Never pass this overlapping mesh directly to Frame's OVER triangle pass.
    StrokeCoverage coverage = StrokeCoverage::UnionBeforeOver;
    Point minimum{}, maximum{}; // conservative geometry bounds, excluding AA
    double positionBound = 0;
    std::size_t visited = 0;
};
// Width+compensation==0 is a one-physical-pixel candidate hairline. Other widths
// use the private policy above. Dash scaled=true uses physical lengths/offset;
// false uses the stroke-time inverse linear map, even for mixed append-time CTMs.
// Curves use parametric position, tangent-cone and dash-length bounds, not a
// centerline-only tolerance. Uncertifiable domains fail closed within the caps.
// This includes ambiguous near-stationary roots and dash events whose uncertainty
// straddles a sharp corner/end; never silently change their cap/join topology.
// Invalid/over-budget input throws atomically; no partial mesh escapes.
PreparedStroke prepareStroke(const PathCommand& command, StrokeStyle style = {}, StrokeLimits limits = {});
// Point commands use their own diameter/units, ignoring stroke width/dashes and
// decorations. Physical points stay circular; model points follow the affine
// map. Both use this bounded round geometry, without invoking a fill tessellator.
// One owned entry per command, including empty strokes/points, in submission order.
// Input path/state storage, output vertices and work are capped for the entire
// batch, not reset per command. Failure publishes no partial batch. Each entry
// still needs its own union-before-OVER pass; this is not a GPU blend operation.
std::vector<PreparedStroke> prepareStrokes(const std::vector<PathCommand>& commands,
                                         StrokeStyle style = {}, StrokeLimits limits = {});

// Provisional private R2.3 policy, not public painter setters or GPU execution.
// Implicitly close contours. Simple, mutually non-touching loops may be nested,
// disconnected or reversed. Crossing/touching/retraced nondegenerate boundaries
// and uncertifiable numeric/curve clearances fail closed, not silently repaired.
// Coordinates are finite physical doubles, zero or1e-80..1e9 in magnitude;
// local-origin recording retains large-world detail. Roundoff-scale Y snapping
// is charged to positionBound (<=1/8px total here); collapsing real area rejects.
enum class FillRule { EvenOdd, NonZero };
struct FillLimits {
    std::size_t vertices = MaxFrameElements, work = 4*MaxFrameElements;
};
struct GradientStop { double offset = 0; std::array<double,4> rgba{{0,0,0,1}}; };
struct LinearGradient { Point start{},end{}; std::vector<GradientStop> stops; }; // physical axis
// Candidate SDR straight-channel interpolation, no transfer function. Stable
// stop ordering, right-continuous duplicate offsets, endpoint padding. Invalid
// offsets/colors, empty stops and zero axes are diagnostics, never fallback ink.
LinearGradient prepareLinearGradient(const LinearGradient& gradient, std::size_t stopLimit = 1024);
std::array<double,4> sampleGradient(const LinearGradient& prepared, Point physicalPoint);
struct PreparedFill {
    PaintState state;
    FillRule rule = FillRule::EvenOdd;
    // Horizontal-band decomposition, not a fan. Interiors are disjoint; still
    // resolve shared boundaries/AA as ONE coverage mask before one color OVER.
    std::vector<std::array<Point,3>> triangles;
    Point minimum{}, maximum{};
    double positionBound = 0;
    std::size_t visited = 0;
    bool hasGradient = false;
    LinearGradient gradient; // owned snapshot, not a pointer/legacy long handle
};
PreparedFill prepareFill(const PathCommand& command, FillRule rule = FillRule::EvenOdd,
                         FillLimits limits = {}, const LinearGradient* gradient = nullptr,
                         double inputPositionBound = 0);
// A certified upstream boundary-recording error (0..1/64 physical pixel) is
// charged to topology clearances AND positionBound, not just a final annotation.
std::vector<PreparedFill> prepareFills(const std::vector<PathCommand>& commands,
                                     FillRule rule = FillRule::EvenOdd, FillLimits limits = {});
struct HatchFamily {
    // Model-space line family. Positive scale multiplies origin/spacing; angle
    // is radians. The stroke-time affine map transforms the whole pattern.
    Point origin{};
    double angle = 0, spacing = 1;
};
struct HatchLimits {
    FillLimits fill;
    std::size_t lines = 65536, segments = MaxFrameElements;
};
struct PreparedHatch {
    PreparedFill mask;
    // Physical centerlines extend past the mask by the physical stroke radius:
    // clip COVERAGE, not just centerlines, against mask BEFORE one OVER. Butt
    // ends lie outside the mask; no arbitrary CAD-entity/dashed adapter yet.
    std::vector<std::array<Point,2>> segments;
    std::size_t visited = 0;
};
PreparedHatch prepareHatch(const PathCommand& boundary, const std::vector<HatchFamily>& families,
                          double scale = 1, FillRule rule = FillRule::EvenOdd, HatchLimits limits = {});

// Real DXF provider input, not an infinite-line approximation. Geometry is an
// owned pattern-local snapshot with identity command transforms; state retains
// the caller's outer transform for later scale/rotation/lattice preparation.
// As in the current hatch drawable, all entities inherit the hatch paint style.
// Point size follows the selected diameter contract (3 physical pixels).
// This records geometry only: it does not repeat, clip, execute or cache tiles.
struct HatchTile {
    Point minimum{},maximum{}; // tight finite ink bounds, not the cell footprint
    Point period{}; // origin-based model cell dimensions supplied by the pattern
    PaintState state;
    std::vector<PathCommand> commands;
};
HatchTile recordHatchTile(const lc::objects::Pattern& pattern, const PaintState& state = {},
                         RecordingLimits limits = {});
struct TiledHatchLimits {
    StrokeLimits geometry; // global mask+ink vertices and work, not per tile
    std::size_t cells = 65536, instances = MaxFrameElements;
};
struct PreparedTiledHatch {
    PreparedFill mask;
    PreparedStroke ink;
    // Union ALL ink triangles, intersect that coverage with mask, then apply
    // inherited color ONCE. Neither mesh is a directly blendable Frame batch.
    // This is CPU preparation only, not GPU masking or a persistent tile cache.
    std::size_t cells = 0, instances = 0, visited = 0;
    double positionBound = 0;
};
// Repeat the actual finite commands at the explicit cell period, rotating and
// scaling the model pattern BEFORE the outer camera transform. Dash phase is
// anchored to each whole primitive, never restarted at the mask/cull boundary.
// Large lattice indices use compensated origin rebasing; uncertifiable numeric
// domains and global input/cell/instance/work/output caps fail atomically.
PreparedTiledHatch prepareTiledHatch(const PathCommand& boundary, const HatchTile& tile,
    double scale = 1, double angle = 0, FillRule rule = FillRule::EvenOdd,
    StrokeStyle style = {}, TiledHatchLimits limits = {}, double inputPositionBound = 0);

struct RegionLimits {
    RecordingLimits recording;
    std::size_t work = 4*MaxFrameElements;
    double joinError = 1.0/64; // cap, NOT a general CAD endpoint snapping tolerance
};
struct RecordedRegion {
    PathCommand boundary; // owned physical coordinates, one implicitly closed contour per CAD Loop
    double positionBound = 0;
    std::size_t visited = 0;
};
// Reorder/reverse only uniquely connected roundoff-close edges. The first
// nondegenerate edge fixes each loop's winding; never normalize holes by guess.
// Preserve curves, reject real gaps/branches/disjoint cycles in one Loop.
// Region LWPolyline widths describe stroke decoration, NOT boundary offset.
// The raw boundary MUST travel with its positionBound; wrappers do so and share
// recording/preparation work caps. No references to mutable CAD entities escape.
RecordedRegion recordRegion(const lc::geo::Region& region, const PaintState& state = {}, RegionLimits limits = {});
// Carry the recording certificate and its work charge into preparation. Keep
// the recorded boundary, positionBound and visited together and unmodified;
// this mutable value cannot authenticate a forged or stale certificate.
PreparedFill prepareRegionFill(const RecordedRegion& region, FillRule rule = FillRule::EvenOdd,
                               FillLimits limits = {}, const LinearGradient* gradient = nullptr);
PreparedFill prepareRegionFill(const lc::geo::Region& region, const PaintState& state = {},
                               FillRule rule = FillRule::EvenOdd, FillLimits limits = {},
                               const LinearGradient* gradient = nullptr);
// The recorded boundary is already physical. Its mask retains the recorded
// state; the tile may have its own transform and supplies the ink state.
PreparedTiledHatch prepareRegionHatch(const RecordedRegion& region, const HatchTile& tile,
    double scale = 1, double angle = 0, FillRule rule = FillRule::EvenOdd,
    StrokeStyle style = {}, TiledHatchLimits limits = {});
PreparedTiledHatch prepareRegionHatch(const lc::geo::Region& region, const HatchTile& tile,
    double scale = 1, double angle = 0, FillRule rule = FillRule::EvenOdd,
    StrokeStyle style = {}, TiledHatchLimits limits = {});
}}}
