#include <gtest/gtest.h>
#include "painters/pathrecorder.h"
#include "buildcontracts_config.h"
#include <rapidjson/document.h>
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
// The nine manifest IDs execute the real recorder using the existing typed
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
