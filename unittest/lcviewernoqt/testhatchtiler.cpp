#include <gtest/gtest.h>
#include "painters/pathgeometry.h"
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <cmath>
#include <limits>

namespace {
using namespace lc::viewer::render;
PathCommand rectangle(double x,double y,double w,double h) {
    PathRecorder r;r.rectangle({x,y},w,h);r.fill();return r.commands()[0];
}
HatchTile tile(Point a={2,2},Point b={4,2},double period=10) {
    PathRecorder r;r.moveTo(a);r.lineTo(b);r.stroke();HatchTile t;
    t.minimum={-3,-7};t.maximum={period-3,period-7};t.commands=r.commands();return t;
}
void inherit(HatchTile& t) {for (auto& c:t.commands) {c.state=t.state;c.state.transform={};}}
int cross(Point a,Point b,Point p) {
    const double x=(b.x-a.x)*(p.y-a.y),y=(b.y-a.y)*(p.x-a.x),v=x-y;
    if (std::abs(v)>16*std::numeric_limits<double>::epsilon()*(std::abs(x)+std::abs(y))) return v>0?1:-1;
    // Exact-sign independent oracle at shared triangle seams, not an epsilon
    // coverage expansion (the two rounded double determinants can both lie).
    using Big=boost::multiprecision::number<boost::multiprecision::cpp_bin_float<100>>;
    const Big exact=(Big(b.x)-Big(a.x))*(Big(p.y)-Big(a.y))-(Big(b.y)-Big(a.y))*(Big(p.x)-Big(a.x));
    return exact>0?1:(exact<0?-1:0);
}
template<class Mesh> bool covers(const Mesh& mesh,Point p) {
    for (const auto& t:mesh.triangles) if (cross(t[0],t[1],p)>=0 && cross(t[1],t[2],p)>=0 && cross(t[2],t[0],p)>=0) return true;
    return false;
}
bool covers(const PreparedTiledHatch& h,Point p) {return covers(h.mask,p) && covers(h.ink,p);}
template<class F> std::string error(F f) {
    try {f();} catch (const std::invalid_argument& e) {return e.what();}
    ADD_FAILURE()<<"Expected atomic diagnostic";return {};
}
}
TEST(CpuHatchTilerTest, ActualFiniteSegmentsUseExtentPeriodsNotMaximumCoordinates) {
    const auto h=prepareTiledHatch(rectangle(0,0,30,20),tile());
    for (int row=0;row<2;++row) for (int col=0;col<3;++col) {
        EXPECT_TRUE(covers(h,{3.+10*col,2.+10*row}));
        EXPECT_FALSE(covers(h,{7.+10*col,2.+10*row}));
        EXPECT_FALSE(covers(h,{3.+10*col,5.+10*row}));
    }
    EXPECT_LE(h.positionBound,.25);EXPECT_EQ(StrokeCoverage::UnionBeforeOver,h.ink.coverage);
}
TEST(CpuHatchTilerTest, ConcaveHolesAndDisconnectedMasksIntersectFullInkCoverage) {
    PathRecorder r;r.rectangle({0,0},30,20);r.rectangle({10,1},10,5);r.rectangle({40,0},10,10);r.fill();
    auto t=tile({-2,2},{12,2});const auto h=prepareTiledHatch(r.commands()[0],t);
    EXPECT_TRUE(covers(h,{3,2}));EXPECT_FALSE(covers(h,{13,2}));EXPECT_TRUE(covers(h,{23,2}));
    EXPECT_FALSE(covers(h,{33,2}));EXPECT_TRUE(covers(h,{43,2}));
    EXPECT_TRUE(covers(h.ink,{13,2})); // mask removes coverage; ink was NOT centerline-clipped
    PathRecorder concave;concave.moveTo({0,0});concave.lineTo({20,0});concave.lineTo({20,5});concave.lineTo({5,5});
    concave.lineTo({5,20});concave.lineTo({0,20});concave.fill();
    const auto c=prepareTiledHatch(concave.commands()[0],t);EXPECT_TRUE(covers(c,{3,12}));EXPECT_FALSE(covers(c,{13,12}));
}
TEST(CpuHatchTilerTest, NeighborTilesContributeFiniteWidthCapsAndPointInkOutsideTheirCenterlines) {
    auto t=tile({-1,-.75},{12,-.75});t.state.lineWidth=2;inherit(t);
    const auto h=prepareTiledHatch(rectangle(0,0,10,4),t);EXPECT_TRUE(covers(h,{3,.1}));EXPECT_FALSE(covers(h,{3,.5}));
    PathRecorder r;r.point({-.5,2},3);t.commands=r.commands();t.state={};
    const auto p=prepareTiledHatch(rectangle(0,0,4,4),t);EXPECT_TRUE(covers(p,{.25,2}));EXPECT_FALSE(covers(p,{1.25,2}));
    t=tile({-.5,2},{-.25,2});StrokeStyle s;s.cap=StrokeCap::Square;t.state.lineWidth=2;inherit(t);
    EXPECT_TRUE(covers(prepareTiledHatch(rectangle(0,0,4,4),t,1,0,FillRule::EvenOdd,s),{.5,2}));
}
TEST(CpuHatchTilerTest, DashPhaseStaysOnWholeFinitePrimitivesAcrossMaskAndTileBoundaries) {
    auto t=tile({0,2},{9,2});t.state.lineWidth=.5;t.state.dash={2,2};t.state.dashOffset=1;t.state.dashScaled=true;inherit(t);
    const auto h=prepareTiledHatch(rectangle(2.5,0,25,5),t);
    for (double x:{3.5,4.5,10.5,13.5,14.5,20.5,23.5}) EXPECT_TRUE(covers(h,{x,2}));
    for (double x:{5.5,6.5,9.5,11.5,12.5,15.5,16.5}) EXPECT_FALSE(covers(h,{x,2}));
    EXPECT_EQ(t.state.dash,h.ink.state.dash);EXPECT_DOUBLE_EQ(1,h.ink.state.dashOffset);
}
TEST(CpuHatchTilerTest, ModelAndPhysicalDashesUseTheCombinedStrokeTimeTransform) {
    auto t=tile({0,2},{9,2});t.state.lineWidth=.5;t.state.dash={2,2};t.state.transform.xx=2;inherit(t);
    const auto boundary=rectangle(0,0,20,5);const auto model=prepareTiledHatch(boundary,t);
    EXPECT_TRUE(covers(model,{3,2}));EXPECT_FALSE(covers(model,{5,2}));
    t.state.dashScaled=true;inherit(t);const auto physical=prepareTiledHatch(boundary,t);
    EXPECT_FALSE(covers(physical,{3,2}));EXPECT_TRUE(covers(physical,{5,2}));
}
TEST(CpuHatchTilerTest, RotationScaleReflectionShearAndCameraOriginComposeInModelOrder) {
    auto t=tile({1,2},{4,2});t.state.transform={-2,.75,.5,3,17,-11,{5,-7}};inherit(t);
    const double angle=.7,scale=1.5;const auto h=prepareTiledHatch(rectangle(-200,-200,400,400),t,scale,angle);
    for (int i=-1;i<=1;++i) for (int j=-1;j<=1;++j) {
        const Point p={2.5+i*10,2.+j*10};const double c=std::cos(angle),s=std::sin(angle);
        const Point q=t.state.transform.point({scale*(c*p.x-s*p.y),scale*(s*p.x+c*p.y)});
        EXPECT_TRUE(covers(h,q));
    }
    EXPECT_LE(h.positionBound,.25);
}
TEST(CpuHatchTilerTest, LargeWorldFractionalPeriodPhaseMatchesIndependentBinaryMultiprecision) {
    using Big=boost::multiprecision::number<boost::multiprecision::cpp_bin_float<100>>;
    auto t=tile({.02,.03},{.07,.03},.1);t.minimum={0,0};t.maximum={.1,.1};
    t.state.transform.origin={1e12,-1e12};t.state.transform.xx=100000;t.state.transform.yy=100000;
    t.state.lineWidth=.5;inherit(t);const auto h=prepareTiledHatch(rectangle(0,0,20000,20000),t);
    const Big w(t.maximum.x),ox(t.state.transform.origin.x),oy(t.state.transform.origin.y);
    const Big ix=floor(ox/w),iy=floor(oy/w);
    unsigned samples=0;
    for (int i=-1;i<=3;++i) for (int j=-1;j<=3;++j) {
        const double x=((ix+i)*w+Big(.045)-ox).convert_to<double>()*100000;
        const double y=((iy+j)*w+Big(.03)-oy).convert_to<double>()*100000;
        if (x>0 && x<20000 && y>0 && y<20000) {++samples;EXPECT_TRUE(covers(h,{x,y}));EXPECT_FALSE(covers(h,{x,y+1}));}
    }
    EXPECT_EQ(4u,samples);EXPECT_LE(h.positionBound,.25);
}
TEST(CpuHatchTilerTest, CurvesBulgesAndClosedSeamsAreRepeatedWithoutReplacingThemByInfiniteLines) {
    PathRecorder r;r.circle({3,3},2);r.stroke();r.ellipse({3,3},2,1,0,0,.3);r.closePath();r.stroke();
    r.polyline({{1,6},{5,6},{7,7}},{.5,0},false);r.stroke();r.moveTo({1,1});r.cubicTo({2,3},{4,3},{5,1});r.stroke();
    auto t=tile();t.commands=r.commands();const auto h=prepareTiledHatch(rectangle(0,0,20,10),t);
    for (double x:{3.,13.}) {EXPECT_TRUE(covers(h,{x,5}));EXPECT_FALSE(covers(h,{x,.1}));}
    EXPECT_FALSE(h.ink.triangles.empty());EXPECT_LE(h.positionBound,.25);
}
TEST(CpuHatchTilerTest, PhysicalPointsKeepDiameterWhileModelPointsFollowPatternAndCameraScale) {
    PathRecorder r;r.point({2,2},3);auto t=tile();t.commands=r.commands();t.state.transform.xx=2;t.state.transform.yy=2;
    auto h=prepareTiledHatch(rectangle(0,0,20,20),t,2);EXPECT_TRUE(covers(h,{9,8}));EXPECT_FALSE(covers(h,{10,8}));
    t.commands[0].pointUnits=PointUnits::Model;h=prepareTiledHatch(rectangle(0,0,20,20),t,2);
    EXPECT_TRUE(covers(h,{12,8}));EXPECT_FALSE(covers(h,{15,8}));
}
TEST(CpuHatchTilerTest, GlobalCellsInstancesVerticesAndWorkCapsFailAtomicallyAndRecover) {
    const auto b=rectangle(0,0,30,20);const auto t=tile();const auto full=prepareTiledHatch(b,t);PreparedTiledHatch saved=full;
    TiledHatchLimits l;l.cells=full.cells-1;EXPECT_EQ("hatch-cell-limit",error([&]{saved=prepareTiledHatch(b,t,1,0,FillRule::EvenOdd,{},l);}));
    l={};l.instances=full.cells-1;EXPECT_EQ("hatch-instance-limit",error([&]{saved=prepareTiledHatch(b,t,1,0,FillRule::EvenOdd,{},l);}));
    l={};l.geometry.vertices=3*(full.mask.triangles.size()+full.ink.triangles.size())-1;
    EXPECT_EQ("vertex-limit",error([&]{saved=prepareTiledHatch(b,t,1,0,FillRule::EvenOdd,{},l);}));
    l={};l.geometry.work=full.visited-1;EXPECT_EQ("hatch-work-limit",error([&]{saved=prepareTiledHatch(b,t,1,0,FillRule::EvenOdd,{},l);}));
    EXPECT_EQ(full.ink.triangles.size(),saved.ink.triangles.size());
    l={};l.cells=full.cells;l.instances=full.cells;l.geometry.vertices=3*(full.mask.triangles.size()+full.ink.triangles.size());l.geometry.work=full.visited;
    EXPECT_EQ(full.ink.triangles.size(),prepareTiledHatch(b,t,1,0,FillRule::EvenOdd,{},l).ink.triangles.size());
}
TEST(CpuHatchTilerTest, AggregateInputStorageAndMutableTileStateCannotEvadeValidation) {
    auto t=tile();const auto b=rectangle(0,0,20,20);
    t.commands[0].state.rgba[3]=.5;EXPECT_EQ("invalid-hatch-tile",error([&]{prepareTiledHatch(b,t);}));
    t=tile();t.commands[0].state.transform.tx=1;EXPECT_EQ("invalid-hatch-tile",error([&]{prepareTiledHatch(b,t);}));
    t=tile();t.commands[0].operation=PaintOperation::Fill;EXPECT_EQ("invalid-hatch-tile",error([&]{prepareTiledHatch(b,t);}));
    t=tile();t.commands[0].path.resize(MaxFrameElements+1);EXPECT_EQ("vertex-limit",error([&]{prepareTiledHatch(b,t);}));
    t=tile();t.state.dash.resize(1025,1);EXPECT_EQ("state-limit",error([&]{prepareTiledHatch(b,t);}));
}
TEST(CpuHatchTilerTest, DegenerateNonfiniteAndUncertifiableDomainsRejectWithoutPartialGeometry) {
    auto t=tile();const auto b=rectangle(0,0,10,10);const double nan=std::numeric_limits<double>::quiet_NaN();
    for (double scale:{0.,-1.,nan}) EXPECT_EQ("invalid-hatch-scale",error([&]{prepareTiledHatch(b,t,scale);}));
    EXPECT_EQ("invalid-hatch-scale",error([&]{prepareTiledHatch(b,t,1,nan);}));
    t.maximum.x=t.minimum.x;EXPECT_EQ("invalid-hatch-tile",error([&]{prepareTiledHatch(b,t);}));
    t=tile();t.state.transform.origin.x=1e18;EXPECT_EQ("precision-limit",error([&]{prepareTiledHatch(b,t);}));
    t=tile();t.state.transform.xx=1e-20;EXPECT_FALSE(error([&]{prepareTiledHatch(b,t);}).empty());
    t=tile();t.commands[0].path.back().point.x=nan;EXPECT_FALSE(error([&]{prepareTiledHatch(b,t);}).empty());
}
TEST(CpuHatchTilerTest, EmptyMasksTilesAndZeroInkRemainValidatedBoundedNoOps) {
    const auto b=rectangle(0,0,10,10);HatchTile empty;EXPECT_TRUE(prepareTiledHatch(b,empty).ink.triangles.empty());
    empty.state.rgba[0]=2;EXPECT_EQ("invalid-color",error([&]{prepareTiledHatch(b,empty);}));
    PathCommand boundary;boundary.operation=PaintOperation::Fill;auto t=tile();const auto h=prepareTiledHatch(boundary,t);
    EXPECT_TRUE(h.mask.triangles.empty());EXPECT_TRUE(h.ink.triangles.empty());EXPECT_EQ(0u,h.cells);
    PathRecorder p;p.point({1,1},0);t.commands=p.commands();EXPECT_TRUE(prepareTiledHatch(b,t).ink.triangles.empty());
}
