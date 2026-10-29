#pragma once
#include "renderframe.h"
#include <array>
#include <vector>

namespace lc { namespace viewer { namespace render {
struct Point { double x = 0, y = 0; };
// CPU-only candidate R2.1 contract, not an LcPainter implementation. Coordinates
// have no implicit Y flip. Column-vector affine transforms postmultiply; origin
// subtraction precedes the linear map to retain CAD-local double precision.
struct Transform {
    double xx = 1, xy = 0, yx = 0, yy = 1, tx = 0, ty = 0;
    Point origin;
    void validate() const;
    Point point(Point p) const;
    Point distance(Point p) const;
    Point inversePoint(Point p) const;
    Point inverseDistance(Point p) const;
    double scale() const; // length of transformed unit X, not matrix.yy
};
struct PaintState {
    Transform transform;
    std::array<double, 4> rgba{{0, 0, 0, 1}}; // straight alpha; no GPU packing yet
    double lineWidth = 1, lineWidthCompensation = 0;
    std::vector<double> dash;
    double dashOffset = 0;
    bool dashScaled = false, antialias = true;
};
enum class PathVerb { Move, Line, Close, Quadratic, Cubic, Conic };
struct PathElement {
    PathVerb verb;
    Point point; // endpoint; controls and conic weight are owned values
    Point control1{}, control2{};
    double weight = 1;
};
enum class PaintOperation { Stroke, Fill, Point };
enum class PointUnits { Physical, Model };
struct PathCommand {
    PaintOperation operation;
    PaintState state;
    std::vector<PathElement> path; // append-time transformed double coordinates
    double pointDiameter = 0;
    PointUnits pointUnits = PointUnits::Physical;
};
struct RecordingLimits {
    std::size_t elements = MaxFrameElements, commands = MaxFrameElements;
    std::size_t stackDepth = 1024, dashElements = 1024;
};
// Owns every path/style value. Validation failures throw a stable diagnostic and
// leave the previous state/path/commands intact (including unmatched restore).
// save/restore excludes the path. Stroke/fill consume it, including empty paths.
// Lvalue access is a const view; temporary access returns an owned snapshot.
// Later preparation reads snapshots, never live painter state.
class PathRecorder {
public:
    explicit PathRecorder(RecordingLimits limits = {});
    const PaintState& state() const & { return _state; }
    PaintState state() const && { return _state; }
    const std::vector<PathElement>& path() const & { return _path; }
    std::vector<PathElement> path() const && { return _path; }
    const std::vector<PathCommand>& commands() const & { return _commands; }
    std::vector<PathCommand> commands() const && { return _commands; }
    void newPath();
    void newSubPath();
    void moveTo(Point p);
    void lineTo(Point p);
    void closePath();
    void rectangle(Point corner, double width, double height);
    void quadraticTo(Point control, Point end);
    void cubicTo(Point control1, Point control2, Point end);
    // Current document provider supports polynomial spans only (2/3/4 controls).
    void bezierSpan(const std::vector<Point>& controls, const std::vector<double>& weights = {});
    // Radians, no implicit Y flip; arc is CCW unless clockwise is true.
    // Ellipse's equal start/end denotes one full turn; circle starts its own subpath.
    void arc(Point center, double radius, double start, double end, bool clockwise = false);
    void ellipse(Point center, double rx, double ry, double start, double end,
                 double rotation = 0, bool clockwise = false);
    void circle(Point center, double radius);
    void polyline(const std::vector<Point>& points, const std::vector<double>& bulges, bool closed);
    // Independent round mark; does not consume/change the current path. Size is
    // diameter, not legacy Cairo's radius. Zero diameter records no ink.
    void point(Point center, double diameter, PointUnits units = PointUnits::Physical);
    void stroke();
    void fill();
    void clearCommands(); // does not reset the current path/style/stack
    void save();
    void restore();
    void setTransform(const Transform& transform);
    void resetTransform();
    void translate(double x, double y);
    void scale(double s);
    void rotate(double radians);
    void source(double r, double g, double b, double a = 1);
    void lineWidth(double width);
    void lineWidthCompensation(double compensation);
    void dash(const std::vector<double>& values, double offset = 0, bool scaled = false);
    void antialias(bool enabled) { _state.antialias = enabled; }
private:
    void reserveElements(std::size_t additional) const;
    void paint(PaintOperation operation);
    void curve(PathElement element);
    void append(const PathRecorder& other);
    void bulgeTo(Point end, Point right, double bulge);
    RecordingLimits _limits;
    PaintState _state;
    std::vector<PaintState> _stack;
    std::vector<PathElement> _path;
    std::vector<PathCommand> _commands;
    std::size_t _recordedElements = 0;
    std::size_t _recordedDashElements = 0;
    Point _start;
    bool _current = false, _closed = false, _hasLine = false;
};
}}}
