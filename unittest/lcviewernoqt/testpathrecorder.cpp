#include <gtest/gtest.h>
#include "painters/pathrecorder.h"
#include "painters/pathgeometry.h"
#include "buildcontracts_config.h"
#include <rapidjson/document.h>
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <cmath>
#include <fstream>
#include <iterator>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <string>

namespace {
using namespace lc::viewer::render;
namespace json = rapidjson;
// Shared manifest IDs execute the real recorder using the existing typed
// corpus, not a second copy of its expected values. Unknown/unused assertions
// fail closed. Extra regression IDs below test the provisional semantic edges.
class Contract {
public:
    Contract(const char* dataset, const char* variant) {
        std::ifstream file(build_contract::pinnedFixture, std::ios::binary);
        if (!file) throw std::runtime_error("missing CPU contract fixture");
        const std::string raw{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
        document.Parse(raw.c_str());
        if (document.HasParseError() || !document.IsObject() || !document.HasMember("datasets"))
            throw std::runtime_error("invalid CPU contract fixture");
        data = &document["datasets"][dataset][variant];
    }
    const json::Value& input(const char* key) const { return (*data)["inputs"][key]["value"]; }
    const json::Value& expected(const char* key) {
        for (const auto& a : (*data)["assertions"].GetArray()) if (std::string(a["observed"].GetString()) == key) {
            if (!used.insert(key).second) throw std::runtime_error("duplicate CPU observation");
            return a["expected"]["value"];
        }
        throw std::runtime_error("unknown CPU observation");
    }
    void observe(const char* key, bool value) { EXPECT_EQ(expected(key).GetBool(), value) << key; }
    void observe(const char* key, std::size_t value) { EXPECT_EQ(expected(key).GetUint64(), value) << key; }
    void observe(const char* key, const std::string& value) { EXPECT_EQ(expected(key).GetString(), value) << key; }
    void observe(const char* key, double value) {
        // At least as strict as model-error/scale-error in the candidate manifest.
        EXPECT_NEAR(expected(key).GetDouble(), value, 1e-12) << key;
    }
    void observe(const char* key, Point value) {
        const auto& e = expected(key);
        EXPECT_NEAR(e[0].GetDouble(), value.x, 1e-12) << key;
        EXPECT_NEAR(e[1].GetDouble(), value.y, 1e-12) << key;
    }
    void observe(const char* key, const std::vector<double>& value) {
        const auto& e=expected(key); ASSERT_EQ(e.Size(),value.size()) << key;
        for (unsigned i=0;i<e.Size();++i) EXPECT_NEAR(e[i].GetDouble(),value[i],1e-12) << key;
    }
    void measure(const char* key, double value, double tolerance) {
        // Only approximate geometry observations use their declared manifest
        // oracle (model/area 1e-9, display .25); existing exact tests stay strict.
        EXPECT_NEAR(expected(key).GetDouble(),value,tolerance) << key;
    }
    void complete() const { EXPECT_EQ((*data)["assertions"].Size(), used.size()); }
private:
    json::Document document;
    const json::Value* data;
    std::set<std::string> used;
};
Point point(const json::Value& p) { return {p[0].GetDouble(), p[1].GetDouble()}; }
Transform matrix(const json::Value& m) {
    if (m.Size() != 9 || m[6].GetDouble() != 0 || m[7].GetDouble() != 0 || m[8].GetDouble() != 1)
        throw std::runtime_error("non-affine CPU fixture");
    Transform t; t.xx = m[0].GetDouble(); t.xy = m[1].GetDouble(); t.tx = m[2].GetDouble();
    t.yx = m[3].GetDouble(); t.yy = m[4].GetDouble(); t.ty = m[5].GetDouble(); return t;
}
template<class Call> std::string diagnostic(Call call) {
    try { call(); } catch (const std::invalid_argument& e) { return e.what(); }
    ADD_FAILURE() << "expected a recoverable diagnostic"; return {};
}
void operations(PathRecorder& r, const json::Value& ops, const json::Value* points = nullptr) {
    unsigned i = 0;
    for (const auto& op : ops.GetArray()) {
        const std::string name = op.GetString();
        if (name == "new-path") r.newPath();
        else if (name == "move-to") r.moveTo(point((*points)[i++]));
        else if (name == "line-to") r.lineTo(point((*points)[i++]));
        else if (name == "close-path") r.closePath();
        else if (name == "stroke") r.stroke();
        else if (name == "fill") r.fill();
        else if (name == "save") r.save();
        else if (name == "restore") r.restore();
        else if (name == "source-rgba-red") r.source(1, 0, 0);
        else if (name == "line-width-two") r.lineWidth(2);
        else if (name == "dash-one-two") r.dash({1, 2});
        else if (name == "scale-two") r.scale(2);
        else throw std::runtime_error("unknown CPU fixture operation");
    }
    if (points) EXPECT_EQ(points->Size(), i);
}
bool sameTransform(const Transform& a, const Transform& b) {
    return a.xx == b.xx && a.xy == b.xy && a.yx == b.yx && a.yy == b.yy && a.tx == b.tx &&
           a.ty == b.ty && a.origin.x == b.origin.x && a.origin.y == b.origin.y;
}
bool sameStyle(const PaintState& a, const PaintState& b) {
    return sameTransform(a.transform, b.transform) && a.rgba == b.rgba && a.lineWidth == b.lineWidth &&
        a.lineWidthCompensation == b.lineWidthCompensation && a.dash == b.dash && a.dashOffset == b.dashOffset &&
        a.dashScaled == b.dashScaled && a.antialias == b.antialias;
}
void segment(PathRecorder& r) { r.moveTo({0, 0}); r.lineTo({3, 4}); }
}
TEST(CpuPainterContractTest, CpuPathStateOrdinary) {
    Contract c("path-state", "ordinary"); PathRecorder r;
    operations(r, c.input("operations"), &c.input("points")); ASSERT_EQ(1u, r.commands().size());
    EXPECT_EQ(c.input("closed").GetBool(), r.commands()[0].path.back().verb == PathVerb::Close);
    double length = 0; Point previous;
    for (const auto& e : r.commands()[0].path) {
        if (e.verb != PathVerb::Move) length += std::hypot(e.point.x - previous.x, e.point.y - previous.y);
        previous = e.point;
    }
    c.observe("closed-length", length); c.observe("stroke-draw-count", r.commands().size());
    c.observe("path-consumed", r.path().empty()); c.complete();
}
TEST(CpuPainterContractTest, CpuPathStateEmpty) {
    Contract c("path-state", "empty"); PathRecorder r;
    operations(r, c.input("operations"), &c.input("points"));
    c.observe("draw-count", r.commands().size()); c.complete();
}
TEST(CpuPainterContractTest, CpuPathStateLimit) {
    Contract c("path-state", "limit"); RecordingLimits limits; limits.elements = c.input("limit").GetUint();
    PathRecorder r(limits);
    c.observe("diagnostic", diagnostic([&] {
        for (unsigned i = 0; i < c.input("requested").GetUint(); ++i) r.lineTo({double(i), 0});
    }));
    EXPECT_EQ(limits.elements, r.path().size()); c.observe("accepted-output-count", r.commands().size()); c.complete();
    r.newPath(); segment(r); r.stroke(); EXPECT_EQ(1u, r.commands().size());
}
TEST(CpuPainterContractTest, CpuPathStateRestore) {
    Contract c("path-state", "restore"); PathRecorder r;
    const auto& path = c.input("path"); r.moveTo(point(path[0])); r.lineTo(point(path[1]));
    const auto before = r.state(); operations(r, c.input("operations"));
    c.observe("style-restored", sameStyle(before, r.state()));
    c.observe("path-preserved-by-restore", r.path().size() == path.Size() &&
              r.path()[1].point.x == point(path[1]).x && r.path()[1].point.y == point(path[1]).y); c.complete();
}
TEST(CpuTransformTest, CpuTransformOrdinary) {
    Contract c("transform", "ordinary"); PathRecorder r; r.setTransform(matrix(c.input("transform")));
    const auto& t = r.state().transform; const auto p = t.point(point(c.input("point")));
    c.observe("device-point", p); c.observe("device-distance", t.distance(point(c.input("distance"))));
    c.observe("roundtrip-point", t.inversePoint(p)); c.observe("scale", t.scale()); c.complete();
    const auto d = point(c.input("distance")); const auto roundtrip = t.inverseDistance(t.distance(d));
    EXPECT_DOUBLE_EQ(d.x, roundtrip.x); EXPECT_DOUBLE_EQ(d.y, roundtrip.y);
}
TEST(CpuTransformTest, CpuTransformEmpty) {
    Contract c("transform", "empty"); PathRecorder r; r.setTransform(matrix(c.input("transform")));
    const auto& t = r.state().transform; c.observe("roundtrip-point", t.inversePoint(t.point(point(c.input("point"))))); c.complete();
}
TEST(CpuTransformTest, CpuTransformLimit) {
    Contract c("transform", "limit"); PathRecorder r; segment(r); const auto before = r.state();
    c.observe("diagnostic", diagnostic([&] { r.setTransform(matrix(c.input("transform"))); }));
    c.observe("accepted-output-count", r.commands().size()); c.complete(); EXPECT_TRUE(sameStyle(before, r.state()));
    r.scale(2); r.stroke(); ASSERT_EQ(1u, r.commands().size()); EXPECT_DOUBLE_EQ(3, r.commands()[0].path[1].point.x);
}
TEST(CpuTransformTest, CpuTransformLargeOrigin) {
    Contract c("transform", "large-origin"); PathRecorder r; Transform t; t.origin = point(c.input("origin"));
    t.xx = t.yy = c.input("pixels-per-model").GetDouble(); r.setTransform(t);
    const auto& points = c.input("local-points");
    for (const auto& p : points.GetArray()) r.lineTo({t.origin.x + point(p).x, t.origin.y + point(p).y});
    ASSERT_EQ(2u, r.path().size());
    c.observe("device-delta", Point{r.path()[1].point.x - r.path()[0].point.x, r.path()[1].point.y - r.path()[0].point.y}); c.complete();
}
TEST(CpuTransformTest, CpuTransformNonfinite) {
    Contract c("transform", "nonfinite"); PathRecorder r; EXPECT_STREQ("NaN", c.input("scale").GetString());
    c.observe("diagnostic", diagnostic([&] { r.scale(std::numeric_limits<double>::quiet_NaN()); }));
    c.observe("accepted-output-count", r.commands().size()); c.complete();
    EXPECT_TRUE(sameTransform(Transform{}, r.state().transform)); segment(r); r.stroke(); EXPECT_EQ(1u, r.commands().size());
}
TEST(CpuPainterContractTest, SnapshotsOwnFullSupportedStateAndPaths) {
    PathRecorder r; std::vector<double> dash{1, 2, 3}; r.dash(dash, -0.5, true); dash[0] = 99;
    r.source(0.25, 0.5, 0.75, 0.5); r.lineWidth(3); r.lineWidthCompensation(-1); r.antialias(false); r.translate(10, 20);
    const auto before = r.state(); segment(r); r.save(); r.save();
    r.source(1, 0, 0); r.lineWidth(2); r.lineWidthCompensation(2); r.dash({4}, 3); r.antialias(true); r.scale(2);
    r.lineTo({9, 9}); r.restore(); r.restore(); EXPECT_TRUE(sameStyle(before, r.state()));
    ASSERT_EQ(3u, r.path().size()); EXPECT_DOUBLE_EQ(28, r.path()[2].point.x); EXPECT_DOUBLE_EQ(38, r.path()[2].point.y);
    r.stroke(); ASSERT_EQ(1u, r.commands().size()); EXPECT_TRUE(sameStyle(before, r.commands()[0].state));
    r.source(0, 1, 0); r.dash({8}); segment(r); r.fill();
    ASSERT_EQ(2u, r.commands().size()); EXPECT_TRUE(sameStyle(before, r.commands()[0].state));
    EXPECT_DOUBLE_EQ(1, r.commands()[0].state.dash[0]); EXPECT_EQ(PaintOperation::Fill, r.commands()[1].operation);
    r.stroke(); r.fill(); EXPECT_EQ(2u, r.commands().size()); EXPECT_TRUE(r.path().empty());
}
TEST(CpuPainterContractTest, SubpathsResetCloseAndRectangleAreAtomic) {
    PathRecorder r; r.closePath(); r.newSubPath(); r.lineTo({2, 3}); EXPECT_EQ(PathVerb::Move, r.path()[0].verb);
    r.lineTo({3, 4}); r.closePath(); r.closePath(); EXPECT_EQ(3u, r.path().size());
    r.lineTo({4, 5}); ASSERT_EQ(5u, r.path().size()); EXPECT_EQ(PathVerb::Move, r.path()[3].verb);
    r.newSubPath(); r.lineTo({10, 20}); EXPECT_EQ(PathVerb::Move, r.path().back().verb);
    r.newPath(); r.stroke(); EXPECT_TRUE(r.commands().empty());
    r.rectangle({1, 2}, -3, 4); ASSERT_EQ(5u, r.path().size()); EXPECT_DOUBLE_EQ(-2, r.path()[2].point.x);
    const auto size = r.path().size();
    EXPECT_EQ("invalid-point", diagnostic([&] { r.rectangle({0, 0}, std::numeric_limits<double>::infinity(), 2); }));
    EXPECT_EQ(size, r.path().size()); r.fill(); r.fill(); EXPECT_EQ(1u, r.commands().size());
}
TEST(CpuPainterContractTest, BoundedRecordingAndStackRecoverWithoutDiscardingValidWork) {
    RecordingLimits limits; limits.elements = 4; limits.commands = 1; limits.stackDepth = 1; limits.dashElements = 2;
    PathRecorder r(limits); r.save(); EXPECT_EQ("state-limit", diagnostic([&] { r.save(); })); r.restore();
    EXPECT_EQ("state-underflow", diagnostic([&] { r.restore(); }));
    EXPECT_EQ("dash-limit", diagnostic([&] { r.dash({1, 2, 3}); })); EXPECT_TRUE(r.state().dash.empty());
    EXPECT_EQ("vertex-limit", diagnostic([&] { r.rectangle({0, 0}, 1, 1); })); EXPECT_TRUE(r.path().empty());
    segment(r); r.stroke(); segment(r); EXPECT_EQ("command-limit", diagnostic([&] { r.fill(); }));
    EXPECT_EQ(2u, r.path().size()); EXPECT_EQ("vertex-limit", diagnostic([&] { r.lineTo({8, 9}); }));
    r.clearCommands(); EXPECT_EQ(2u, r.path().size()); r.fill(); EXPECT_EQ(1u, r.commands().size());
}
TEST(CpuPainterContractTest, InvalidStyleAndPointsLeaveStateAndPathUsable) {
    PathRecorder r; segment(r); const auto before = r.state();
    for (double v : {-1.0, std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()}) {
        EXPECT_EQ("invalid-color", diagnostic([&] { r.source(v, 0, 0); }));
        EXPECT_EQ("invalid-line-width", diagnostic([&] { r.lineWidth(v); }));
        EXPECT_EQ("invalid-dash", diagnostic([&] { r.dash({v}); }));
    }
    EXPECT_EQ("invalid-dash", diagnostic([&] { r.dash({0, 0}); }));
    EXPECT_EQ("invalid-point", diagnostic([&] { r.lineTo({std::numeric_limits<double>::quiet_NaN(), 0}); }));
    EXPECT_TRUE(sameStyle(before, r.state())); EXPECT_EQ(2u, r.path().size()); r.stroke(); EXPECT_EQ(1u, r.commands().size());
}
TEST(CpuPainterContractTest, AggregateDashSnapshotsAreBoundedAndRecoverable) {
    RecordingLimits limits; limits.elements = 6;
    PathRecorder r(limits); r.dash({1, 2, 3, 4}); segment(r); r.stroke(); segment(r);
    EXPECT_EQ("state-limit", diagnostic([&] { r.stroke(); }));
    EXPECT_EQ(1u, r.commands().size()); EXPECT_EQ(2u, r.path().size());
    r.dash({1, 2}); r.stroke(); EXPECT_EQ(2u, r.commands().size());
    r.clearCommands(); segment(r); r.stroke(); EXPECT_EQ(1u, r.commands().size());
    limits.elements = MaxFrameElements + 1;
    EXPECT_EQ("invalid-limits", diagnostic([&] { PathRecorder invalid(limits); }));
}
TEST(CpuPainterContractTest, LongPathsGrowStorageGeometrically) {
    // Count storage growth, not wall time: reserve(size + 1) on every append
    // turns a bounded path into quadratic copying on all supported STL providers.
    for (bool closed : {false, true}) {
        PathRecorder r; r.moveTo({0, 0}); std::size_t growths = 0;
        for (unsigned i = 1; i <= 4096; ++i) {
            const auto capacity = r.path().capacity();
            if (closed) r.closePath();
            r.lineTo({double(i), double(i % 7)});
            growths += r.path().capacity() != capacity;
        }
        RecordProperty(closed ? "closed-growths" : "open-growths", int(growths));
        EXPECT_LE(growths, 32u) << "closed=" << closed;
        EXPECT_EQ(closed ? 12289u : 4097u, r.path().size());
        EXPECT_DOUBLE_EQ(4096, r.path().back().point.x);
        r.stroke(); ASSERT_EQ(1u, r.commands().size()); EXPECT_TRUE(r.path().empty());
    }
}
TEST(CpuTransformTest, CompositionResetReflectionAndAppendTimeCapture) {
    PathRecorder r; r.moveTo({0, 0}); r.translate(10, 20); r.lineTo({1, 2}); r.resetTransform();
    EXPECT_DOUBLE_EQ(0, r.path()[0].point.x); EXPECT_DOUBLE_EQ(11, r.path()[1].point.x); EXPECT_DOUBLE_EQ(22, r.path()[1].point.y);
    r.translate(10, 20); r.rotate(std::acos(-1.0) / 2); r.scale(2);
    const auto p = r.state().transform.point({3, 4}); EXPECT_NEAR(2, p.x, 1e-12); EXPECT_NEAR(26, p.y, 1e-12);
    EXPECT_NEAR(2, r.state().transform.scale(), 1e-12); r.scale(-1);
    const auto q = r.state().transform.inversePoint(r.state().transform.point({3, 4}));
    EXPECT_NEAR(3, q.x, 1e-12); EXPECT_NEAR(4, q.y, 1e-12); r.resetTransform();
    EXPECT_TRUE(sameTransform(Transform{}, r.state().transform)); EXPECT_EQ(2u, r.path().size());
}
TEST(CpuTransformTest, SingularNonfiniteAndOverflowAreRecoverable) {
    PathRecorder r; r.translate(1, 2); const auto before = r.state(); const double max = std::numeric_limits<double>::max();
    for (double v : {0.0, std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()})
        EXPECT_EQ("invalid-transform", diagnostic([&] { r.scale(v); }));
    Transform singular; singular.xy = singular.yx = 1;
    EXPECT_EQ("invalid-transform", diagnostic([&] { r.setTransform(singular); }));
    EXPECT_EQ("invalid-transform", diagnostic([&] { r.rotate(std::numeric_limits<double>::infinity()); }));
    EXPECT_TRUE(sameStyle(before, r.state()));
    Transform large; large.xx = large.yy = max; r.setTransform(large); r.moveTo({0, 0});
    EXPECT_EQ("invalid-transform", diagnostic([&] { r.lineTo({2, 2}); })); EXPECT_EQ(1u, r.path().size());
    r.resetTransform(); r.lineTo({3, 4}); r.stroke(); EXPECT_EQ(1u, r.commands().size());
    for (double v : {1e-200, 1e200}) { Transform t; t.xx = t.yy = v; r.setTransform(t);
        const auto p = t.inversePoint(t.point({1, 2})); EXPECT_NEAR(1, p.x, 1e-12); EXPECT_NEAR(2, p.y, 1e-12); }
    Transform tiny; tiny.xx = tiny.yy = 1e-200; r.setTransform(tiny);
    EXPECT_EQ("invalid-transform", diagnostic([&] { tiny.inverseDistance({max, max}); }));
    EXPECT_EQ("invalid-transform", diagnostic([&] { tiny.point({std::numeric_limits<double>::infinity(), 0}); }));
    EXPECT_TRUE(sameTransform(tiny, r.state().transform));
}

TEST(CpuTransformTest, AffineCancellationPreservesModelDetailBeforePreparation) {
    using Exact=boost::multiprecision::cpp_bin_float_100;
    const Point a{1,std::nextafter(-1.,0.)},b{std::nextafter(1.,2.),-1};
    for (double magnitude:{1e12,1e16,1e20}) for (double sign:{-1.,1.}) {
        Transform t;t.xx=sign*magnitude;t.xy=sign*magnitude;t.yx=1;t.yy=-1;
        PathRecorder r;r.setTransform(t);r.moveTo(a);r.lineTo(b);r.stroke();
        const auto& command=r.commands().front();
        for (std::size_t i=0;i<2;++i) {
            const Point p=i?b:a;
            const double x=double(Exact(t.xx)*Exact(p.x)+Exact(t.xy)*Exact(p.y));
            const double y=double(Exact(t.yx)*Exact(p.x)+Exact(t.yy)*Exact(p.y));
            EXPECT_DOUBLE_EQ(x,command.path[i].point.x);
            EXPECT_DOUBLE_EQ(y,command.path[i].point.y);
        }
        const auto mesh=prepareStroke(command);
        EXPECT_FALSE(mesh.triangles.empty());EXPECT_LE(mesh.positionBound,.25);
        // At1e16 the original rounded products shortened this segment by .89px.
        EXPECT_GT(std::abs(command.path[1].point.x-command.path[0].point.x),0.);
    }
    Transform t;t.origin={1,-1};t.tx=-1e16;t.ty=1e16;
    const Point model{1e16+2,-1e16-2};
    const auto physical=t.point(model);
    EXPECT_DOUBLE_EQ(1,physical.x);EXPECT_DOUBLE_EQ(-1,physical.y);
    const auto restored=t.inversePoint(physical);
    EXPECT_DOUBLE_EQ(model.x,restored.x);EXPECT_DOUBLE_EQ(model.y,restored.y);
    PathRecorder r;r.setTransform(t);r.moveTo(model);
    EXPECT_DOUBLE_EQ(1,r.path().front().point.x);EXPECT_DOUBLE_EQ(-1,r.path().front().point.y);
    // Unrepresentable products still fail atomically with the existing diagnostic.
    Transform overflow;overflow.xx=overflow.yy=std::numeric_limits<double>::max();
    r.setTransform(overflow);
    EXPECT_EQ("invalid-transform",diagnostic([&]{r.lineTo({2,2});}));
    ASSERT_EQ(1u,r.path().size());r.resetTransform();r.lineTo({2,2});r.stroke();
    ASSERT_EQ(1u,r.commands().size());EXPECT_EQ(2u,r.commands().front().path.size());
}

namespace {
// Independent Bernstein evaluation in long double, not the subdivision routine.
Point evaluate(const std::vector<Point>& p, long double t) {
    const long double u = 1-t;
    if (p.size() == 3) return {double(u*u*p[0].x+2*u*t*p[1].x+t*t*p[2].x),
                               double(u*u*p[0].y+2*u*t*p[1].y+t*t*p[2].y)};
    return {double(u*u*u*p[0].x+3*u*u*t*p[1].x+3*u*t*t*p[2].x+t*t*t*p[3].x),
            double(u*u*u*p[0].y+3*u*u*t*p[1].y+3*u*t*t*p[2].y+t*t*t*p[3].y)};
}
double sampledDistance(Point p, const Polyline& line) {
    long double best = std::numeric_limits<long double>::infinity();
    for (std::size_t i=1; i<line.points.size(); ++i) {
        const auto a=line.points[i-1], b=line.points[i];
        const long double dx=static_cast<long double>(b.x)-a.x, dy=static_cast<long double>(b.y)-a.y;
        const long double n=dx*dx+dy*dy;
        const long double t=n ? std::max(0.L,std::min(1.L,((p.x-a.x)*dx+(p.y-a.y)*dy)/n)) : 0;
        best=std::min(best,std::hypot(p.x-a.x-t*dx,p.y-a.y-t*dy));
    }
    return double(best);
}
PathCommand recorded(PathRecorder& r) {
    r.stroke(); if (r.commands().size()!=1) throw std::runtime_error("expected one path command");
    return r.commands().front();
}
std::vector<Point> controls(const json::Value& values, std::size_t begin, std::size_t count) {
    std::vector<Point> p; for (std::size_t i=0; i<count; ++i) p.push_back(point(values[json::SizeType(begin+i)])); return p;
}
void sampleBezier(const std::vector<Point>& p, const Polyline& line, double bound) {
    for (unsigned i=0; i<=4096; ++i) EXPECT_LE(sampledDistance(evaluate(p,i/4096.L),line),bound+1e-10);
}
}
TEST(CpuCurveTest, CpuSplineOrdinary) {
    Contract c("spline","ordinary"); PathRecorder r; const auto& counts=c.input("span-control-counts");
    const auto& cp=c.input("control-points"); std::vector<std::vector<Point>> spans; std::size_t offset=0;
    for (const auto& count:counts.GetArray()) {
        spans.push_back(controls(cp,offset,count.GetUint())); r.bezierSpan(spans.back()); offset+=count.GetUint();
    }
    EXPECT_EQ(cp.Size(),offset); const auto f=flatten(recorded(r)); ASSERT_EQ(3u,f.subpaths.size());
    const auto& endpoints=c.expected("endpoints"); ASSERT_EQ(3u,endpoints.Size());
    for (unsigned i=0; i<3; ++i) {
        EXPECT_DOUBLE_EQ(endpoints[i][0].GetDouble(),f.subpaths[i].points.back().x);
        EXPECT_DOUBLE_EQ(endpoints[i][1].GetDouble(),f.subpaths[i].points.back().y);
    }
    c.observe("quadratic-midpoint",evaluate(spans[1],0.5L)); c.observe("cubic-midpoint",evaluate(spans[2],0.5L));
    double error=0;
    for (unsigned i=1; i<3; ++i) for (unsigned j=0; j<=4096; ++j)
        error=std::max(error,sampledDistance(evaluate(spans[i],j/4096.L),f.subpaths[i]));
    EXPECT_LE(error,CurveLimits{}.positionError); EXPECT_LE(f.positionBound,CurveLimits{}.positionError);
    EXPECT_NEAR(c.expected("max-display-error").GetDouble(),error,0.25); c.complete();
}
TEST(CpuCurveTest, CpuSplineEmpty) {
    Contract c("spline","empty"); PathRecorder r; EXPECT_TRUE(c.input("control-points").Empty()); r.stroke();
    c.observe("draw-count",r.commands().size()); c.complete();
}
TEST(CpuCurveTest, CpuSplineLimit) {
    Contract c("spline","limit"); PathRecorder r; r.moveTo({0,0}); r.cubicTo({0,10},{10,0},{10,10});
    const auto command=recorded(r); CurveLimits limits; limits.work=c.input("limit").GetUint();
    EXPECT_EQ(limits.work+1,c.input("requested").GetUint()); limits.positionError=1e-9;
    FlattenedPath output;
    c.observe("diagnostic",diagnostic([&] { output=flatten(command,limits); }));
    c.observe("accepted-output-count",output.subpaths.size()); c.complete();
    EXPECT_EQ(2u,command.path.size()); EXPECT_FALSE(flatten(command).subpaths.empty());
}
TEST(CpuCurveTest, CpuSplineClosed) {
    Contract c("spline","closed"); PathRecorder r; const auto p=controls(c.input("control-points"),0,4);
    r.bezierSpan(p); const auto f=flatten(recorded(r)); const auto& line=f.subpaths.front();
    c.observe("end",line.points.back());
    c.observe("closed",line.points.front().x==line.points.back().x && line.points.front().y==line.points.back().y);
    c.complete(); sampleBezier(p,line,CurveLimits{}.positionError);
}
TEST(CpuCurveTest, CpuSplinePeriodic) {
    Contract c("spline","periodic"); PathRecorder r; const auto& cp=c.input("control-points");
    const auto& counts=c.input("span-control-counts"); std::size_t offset=0; r.moveTo(point(cp[0]));
    for (const auto& count:counts.GetArray()) {
        EXPECT_EQ(4u,count.GetUint()); const auto p=controls(cp,offset,4); r.cubicTo(p[1],p[2],p[3]); offset+=4;
    }
    EXPECT_EQ(cp.Size(),offset); r.closePath(); const auto f=flatten(recorded(r)); ASSERT_EQ(1u,f.subpaths.size());
    c.observe("seam-tangent-gap",Point{3*((point(cp[1]).x-point(cp[0]).x)-(point(cp[15]).x-point(cp[14]).x)),
                                      3*((point(cp[1]).y-point(cp[0]).y)-(point(cp[15]).y-point(cp[14]).y))});
    c.observe("closed",f.subpaths.front().closed); c.complete();
}
TEST(CpuCurveTest, CpuSplineHighCurvature) {
    Contract c("spline","high-curvature"); auto p=controls(c.input("control-points"),0,4);
    const auto& zd=c.input("zoom-dpr"); const double scale=zd[0].GetDouble()*zd[1].GetDouble();
    PathRecorder r; r.scale(scale); r.bezierSpan(p); const auto f=flatten(recorded(r));
    for (auto& v:p) { v.x*=scale; v.y*=scale; }
    double error=0;
    for (unsigned i=0; i<=4096; ++i) error=std::max(error,sampledDistance(evaluate(p,i/4096.L),f.subpaths[0]));
    EXPECT_NEAR(c.expected("max-display-error").GetDouble(),error,0.25);
    EXPECT_LE(error,CurveLimits{}.positionError);
    bool finite=true; for (auto v:f.subpaths[0].points) finite &= std::isfinite(v.x)&&std::isfinite(v.y);
    c.observe("finite-output",finite); c.complete(); EXPECT_GT(f.lengthLower,290); EXPECT_GT(f.visited,1u);
    EXPECT_LE(f.lengthUpper-f.lengthLower,CurveLimits{}.lengthError);
}
TEST(CpuCurveTest, CpuSplineRationalDomain) {
    Contract c("spline","rational-domain"); EXPECT_STREQ("current-bezier-spans",c.input("document-provider").GetString());
    auto p=controls(c.input("control-points"),0,3); std::vector<double> w;
    for (const auto& v:c.input("weights").GetArray()) w.push_back(v.GetDouble()); PathRecorder r;
    c.observe("diagnostic",diagnostic([&] { r.bezierSpan(p,w); }));
    c.observe("accepted-output-count",r.path().size()); c.complete(); r.bezierSpan(p); EXPECT_FALSE(recorded(r).path.empty());
}
TEST(CpuCurveTest, ExactConicsBoundCircleLengthAndRotatedEllipseError) {
    constexpr double pi=3.14159265358979323846; PathRecorder r; r.circle({2,3},10);
    const auto circle=flatten(recorded(r)); ASSERT_EQ(1u,circle.subpaths.size()); EXPECT_TRUE(circle.subpaths[0].closed);
    EXPECT_LE(circle.lengthLower,20*pi); EXPECT_GE(circle.lengthUpper,20*pi);
    EXPECT_LE(circle.lengthUpper-circle.lengthLower,CurveLimits{}.lengthError);
    for (unsigned i=0; i<=8192; ++i) {
        const double a=2*pi*i/8192; EXPECT_LE(sampledDistance({2+10*std::cos(a),3+10*std::sin(a)},circle.subpaths[0]),CurveLimits{}.positionError);
    }
    PathRecorder e; e.ellipse({0,0},30,2,0,2*pi,0.73); const auto ellipse=flatten(recorded(e));
    // Independent composite Simpson integral of the analytic ellipse speed.
    long double length=0; constexpr unsigned samples=65536;
    for (unsigned i=0; i<=samples; ++i) {
        const long double a=2*static_cast<long double>(pi)*i/samples;
        length+=(i==0 || i==samples ? 1 : i%2 ? 4 : 2)*std::hypot(30*std::sin(a),2*std::cos(a));
    }
    length*=2*static_cast<long double>(pi)/(3*samples);
    EXPECT_LE(ellipse.lengthLower,length); EXPECT_GE(ellipse.lengthUpper,length);
    EXPECT_LE(ellipse.lengthUpper-ellipse.lengthLower,CurveLimits{}.lengthError);
    for (unsigned i=0; i<=8192; ++i) {
        const double a=2*pi*i/8192,x=30*std::cos(a),y=2*std::sin(a);
        EXPECT_LE(sampledDistance({x*std::cos(.73)-y*std::sin(.73),x*std::sin(.73)+y*std::cos(.73)},ellipse.subpaths[0]),CurveLimits{}.positionError);
    }
}
TEST(CpuCurveTest, ClockwiseArcsBulgesAndClosedPolylineLength) {
    constexpr double pi=3.14159265358979323846;
    for (double sign:{-1.,1.}) {
        PathRecorder r; r.arc({0,0},2,0,sign*pi/2,sign<0); const auto f=flatten(recorded(r));
        EXPECT_NEAR(0,f.subpaths[0].points.back().x,1e-14); EXPECT_NEAR(sign*2,f.subpaths[0].points.back().y,1e-14);
        EXPECT_LE(f.lengthLower,pi); EXPECT_GE(f.lengthUpper,pi);
        PathRecorder b; b.polyline({{-1,0},{1,0}},{sign},false); const auto bf=flatten(recorded(b));
        EXPECT_LE(bf.lengthLower,pi); EXPECT_GE(bf.lengthUpper,pi);
        EXPECT_DOUBLE_EQ(1,bf.subpaths[0].points.back().x);
    }
    PathRecorder r; r.polyline({{0,0},{3,0},{3,4}},{0,0,0},true); const auto f=flatten(recorded(r));
    EXPECT_TRUE(f.subpaths[0].closed); EXPECT_LE(f.lengthLower,12); EXPECT_GE(f.lengthUpper,12);
    EXPECT_LE(f.lengthUpper-f.lengthLower,CurveLimits{}.lengthError);
}
TEST(CpuCurveTest, TinyBulgesRetainTheSegmentInsteadOfDroppingThePath) {
    for (double bulge : {1e-20,-1e-20,1e-12,-1e-12}) {
        PathRecorder r; r.polyline({{-1,0},{1,0}},{bulge},false); r.stroke();
        ASSERT_EQ(1u,r.commands().size()) << bulge;
        const auto f=flatten(r.commands()[0]); ASSERT_EQ(1u,f.subpaths.size());
        EXPECT_DOUBLE_EQ(-1,f.subpaths[0].points.front().x);
        EXPECT_DOUBLE_EQ(1,f.subpaths[0].points.back().x);
        EXPECT_NEAR(2,f.lengthLower,1e-12); EXPECT_NEAR(2,f.lengthUpper,1e-12);
    }
}
TEST(CpuCurveTest, CurvesCaptureTransformAndRetainLargeLocalOrigin) {
    PathRecorder r; Transform t; t.origin={1e12,-1e12}; t.xx=t.yy=8; r.setTransform(t);
    r.moveTo({1e12,-1e12}); r.quadraticTo({1e12+.5,-1e12+.5},{1e12+1,-1e12});
    r.resetTransform(); r.cubicTo({9,3},{12,-3},{15,0}); const auto command=recorded(r);
    EXPECT_DOUBLE_EQ(4,command.path[1].control1.x); EXPECT_DOUBLE_EQ(4,command.path[1].control1.y);
    EXPECT_DOUBLE_EQ(8,command.path[1].point.x); const auto f=flatten(command);
    EXPECT_DOUBLE_EQ(15,f.subpaths[0].points.back().x);
    PathRecorder a; a.setTransform(t); a.circle({1e12,-1e12},.125); const auto af=flatten(recorded(a));
    EXPECT_NEAR(2*3.14159265358979323846, (af.lengthLower+af.lengthUpper)/2,CurveLimits{}.lengthError);
}
TEST(CpuCurveTest, WorkCapCountsMoveLineCloseAndEmptySubpaths) {
    PathRecorder r;segment(r);r.closePath();const auto command=recorded(r);ASSERT_EQ(3u,command.path.size());
    CurveLimits limits;limits.work=command.path.size()-1;FlattenedPath output;
    EXPECT_EQ("subdivision-limit",diagnostic([&]{output=flatten(command,limits);}));EXPECT_TRUE(output.subpaths.empty());
    ++limits.work;const auto full=flatten(command,limits);EXPECT_EQ(command.path.size(),full.visited);
    ASSERT_EQ(1u,full.subpaths.size());EXPECT_TRUE(full.subpaths[0].closed);
    PathCommand moves{PaintOperation::Stroke,{},{{PathVerb::Move,{0,0}},{PathVerb::Move,{1,0}},
                                               {PathVerb::Move,{2,0}},{PathVerb::Move,{3,0}}}};
    limits.work=3;EXPECT_EQ("subdivision-limit",diagnostic([&]{flatten(moves,limits);}));
    limits.work=4;const auto empty=flatten(moves,limits);EXPECT_EQ(4u,empty.visited);EXPECT_EQ(4u,empty.subpaths.size());
    // Oversized input is rejected before walking it or allocating subpaths.
    moves.path[0].point.x=NAN;limits.work=3;
    EXPECT_EQ("subdivision-limit",diagnostic([&]{flatten(moves,limits);}));
    limits.work=0;PathCommand none{PaintOperation::Stroke,{}, {}};EXPECT_EQ(0u,flatten(none,limits).visited);
    EXPECT_EQ("subdivision-limit",diagnostic([&]{flatten(command,limits);}));
}
TEST(CpuCurveTest, MixedCurveWorkSharesTheCapAndFailurePreservesPriorOutput) {
    PathRecorder r;r.moveTo({0,0});r.lineTo({1,0});r.quadraticTo({2,3},{3,0});r.lineTo({4,0});r.closePath();
    const auto command=recorded(r);const auto full=flatten(command);
    EXPECT_GT(full.visited,command.path.size());
    PathRecorder prior;segment(prior);auto output=flatten(recorded(prior));const auto before=output.subpaths[0].points;
    CurveLimits limits;limits.work=full.visited-1;
    EXPECT_EQ("subdivision-limit",diagnostic([&]{output=flatten(command,limits);}));
    ASSERT_EQ(1u,output.subpaths.size());ASSERT_EQ(before.size(),output.subpaths[0].points.size());
    for (unsigned i=0;i<before.size();++i) {EXPECT_DOUBLE_EQ(before[i].x,output.subpaths[0].points[i].x);
        EXPECT_DOUBLE_EQ(before[i].y,output.subpaths[0].points[i].y);}
    ++limits.work;output=flatten(command,limits);EXPECT_EQ(full.visited,output.visited);
    EXPECT_EQ(full.subpaths[0].points.size(),output.subpaths[0].points.size());
    EXPECT_DOUBLE_EQ(full.lengthLower,output.lengthLower);EXPECT_DOUBLE_EQ(full.lengthUpper,output.lengthUpper);
    const auto stroke=prepareStroke(command);StrokeLimits cap;cap.work=stroke.visited-1;
    EXPECT_EQ("subdivision-limit",diagnostic([&]{prepareStroke(command,{},cap);}));
    ++cap.work;EXPECT_EQ(stroke.visited,prepareStroke(command,{},cap).visited);
}
TEST(CpuCurveTest, InvalidCurvesLimitsAndRestartPreservePriorWork) {
    PathRecorder r; segment(r); const auto before=r.path().size();
    EXPECT_EQ("invalid-curve",diagnostic([&] { r.arc({0,0},-1,0,1); }));
    EXPECT_EQ("invalid-point",diagnostic([&] { r.cubicTo({0,0},{1,1},{NAN,0}); }));
    EXPECT_EQ("unsupported-span",diagnostic([&] { r.bezierSpan({{1,2}}); }));
    EXPECT_EQ(before,r.path().size()); r.closePath(); r.arc({0,0},1,0,1); const auto command=recorded(r);
    EXPECT_EQ(2u,flatten(command).subpaths.size());
    CurveLimits limits; limits.vertices=1; EXPECT_EQ("vertex-limit",diagnostic([&] { flatten(command,limits); }));
    limits={}; limits.depth=0; EXPECT_EQ("subdivision-limit",diagnostic([&] { flatten(command,limits); }));
    limits={}; limits.positionError=.25; EXPECT_EQ("invalid-limits",diagnostic([&] { flatten(command,limits); }));
    EXPECT_FALSE(flatten(command).subpaths.empty());
    PathRecorder p; EXPECT_EQ("precision-limit",diagnostic([&] { p.arc({1e16,1e16},10,0,1); })); EXPECT_TRUE(p.path().empty());
}

namespace {
// Independent adaptive high-precision half-plane oracle, not topology math.
// Apple ARM64 long double is not assumed to provide extra significand bits.
int orientation(Point a,Point b,Point p) {
    const double x=(b.x-a.x)*(p.y-a.y),y=(b.y-a.y)*(p.x-a.x),d=x-y;
    if (std::abs(d)>16*std::numeric_limits<double>::epsilon()*(std::abs(x)+std::abs(y))) return d>0?1:-1;
    using Exact=boost::multiprecision::cpp_bin_float_100;
    const Exact exact=(Exact(b.x)-a.x)*(Exact(p.y)-a.y)-(Exact(b.y)-a.y)*(Exact(p.x)-a.x);
    return exact>0?1:exact<0?-1:0;
}
bool inTriangle(Point p,const std::array<Point,3>& t) {
    if (p.x<std::min({t[0].x,t[1].x,t[2].x}) || p.x>std::max({t[0].x,t[1].x,t[2].x}) ||
        p.y<std::min({t[0].y,t[1].y,t[2].y}) || p.y>std::max({t[0].y,t[1].y,t[2].y})) return false;
    if (orientation(t[0],t[1],t[2])<=0) return false;
    for (unsigned i=0;i<3;++i) {
        if (orientation(t[i],t[(i+1)%3],p)<0) return false;
    }
    return true;
}
std::size_t overlaps(const PreparedStroke& s,Point p) {
    std::size_t hits=0;for (const auto& t:s.triangles) if (inTriangle(p,t)) ++hits;return hits;
}
bool covers(const PreparedStroke& s,Point p) {
    for (const auto& t:s.triangles) if (inTriangle(p,t)) return true;return false;
}
PathCommand strokeLine(Point a={0,0},Point b={4,0},double width=2) {
    PathRecorder r;r.lineWidth(width);r.moveTo(a);r.lineTo(b);return recorded(r);
}
}
namespace {
std::vector<double> numbers(const json::Value& a) {
    std::vector<double> result; for (const auto& v:a.GetArray()) result.push_back(v.GetDouble()); return result;
}
std::vector<Point> points(const json::Value& a) { return controls(a,0,a.Size()); }
void linePath(PathRecorder& r,const json::Value& values,bool closed=false) {
    if (values.Empty()) return;
    r.moveTo(point(values[0]));for (unsigned i=1;i<values.Size();++i) r.lineTo(point(values[i]));
    if (closed) r.closePath();
}
std::vector<double> bounds(const Polyline& line) {
    Point low=line.points.front(),high=low;
    for (auto p:line.points) {low.x=std::min(low.x,p.x);low.y=std::min(low.y,p.y);
        high.x=std::max(high.x,p.x);high.y=std::max(high.y,p.y);}
    return {low.x,low.y,high.x,high.y};
}
double lineLength(const Polyline& line) {
    double length=0;for (std::size_t i=1;i<line.points.size();++i)
        length+=std::hypot(line.points[i].x-line.points[i-1].x,line.points[i].y-line.points[i-1].y);
    if (line.closed) length+=std::hypot(line.points.back().x-line.points.front().x,line.points.back().y-line.points.front().y);
    return length;
}
// Independent rational Bernstein evaluation/differentiation and composite
// Simpson integration of the OWNED conics, never the input circle formula or
// production subdivision. Refinement agreement supplements the corpus oracle.
double conicIntegral(const PathCommand& c, bool area, unsigned panels) {
    long double sum=0;Point a{};
    for (const auto& e:c.path) {
        if (e.verb==PathVerb::Conic) {
            const auto integrand=[&](long double t) {
                const long double u=1-t,w=e.weight;
                const long double d=u*u+2*w*u*t+t*t,dd=2*((w-1)*u+(1-w)*t);
                const long double nx=u*u*a.x+2*w*u*t*e.control1.x+t*t*e.point.x;
                const long double ny=u*u*a.y+2*w*u*t*e.control1.y+t*t*e.point.y;
                const long double dx=(2*(u*(w*e.control1.x-a.x)+t*(e.point.x-w*e.control1.x))*d-nx*dd)/(d*d);
                const long double dy=(2*(u*(w*e.control1.y-a.y)+t*(e.point.y-w*e.control1.y))*d-ny*dd)/(d*d);
                return area?(nx*dy-ny*dx)/(2*d):std::hypot(dx,dy);
            };
            long double subtotal=integrand(0)+integrand(1);
            for (unsigned i=1;i<panels;++i) subtotal+=(i%2?4:2)*integrand(static_cast<long double>(i)/panels);
            sum+=subtotal/(3*panels);
        } else EXPECT_TRUE(e.verb==PathVerb::Move || e.verb==PathVerb::Close);
        a=e.point;
    }
    return double(sum);
}
double measuredConics(const PathCommand& c,bool area) {
    const double fine=conicIntegral(c,area,1024);
    EXPECT_NEAR(fine,conicIntegral(c,area,512),1e-10);return fine;
}
double sweep(const Polyline& line,Point center) {
    double angle=0;
    for (std::size_t i=1;i<line.points.size();++i) {
        const auto a=line.points[i-1],b=line.points[i];
        angle+=std::atan2((a.x-center.x)*(b.y-center.y)-(a.y-center.y)*(b.x-center.x),
                         (a.x-center.x)*(b.x-center.x)+(a.y-center.y)*(b.y-center.y));
    }
    return angle;
}
void recordingLimit(Contract& c) {
    RecordingLimits limits;limits.elements=c.input("limit").GetUint();PathRecorder r(limits);
    c.observe("diagnostic",diagnostic([&] {
        for (unsigned i=0;i<c.input("requested").GetUint();++i) r.lineTo({double(i),0});
    }));
    EXPECT_EQ(limits.elements,r.path().size());c.observe("accepted-output-count",r.commands().size());c.complete();
}
void curveLimit(Contract& c,PathRecorder& r) {
    const auto command=recorded(r);CurveLimits limits;limits.work=c.input("limit").GetUint();
    EXPECT_EQ(limits.work+1,c.input("requested").GetUint());FlattenedPath output;
    c.observe("diagnostic",diagnostic([&]{output=flatten(command,limits);}));
    c.observe("accepted-output-count",output.subpaths.size());c.complete();
    EXPECT_FALSE(flatten(command).subpaths.empty());
}
void arcCase(const char* variant) {
    Contract c("arc",variant);PathRecorder r;const auto& angles=c.input("angles");
    const auto center=point(c.input("center"));const bool clockwise=std::string(c.input("direction").GetString())=="clockwise";
    r.arc(center,c.input("radius").GetDouble(),angles[0].GetDouble(),angles[1].GetDouble(),clockwise);
    const auto command=recorded(r);const auto f=flatten(command);ASSERT_EQ(1u,f.subpaths.size());const auto& line=f.subpaths.front();
    if (clockwise) c.measure("sweep",sweep(line,center),1e-9);
    else {
        c.observe("start",line.points.front());const double length=measuredConics(command,false);
        c.measure("length",length,1e-9);EXPECT_LE(f.lengthLower,length);EXPECT_GE(f.lengthUpper,length);
    }
    c.observe("end",line.points.back());c.complete();EXPECT_FALSE(prepareStroke(command).triangles.empty());
}
void bulgeCase(const char* variant) {
    Contract c("lwpolyline",variant);PathRecorder r;
    r.polyline(points(c.input("points")),numbers(c.input("bulges")),c.input("closed").GetBool());
    const auto command=recorded(r);const auto f=flatten(command);ASSERT_EQ(1u,f.subpaths.size());const auto& line=f.subpaths.front();
    // Circumcircle through three actual flattened points, independent of bulge
    // conversion. Verify every vertex and the signed sweep, not just a midpoint.
    const auto a=line.points.front(),b=line.points[line.points.size()/2],d=line.points.back();
    const double determinant=2*(a.x*(b.y-d.y)+b.x*(d.y-a.y)+d.x*(a.y-b.y));ASSERT_NE(0,determinant);
    const auto n=[](Point p){return p.x*p.x+p.y*p.y;};
    const Point center={(n(a)*(b.y-d.y)+n(b)*(d.y-a.y)+n(d)*(a.y-b.y))/determinant,
                        (n(a)*(d.x-b.x)+n(b)*(a.x-d.x)+n(d)*(b.x-a.x))/determinant};
    const double radius=std::hypot(a.x-center.x,a.y-center.y);c.measure("bulge-radius",radius,1e-9);
    const double angle=sweep(line,center);c.measure("absolute-sweep",std::abs(angle),1e-9);
    EXPECT_EQ(c.input("bulges")[0].GetDouble()>0,angle>0);
    for (auto p:line.points) EXPECT_NEAR(radius,std::hypot(p.x-center.x,p.y-center.y),1e-9);
    c.complete();EXPECT_FALSE(prepareStroke(command).triangles.empty());
}
}
TEST(CpuCurveTest, CpuLineOrdinary) {
    Contract c("line","ordinary");PathRecorder r;linePath(r,c.input("points"));
    const auto command=recorded(r);const auto f=flatten(command);ASSERT_EQ(1u,f.subpaths.size());
    const auto& line=f.subpaths.front();c.observe("length",lineLength(line));c.observe("endpoint",line.points.back());
    c.observe("bounds",bounds(line));c.complete();EXPECT_FALSE(prepareStroke(command).triangles.empty());
}
TEST(CpuCurveTest, CpuLineEmpty) {
    Contract c("line","empty");PathRecorder r;linePath(r,c.input("points"));r.stroke();
    const auto prepared=prepareStrokes(r.commands());std::size_t ink=0;for (const auto& s:prepared) ink+=!s.triangles.empty();
    c.observe("draw-count",ink);c.complete();
}
TEST(CpuCurveTest, CpuLineLimit) { Contract c("line","limit");recordingLimit(c); }
TEST(CpuCurveTest, CpuArcOrdinary) { arcCase("ordinary"); }
TEST(CpuCurveTest, CpuArcClockwise) { arcCase("clockwise"); }
TEST(CpuCurveTest, CpuArcEmpty) {
    Contract c("arc","empty");PathRecorder r;const auto& a=c.input("angles");
    r.arc(point(c.input("center")),c.input("radius").GetDouble(),a[0].GetDouble(),a[1].GetDouble());r.stroke();
    c.observe("draw-count",r.commands().size());c.complete();
}
TEST(CpuCurveTest, CpuArcLimit) { Contract c("arc","limit");PathRecorder r;r.arc({0,0},2,0,1.5707963267948966);curveLimit(c,r); }
TEST(CpuCurveTest, CpuCircleOrdinary) {
    Contract c("circle","ordinary");PathRecorder r;const auto center=point(c.input("center"));const double radius=c.input("radius").GetDouble();
    r.circle(center,radius);const auto command=recorded(r);const auto f=flatten(command);ASSERT_EQ(1u,f.subpaths.size());
    const auto& line=f.subpaths.front();EXPECT_TRUE(line.closed);c.observe("bounds",bounds(line));
    c.measure("area",measuredConics(command,true),1e-9);double error=0;
    for (unsigned i=0;i<=4096;++i) {const double a=2*std::acos(-1.)*i/4096;
        error=std::max(error,sampledDistance({center.x+radius*std::cos(a),center.y+radius*std::sin(a)},line));}
    EXPECT_LE(error,f.positionBound+1e-10);c.measure("max-display-error",error,.25);c.complete();
    EXPECT_FALSE(prepareStroke(command).triangles.empty());
}
TEST(CpuCurveTest, CpuCircleEmpty) {
    Contract c("circle","empty");PathRecorder r;r.circle(point(c.input("center")),c.input("radius").GetDouble());r.stroke();
    c.observe("draw-count",r.commands().size());c.complete();
}
TEST(CpuCurveTest, CpuCircleLimit) { Contract c("circle","limit");PathRecorder r;r.circle({0,0},2);curveLimit(c,r); }
TEST(CpuCurveTest, CpuEllipseOrdinary) {
    Contract c("ellipse","ordinary");PathRecorder r;const auto center=point(c.input("center")),radii=point(c.input("radii"));
    const auto angles=numbers(c.input("angles"));const double rotation=c.input("rotation").GetDouble();
    r.ellipse(center,radii.x,radii.y,angles[0],angles[1],rotation);const auto command=recorded(r);const auto f=flatten(command);
    ASSERT_EQ(1u,f.subpaths.size());const auto& line=f.subpaths.front();
    c.observe("start",line.points.front());c.observe("end",line.points.back());double error=0;
    for (unsigned i=0;i<=4096;++i) {const double a=angles[0]+(angles[1]-angles[0])*i/4096;
        const double x=radii.x*std::cos(a),y=radii.y*std::sin(a);
        error=std::max(error,sampledDistance({center.x+x*std::cos(rotation)-y*std::sin(rotation),
                                            center.y+x*std::sin(rotation)+y*std::cos(rotation)},line));}
    EXPECT_LE(error,f.positionBound+1e-10);c.measure("max-display-error",error,.25);c.complete();
    EXPECT_FALSE(prepareStroke(command).triangles.empty());
}
TEST(CpuCurveTest, CpuEllipseEmpty) {
    Contract c("ellipse","empty");PathRecorder r;const auto radii=point(c.input("radii"));
    r.ellipse(point(c.input("center")),radii.x,radii.y,0,0);r.stroke();c.observe("draw-count",r.commands().size());c.complete();
}
TEST(CpuCurveTest, CpuEllipseLimit) { Contract c("ellipse","limit");PathRecorder r;r.ellipse({0,0},3,1,0,1.5707963267948966);curveLimit(c,r); }
TEST(CpuCurveTest, CpuLwpolylineOrdinary) { bulgeCase("ordinary"); }
TEST(CpuCurveTest, CpuLwpolylineNegativeBulge) { bulgeCase("negative-bulge"); }
TEST(CpuCurveTest, CpuLwpolylineEmpty) {
    Contract c("lwpolyline","empty");PathRecorder r;r.polyline(points(c.input("points")),numbers(c.input("bulges")),false);r.stroke();
    c.observe("draw-count",r.commands().size());c.complete();
}
TEST(CpuCurveTest, CpuLwpolylineLimit) { Contract c("lwpolyline","limit");recordingLimit(c); }
TEST(CpuCurveTest, CpuLwpolylineClosed) {
    Contract c("lwpolyline","closed");PathRecorder r;r.polyline(points(c.input("points")),numbers(c.input("bulges")),c.input("closed").GetBool());
    const auto command=recorded(r);const auto f=flatten(command);ASSERT_EQ(1u,f.subpaths.size());
    c.observe("length",lineLength(f.subpaths.front()));c.observe("closed",f.subpaths.front().closed);c.complete();
    EXPECT_FALSE(prepareStroke(command).triangles.empty());
}
TEST(CpuStrokeTest, CpuStrokeOrdinary) {
    Contract c("stroke","ordinary");PathRecorder r;r.lineWidth(c.input("width").GetDouble());
    r.dash(numbers(c.input("dash")),c.input("offset").GetDouble(),true);r.source(1,0,0,.25);
    linePath(r,c.input("points"));r.stroke();r.source(0,0,1,.75);
    linePath(r,c.input("points"));r.stroke();
    StrokeStyle style;EXPECT_STREQ("butt",c.input("cap").GetString());EXPECT_STREQ("miter",c.input("join").GetString());
    style.miterLimit=c.input("miter-limit").GetDouble();const auto batch=prepareStrokes(r.commands(),style);ASSERT_EQ(2u,batch.size());
    const auto dash=normalizeDash(batch[0].state);c.observe("normalized-dash",dash.values);c.observe("phase",dash.phase);
    // CPU binding: ordered straight-alpha snapshots plus separate union domains.
    // No pixel blending is claimed; the original GPU stroke case stays required.
    c.observe("ordered-alpha",batch[0].state.rgba==r.commands()[0].state.rgba &&
        batch[1].state.rgba==r.commands()[1].state.rgba && batch[0].state.rgba!=batch[1].state.rgba &&
        batch[0].coverage==StrokeCoverage::UnionBeforeOver && batch[1].coverage==StrokeCoverage::UnionBeforeOver &&
        !batch[0].triangles.empty() && !batch[1].triangles.empty());c.complete();
}
TEST(CpuStrokeTest, CpuStrokeEmpty) {
    Contract c("stroke","empty");PathRecorder r;r.dash(numbers(c.input("dash")));linePath(r,c.input("points"));r.stroke();
    c.observe("draw-count",prepareStrokes(r.commands()).size());c.complete();
}
TEST(CpuStrokeTest, CpuStrokeLimit) {
    Contract c("stroke","limit");StrokeLimits limits;limits.vertices=c.input("limit").GetUint();
    EXPECT_EQ(limits.vertices+1,c.input("requested").GetUint());std::vector<PreparedStroke> output;
    c.observe("diagnostic",diagnostic([&]{output=prepareStrokes({strokeLine()}, {},limits);}));
    c.observe("accepted-output-count",output.size());c.complete();
}
TEST(CpuStrokeTest, CpuStrokeInvalidDash) {
    Contract c("stroke","invalid-dash");auto command=strokeLine();command.state.dash=numbers(c.input("dash"));
    std::vector<PreparedStroke> output;c.observe("diagnostic",diagnostic([&]{output=prepareStrokes({command});}));
    c.observe("accepted-output-count",output.size());c.complete();
}
TEST(CpuStrokeTest, CpuStrokeClosedSeam) {
    Contract c("stroke","closed-seam");PathRecorder r;r.lineWidth(.2);r.dash(numbers(c.input("dash")),c.input("offset").GetDouble(),false);
    linePath(r,c.input("points"),c.input("closed").GetBool());const auto command=recorded(r);
    const auto f=flatten(command);ASSERT_EQ(1u,f.subpaths.size());const double length=lineLength(f.subpaths.front());
    c.observe("path-length",length);const auto batch=prepareStrokes({command});ASSERT_EQ(1u,batch.size());
    auto state=batch[0].state;state.dashOffset+=length;c.observe("seam-phase",normalizeDash(state).phase);c.complete();
    EXPECT_TRUE(covers(batch[0],{1,0}));EXPECT_FALSE(covers(batch[0],{2.5,0}));
    EXPECT_TRUE(covers(batch[0],{.9,1.2}));EXPECT_FALSE(covers(batch[0],{.3,.4}));
}
TEST(CpuStrokeTest, BatchOwnsSnapshotsIncludingEmptyCommands) {
    auto a=strokeLine();a.state.rgba={{1,0,0,.25}};auto b=a;b.state.rgba={{0,0,1,.75}};
    PathCommand empty{PaintOperation::Stroke,{}, {}};std::vector<PathCommand> commands{a,empty,b};
    const auto result=prepareStrokes(commands);ASSERT_EQ(3u,result.size());EXPECT_TRUE(result[1].triangles.empty());
    commands[0].state.rgba={{0,1,0,1}};commands[0].path.clear();
    EXPECT_EQ(a.state.rgba,result[0].state.rgba);EXPECT_EQ(b.state.rgba,result[2].state.rgba);
    EXPECT_TRUE(covers(result[0],{2,0}));EXPECT_TRUE(covers(result[2],{2,0}));
    for (const auto& s:result) EXPECT_EQ(StrokeCoverage::UnionBeforeOver,s.coverage);
    EXPECT_TRUE(prepareStrokes({}).empty());StrokeLimits bad;bad.positionError=0;
    EXPECT_EQ("invalid-limits",diagnostic([&]{prepareStrokes({}, {},bad);}));
    StrokeStyle invalid;invalid.miterLimit=0;EXPECT_EQ("invalid-stroke",diagnostic([&]{prepareStrokes({},invalid);}));
}
TEST(CpuStrokeTest, BatchSharesWorkVerticesAndInputStateBudgets) {
    const auto c=strokeLine();const auto single=prepareStroke(c);ASSERT_EQ(2u,single.triangles.size());
    StrokeLimits limits;limits.vertices=11;std::vector<PreparedStroke> output;
    EXPECT_EQ("vertex-limit",diagnostic([&]{output=prepareStrokes({c,c},{},limits);}));EXPECT_TRUE(output.empty());
    limits.vertices=12;EXPECT_EQ(2u,prepareStrokes({c,c},{},limits).size());
    limits.work=2*(single.visited+1)-1;EXPECT_EQ("subdivision-limit",diagnostic([&]{prepareStrokes({c,c},{},limits);}));
    ++limits.work;EXPECT_EQ(2u,prepareStrokes({c,c},{},limits).size());
    PathCommand moves{PaintOperation::Stroke,{},{{PathVerb::Move,{0,0}},{PathVerb::Move,{1,0}},{PathVerb::Move,{2,0}}}};
    limits={};limits.vertices=4;EXPECT_EQ("vertex-limit",diagnostic([&]{prepareStrokes({moves,moves},{},limits);}));
    moves.path.clear();moves.state.dash={1,1,1,1,1};limits.vertices=8;
    EXPECT_EQ("state-limit",diagnostic([&]{prepareStrokes({moves,moves},{},limits);}));
    limits={};limits.work=1;EXPECT_EQ("subdivision-limit",diagnostic([&]{prepareStrokes({c,c},{},limits);}));
    EXPECT_EQ(1u,prepareStrokes({c}).size());
}
TEST(CpuStrokeTest, BatchLaterInvalidCommandLeavesPriorOutputIntact) {
    const auto c=strokeLine();auto fill=c;fill.operation=PaintOperation::Fill;auto previous=prepareStrokes({c});
    EXPECT_EQ("invalid-stroke",diagnostic([&]{previous=prepareStrokes({c,fill});}));
    ASSERT_EQ(1u,previous.size());EXPECT_EQ(2u,previous[0].triangles.size());EXPECT_TRUE(covers(previous[0],{2,0}));
}
TEST(CpuStrokeTest, OddDashesAndSignedOffsetsUseLegacyZeroCanonicalization) {
    PaintState s;s.dash={1,2,3};s.dashOffset=7;auto d=normalizeDash(s);
    EXPECT_EQ((std::vector<double>{1,2,3,1,2,3}),d.values);EXPECT_DOUBLE_EQ(12,d.period);EXPECT_DOUBLE_EQ(7,d.phase);
    s.dashOffset=-5;EXPECT_DOUBLE_EQ(7,normalizeDash(s).phase);
    s.dash={0,2,1};EXPECT_EQ((std::vector<double>{0,2,3,1}),normalizeDash(s).values);
    s.dash.clear();s.dashOffset=-1e200;EXPECT_TRUE(normalizeDash(s).values.empty());
    for (auto bad:std::vector<std::vector<double>>{{0,0},{1,-1},{NAN,1},{INFINITY,1},std::vector<double>(1025,1),{1e308,1e308}}) {
        s.dash=bad;EXPECT_EQ("invalid-dash",diagnostic([&]{normalizeDash(s);}));
    }
}
TEST(CpuStrokeTest, InternalZeroOnIntervalsUseLegacyDotsWithoutMutatingTheSnapshot) {
    auto c=strokeLine();c.state.dash={0,1,0,1};c.state.dashOffset=-3.25;c.state.dashScaled=true;
    const auto normalized=normalizeDash(c.state);EXPECT_EQ((std::vector<double>{0,2}),normalized.values);
    StrokeStyle style;style.cap=StrokeCap::Round;const auto mesh=prepareStroke(c,style);
    EXPECT_FALSE(covers(mesh,{-.5,0}));EXPECT_TRUE(covers(mesh,{1.25,0}));
    EXPECT_EQ((std::vector<double>{0,1,0,1}),mesh.state.dash);
    c.state.dash={2,0,0,0};EXPECT_TRUE(normalizeDash(c.state).values.empty());
}
TEST(CpuStrokeTest, SignedDashPhaseAndSubpathRestartMatchAnalyticIntervals) {
    auto c=strokeLine({0,0},{12,0});c.state.dash={1,2,3};c.state.dashOffset=-5;c.state.dashScaled=true;
    const auto mesh=prepareStroke(c);
    for (unsigned i=0;i<48;++i) {
        const double x=(i+.5)/4,phase=std::fmod(x+7,12.);
        const bool on=phase<1 || (phase>=3 && phase<6) || (phase>=7 && phase<9);
        EXPECT_EQ(on,covers(mesh,{x,0}))<<x;
    }
    PathRecorder r;r.dash({2,1},1,true);r.moveTo({0,0});r.lineTo({4,0});r.newSubPath();r.moveTo({0,4});r.lineTo({4,4});
    const auto subpaths=prepareStroke(recorded(r));
    for (double y:{0.,4.}) {EXPECT_TRUE(covers(subpaths,{.5,y}));EXPECT_FALSE(covers(subpaths,{1.5,y}));}
}
TEST(CpuStrokeTest, DashReachingTheEdgeEndStopsExactlyAtTheEnd) {
    // Off the origin, at+(length-at) can round below length and leave a step
    // too small to move the point; the dash must end the edge exactly instead.
    PathRecorder r;r.lineWidth(.02);r.dash({.86,100},0,true);
    r.moveTo({400,300});r.lineTo({401.34,301.36});r.lineTo({406.34,301.36});
    const auto mesh=prepareStroke(recorded(r));
    EXPECT_TRUE(covers(mesh,{400.3,300.3}));EXPECT_FALSE(covers(mesh,{403,301.36}));
}
TEST(CpuStrokeTest, DashUnitsUseTheStrokeTimeAffineMapNotAppendTimeState) {
    PathRecorder r;r.scale(4);r.moveTo({0,0});r.lineTo({3,0});r.dash({1,1},0,false);auto c=recorded(r);
    auto mesh=prepareStroke(c);EXPECT_TRUE(covers(mesh,{1.5,0}));EXPECT_FALSE(covers(mesh,{4.5,0}));
    c.state.dashScaled=true;mesh=prepareStroke(c);EXPECT_FALSE(covers(mesh,{1.5,0}));EXPECT_TRUE(covers(mesh,{4.5,0}));
    c.state.dashScaled=false;c.state.transform={};mesh=prepareStroke(c);EXPECT_FALSE(covers(mesh,{1.5,0}));
    c.state.transform.xx=2;c.state.transform.xy=1;c.state.transform.yy=3;
    c.path={{PathVerb::Move,{0,0}},{PathVerb::Line,{6,12}}};c.state.dash={2,1};
    mesh=prepareStroke(c); // inverse(6,12)=(1,4), length=sqrt(17)
    EXPECT_TRUE(covers(mesh,{.6,1.2}));EXPECT_FALSE(covers(mesh,{3.6,7.2}));
}
TEST(CpuStrokeTest, PhysicalModelCompensationAndHairlineWidthsAreDistinct) {
    auto c=strokeLine({0,0},{40,0});c.state.transform.xx=c.state.transform.yy=4;c.state.lineWidthCompensation=1;
    auto mesh=prepareStroke(c);EXPECT_TRUE(covers(mesh,{20,1.4}));EXPECT_FALSE(covers(mesh,{20,1.6}));
    StrokeStyle model;model.widthUnits=WidthUnits::Model;mesh=prepareStroke(c,model);
    EXPECT_TRUE(covers(mesh,{20,5.9}));EXPECT_FALSE(covers(mesh,{20,6.1}));
    c.state.lineWidth=0;c.state.lineWidthCompensation=0;mesh=prepareStroke(c,model);
    EXPECT_TRUE(covers(mesh,{20,.49}));EXPECT_FALSE(covers(mesh,{20,.51}));
}
TEST(CpuStrokeTest, CapsMatchIndependentAnalyticDomains) {
    const auto c=strokeLine();
    for (auto cap:{StrokeCap::Butt,StrokeCap::Round,StrokeCap::Square}) {
        StrokeStyle style;style.cap=cap;const auto mesh=prepareStroke(c,style);
        for (unsigned i=0;i<=56;++i) for (unsigned j=0;j<=24;++j) {
            const Point p={-1.5+i/8.,-1.5+j/8.};double margin;
            if (cap==StrokeCap::Round) {
                const double x=p.x<0?p.x:p.x>4?p.x-4:0;margin=1-std::hypot(x,p.y);
            } else {const double extension=cap==StrokeCap::Square?1:0;
                margin=std::min({p.x+extension,4+extension-p.x,1-std::abs(p.y)});}
            if (std::abs(margin)>.07) EXPECT_EQ(margin>0,covers(mesh,p))<<int(cap)<<" "<<p.x<<","<<p.y;
        }
        EXPECT_LE(mesh.positionBound,1.0/16);
    }
}
TEST(CpuStrokeTest, JoinsAndMiterFallbackMatchIndependentLandmarks) {
    PathRecorder r;r.lineWidth(2);r.moveTo({0,0});r.lineTo({4,0});r.lineTo({4,4});const auto c=recorded(r);
    StrokeStyle style;auto mesh=prepareStroke(c,style);EXPECT_TRUE(covers(mesh,{4.9,-.9}));
    style.join=StrokeJoin::Bevel;mesh=prepareStroke(c,style);EXPECT_FALSE(covers(mesh,{4.6,-.6}));EXPECT_TRUE(covers(mesh,{4.2,-.2}));
    style.join=StrokeJoin::Round;mesh=prepareStroke(c,style);EXPECT_TRUE(covers(mesh,{4.6,-.6}));EXPECT_FALSE(covers(mesh,{4.9,-.9}));
    style.join=StrokeJoin::Miter;style.miterLimit=1;mesh=prepareStroke(c,style);EXPECT_FALSE(covers(mesh,{4.9,-.9}));
}
TEST(CpuStrokeTest, NearlyReversingAndRepeatedSegmentsStayFiniteAndBounded) {
    for (double y:{0.,1e-10,-1e-10}) {
        PathRecorder r;r.lineWidth(2);r.moveTo({0,0});r.lineTo({0,0});r.lineTo({4,0});r.lineTo({0,y});
        const auto c=recorded(r);auto mesh=prepareStroke(c);EXPECT_LT(mesh.maximum.x,4.1);EXPECT_GT(mesh.minimum.x,-.1);
        StrokeStyle style;style.join=StrokeJoin::Round;mesh=prepareStroke(c,style);
        EXPECT_LT(mesh.triangles.size(),100u);EXPECT_TRUE(covers(mesh,{4.7,0}));
        for (const auto& t:mesh.triangles) for (Point p:t) EXPECT_TRUE(std::isfinite(p.x)&&std::isfinite(p.y));
    }
}
TEST(CpuStrokeTest, RoundJoinDoesNotOverpaintNearbyButtCaps) {
    PathRecorder r;r.lineWidth(2);r.moveTo({-.1,0});r.lineTo({0,0});r.lineTo({0,.1});
    StrokeStyle style;style.join=StrokeJoin::Round;const auto mesh=prepareStroke(recorded(r),style);
    EXPECT_FALSE(covers(mesh,{-.75,-.25}));EXPECT_TRUE(covers(mesh,{-.25,.75})); // corrected inner fan
    EXPECT_TRUE(covers(mesh,{.5,-.5}));EXPECT_TRUE(covers(mesh,{-.05,-.8}));EXPECT_TRUE(covers(mesh,{-.8,.05}));
}
TEST(CpuStrokeTest, RoundCapsDoNotOverpaintNearbyBevelJoins) {
    PathRecorder r;r.lineWidth(2);r.moveTo({-.1,0});r.lineTo({0,0});r.lineTo({0,.1});
    StrokeStyle style;style.cap=StrokeCap::Round;style.join=StrokeJoin::Bevel;
    const auto mesh=prepareStroke(recorded(r),style);
    EXPECT_FALSE(covers(mesh,{.8,-.4}));EXPECT_TRUE(covers(mesh,{-.8,-.4}));
}
TEST(CpuStrokeTest, RotatedRoundCapDiameterIsWeldedToItsStrip) {
    Transform t;t.xx=1.6;t.xy=-1.2;t.yx=1.2;t.yy=1.6;
    PathRecorder r;r.setTransform(t);r.lineWidth(2);r.moveTo({.25,0});r.lineTo({2.25,0});
    StrokeStyle style;style.cap=StrokeCap::Round;style.widthUnits=WidthUnits::Model;
    EXPECT_TRUE(covers(prepareStroke(recorded(r),style),{1,-.5}));
    const std::array<Point,3> thin={{{0,0},{1,0},{1,1e-16}}};
    EXPECT_FALSE(inTriangle({100,0},thin));EXPECT_TRUE(inTriangle({.5,1e-17},thin));
    EXPECT_FALSE(inTriangle({.5,.9e-16},thin));
    const std::array<Point,3> flat={{{0,0},{1,1},{2,2}}};EXPECT_FALSE(inTriangle({100,100},flat));EXPECT_FALSE(inTriangle({1.5,1.5},flat));
}
TEST(CpuStrokeTest, ZeroOnSquareDotIsNotSuppressedByAnAdjacentPositiveDash) {
    PathRecorder r;r.lineWidth(2);r.rectangle({0,0},4,4);r.dash({0,1,2,0},-3.25,true);
    StrokeStyle style;style.cap=StrokeCap::Square;style.join=StrokeJoin::Bevel;
    EXPECT_TRUE(covers(prepareStroke(recorded(r),style),{-.8,4.5}));
}
TEST(CpuStrokeTest, ZeroGapRetainsJoinsRatherThanIntermediateCaps) {
    PathRecorder r;r.lineWidth(2);r.moveTo({0,0});r.lineTo({4,0});r.lineTo({4,4});r.lineTo({0,4});
    r.dash({2,0},-.75,true);StrokeStyle style;style.cap=StrokeCap::Square;style.join=StrokeJoin::Bevel;
    const auto mesh=prepareStroke(recorded(r),style);EXPECT_FALSE(covers(mesh,{5,4.25}));
    EXPECT_TRUE(covers(mesh,{4.2,4.2}));
}
TEST(CpuStrokeTest, ClosedDashSeamHasJoinRatherThanTwoButtCaps) {
    PathRecorder r;r.lineWidth(2);r.rectangle({0,0},4,4);r.dash({5,1},1,true);auto c=recorded(r);
    const auto mesh=prepareStroke(c);EXPECT_TRUE(covers(mesh,{-.9,-.9}));
    c.state.dash.clear();const auto solid=prepareStroke(c);EXPECT_TRUE(covers(solid,{-.9,-.9}));
    c.path.insert(c.path.end()-1,{PathVerb::Line,c.path.front().point});
    const auto explicitClose=prepareStroke(c);EXPECT_EQ(solid.triangles.size(),explicitClose.triangles.size());
}
TEST(CpuStrokeTest, ZeroOnDashesProduceRoundAndOrientedSquareDots) {
    auto c=strokeLine();c.state.dash={0,2};c.state.dashScaled=true;StrokeStyle style;style.cap=StrokeCap::Round;
    auto mesh=prepareStroke(c,style);for (double x:{0.,2.,4.}) EXPECT_TRUE(covers(mesh,{x,.8}));
    EXPECT_FALSE(covers(mesh,{1,.8}));style.cap=StrokeCap::Square;mesh=prepareStroke(c,style);
    EXPECT_TRUE(covers(mesh,{1,.8}));style.cap=StrokeCap::Butt;EXPECT_TRUE(prepareStroke(c,style).triangles.empty());
}
TEST(CpuStrokeTest, ZeroLengthAndZeroGapPathsTerminateWithDeclaredCaps) {
    auto c=strokeLine({2,3},{2,3});StrokeStyle style;style.cap=StrokeCap::Round;
    EXPECT_TRUE(covers(prepareStroke(c,style),{2.2,3.2}));style.cap=StrokeCap::Square;
    EXPECT_TRUE(prepareStroke(c,style).triangles.empty());
    c=strokeLine();c.state.dash={2,0};c.state.dashScaled=true;const auto mesh=prepareStroke(c);
    EXPECT_LT(mesh.visited,100u);for (double x:{.5,1.5,2.5,3.5}) EXPECT_TRUE(covers(mesh,{x,0}));
}
TEST(CpuStrokeTest, CoverageContractRetainsOneOwnedCommandNotPerTriangleAlpha) {
    PathRecorder r;r.source(.2,.3,.4,.5);r.moveTo({-2,0});r.lineTo({2,0});r.lineTo({0,-2});r.lineTo({0,2});auto c=recorded(r);
    const auto mesh=prepareStroke(c);EXPECT_GT(overlaps(mesh,{0,0}),1u);
    EXPECT_EQ(StrokeCoverage::UnionBeforeOver,mesh.coverage);EXPECT_DOUBLE_EQ(.5,mesh.state.rgba[3]);
    c.state.rgba[3]=1;c.path.clear();EXPECT_DOUBLE_EQ(.5,mesh.state.rgba[3]);EXPECT_FALSE(mesh.triangles.empty());
    // This verifies the CPU boundary's union requirement, not a GPU implementation.
}
TEST(CpuStrokeTest, WorkVertexAndPrecisionLimitsAreAtomic) {
    const auto c=strokeLine();PreparedStroke output;StrokeLimits limits;limits.vertices=5;
    EXPECT_EQ("vertex-limit",diagnostic([&]{output=prepareStroke(c,{},limits);}));EXPECT_TRUE(output.triangles.empty());
    limits={};limits.work=1;EXPECT_EQ("subdivision-limit",diagnostic([&]{output=prepareStroke(c,{},limits);}));
    EXPECT_TRUE(output.triangles.empty());EXPECT_FALSE(prepareStroke(c).triangles.empty());EXPECT_EQ(2u,c.path.size());
    const auto far=strokeLine({1e16,0},{1e16+4,0});EXPECT_EQ("precision-limit",diagnostic([&]{prepareStroke(far);}));
    auto huge=strokeLine();huge.state.lineWidth=1e8;StrokeStyle round;round.cap=StrokeCap::Round;limits={};limits.vertices=128;
    EXPECT_EQ("vertex-limit",diagnostic([&]{prepareStroke(huge,round,limits);}));
    limits={};limits.positionError=.25;EXPECT_EQ("invalid-limits",diagnostic([&]{prepareStroke(c,{},limits);}));
    const auto unresolved=strokeLine({1,1},{2,1},1e-300);
    EXPECT_EQ("precision-limit",diagnostic([&]{prepareStroke(unresolved);}));
    PathRecorder partial;partial.lineWidth(1e-300);partial.moveTo({0,0});partial.lineTo({1,0});
    partial.newSubPath();partial.moveTo({1,1});partial.lineTo({2,1});const auto mixed=recorded(partial);
    EXPECT_EQ("precision-limit",diagnostic([&]{prepareStroke(mixed);}));
    PathRecorder dots;dots.lineWidth(1e-150);dots.moveTo({0,0});dots.lineTo({0,0});
    dots.newSubPath();dots.moveTo({1,1});dots.lineTo({1,1});const auto mixedDots=recorded(dots);
    StrokeStyle roundDots;roundDots.cap=StrokeCap::Round;
    EXPECT_EQ("precision-limit",diagnostic([&]{prepareStroke(mixedDots,roundDots);}));
}
TEST(CpuStrokeTest, InvalidStyleAndPathsDoNotCorruptLaterPreparation) {
    auto c=strokeLine();StrokeStyle style;style.miterLimit=0;EXPECT_EQ("invalid-stroke",diagnostic([&]{prepareStroke(c,style);}));
    style={};style.cap=static_cast<StrokeCap>(99);EXPECT_EQ("invalid-stroke",diagnostic([&]{prepareStroke(c,style);}));
    c.operation=PaintOperation::Fill;EXPECT_EQ("invalid-stroke",diagnostic([&]{prepareStroke(c);}));c.operation=PaintOperation::Stroke;
    c.state.dash={1,-1};EXPECT_EQ("invalid-dash",diagnostic([&]{prepareStroke(c);}));c.state.dash.clear();
    c.path[0].verb=PathVerb::Line;EXPECT_EQ("invalid-path",diagnostic([&]{prepareStroke(c);}));c.path[0].verb=PathVerb::Move;
    c.path[1].point.x=NAN;EXPECT_EQ("invalid-stroke",diagnostic([&]{prepareStroke(c);}));
    c=strokeLine();EXPECT_FALSE(prepareStroke(c).triangles.empty());
}
TEST(CpuStrokeTest, CurvedStrokeRemainsExplicitlyUnfinishedNotSilentlyApproximate) {
    PathRecorder r;r.moveTo({0,0});r.cubicTo({0,1},{1,1},{1,0});const auto c=recorded(r);
    // Retain the historical ID; it now rejects exhaustion of the new certified
    // preparation rather than rejecting every curve or silently using flatness.
    StrokeLimits limits;limits.work=1;
    EXPECT_EQ("subdivision-limit",diagnostic([&]{prepareStroke(c,{},limits);}));
    EXPECT_FALSE(prepareStroke(c).triangles.empty());EXPECT_FALSE(flatten(c).subpaths.empty());
    PathCommand empty{PaintOperation::Stroke,{}, {}};EXPECT_TRUE(prepareStroke(empty).triangles.empty());
}
TEST(CpuStrokeTest, ReflectionShearAndLocalOriginsPreserveOrientationAndBounds) {
    PathRecorder r;Transform t;t.xx=-2;t.xy=1;t.yx=.5;t.yy=3;t.origin={1e12,-1e12};r.setTransform(t);r.lineWidth(2);
    r.moveTo(t.origin);r.lineTo({t.origin.x+4,t.origin.y});StrokeStyle style;style.widthUnits=WidthUnits::Model;
    const auto mesh=prepareStroke(recorded(r),style);
    // Independent affine image of interior/exterior user-space landmarks.
    EXPECT_TRUE(covers(mesh,t.distance({2,.9})));EXPECT_FALSE(covers(mesh,t.distance({2,1.1})));
    for (const auto& triangle:mesh.triangles) {
        const auto a=triangle[0],b=triangle[1],c=triangle[2];
        EXPECT_GT((b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x),0);
        for (Point p:triangle) {EXPECT_LE(mesh.minimum.x,p.x);EXPECT_GE(mesh.maximum.x,p.x);EXPECT_LE(mesh.minimum.y,p.y);EXPECT_GE(mesh.maximum.y,p.y);}
    }
}

namespace {
double meshDistance(const PreparedStroke& s,Point p) {
    if (covers(s,p)) return 0;double best=std::numeric_limits<double>::infinity();
    for (const auto& t:s.triangles) for (unsigned i=0;i<3;++i) {
        const Point a=t[i],b=t[(i+1)%3];const double x=b.x-a.x,y=b.y-a.y,n=x*x+y*y;
        const double u=n?std::max(0.,std::min(1.,((p.x-a.x)*x+(p.y-a.y)*y)/n)):0;
        best=std::min(best,std::hypot(p.x-a.x-u*x,p.y-a.y-u*y));
    }
    return best;
}
Point derivative(const std::vector<Point>& p,double t) {
    const double u=1-t;
    if (p.size()==3) return {2*(u*(p[1].x-p[0].x)+t*(p[2].x-p[1].x)),2*(u*(p[1].y-p[0].y)+t*(p[2].y-p[1].y))};
    return {3*(u*u*(p[1].x-p[0].x)+2*u*t*(p[2].x-p[1].x)+t*t*(p[3].x-p[2].x)),
            3*(u*u*(p[1].y-p[0].y)+2*u*t*(p[2].y-p[1].y)+t*t*(p[3].y-p[2].y))};
}
}
TEST(CpuCurvedStrokeTest, WidthAwareSubdivisionCarriesWeldedTangentsAndMetricBounds) {
    PathRecorder r;r.moveTo({0,0});r.cubicTo({0,12},{12,-8},{16,4});const auto c=recorded(r);
    CurveMetrics m;m.physicalRadius=1;auto thin=flatten(c,{},m);m.physicalRadius=8;auto wide=flatten(c,{},m);
    EXPECT_GT(wide.subpaths[0].points.size(),thin.subpaths[0].points.size());
    EXPECT_LE(wide.positionBound,1.0/16);EXPECT_LE(wide.normalBound,1.0/32);
    const auto& line=wide.subpaths[0];ASSERT_EQ(line.points.size()-1,line.tangents.size());
    for (std::size_t i=0;i<line.tangents.size();++i) {
        const auto& n=line.tangents[i];EXPECT_NEAR(1,std::hypot(n.start.x,n.start.y),1e-14);
        EXPECT_NEAR(1,std::hypot(n.end.x,n.end.y),1e-14);
        if (i) {EXPECT_DOUBLE_EQ(line.tangents[i-1].end.x,n.start.x);EXPECT_DOUBLE_EQ(line.tangents[i-1].end.y,n.start.y);}
    }
    m.length.xx=m.length.yy=.25;m.lengthScale=4;wide=flatten(c,{},m);
    EXPECT_LE((wide.lengthUpper-wide.lengthLower)*4,1.0/32);
}
TEST(CpuPointTest, CpuPointOrdinary) {
    Contract c("point","ordinary");PathRecorder r;
    ASSERT_TRUE(c.input("device-coordinates").GetBool());r.point(point(c.input("position")),c.input("size").GetDouble());
    const auto mesh=prepareStrokes(r.commands());ASSERT_EQ(1u,mesh.size());ASSERT_FALSE(mesh[0].triangles.empty());
    const Point center=mesh[0].triangles[0][0];c.observe("center",center);double radius=0;
    for (const auto& t:mesh[0].triangles) for (Point p:t) radius=std::max(radius,std::hypot(p.x-center.x,p.y-center.y));
    c.measure("diameter",2*radius,.25);c.complete();
    EXPECT_TRUE(covers(mesh[0],center));EXPECT_FALSE(covers(mesh[0],{center.x+4,center.y}));
}
TEST(CpuPointTest, CpuPointEmpty) {
    Contract c("point","empty");PathRecorder r;r.point(point(c.input("position")),c.input("size").GetDouble());
    std::size_t ink=0;for (const auto& p:prepareStrokes(r.commands())) ink+=!p.triangles.empty();
    c.observe("draw-count",ink);c.complete();
}
TEST(CpuPointTest, CpuPointLimit) {
    Contract c("point","limit");StrokeLimits limits;limits.pointDiameter=c.input("limit").GetDouble();
    PathRecorder r;r.point({},c.input("requested").GetDouble());std::vector<PreparedStroke> output;
    c.observe("diagnostic",diagnostic([&]{output=prepareStrokes(r.commands(),{},limits);}));
    c.observe("accepted-output-count",output.size());c.complete();
    r.clearCommands();r.point({},limits.pointDiameter);EXPECT_FALSE(prepareStrokes(r.commands(),{},limits).empty());
}
TEST(CpuPointTest, PhysicalAndModelDiameterFollowIndependentAffineLandmarks) {
    constexpr double pi=3.14159265358979323846;
    for (bool model:{false,true}) for (double sign:{-1.,1.}) {
        Transform t;t.xx=sign*2;t.xy=1;t.yx=.5;t.yy=3;t.origin={1e12,-1e12};t.tx=4;t.ty=-2;
        PathRecorder r;r.setTransform(t);r.point(t.origin,4,model?PointUnits::Model:PointUnits::Physical);
        const auto mesh=prepareStroke(r.commands()[0]);EXPECT_LE(mesh.positionBound,.25);
        for (unsigned i=0;i<128;++i) {
            const double a=2*pi*(i+.3)/128;
            const auto at=[&](double radius) {
                Point p{radius*std::cos(a),radius*std::sin(a)};if (model) p=t.distance(p);return Point{4+p.x,-2+p.y};
            };
            EXPECT_TRUE(covers(mesh,at(1.9)));EXPECT_FALSE(covers(mesh,at(2.1)));
            EXPECT_LE(meshDistance(mesh,at(2)),mesh.positionBound+1e-10);
        }
        for (const auto& face:mesh.triangles) for (Point p:face) {
            EXPECT_LE(mesh.minimum.x,p.x);EXPECT_LE(mesh.minimum.y,p.y);EXPECT_GE(mesh.maximum.x,p.x);EXPECT_GE(mesh.maximum.y,p.y);
        }
    }
}
TEST(CpuPointTest, MarksOwnStatePreservePathAndBatchOrderWithoutStrokeDecorations) {
    PathRecorder r;segment(r);const auto path=r.path();r.source(1,0,0,.25);r.lineWidth(100);r.dash({1,2});
    r.point({2,3},5);r.source(0,0,1,.75);r.point({4,5},0);r.lineWidth(2);r.stroke();
    ASSERT_EQ(3u,r.commands().size());EXPECT_EQ(path.size(),r.commands().back().path.size());EXPECT_TRUE(r.path().empty());
    StrokeStyle style;style.cap=StrokeCap::Square;style.join=StrokeJoin::Bevel;const auto output=prepareStrokes(r.commands(),style);
    ASSERT_EQ(3u,output.size());EXPECT_TRUE(sameStyle(output[0].state,r.commands()[0].state));EXPECT_TRUE(output[1].triangles.empty());
    EXPECT_NE(output[0].state.rgba,output[2].state.rgba);EXPECT_TRUE(covers(output[0],{2,3}));EXPECT_FALSE(covers(output[0],{5,3}));
    r.clearCommands();r.point({},2);EXPECT_TRUE(covers(output[0],{2,3})); // snapshots remain owned
}
TEST(CpuPointTest, InvalidInputsAndRecordingCapsAreAtomic) {
    RecordingLimits limits;limits.elements=3;PathRecorder r(limits);segment(r);
    for (double v:{-1.,double(INFINITY),double(NAN)}) EXPECT_EQ("invalid-point",diagnostic([&]{r.point({},v);}));
    EXPECT_EQ("invalid-point",diagnostic([&]{r.point({NAN,0},1);}));
    EXPECT_EQ("invalid-point",diagnostic([&]{r.point({},1,static_cast<PointUnits>(99));}));
    r.point({},1);EXPECT_EQ("vertex-limit",diagnostic([&]{r.point({},1);}));EXPECT_EQ(2u,r.path().size());EXPECT_EQ(1u,r.commands().size());
    r.stroke();EXPECT_EQ(2u,r.commands().size());
    limits={};limits.commands=0;PathRecorder noCommands(limits);segment(noCommands);
    EXPECT_EQ("command-limit",diagnostic([&]{noCommands.point({},1);}));EXPECT_EQ(2u,noCommands.path().size());
    limits={};limits.elements=2;PathRecorder state(limits);state.dash({1,2});state.point({},0);
    EXPECT_EQ("state-limit",diagnostic([&]{state.point({},0);}));EXPECT_EQ(1u,state.commands().size());
}
TEST(CpuPointTest, AggregateWorkVerticesAndPhysicalMajorAxisLimitsFailClosed) {
    PathRecorder r;r.point({},4);r.point({10,0},4);const auto mesh=prepareStroke(r.commands()[0]);
    StrokeLimits limits;limits.work=1;PreparedStroke output;
    EXPECT_EQ("subdivision-limit",diagnostic([&]{output=prepareStroke(r.commands()[0],{},limits);}));EXPECT_TRUE(output.triangles.empty());
    limits={};limits.vertices=3;EXPECT_EQ("vertex-limit",diagnostic([&]{output=prepareStroke(r.commands()[0],{},limits);}));
    std::vector<PreparedStroke> batch;limits={};limits.vertices=3*mesh.triangles.size();
    EXPECT_EQ("vertex-limit",diagnostic([&]{batch=prepareStrokes(r.commands(),{},limits);}));EXPECT_TRUE(batch.empty());
    limits={};limits.work=mesh.visited+2;
    EXPECT_EQ("subdivision-limit",diagnostic([&]{batch=prepareStrokes(r.commands(),{},limits);}));EXPECT_TRUE(batch.empty());
    r.clearCommands();r.scale(2);r.point({},33,PointUnits::Model);
    EXPECT_EQ("point-size-limit",diagnostic([&]{prepareStroke(r.commands()[0]);}));
    r.clearCommands();r.point({},32,PointUnits::Model);EXPECT_FALSE(prepareStroke(r.commands()[0]).triangles.empty());
    r.clearCommands();r.point({},64);EXPECT_FALSE(prepareStroke(r.commands()[0]).triangles.empty());
    auto bad=r.commands()[0];bad.path.clear();EXPECT_EQ("invalid-point",diagnostic([&]{prepareStroke(bad);}));
    bad=r.commands()[0];bad.pointUnits=static_cast<PointUnits>(99);EXPECT_EQ("invalid-point",diagnostic([&]{prepareStroke(bad);}));
    bad=r.commands()[0];bad.pointDiameter=NAN;EXPECT_EQ("invalid-point",diagnostic([&]{prepareStroke(bad);}));
    EXPECT_FALSE(prepareStrokes(r.commands()).empty());
}
TEST(CpuStrokeTest, CorrectedInnerFanClosesShortCornersWithoutUnboundedMiters) {
    for (double sign:{-1.,1.}) for (auto join:{StrokeJoin::Miter,StrokeJoin::Round,StrokeJoin::Bevel}) {
        PathRecorder r;r.lineWidth(2);r.moveTo({-.1,0});r.lineTo({0,0});r.lineTo({0,sign*.1});
        StrokeStyle style;style.join=join;const auto mesh=prepareStroke(recorded(r),style);
        for (unsigned i=1;i<32;++i) {
            const double a=1.5707963267948966*i/32;
            EXPECT_TRUE(covers(mesh,{-.8*std::cos(a),sign*.8*std::sin(a)}));
        }
        EXPECT_FALSE(covers(mesh,{-.9,sign*.9}));EXPECT_FALSE(covers(mesh,{-.75,-sign*.25}));
        if (join==StrokeJoin::Bevel) EXPECT_FALSE(covers(mesh,{.8,-sign*.4}));
        EXPECT_LE(mesh.positionBound,.25);
    }
}
TEST(CpuCurvedStrokeTest, InternalCuspContinuityDoesNotDecorateExplicitReversals) {
    for (auto join:{StrokeJoin::Miter,StrokeJoin::Round,StrokeJoin::Bevel}) {
        StrokeStyle style;style.join=join;PathRecorder curve;curve.lineWidth(1);curve.moveTo({0,0});curve.quadraticTo({10,0},{0,0});
        const auto c=recorded(curve);CurveMetrics metrics;metrics.physicalRadius=.5;const auto flat=flatten(c,{},metrics);
        ASSERT_EQ(2u,flat.subpaths[0].tangents.size());EXPECT_FALSE(flat.subpaths[0].tangents[0].internal);EXPECT_TRUE(flat.subpaths[0].tangents[1].internal);
        EXPECT_TRUE(covers(prepareStroke(c,style),{5.3,0}));
        PathRecorder lines;lines.lineWidth(1);lines.moveTo({0,0});lines.lineTo({5,0});lines.lineTo({0,0});
        EXPECT_EQ(join==StrokeJoin::Round,covers(prepareStroke(recorded(lines),style),{5.3,0}));
    }
}
TEST(CpuCurvedStrokeTest, DashedCuspKeepsContinuityOnlyInsideAnOnRun) {
    for (double offset:{0.,5.5}) {
        PathRecorder r;r.lineWidth(1);r.moveTo({0,0});r.quadraticTo({10,0},{0,0});r.dash({6,10},offset,true);
        EXPECT_EQ(offset==0,covers(prepareStroke(recorded(r)),{5.3,0}));
    }
}
TEST(CpuCurvedStrokeTest, ThickClosedCircleKeepsAnnulusAndFoldedInterior) {
    constexpr double pi=3.14159265358979323846;
    for (double width:{1.,8.,14.}) {
        PathRecorder r;r.lineWidth(width);r.circle({0,0},5);const auto mesh=prepareStroke(recorded(r));
        EXPECT_LE(mesh.positionBound,.25);EXPECT_FALSE(mesh.triangles.empty());
        for (unsigned i=0;i<64;++i) {
            const double angle=2*pi*(i+.3)/64,x=std::cos(angle),y=std::sin(angle),outside=5+width/2+.3;
            EXPECT_FALSE(covers(mesh,{outside*x,outside*y}));
            EXPECT_TRUE(covers(mesh,{5*x,5*y}));
            if (width<10) {const double hole=5-width/2-.3;EXPECT_FALSE(covers(mesh,{hole*x,hole*y}));}
            else EXPECT_TRUE(covers(mesh,{.5*x,.5*y}));
        }
        if (width>10) {
            EXPECT_TRUE(covers(mesh,{0,0}));
            for (int x=-8;x<=8;++x) for (int y=-8;y<=8;++y) {
                const Point p={x*1.4,y*1.4};const double margin=12-std::hypot(p.x,p.y);
                if (std::abs(margin)>.3) EXPECT_EQ(margin>0,covers(mesh,p))<<p.x<<","<<p.y;
            }
        }
    }
}
TEST(CpuCurvedStrokeTest, RotatedEllipseOffsetsRemainWithinTheCombinedBound) {
    constexpr double pi=3.14159265358979323846;const double angle=.73,c=std::cos(angle),s=std::sin(angle);
    PathRecorder r;r.lineWidth(2);r.ellipse({0,0},12,6,0,2*pi,angle);r.closePath();const auto mesh=prepareStroke(recorded(r));
    for (unsigned i=0;i<64;++i) {
        const double t=2*pi*(i+.27)/64,x=12*std::cos(t),y=6*std::sin(t),dx=-12*std::sin(t),dy=6*std::cos(t),n=std::hypot(dx,dy);
        const Point p={c*x-s*y,s*x+c*y},normal={(-c*dy-s*dx)/n,(-s*dy+c*dx)/n};
        for (double side:{-1.,1.}) EXPECT_LE(meshDistance(mesh,{p.x+side*normal.x,p.y+side*normal.y}),mesh.positionBound+1e-10);
        EXPECT_TRUE(covers(mesh,p));
    }
}
TEST(CpuCurvedStrokeTest, PolynomialNormalSweepsUseWidthNotOnlyCenterlineError) {
    for (auto p:std::vector<std::vector<Point>>{{{0,0},{8,12},{16,0}},{{0,0},{0,12},{12,-8},{16,4}}}) {
        PathRecorder r;r.lineWidth(12);r.bezierSpan(p);const auto mesh=prepareStroke(recorded(r));
        EXPECT_LE(mesh.positionBound,.25);
        for (unsigned i=0;i<=64;++i) {
            const double t=i/64.;const Point q=evaluate(p,t),d=derivative(p,t);const double n=std::hypot(d.x,d.y);
            for (double side:{-1.,1.}) EXPECT_LE(meshDistance(mesh,{q.x-side*6*d.y/n,q.y+side*6*d.x/n}),mesh.positionBound+1e-10);
        }
    }
}
TEST(CpuCurvedStrokeTest, PhysicalDashesFollowAnalyticCircularArcLength) {
    constexpr double pi=3.14159265358979323846;PathRecorder r;r.lineWidth(1);r.arc({0,0},10,0,pi);r.dash({2,1},0,true);
    const auto mesh=prepareStroke(recorded(r));
    for (unsigned i=0;i<31;++i) {
        const double length=i+.5,t=length/10;EXPECT_EQ(i%3!=2,covers(mesh,{10*std::cos(t),10*std::sin(t)}))<<length;
    }
    EXPECT_LE(mesh.positionBound,.25);
}
TEST(CpuCurvedStrokeTest, ModelDashesAndWidthsFollowTheStrokeTimeAffineMetric) {
    constexpr double pi=3.14159265358979323846;PathRecorder r;r.scale(4);r.lineWidth(1);r.arc({0,0},10,0,pi/2);r.dash({2,1},0,false);
    StrokeStyle style;style.widthUnits=WidthUnits::Model;const auto mesh=prepareStroke(recorded(r),style);
    for (unsigned i=0;i<15;++i) {
        const double t=(i+.5)/10;EXPECT_EQ(i%3!=2,covers(mesh,{40*std::cos(t),40*std::sin(t)}))<<i;
    }
    EXPECT_LE(mesh.positionBound,.25);
}
TEST(CpuCurvedStrokeTest, ExactCollinearReversalHasExplicitJoinPolicy) {
    PathRecorder r;r.lineWidth(1);r.moveTo({0,0});r.quadraticTo({10,0},{0,0});const auto c=recorded(r);
    const auto bevel=prepareStroke(c);EXPECT_TRUE(covers(bevel,{4,.3}));EXPECT_TRUE(covers(bevel,{5.3,0}));
    StrokeStyle style;style.join=StrokeJoin::Round;EXPECT_TRUE(covers(prepareStroke(c,style),{5.3,0}));
}
TEST(CpuCurvedStrokeTest, OffDyadicCollinearStationarySpansRetainTheSharedFixture) {
    Contract c("spline","high-curvature");auto p=controls(c.input("control-points"),0,4);const auto& z=c.input("zoom-dpr");
    const double scale=z[0].GetDouble()*z[1].GetDouble();PathRecorder r;r.scale(scale);r.bezierSpan(p);const auto mesh=prepareStroke(recorded(r));
    double error=0;bool finite=true;
    for (auto& q:p) {q.x*=scale;q.y*=scale;}
    for (unsigned i=0;i<=512;++i) error=std::max(error,meshDistance(mesh,evaluate(p,i/512.L)));
    EXPECT_NEAR(c.expected("max-display-error").GetDouble(),error,.25);
    for (const auto& t:mesh.triangles) for (auto q:t) finite&=std::isfinite(q.x)&&std::isfinite(q.y);
    c.observe("finite-output",finite);c.complete();EXPECT_GT(mesh.maximum.x,73);EXPECT_LT(mesh.minimum.x,-73);
}
TEST(CpuCurvedStrokeTest, FullEllipseClosesWithExactOwnedEndpoint) {
    PathRecorder r;r.ellipse({2,3},12,6,.37,.37,.73);const auto& p=r.path();
    ASSERT_GT(p.size(),1u);EXPECT_DOUBLE_EQ(p.front().point.x,p.back().point.x);EXPECT_DOUBLE_EQ(p.front().point.y,p.back().point.y);
    r.closePath();EXPECT_FALSE(prepareStroke(recorded(r)).triangles.empty());
}
TEST(CpuCurvedStrokeTest, CurveLimitsFailureIsAtomicAndLaterWorkRecovers) {
    PathRecorder r;r.moveTo({0,0});r.cubicTo({0,12},{12,-8},{16,4});const auto c=recorded(r);
    StrokeLimits limits;limits.vertices=16;PreparedStroke output;
    EXPECT_EQ("vertex-limit",diagnostic([&]{output=prepareStroke(c,{},limits);}));EXPECT_TRUE(output.triangles.empty());
    limits={};limits.work=1;EXPECT_EQ("subdivision-limit",diagnostic([&]{output=prepareStroke(c,{},limits);}));
    EXPECT_FALSE(prepareStroke(c).triangles.empty());EXPECT_EQ(2u,c.path.size());
    CurveMetrics metrics;metrics.physicalRadius=-1;EXPECT_EQ("invalid-limits",diagnostic([&]{flatten(c,{},metrics);}));
}
TEST(CpuCurvedStrokeTest, StationaryTwoDimensionalCuspUsesOneSidedTangents) {
    PathRecorder r;r.lineWidth(1);r.bezierSpan({{3,-1},{-1,1},{-1,-1},{3,1}});const auto c=recorded(r);
    const auto bevel=prepareStroke(c);EXPECT_FALSE(bevel.triangles.empty());EXPECT_TRUE(covers(bevel,{-.3,0}));
    StrokeStyle style;style.join=StrokeJoin::Round;EXPECT_TRUE(covers(prepareStroke(c,style),{-.3,0}));
}
TEST(CpuCurvedStrokeTest, TinyNoncollinearControlsCannotUnderflowIntoAStraightSpan) {
    PathRecorder r;r.moveTo({0,0});r.cubicTo({1e-200,1e-200},{-1e-200,1e-200},{0,0});const auto c=recorded(r);
    PreparedStroke output;EXPECT_EQ("precision-limit",diagnostic([&]{output=prepareStroke(c);}));
    EXPECT_TRUE(output.triangles.empty());EXPECT_FALSE(prepareStroke(strokeLine()).triangles.empty());
}
TEST(CpuCurvedStrokeTest, MonotoneControlPolygonsDoNotNeedNearCriticalDiscriminantRoots) {
    for (double sign:{-1.,1.}) {
        PathRecorder r;r.lineWidth(2);
        const std::vector<Point> p={{0,0},{sign,0},{3*sign,0},{sign*std::nextafter(7.,8.),0}};
        r.bezierSpan(p);const auto command=recorded(r);const auto mesh=prepareStroke(command);
        EXPECT_LT(mesh.visited,32u);EXPECT_LE(mesh.positionBound,.25);
        for (unsigned i=0;i<=64;++i) {
            const auto q=evaluate(p,i/64.L);EXPECT_TRUE(covers(mesh,{q.x,.8}));EXPECT_TRUE(covers(mesh,{q.x,-.8}));
        }
        EXPECT_FALSE(covers(mesh,{3*sign,1.2}));
    }
}
TEST(CpuCurvedStrokeTest, StrictlyMonotoneNearStationaryCubicsUseAnExactSignCertificate) {
    using Exact=boost::multiprecision::cpp_bin_float_100;
    for (double sign:{-1.,1.}) {
        const double end=std::nextafter(1.,2.);const std::vector<Point> p={{0,0},{sign,0},{0,0},{sign*end,0}};
        const Exact d0=Exact(p[1].x)-p[0].x,d1=Exact(p[2].x)-p[1].x,d2=Exact(p[3].x)-p[2].x;
        ASSERT_GT(d0*d2-d1*d1,0); // derivative has no zero/reversal anywhere
        PathRecorder r;r.lineWidth(2);r.bezierSpan(p);const auto mesh=prepareStroke(recorded(r));
        EXPECT_LT(mesh.visited,32u);EXPECT_LE(mesh.positionBound,.25);
        for (unsigned i=1;i<64;++i) {
            const auto q=evaluate(p,i/64.L);EXPECT_TRUE(covers(mesh,{q.x,.8}));EXPECT_TRUE(covers(mesh,{q.x,-.8}));
        }
        EXPECT_FALSE(covers(mesh,{-sign*.3,0}));EXPECT_FALSE(covers(mesh,{sign*(end+.3),0}));
    }
}
TEST(CpuCurvedStrokeTest, FmaResidualOrderingCertifiesAxisAlignedOffsetsWithoutHidingReversals) {
    using Exact=boost::multiprecision::cpp_bin_float_100;
    for (int exponent:{-10,0,10}) for (unsigned neighbor=1;neighbor<=8;++neighbor) {
        const double delta=std::ldexp(1.,-26)-neighbor*std::ldexp(1.,-52),scale=std::ldexp(1.,exponent);
        const double a=(1+delta)*scale,b=(1-std::ldexp(1.,-53))*scale,c=(1-delta)*scale;
        ASSERT_EQ(a*c,b*b);ASSERT_GT(std::fma(a,c,-a*c),std::fma(b,b,-b*b));
        for (bool vertical:{false,true}) for (double sign:{-1.,1.}) {
            const auto at=[&](double v){return vertical?Point{0,sign*v}:Point{sign*v,0};};
            const std::vector<Point> p={at(-a),at(0),at(-b),at(c-b)};
            const Exact d0=a,d1=-Exact(b),d2=c;ASSERT_GT(d0*d2-d1*d1,0);
            ASSERT_NE(Exact((c-b)-(-a)),Exact(c-b)-(-a)); // old global-offset test was inexact
            PathRecorder r;r.lineWidth(2);r.bezierSpan(p);const auto mesh=prepareStroke(recorded(r));
            EXPECT_LT(mesh.visited,32u);EXPECT_LE(mesh.positionBound,.25);
            for (unsigned i=1;i<64;++i) {
                auto q=evaluate(p,i/64.L);if (vertical) q.x=.8;else q.y=.8;EXPECT_TRUE(covers(mesh,q));
            }
            // Adjacent binary input really does reverse. It is still unresolved,
            // never classified as monotone by an epsilon or extended precision.
            const double opposite=std::nextafter(c,0.);ASSERT_LT(Exact(a)*opposite-Exact(b)*b,0);
            PathRecorder bad;bad.bezierSpan({at(-a),at(0),at(-b),at(opposite-b)});PreparedStroke output;
            EXPECT_EQ("precision-limit",diagnostic([&]{output=prepareStroke(recorded(bad));}));EXPECT_TRUE(output.triangles.empty());
            EXPECT_FALSE(prepareStroke(recorded(r)).triangles.empty());
        }
    }
}
TEST(CpuCurvedStrokeTest, NearCriticalScalarRoundingFailsClosedButExactStationarySpansWork) {
    PathRecorder r;r.lineWidth(2);r.moveTo({0,0});r.cubicTo({1,0},{-std::numeric_limits<double>::denorm_min(),0},{1,0});
    const auto uncertain=recorded(r);StrokeStyle style;style.join=StrokeJoin::Round;PreparedStroke output;
    EXPECT_EQ("precision-limit",diagnostic([&]{output=prepareStroke(uncertain,style);}));EXPECT_TRUE(output.triangles.empty());
    PathRecorder exact;exact.lineWidth(2);exact.bezierSpan({{0,0},{1,0},{0,0},{1,0}});
    const auto mesh=prepareStroke(recorded(exact),style);EXPECT_TRUE(covers(mesh,{.5,.8}));EXPECT_FALSE(covers(mesh,{-.3,0}));
}
TEST(CpuCurvedStrokeTest, CurvedModelWidthRetainsLargeOriginAndReflectedAffineDetail) {
    Transform t;t.origin={1e12,-1e12};t.xx=-2;t.xy=1;t.yx=.5;t.yy=3;
    PathRecorder r;r.setTransform(t);r.lineWidth(2);r.moveTo(t.origin);r.quadraticTo({t.origin.x+4,t.origin.y+6},{t.origin.x+8,t.origin.y});
    StrokeStyle style;style.widthUnits=WidthUnits::Model;const auto mesh=prepareStroke(recorded(r),style);
    const std::vector<Point> p={{0,0},{4,6},{8,0}};
    for (unsigned i=0;i<=32;++i) {
        const double t=i/32.;const Point q=evaluate(p,t),d=derivative(p,t);const double n=std::hypot(d.x,d.y);
        for (double side:{-1.,1.}) {
            const auto physical=r.commands()[0].state.transform.distance({q.x-side*d.y/n,q.y+side*d.x/n});
            EXPECT_LE(meshDistance(mesh,physical),mesh.positionBound+1e-10);
        }
    }
    EXPECT_LT(std::abs(mesh.minimum.x),100);EXPECT_LT(std::abs(mesh.maximum.y),100);
}
TEST(CpuCurvedStrokeTest, SharpCurveBoundaryUsesExactTangentsAtTheMiterThreshold) {
    PathRecorder r;r.lineWidth(2);r.moveTo({-4,0});r.quadraticTo({-2,2},{0,0});r.lineTo({4,4});const auto c=recorded(r);
    StrokeStyle style;style.miterLimit=std::sqrt(2.)*(1-1e-10);EXPECT_FALSE(covers(prepareStroke(c,style),{0,-1.3}));
    style.miterLimit=std::sqrt(2.)*(1+1e-10);EXPECT_TRUE(covers(prepareStroke(c,style),{0,-1.3}));
}
TEST(CpuStrokeTest, NearReversalMitersUseStableIntersectionsAndCertifiedNumericBounds) {
    for (double slope:{1e-3,1e-4,1e-5,1e-6}) for (double sign:{-1.,1.}) for (double angle:{0.,.37}) {
        PathRecorder r;r.lineWidth(1);r.rotate(angle);r.moveTo({-1,0});r.lineTo({0,0});r.lineTo({-1,sign*slope});
        StrokeStyle style;style.miterLimit=1e9;const auto mesh=prepareStroke(recorded(r),style);
        // Independent intersection of the radius-.5 offset rays. This stable
        // formula uses the input slope, not the implementation's rounded dot.
        const Point tip{(std::hypot(1.,slope)+1)/(2*slope),-sign*.5};
        const auto expected=r.state().transform.distance(tip);
        EXPECT_LE(meshDistance(mesh,expected),mesh.positionBound)<<slope<<","<<sign<<","<<angle;
        EXPECT_TRUE(covers(mesh,r.state().transform.distance({tip.x/4,-sign*.1})));
        EXPECT_LE(mesh.positionBound,.25);
    }
}
TEST(CpuStrokeTest, UncertifiableNearReversalMitersFailRatherThanSilentlyBecomingBevels) {
    auto saved=prepareStroke(strokeLine());const auto count=saved.triangles.size();
    for (double slope:{1e-7,1e-8,1e-12}) {
        PathRecorder r;r.lineWidth(1);r.moveTo({-1,0});r.lineTo({0,0});r.lineTo({-1,slope});const auto command=recorded(r);
        StrokeStyle style;style.miterLimit=1e15;
        EXPECT_EQ("precision-limit",diagnostic([&]{saved=prepareStroke(command,style);}));
        EXPECT_EQ(count,saved.triangles.size());
        // A genuinely clipped miter still has a well-conditioned bevel; do not
        // reject it merely because an unused infinite intersection is distant.
        EXPECT_FALSE(prepareStroke(command).triangles.empty());
    }
}
TEST(CpuCurvedStrokeTest, DashPhaseUncertaintyCannotChangeSharpCornerOrEndpointTopology) {
    constexpr double pi=3.14159265358979323846;
    PathRecorder corner;corner.lineWidth(2);corner.arc({0,0},1,0,pi/2);corner.lineTo({0,2});corner.dash({pi/2,1},0,true);
    const auto c=recorded(corner);PreparedStroke output;
    EXPECT_EQ("precision-limit",diagnostic([&]{output=prepareStroke(c);}));EXPECT_TRUE(output.triangles.empty());
    PathRecorder end;end.arc({0,0},1,0,pi/2);end.dash({.25,pi/2-.25},0,true);
    StrokeStyle style;style.cap=StrokeCap::Round;
    EXPECT_EQ("precision-limit",diagnostic([&]{output=prepareStroke(recorded(end),style);}));EXPECT_TRUE(output.triangles.empty());
}
TEST(CpuCurvedStrokeTest, ShearedModelDashMetricPreservesPhysicalPhaseAndPositionBounds) {
    constexpr double pi=3.14159265358979323846;Transform t;t.xy=16;
    PathRecorder r;r.setTransform(t);r.lineWidth(.1);r.arc({0,0},10,0,pi/2);r.dash({2,1},0,false);
    const auto mesh=prepareStroke(recorded(r));EXPECT_LE(mesh.positionBound,.25);
    for (unsigned i=0;i<15;++i) {
        const double a=(i+.5)/10;const Point p=t.distance({10*std::cos(a),10*std::sin(a)});
        EXPECT_EQ(i%3!=2,covers(mesh,p))<<i;
    }
}

namespace {
PathCommand fillPath(const std::vector<std::vector<Point>>& contours,Transform transform={}) {
    PathRecorder r;r.setTransform(transform);
    for (const auto& loop:contours) {
        if (loop.empty()) continue;
        r.moveTo(loop.front());for (std::size_t i=1;i<loop.size();++i) r.lineTo(loop[i]);
        // Deliberately leave open: fill must implicitly close every contour.
    }
    r.fill();return r.commands().empty()?PathCommand{PaintOperation::Fill,{}, {}}:r.commands()[0];
}
PathCommand fillFixture(Contract& c) {
    std::vector<std::vector<Point>> loops;std::set<unsigned> starts{0};
    for (const auto& s:c.input("contour-starts").GetArray()) starts.insert(s.GetUint());
    const auto& vertices=c.input("vertices");
    for (unsigned i=0;i<vertices.Size();++i) {if (starts.count(i)) loops.emplace_back();loops.back().push_back(point(vertices[i]));}
    EXPECT_EQ("even-odd",std::string(c.input("fill-rule").GetString()));return fillPath(loops);
}
double fillArea(const PreparedFill& fill) {
    using Exact=boost::multiprecision::cpp_bin_float_100;Exact area=0;
    for (const auto& t:fill.triangles) {
        EXPECT_EQ(1,orientation(t[0],t[1],t[2]));
        area+=((Exact(t[1].x)-t[0].x)*(Exact(t[2].y)-t[0].y)-(Exact(t[1].y)-t[0].y)*(Exact(t[2].x)-t[0].x))/2;
    }
    return area.convert_to<double>();
}
bool covers(const PreparedFill& fill,Point p) {for (const auto& t:fill.triangles) if (inTriangle(p,t)) return true;return false;}
std::vector<Point> square(double low,double high) {return {{low,low},{high,low},{high,high},{low,high}};}
}
TEST(CpuFillTest, CpuHatchFillOrdinary) {
    Contract c("hatch-fill","ordinary");const auto command=fillFixture(c);const auto fill=prepareFill(command);
    c.measure("area",fillArea(fill),1e-9);const auto& expected=c.expected("membership");const auto& samples=c.input("sample-points");
    ASSERT_EQ(samples.Size(),expected.Size());for (unsigned i=0;i<samples.Size();++i)
        EXPECT_EQ(expected[i].GetString(),std::string(covers(fill,point(samples[i]))?"inside":"outside"));
    c.complete();EXPECT_EQ(command.path.size(),8u);
}
TEST(CpuFillTest, CpuHatchFillEmpty) {
    Contract c("hatch-fill","empty");std::vector<Point> points;for (const auto& p:c.input("vertices").GetArray()) points.push_back(point(p));
    const auto hatch=prepareHatch(fillPath({points}),{HatchFamily{}},c.input("pattern-scale").GetDouble());
    c.observe("draw-count",hatch.mask.triangles.size()+hatch.segments.size());c.complete();
}
TEST(CpuFillTest, CpuHatchFillLimit) {
    Contract c("hatch-fill","limit");HatchLimits limits;limits.lines=c.input("limit").GetUint();
    // Width .1 and unit spacing produce exactly the requested five lines0..4.
    auto command=fillPath({square(0,c.input("requested").GetDouble()-1)});command.state.lineWidth=.1;
    PreparedHatch output;c.observe("diagnostic",diagnostic([&]{output=prepareHatch(command,{HatchFamily{}},1,FillRule::EvenOdd,limits);}));
    c.observe("accepted-output-count",output.segments.size());EXPECT_TRUE(output.mask.triangles.empty());c.complete();
    EXPECT_EQ(5u,prepareHatch(command,{HatchFamily{}}).segments.size());
}
TEST(CpuFillTest, CpuHatchFillConcave) {
    Contract c("hatch-fill","concave");std::vector<Point> loop;for (const auto& p:c.input("vertices").GetArray()) loop.push_back(point(p));
    EXPECT_EQ("even-odd",std::string(c.input("fill-rule").GetString()));const auto fill=prepareFill(fillPath({loop}));
    c.measure("area",fillArea(fill),1e-9);c.complete();EXPECT_TRUE(covers(fill,{.5,1.5}));EXPECT_FALSE(covers(fill,{1.5,1.5}));
}
TEST(CpuFillTest, CpuHatchFillReversedHole) {
    Contract c("hatch-fill","reversed-hole");const auto fill=prepareFill(fillFixture(c));c.measure("area",fillArea(fill),1e-9);c.complete();
    EXPECT_FALSE(covers(fill,{2,2}));
}
TEST(CpuFillTest, CpuHatchFillInvalidPattern) {
    Contract c("hatch-fill","invalid-pattern");PreparedHatch output;
    c.observe("diagnostic",diagnostic([&]{output=prepareHatch(fillPath({square(0,4)}),{HatchFamily{}},c.input("pattern-scale").GetDouble());}));
    c.observe("accepted-output-count",output.segments.size());EXPECT_TRUE(output.mask.triangles.empty());c.complete();
}
TEST(CpuFillTest, WindingOrientationNestingAndDisconnectedRegions) {
    auto outer=square(0,8),hole=square(1,7),island=square(2,6),other=square(10,12);
    for (bool reverse:{false,true}) {
        if (reverse) std::reverse(hole.begin(),hole.end());
        const auto command=fillPath({outer,hole,island,other});
        const auto even=prepareFill(command);EXPECT_NEAR(48,fillArea(even),1e-12);
        EXPECT_TRUE(covers(even,{.5,.5}));EXPECT_FALSE(covers(even,{1.5,1.5}));EXPECT_TRUE(covers(even,{3,3}));EXPECT_TRUE(covers(even,{11,11}));
        const auto nonzero=prepareFill(command,FillRule::NonZero);EXPECT_NEAR(reverse?48:68,fillArea(nonzero),1e-12);
    }
}
TEST(CpuFillTest, TopologyCrossingTouchingAndRetracingRejectAtomically) {
    for (const auto& contours:std::vector<std::vector<std::vector<Point>>>{
            {{{0,0},{4,4},{0,4},{4,0}}}, // bow tie, not an empty zero signed area
            {square(0,4),{{4,1},{5,1},{5,2},{4,2}}},
            {{{0,0},{4,0},{2,0},{4,4},{0,4}}},
            {square(0,4),square(0,4)}}) {
        PreparedFill output;EXPECT_EQ("unsupported-fill-topology",diagnostic([&]{output=prepareFill(fillPath(contours));}));
        EXPECT_TRUE(output.triangles.empty());EXPECT_DOUBLE_EQ(16,fillArea(prepareFill(fillPath({square(0,4)}))));
    }
}
TEST(CpuFillTest, DegenerateBoundsDuplicateVerticesAndThinValidContours) {
    for (const auto& loop:std::vector<std::vector<Point>>{{},{ {0,1} },{{0,1},{1,1},{2,1}},{{1,0},{1,1},{1,2}}}) {
        const auto hatch=prepareHatch(fillPath({loop}),{HatchFamily{}});EXPECT_TRUE(hatch.mask.triangles.empty());EXPECT_TRUE(hatch.segments.empty());
    }
    const auto fill=prepareFill(fillPath({{{0,0},{4,0},{4,0},{4,.001},{0,.001},{0,0}}}));EXPECT_NEAR(.004,fillArea(fill),1e-12);
}
TEST(CpuFillTest, AffineReflectionLargeOriginAndIndependentGridMembership) {
    const std::vector<std::vector<Point>> loops={square(0,4),square(1,3)};
    for (double sx:{-2.,2.}) for (double shear:{0.,.5}) {
        Transform t;t.xx=sx;t.xy=shear;t.yy=3;t.origin={1e12,-1e12};
        auto shifted=loops;for (auto& loop:shifted) for (auto& p:loop) {p.x+=t.origin.x;p.y+=t.origin.y;}
        const auto fill=prepareFill(fillPath(shifted,t));EXPECT_NEAR(72,fillArea(fill),1e-10);
        for (int x=-2;x<19;++x) for (int y=-2;y<19;++y) {
            const Point p{(x+.37)/4,(y+.61)/4};
            const bool inside=p.x>0 && p.x<4 && p.y>0 && p.y<4 && !(p.x>1 && p.x<3 && p.y>1 && p.y<3);
            EXPECT_EQ(inside,covers(fill,t.distance(p)));
        }
    }
}
TEST(CpuFillTest, CurvedCircleAndPolynomialBoundaryHaveIndependentAreaAndDistance) {
    PathRecorder r;r.circle({0,0},10);r.fill();const auto circle=prepareFill(r.commands()[0]);
    EXPECT_LE(circle.positionBound,.125);EXPECT_NEAR(100*3.141592653589793,fillArea(circle),4);
    for (unsigned i=0;i<128;++i) {
        const double a=i*2*3.141592653589793/128;
        EXPECT_TRUE(covers(circle,{9.8*std::cos(a),9.8*std::sin(a)}));EXPECT_FALSE(covers(circle,{10.2*std::cos(a),10.2*std::sin(a)}));
    }
    PathRecorder p;p.moveTo({0,0});p.quadraticTo({2,4},{4,0});p.fill();const auto curved=prepareFill(p.commands()[0]);
    EXPECT_NEAR(16./3,fillArea(curved),.15);EXPECT_TRUE(covers(curved,{2,1}));EXPECT_FALSE(covers(curved,{2,2.2}));
}
TEST(CpuFillTest, WorkVertexStateAndBatchCapsPreserveOrderAndPriorOutput) {
    auto command=fillPath({square(0,4)});command.state.rgba={{1,0,0,.5}};const auto good=prepareFill(command);
    FillLimits exact;exact.work=good.visited;exact.vertices=6;EXPECT_EQ(good.visited,prepareFill(command,FillRule::EvenOdd,exact).visited);
    --exact.work;PreparedFill output;EXPECT_EQ("fill-work-limit",diagnostic([&]{output=prepareFill(command,FillRule::EvenOdd,exact);}));EXPECT_TRUE(output.triangles.empty());
    exact={};exact.vertices=5;EXPECT_EQ("vertex-limit",diagnostic([&]{output=prepareFill(command,FillRule::EvenOdd,exact);}));
    auto second=command;second.state.rgba={{0,0,1,.25}};auto batch=prepareFills({command,second});ASSERT_EQ(2u,batch.size());
    EXPECT_DOUBLE_EQ(.5,batch[0].state.rgba[3]);EXPECT_DOUBLE_EQ(.25,batch[1].state.rgba[3]);
    FillLimits limits;limits.work=2*(good.visited+1)-1;EXPECT_EQ("fill-work-limit",diagnostic([&]{batch=prepareFills({command,second},FillRule::EvenOdd,limits);}));
    EXPECT_EQ(2u,batch.size());EXPECT_DOUBLE_EQ(16,fillArea(batch[0]));
    limits={};limits.vertices=11;EXPECT_EQ("vertex-limit",diagnostic([&]{output=prepareFills({command,second},FillRule::EvenOdd,limits)[0];}));
}
TEST(CpuFillTest, InvalidInputsAndPrecisionDomainsDoNotPublishPartialGeometry) {
    const auto good=fillPath({square(0,4)});PreparedFill output;
    for (auto command:{good,good,good}) {
        command.path[1].point.x=std::numeric_limits<double>::infinity();EXPECT_THROW(prepareFill(command),std::invalid_argument);
    }
    auto bad=good;bad.operation=PaintOperation::Stroke;EXPECT_EQ("invalid-fill",diagnostic([&]{output=prepareFill(bad);}));
    EXPECT_EQ("invalid-fill-rule",diagnostic([&]{output=prepareFill(good,static_cast<FillRule>(99));}));
    bad=good;bad.path[1].point.x=1e10;EXPECT_EQ("precision-limit",diagnostic([&]{output=prepareFill(bad);}));
    bad=good;bad.state.rgba[0]=2;EXPECT_EQ("invalid-color",diagnostic([&]{output=prepareFill(bad);}));
    EXPECT_TRUE(output.triangles.empty());EXPECT_DOUBLE_EQ(16,fillArea(prepareFill(good)));
}
TEST(CpuHatchTest, FiniteWidthCoverageUsesMaskNotCenterlineClipping) {
    auto command=fillPath({square(0,4),square(1,3)});command.state.lineWidth=1.2;
    HatchFamily family;family.origin={0,.5};const auto hatch=prepareHatch(command,{family});
    ASSERT_EQ(6u,hatch.segments.size());EXPECT_NEAR(-.5,hatch.segments.front()[0].y,1e-12); // outside centerline still contributes ink
    for (const auto& line:hatch.segments) {EXPECT_LT(line[0].x,-.6);EXPECT_GT(line[1].x,4.6);}
    EXPECT_TRUE(covers(hatch.mask,{.2,.05}));EXPECT_FALSE(covers(hatch.mask,{2,2}));
    EXPECT_EQ(command.state.rgba,hatch.mask.state.rgba);
}
TEST(CpuHatchTest, PatternAffineScaleRotationAndNegativeInputsAreExplicit) {
    Transform t;t.xx=-2;t.xy=.5;t.yy=3;auto command=fillPath({square(0,4)},t);command.state.lineWidth=.1;
    HatchFamily f;f.origin={.125,.25};f.angle=.37;f.spacing=.5;
    const auto hatch=prepareHatch(command,{f},2);EXPECT_FALSE(hatch.segments.empty());
    const Point origin=t.point({.25,.5}),axis=t.distance({std::cos(.37),std::sin(.37)}),delta=t.distance({-std::sin(.37),std::cos(.37)});
    for (const auto& line:hatch.segments) {
        const Point d{line[1].x-line[0].x,line[1].y-line[0].y};EXPECT_NEAR(0,d.x*axis.y-d.y*axis.x,1e-10);
        const Point p{line[0].x-origin.x,line[0].y-origin.y};const double k=(axis.x*p.y-axis.y*p.x)/(axis.x*delta.y-axis.y*delta.x);
        EXPECT_NEAR(std::round(k),k,1e-10);
    }
    PreparedHatch output;for (double scale:{-1.,0.,std::numeric_limits<double>::infinity()})
        EXPECT_EQ("invalid-hatch-scale",diagnostic([&]{output=prepareHatch(command,{f},scale);}));
    f.spacing=0;EXPECT_EQ("invalid-hatch-pattern",diagnostic([&]{output=prepareHatch(command,{f});}));
    f.spacing=-1;EXPECT_EQ("invalid-hatch-pattern",diagnostic([&]{output=prepareHatch(command,{f});}));
    EXPECT_TRUE(output.mask.triangles.empty());EXPECT_TRUE(output.segments.empty());
}
TEST(CpuHatchTest, AggregateLinesSegmentsVerticesWorkAndUnsupportedDashesFailClosed) {
    auto c=fillPath({square(0,4)});c.state.lineWidth=.1;const auto good=prepareHatch(c,{HatchFamily{}});
    HatchLimits limits;limits.lines=9;PreparedHatch out;
    EXPECT_EQ("hatch-expansion-limit",diagnostic([&]{out=prepareHatch(c,{HatchFamily{},HatchFamily{}},1,FillRule::EvenOdd,limits);}));
    limits={};limits.segments=4;EXPECT_EQ("hatch-expansion-limit",diagnostic([&]{out=prepareHatch(c,{HatchFamily{}},1,FillRule::EvenOdd,limits);}));
    limits={};limits.fill.vertices=15;EXPECT_EQ("hatch-expansion-limit",diagnostic([&]{out=prepareHatch(c,{HatchFamily{}},1,FillRule::EvenOdd,limits);}));
    limits={};limits.fill.work=good.visited;EXPECT_EQ(good.visited,prepareHatch(c,{HatchFamily{}},1,FillRule::EvenOdd,limits).visited);
    --limits.fill.work;EXPECT_EQ("hatch-expansion-limit",diagnostic([&]{out=prepareHatch(c,{HatchFamily{}},1,FillRule::EvenOdd,limits);}));
    c.state.dash={1,1};EXPECT_EQ("unsupported-hatch-pattern",diagnostic([&]{out=prepareHatch(c,{HatchFamily{}});}));
    EXPECT_TRUE(out.mask.triangles.empty());EXPECT_TRUE(out.segments.empty());
}
TEST(CpuGradientTest, FixtureLandmarksAreCpuSamplesNotGpuPixels) {
    Contract c("gradient","ordinary");LinearGradient g;g.start=point(c.input("axis")[0]);g.end=point(c.input("axis")[1]);
    const auto& colors=c.input("colors");unsigned i=0;
    for (const auto& offset:c.input("offsets").GetArray()) {
        GradientStop s;s.offset=offset.GetDouble();for (unsigned j=0;j<4;++j) s.rgba[j]=colors[i++].GetDouble()/255;g.stops.push_back(s);
    }
    const auto prepared=prepareLinearGradient(g);const auto& expected=c.expected("landmark-rgba");i=0;
    for (const auto& p:c.input("sample-points").GetArray()) for (double v:sampleGradient(prepared,point(p))) EXPECT_EQ(expected[i++].GetInt(),std::lround(v*255));
    EXPECT_EQ(expected.Size(),i);c.complete();
}
TEST(CpuGradientTest, InvalidEmptyLimitDegenerateAndStopsUseFixtureDiagnostics) {
    for (const char* variant:{"empty","limit","degenerate-axis","invalid-stop"}) {
        Contract c("gradient",variant);LinearGradient g;g.end={4,0};std::size_t limit=1024;const std::string name=variant;
        if (name=="empty") EXPECT_TRUE(c.input("offsets").Empty());
        else if (name=="limit") {limit=c.input("limit").GetUint();g.stops.resize(c.input("requested").GetUint());}
        else if (name=="degenerate-axis") {g.start=point(c.input("axis")[0]);g.end=point(c.input("axis")[1]);g.stops.resize(1);}
        else for (const auto& offset:c.input("offsets").GetArray()) {GradientStop s;s.offset=offset.GetDouble();g.stops.push_back(s);}
        LinearGradient out;c.observe("diagnostic",diagnostic([&]{out=prepareLinearGradient(g,limit);}));
        c.observe("accepted-output-count",out.stops.size());c.complete();
    }
}
TEST(CpuGradientTest, StableDuplicatesStraightAlphaPaddingAndOwnedSnapshot) {
    LinearGradient g;g.end={4,0};g.stops={{1,{{0,0,1,1}}},{.5,{{1,0,0,.5}}},{0,{{0,0,0,0}}},{.5,{{0,1,0,.25}}}};
    const auto c=fillPath({square(0,4)});const auto fill=prepareFill(c,FillRule::EvenOdd,{},&g);ASSERT_TRUE(fill.hasGradient);
    EXPECT_EQ((std::array<double,4>{{0,1,0,.25}}),sampleGradient(fill.gradient,{2,0}));
    EXPECT_EQ((std::array<double,4>{{.5,0,0,.25}}),sampleGradient(fill.gradient,{1,0}));
    EXPECT_EQ((std::array<double,4>{{0,0,0,0}}),sampleGradient(fill.gradient,{-1,0}));
    EXPECT_EQ((std::array<double,4>{{0,0,1,1}}),sampleGradient(fill.gradient,{5,0}));
    g.stops.clear();EXPECT_EQ(4u,fill.gradient.stops.size());
    FillLimits limits;limits.work=fill.visited-1;PreparedFill output;
    EXPECT_EQ("fill-work-limit",diagnostic([&]{output=prepareFill(c,FillRule::EvenOdd,limits,&fill.gradient);}));EXPECT_FALSE(output.hasGradient);
    auto bad=fill.gradient;bad.stops[0].rgba[3]=std::numeric_limits<double>::quiet_NaN();EXPECT_EQ("invalid-gradient-stop",diagnostic([&]{sampleGradient(bad,{1,0});}));
}
TEST(CpuGradientTest, SamplingMutableUnsortedValuesKeepsStableDuplicatesAndValidatesEveryCall) {
    LinearGradient g;g.end={4,0};g.stops={{1,{{0,0,1,1}}},{.5,{{1,0,0,.5}}},{0,{{0,0,0,0}}},{.5,{{0,1,0,.25}}}};
    EXPECT_EQ((std::array<double,4>{{0,1,0,.25}}),sampleGradient(g,{2,0}));
    EXPECT_EQ((std::array<double,4>{{.5,0,0,.25}}),sampleGradient(g,{1,0}));
    EXPECT_DOUBLE_EQ(1,g.stops.front().offset); // unsorted fallback never changes caller storage
    auto prepared=prepareLinearGradient(g);
    EXPECT_EQ((std::array<double,4>{{0,1,0,.25}}),sampleGradient(prepared,{2,0}));
    // Even previously prepared data can be reordered or mutated by its owner.
    std::swap(prepared.stops[1],prepared.stops[2]);
    EXPECT_EQ((std::array<double,4>{{1,0,0,.5}}),sampleGradient(prepared,{2,0}));
    prepared.stops[0].offset=.75;
    EXPECT_EQ((std::array<double,4>{{1,0,0,.5}}),sampleGradient(prepared,{2,0}));
    prepared.stops[0].rgba[3]=2;EXPECT_EQ("invalid-gradient-stop",diagnostic([&]{sampleGradient(prepared,{});}));
    prepared=prepareLinearGradient(g);prepared.end=prepared.start;
    EXPECT_EQ("degenerate-gradient",diagnostic([&]{sampleGradient(prepared,{});}));
    prepared=prepareLinearGradient(g);prepared.stops.clear();
    EXPECT_EQ("empty-gradient",diagnostic([&]{sampleGradient(prepared,{});}));
    EXPECT_EQ("precision-limit",diagnostic([&]{sampleGradient(g,{NAN,0});}));
}
TEST(CpuFillTest, ConcaveNestedMeshAgreesWithIndependentWindingAndHasNoInteriorOverlaps) {
    using Exact=boost::multiprecision::cpp_bin_float_100;
    std::vector<Point> star;for (unsigned i=0;i<16;++i) {
        const double a=i*2*3.141592653589793/16,r=i%2?2.5:5;star.push_back({r*std::cos(a),r*std::sin(a)});
    }
    for (bool reversed:{false,true}) for (auto rule:{FillRule::EvenOdd,FillRule::NonZero}) {
        auto hole=square(-1,1);if (reversed) std::reverse(hole.begin(),hole.end());
        const std::vector<std::vector<Point>> loops={star,hole,square(8,10)};const auto fill=prepareFill(fillPath(loops),rule);
        Exact area=0;for (unsigned i=0;i<star.size();++i) area+=(Exact(star[i].x)*star[(i+1)%star.size()].y-Exact(star[i].y)*star[(i+1)%star.size()].x)/2;
        const double expected=area.convert_to<double>()+4-(rule==FillRule::EvenOdd || reversed?4:0);
        EXPECT_NEAR(expected,fillArea(fill),1e-9);
        for (int ix=-22;ix<43;++ix) for (int iy=-22;iy<43;++iy) {
            const Point p{(ix+.371)/4,(iy+.613)/4};int winding=0;
            for (const auto& loop:loops) for (unsigned i=0;i<loop.size();++i) {
                const Point a=loop[i],b=loop[(i+1)%loop.size()];
                const Exact side=(Exact(b.x)-a.x)*(Exact(p.y)-a.y)-(Exact(b.y)-a.y)*(Exact(p.x)-a.x);
                if (a.y<=p.y && b.y>p.y && side>0) ++winding;
                if (a.y>p.y && b.y<=p.y && side<0) --winding;
            }
            EXPECT_EQ(rule==FillRule::EvenOdd?winding%2!=0:winding!=0,covers(fill,p));
            unsigned strictHits=0;for (const auto& t:fill.triangles)
                if (orientation(t[0],t[1],p)>0 && orientation(t[1],t[2],p)>0 && orientation(t[2],t[0],p)>0) ++strictHits;
            EXPECT_LE(strictHits,1u);
        }
    }
}
TEST(CpuFillTest, RoundoffCanonicalizationCannotEraseARealThinContourOrTouchingCurves) {
    PreparedFill out;EXPECT_EQ("precision-limit",diagnostic([&]{out=prepareFill(fillPath({{{0,0},{4,0},{4,1e-15},{0,1e-15}}}));}));
    PathRecorder r;r.circle({0,0},4);r.circle({8,0},4);r.fill();
    EXPECT_THROW(out=prepareFill(r.commands()[0]),std::invalid_argument);EXPECT_TRUE(out.triangles.empty());
    EXPECT_DOUBLE_EQ(16,fillArea(prepareFill(fillPath({square(0,4)}))));
}

TEST(CpuFillTest, EllipsesUseLocalCurveCertificatesAndRetainAffineAreaAndMembership) {
    constexpr double pi=3.14159265358979323846;
    for (double ry:{1.,2.}) for (double reflection:{-1.,1.}) for (double start:{0.,100.}) {
        Transform t;const double c=std::cos(.37),s=std::sin(.37);
        t.xx=reflection*c;t.xy=-s;t.yx=reflection*s;t.yy=c;t.tx=17.125;t.ty=-8.375;
        PathRecorder r;r.setTransform(t);r.ellipse({0,0},10,ry,start,start+2*pi);r.closePath();r.fill();
        const auto command=r.commands()[0];const auto fill=prepareFill(command);
        EXPECT_LE(fill.positionBound,.125);
        // An inscribed convex approximation loses at most perimeter*distance;
        // 2*pi*majorRadius bounds the analytic ellipse perimeter independently.
        EXPECT_NEAR(pi*10*ry,fillArea(fill),2*pi*10*fill.positionBound);
        for (unsigned i=0;i<64;++i) {
            const double a=2*pi*(i+.31)/64;
            EXPECT_TRUE(covers(fill,t.point({9*std::cos(a),.9*ry*std::sin(a)})));
            EXPECT_FALSE(covers(fill,t.point({11*std::cos(a),1.1*ry*std::sin(a)})));
        }
        const auto flat=flatten(command);
        for (const auto& path:flat.subpaths) EXPECT_EQ(path.points.size()-1,path.edges.size());
    }
}
TEST(CpuFillTest, ScreenSizedCircleUsesBoundedSweepWorkAndPreservesLimits) {
    constexpr double pi=3.14159265358979323846;
    PathRecorder r;r.circle({.125,-.375},500);r.fill();const auto command=r.commands()[0];
    const auto fill=prepareFill(command);
    EXPECT_LT(fill.visited,4*MaxFrameElements);EXPECT_LE(fill.positionBound,.125);
    EXPECT_NEAR(pi*500*500,fillArea(fill),2*pi*500*fill.positionBound);
    for (unsigned i=0;i<64;++i) {
        const double a=2*pi*(i+.23)/64;
        EXPECT_TRUE(covers(fill,{.125+499.75*std::cos(a),-.375+499.75*std::sin(a)}));
        EXPECT_FALSE(covers(fill,{.125+500.25*std::cos(a),-.375+500.25*std::sin(a)}));
    }
    FillLimits exact;exact.work=fill.visited;
    EXPECT_EQ(fill.visited,prepareFill(command,FillRule::EvenOdd,exact).visited);
    --exact.work;PreparedFill saved=fill;
    EXPECT_EQ("fill-work-limit",diagnostic([&]{saved=prepareFill(command,FillRule::EvenOdd,exact);}));
    EXPECT_EQ(fill.triangles.size(),saved.triangles.size());
}
TEST(CpuFillTest, AdjacentCurvesCannotHideCrossingsBehindSharedEndpoints) {
    PathRecorder r;r.moveTo({0,0});r.quadraticTo({5,.05},{10,0});r.lineTo({0,.01});r.fill();
    // Q(t)=(10t,.1t(1-t)) crosses B->D at t=.1: (1,.009),
    // away from the common endpoint. Its coarse chord alone looks triangular.
    PreparedFill saved;
    EXPECT_EQ("unsupported-fill-topology",diagnostic([&]{saved=prepareFill(r.commands()[0]);}));
    EXPECT_TRUE(saved.triangles.empty());
    // This tiny cubic has two parameters t and1-t with t(1-t)=1/61
    // at the same point. Its whole loop previously fitted inside one chord.
    PathRecorder loop;loop.moveTo({0,0});loop.cubicTo({.001,.001},{-.001,.001},{.0001,0});
    loop.lineTo({0,-1});loop.closePath();loop.fill();
    EXPECT_THROW(saved=prepareFill(loop.commands()[0]),std::invalid_argument);EXPECT_TRUE(saved.triangles.empty());
    PathRecorder valid;valid.moveTo({0,0});valid.quadraticTo({5,.05},{10,0});valid.lineTo({0,-.01});valid.fill();
    EXPECT_FALSE(prepareFill(valid.commands()[0]).triangles.empty());
    EXPECT_DOUBLE_EQ(16,fillArea(prepareFill(fillPath({square(0,4)}))));
}
TEST(CpuFillTest, CurveRefinementChargesDiscardedAttemptsWithinTheOriginalBatchBudget) {
    PathRecorder r;r.moveTo({0,0});r.quadraticTo({5,.05},{10,0});r.lineTo({10,-.01});
    r.quadraticTo({5,.04},{0,-.01});r.closePath();r.fill();const auto command=r.commands()[0];
    const auto fill=prepareFill(command);
    EXPECT_NEAR(.1,fillArea(fill),1e-11);EXPECT_LT(fill.positionBound,.005);
    EXPECT_TRUE(covers(fill,{5,.02}));EXPECT_FALSE(covers(fill,{5,.005}));EXPECT_FALSE(covers(fill,{5,.035}));
    FillLimits exact;exact.work=fill.visited;
    EXPECT_EQ(fill.visited,prepareFill(command,FillRule::EvenOdd,exact).visited);
    --exact.work;PreparedFill saved=fill;
    EXPECT_EQ("fill-work-limit",diagnostic([&]{saved=prepareFill(command,FillRule::EvenOdd,exact);}));
    EXPECT_EQ(fill.triangles.size(),saved.triangles.size());
    FillLimits batch;batch.work=2*(fill.visited+1)-1;
    EXPECT_EQ("fill-work-limit",diagnostic([&]{prepareFills({command,command},FillRule::EvenOdd,batch);}));
    EXPECT_FALSE(prepareFill(command).triangles.empty());
}
