#include "pathrecorder.h"
#include <algorithm>
#include <cmath>
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
void PathRecorder::paint(PaintOperation operation) {
    if (!_hasLine) { newPath(); return; }
    require(_commands.size() < _limits.commands, "command-limit");
    require(_state.dash.size() <= _limits.elements - _recordedDashElements, "state-limit");
    PathCommand command{operation, _state, _path};
    _commands.push_back(std::move(command)); _recordedElements += _path.size();
    _recordedDashElements += _state.dash.size(); newPath();
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
