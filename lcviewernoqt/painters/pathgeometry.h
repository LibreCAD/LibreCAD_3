#pragma once
#include "pathrecorder.h"

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
}}}
