#include "pathrecorder.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace lc { namespace viewer { namespace render {
namespace {
void require(bool condition, const char* diagnostic) {
    if (!condition) throw std::invalid_argument(diagnostic);
}
Point finite(Point p, const char* diagnostic = "invalid-transform") {
    require(std::isfinite(p.x) && std::isfinite(p.y), diagnostic); return p;
}
std::array<double, 4> inverse(const Transform& t) {
    const double m = std::max({std::abs(t.xx), std::abs(t.xy), std::abs(t.yx), std::abs(t.yy)});
    require(std::isfinite(m) && m > 0, "invalid-transform");
    const double a = t.xx / m, b = t.xy / m, c = t.yx / m, d = t.yy / m;
    const double det = a * d - b * c;
    require(std::isfinite(det) && det != 0, "invalid-transform");
    std::array<double, 4> result{{(d / det) / m, (-b / det) / m, (-c / det) / m, (a / det) / m}};
    for (double v : result) require(std::isfinite(v), "invalid-transform");
    return result;
}
}
void Transform::validate() const {
    for (double v : {xx, xy, yx, yy, tx, ty, origin.x, origin.y})
        require(std::isfinite(v), "invalid-transform");
    inverse(*this);
    require(std::isfinite(std::hypot(xx, yx)), "invalid-transform");
}
Point Transform::distance(Point p) const {
    finite(p); return finite({xx * p.x + xy * p.y, yx * p.x + yy * p.y});
}
Point Transform::point(Point p) const {
    finite(p); p = distance(finite({p.x - origin.x, p.y - origin.y}));
    return finite({p.x + tx, p.y + ty});
}
Point Transform::inverseDistance(Point p) const {
    finite(p); const auto i = inverse(*this);
    return finite({i[0] * p.x + i[1] * p.y, i[2] * p.x + i[3] * p.y});
}
Point Transform::inversePoint(Point p) const {
    finite(p); p = inverseDistance(finite({p.x - tx, p.y - ty}));
    return finite({p.x + origin.x, p.y + origin.y});
}
double Transform::scale() const { return std::hypot(xx, yx); }
PathRecorder::PathRecorder(RecordingLimits limits) : _limits(limits) {
    require(limits.elements <= MaxFrameElements && limits.commands <= MaxFrameElements &&
            limits.stackDepth <= 1024 && limits.dashElements <= 1024, "invalid-limits");
}
void PathRecorder::reserveElements(std::size_t n) const {
    require(n <= _limits.elements - _recordedElements - _path.size(), "vertex-limit");
}
void PathRecorder::newPath() { _path.clear(); _current = _closed = _hasLine = false; }
void PathRecorder::newSubPath() { _current = _closed = false; }
void PathRecorder::moveTo(Point p) {
    p = _state.transform.point(finite(p, "invalid-point")); reserveElements(1);
    _path.push_back({PathVerb::Move, p}); _start = p; _current = true; _closed = false;
}
void PathRecorder::lineTo(Point p) {
    if (!_current) { moveTo(p); return; }
    p = _state.transform.point(finite(p, "invalid-point")); reserveElements(_closed ? 2 : 1);
    if (_closed) {
        // One range insertion is atomic for these nonthrowing value elements;
        // let vector retain geometric growth instead of reserve(size + n).
        const PathElement elements[] = {{PathVerb::Move, _start}, {PathVerb::Line, p}};
        _path.insert(_path.end(), elements, elements + 2);
    } else _path.push_back({PathVerb::Line, p});
    _closed = false; _hasLine = true;
}
void PathRecorder::closePath() {
    if (!_current || _closed) return;
    reserveElements(1); _path.push_back({PathVerb::Close, _start}); _closed = true;
}
void PathRecorder::rectangle(Point p, double width, double height) {
    finite(p, "invalid-point"); require(std::isfinite(width) && std::isfinite(height), "invalid-point");
    const Point corners[] = {p, {p.x + width, p.y}, {p.x + width, p.y + height}, {p.x, p.y + height}};
    PathElement elements[5];
    for (unsigned i = 0; i < 4; ++i)
        elements[i] = {i == 0 ? PathVerb::Move : PathVerb::Line, _state.transform.point(finite(corners[i], "invalid-point"))};
    elements[4] = {PathVerb::Close, elements[0].point}; reserveElements(5);
    _path.insert(_path.end(), elements, elements + 5); _start = elements[0].point;
    _current = _closed = _hasLine = true;
}
void PathRecorder::curve(PathElement e) {
    e.point = _state.transform.point(finite(e.point, "invalid-point"));
    e.control1 = _state.transform.point(finite(e.control1, "invalid-point"));
    if (e.verb == PathVerb::Cubic) e.control2 = _state.transform.point(finite(e.control2, "invalid-point"));
    const bool restart = !_current || _closed;
    reserveElements(restart ? 2 : 1);
    if (restart) {
        const Point start = _current ? _start : e.control1;
        const PathElement elements[] = {{PathVerb::Move, start}, e};
        _path.insert(_path.end(), elements, elements + 2); _start = start;
    } else _path.push_back(e);
    _current = _hasLine = true; _closed = false;
}
void PathRecorder::quadraticTo(Point c, Point end) { curve({PathVerb::Quadratic, end, c}); }
void PathRecorder::cubicTo(Point c1, Point c2, Point end) { curve({PathVerb::Cubic, end, c1, c2}); }
void PathRecorder::append(const PathRecorder& other) {
    if (other._path.empty()) return;
    reserveElements(other._path.size()); _path.insert(_path.end(), other._path.begin(), other._path.end());
    _start = other._start; _current = other._current; _closed = other._closed; _hasLine |= other._hasLine;
}
void PathRecorder::bezierSpan(const std::vector<Point>& points, const std::vector<double>& weights) {
    require(weights.empty(), "unsupported-rational-provider");
    require(points.size() >= 2 && points.size() <= 4, "unsupported-span");
    PathRecorder part(_limits); part._state = _state; part.moveTo(points.front());
    if (points.size() == 2) part.lineTo(points[1]);
    else if (points.size() == 3) part.quadraticTo(points[1], points[2]);
    else part.cubicTo(points[1], points[2], points[3]);
    append(part);
}
void PathRecorder::arc(Point center, double radius, double start, double end, bool clockwise) {
    require(std::isfinite(start) && std::isfinite(end), "invalid-curve");
    if (start == end) { require(std::isfinite(radius) && radius >= 0, "invalid-curve"); finite(center,"invalid-point"); return; }
    ellipse(center, radius, radius, start, end, 0, clockwise);
}
void PathRecorder::ellipse(Point center, double rx, double ry, double start, double end, double rotation, bool clockwise) {
    constexpr double pi = 3.141592653589793238462643383279502884;
    for (double v : {rx, ry, start, end, rotation}) require(std::isfinite(v), "invalid-curve");
    require(rx >= 0 && ry >= 0, "invalid-curve");
    center = _state.transform.point(finite(center, "invalid-point"));
    if (rx == 0 || ry == 0) return; // explicit degenerate, never the old 0.1-radius substitution
    double sweep = end - start; require(std::isfinite(sweep), "invalid-curve");
    if (sweep == 0) sweep = clockwise ? -2*pi : 2*pi;
    else if ((clockwise && sweep > 0) || (!clockwise && sweep < 0)) {
        sweep = std::fmod(sweep, 2*pi);
        if (clockwise && sweep > 0) sweep -= 2*pi;
        if (!clockwise && sweep < 0) sweep += 2*pi;
    }
    if (sweep == 0) return;
    const double count = std::ceil(std::abs(sweep) / (pi/2));
    require(count <= double(_limits.elements), "vertex-limit");
    const auto n = static_cast<std::size_t>(count); reserveElements(n + (_closed ? 2 : 1));
    const double cr = std::cos(rotation), sr = std::sin(rotation);
    Point u = _state.transform.distance({rx*cr, rx*sr});
    Point v = _state.transform.distance({-ry*sr, ry*cr});
    const double ca = std::cos(start), sa = std::sin(start);
    const Point a = finite({u.x*ca + v.x*sa, u.y*ca + v.y*sa});
    v = finite({-u.x*sa + v.x*ca, -u.y*sa + v.y*ca}); u = a;
    const double magnitude = std::max({1.0,std::abs(center.x),std::abs(center.y),
        std::abs(u.x),std::abs(u.y),std::abs(v.x),std::abs(v.y)});
    require(64*std::numeric_limits<double>::epsilon()*magnitude < 1.0/64,"precision-limit");
    const auto at = [&](double angle, double w) {
        return finite({center.x + (u.x*std::cos(angle)+v.x*std::sin(angle))/w,
                       center.y + (u.y*std::cos(angle)+v.y*std::sin(angle))/w});
    };
    std::vector<PathElement> elements; elements.reserve(n + (_closed ? 2 : 1));
    const Point first = at(0, 1);
    if (_closed) elements.push_back({PathVerb::Move, _start});
    elements.push_back({_current ? PathVerb::Line : PathVerb::Move, first});
    for (std::size_t i = 0; i < n; ++i) {
        const double left = sweep*(double(i)/double(n)), right = sweep*(double(i+1)/double(n));
        const double w = std::cos((right-left)/2);
        elements.push_back({PathVerb::Conic, at(right, 1), at((left+right)/2, w), {}, w});
    }
    // A full turn ends at the owned start exactly, not at sin(2*pi)'s residual.
    // Otherwise closing a wide ellipse introduces a spurious tiny line/tangent.
    if (std::fmod(std::abs(sweep),2*pi)==0) elements.back().point=first;
    _path.insert(_path.end(), elements.begin(), elements.end());
    if (!_current) _start = first;
    _current = _hasLine = true; _closed = false;
}
void PathRecorder::circle(Point center, double radius) {
    constexpr double tau = 6.283185307179586476925286766559005768;
    PathRecorder part(_limits); part._state = _state; part.arc(center, radius, 0, tau);
    if (!part._path.empty()) { part._path.back().point = part._start; part.closePath(); }
    append(part);
}
void PathRecorder::polyline(const std::vector<Point>& points, const std::vector<double>& bulges, bool closed) {
    const std::size_t edges = points.empty() ? 0 : points.size() - (closed ? 0 : 1);
    require(bulges.size() == edges, "invalid-bulges");
    require(points.size() <= _limits.elements, "vertex-limit");
    PathRecorder part(_limits); part._state = _state;
    if (!points.empty()) part.moveTo(points.front());
    for (std::size_t i = 0; i < edges; ++i) {
        const Point a = finite(points[i], "invalid-point"), b = finite(points[(i+1)%points.size()], "invalid-point");
        const double bulge = bulges[i]; require(std::isfinite(bulge), "invalid-bulges");
        const double dx = b.x-a.x, dy = b.y-a.y, length = std::hypot(dx,dy);
        require(std::isfinite(length), "invalid-bulges");
        if (bulge == 0 || length == 0) { part.lineTo(b); continue; }
        part.bulgeTo(_state.transform.point(b),_state.transform.distance({dy,-dx}),bulge);
    }
    if (closed && !points.empty()) part.closePath();
    append(part);
}
void PathRecorder::bulgeTo(Point end, Point right, double bulge) {
    const Point a = _path.back().point;
    const Point mid = finite({a.x/2+end.x/2,a.y/2+end.y/2},"invalid-bulges");
    if (std::abs(bulge) <= 0.4142135623730951) {
        const double b2 = bulge*bulge, f = bulge/(1-b2);
        const Point control = finite({mid.x+right.x*f,mid.y+right.y*f},"invalid-bulges");
        reserveElements(1); _path.push_back({PathVerb::Conic,end,control,{},(1-b2)/(1+b2)});
        _hasLine = true; return;
    }
    // Bulge half-angle construction avoids large centers and angle+sweep
    // cancellation; even an almost-full circle needs only bounded bisections.
    const Point m = finite({mid.x+right.x*(bulge/2),mid.y+right.y*(bulge/2)},"invalid-bulges");
    const Point d = finite({end.x-a.x,end.y-a.y},"invalid-bulges");
    const Point leftRight = finite({right.x/2-d.x*(bulge/2),right.y/2-d.y*(bulge/2)},"invalid-bulges");
    const Point rightRight = finite({right.x/2+d.x*(bulge/2),right.y/2+d.y*(bulge/2)},"invalid-bulges");
    const double half = bulge/(std::hypot(1.,bulge)+1);
    bulgeTo(m,leftRight,half); bulgeTo(end,rightRight,half);
}
void PathRecorder::paint(PaintOperation operation) {
    if (!_hasLine) { newPath(); return; }
    require(_commands.size() < _limits.commands, "command-limit");
    require(_state.dash.size() <= _limits.elements - _recordedDashElements, "state-limit");
    PathCommand command{operation, _state, _path};
    _commands.push_back(std::move(command)); _recordedElements += _path.size();
    _recordedDashElements += _state.dash.size(); newPath();
}
void PathRecorder::point(Point center,double diameter,PointUnits units) {
    require(std::isfinite(diameter) && diameter>=0,"invalid-point");
    require(units==PointUnits::Physical || units==PointUnits::Model,"invalid-point");
    center=_state.transform.point(finite(center,"invalid-point"));reserveElements(1);
    require(_commands.size()<_limits.commands,"command-limit");
    require(_state.dash.size()<=_limits.elements-_recordedDashElements,"state-limit");
    PathCommand command{PaintOperation::Point,_state,{{PathVerb::Move,center}}};
    command.pointDiameter=diameter;command.pointUnits=units;
    _commands.push_back(std::move(command));++_recordedElements;_recordedDashElements+=_state.dash.size();
}
void PathRecorder::stroke() { paint(PaintOperation::Stroke); }
void PathRecorder::fill() { paint(PaintOperation::Fill); }
void PathRecorder::clearCommands() { _commands.clear(); _recordedElements = _recordedDashElements = 0; }
void PathRecorder::save() {
    require(_stack.size() < _limits.stackDepth, "state-limit"); _stack.push_back(_state);
}
void PathRecorder::restore() {
    require(!_stack.empty(), "state-underflow"); _state = std::move(_stack.back()); _stack.pop_back();
}
void PathRecorder::setTransform(const Transform& t) { t.validate(); _state.transform = t; }
void PathRecorder::resetTransform() { _state.transform = Transform{}; }
void PathRecorder::translate(double x, double y) {
    auto t = _state.transform; const auto d = t.distance({x, y});
    t.tx += d.x; t.ty += d.y; setTransform(t);
}
void PathRecorder::scale(double s) {
    auto t = _state.transform; t.xx *= s; t.xy *= s; t.yx *= s; t.yy *= s; setTransform(t);
}
void PathRecorder::rotate(double r) {
    require(std::isfinite(r), "invalid-transform"); auto t = _state.transform;
    const double c = std::cos(r), s = std::sin(r);
    t.xx = _state.transform.xx * c + _state.transform.xy * s;
    t.xy = -_state.transform.xx * s + _state.transform.xy * c;
    t.yx = _state.transform.yx * c + _state.transform.yy * s;
    t.yy = -_state.transform.yx * s + _state.transform.yy * c; setTransform(t);
}
void PathRecorder::source(double r, double g, double b, double a) {
    for (double v : {r, g, b, a}) require(std::isfinite(v) && v >= 0 && v <= 1, "invalid-color");
    _state.rgba = {{r, g, b, a}};
}
void PathRecorder::lineWidth(double v) {
    require(std::isfinite(v) && v >= 0 && std::isfinite(v + _state.lineWidthCompensation) &&
            v + _state.lineWidthCompensation >= 0, "invalid-line-width"); _state.lineWidth = v;
}
void PathRecorder::lineWidthCompensation(double v) {
    require(std::isfinite(v) && std::isfinite(v + _state.lineWidth) &&
            v + _state.lineWidth >= 0, "invalid-line-width"); _state.lineWidthCompensation = v;
}
void PathRecorder::dash(const std::vector<double>& values, double offset, bool scaled) {
    require(values.size() <= _limits.dashElements, "dash-limit");
    require(std::isfinite(offset), "invalid-dash"); double total = 0;
    for (double v : values) { require(std::isfinite(v) && v >= 0, "invalid-dash"); total += v; }
    require(std::isfinite(total) && (values.empty() || total > 0), "invalid-dash");
    // Keep the caller's odd pattern and signed offset; phase normalization and
    // tessellation belong to R2.2, not a second partial stroke implementation.
    auto copy = values; _state.dash.swap(copy); _state.dashOffset = offset; _state.dashScaled = scaled;
}
}}}
