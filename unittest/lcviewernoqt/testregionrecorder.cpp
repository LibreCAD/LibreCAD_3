#include <gtest/gtest.h>
#include "painters/pathgeometry.h"
#include "cad/geometry/georegion.h"
#include "cad/primitive/circle.h"
#include "cad/primitive/point.h"
#include "cad/objects/pattern.h"
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <cmath>
#include <limits>

namespace {
using namespace lc::viewer::render;
using Entity=lc::entity::CADEntity_CSPtr;
using Coordinate=lc::geo::Coordinate;
Entity line(Coordinate a,Coordinate b) {return std::make_shared<lc::entity::Line>(a,b,nullptr);}
std::vector<Entity> box(double x=0,double y=0,double w=10,double h=10,bool reverse=false) {
    std::vector<Coordinate> p{{x,y},{x+w,y},{x+w,y+h},{x,y+h}};std::vector<Entity> out;
    for (unsigned i=0;i<4;++i) out.push_back(reverse?line(p[(4-i)%4],p[(3-i)%4]):line(p[i],p[(i+1)%4]));return out;
}
lc::geo::Region region(std::vector<Entity> edges=box()) {return lc::geo::Region(std::move(edges));}
double area(const PreparedFill& p) {
    double a=0;for (const auto& t:p.triangles) a+=((t[1].x-t[0].x)*(t[2].y-t[0].y)-(t[1].y-t[0].y)*(t[2].x-t[0].x))/2;return a;
}
int orient(Point a,Point b,Point c) {
    using Big=boost::multiprecision::number<boost::multiprecision::cpp_bin_float<100>>;
    const Big v=(Big(b.x)-Big(a.x))*(Big(c.y)-Big(a.y))-(Big(b.y)-Big(a.y))*(Big(c.x)-Big(a.x));return v>0?1:(v<0?-1:0);
}
template<class Mesh> bool covers(const Mesh& p,Point q) {
    for (const auto& t:p.triangles) if (orient(t[0],t[1],q)>=0 && orient(t[1],t[2],q)>=0 && orient(t[2],t[0],q)>=0) return true;return false;
}
template<class Call> std::string error(Call call) {
    try {call();} catch (const std::invalid_argument& e) {return e.what();}ADD_FAILURE()<<"Expected an atomic diagnostic";return {};
}
lc::entity::Spline_CSPtr spline(unsigned degree,std::vector<Coordinate> points) {
    lc::builder::SplineBuilder b;b.setLayer(std::make_shared<lc::meta::Layer>("region"));b.setDegree(degree);b.setClosed(false);
    b.setFlags(lc::geo::Spline::PLANAR);b.setControlPoints(points);return b.build();
}
HatchTile pattern(PaintState state={}) {
    lc::objects::Pattern p;p.boundingBox=lc::geo::Area({-5,-5},{5,5});p.entities={line({2,2},{8,2})};return recordHatchTile(p,state);
}
}
TEST(CpuRegionRecorderTest, ActualUnorderedAndReversedCadEdgesProduceOneOwnedContour) {
    auto edges=box();auto r=region({edges[0],line({0,10},{10,10}),edges[3],line({10,10},{10,0})});
    PaintState state;state.rgba={{.2,.4,.6,.5}};auto recorded=recordRegion(r,state);
    ASSERT_EQ(6u,recorded.boundary.path.size());EXPECT_EQ(PathVerb::Close,recorded.boundary.path.back().verb);
    EXPECT_DOUBLE_EQ(0,recorded.positionBound);EXPECT_EQ(PaintOperation::Fill,recorded.boundary.operation);
    r={};state.rgba[0]=1;EXPECT_DOUBLE_EQ(.2,recorded.boundary.state.rgba[0]);
    const auto filled=prepareFill(recorded.boundary,FillRule::EvenOdd,{},nullptr,recorded.positionBound);
    EXPECT_DOUBLE_EQ(100,area(filled));EXPECT_TRUE(covers(filled,{5,5}));EXPECT_FALSE(covers(filled,{11,5}));
}
TEST(CpuRegionRecorderTest, FirstEdgeWindingIsPreservedForNestedAndDisconnectedCadLoops) {
    auto same=region();same.addLoop(lc::geo::Loop(box(2,2,6,6)));same.addLoop(lc::geo::Loop(box(20,0,5,5)));
    EXPECT_DOUBLE_EQ(89,area(prepareRegionFill(same)));EXPECT_DOUBLE_EQ(125,area(prepareRegionFill(same,{},FillRule::NonZero)));
    auto opposite=region();opposite.addLoop(lc::geo::Loop(box(2,2,6,6,true)));opposite.addLoop(lc::geo::Loop(box(20,0,5,5)));
    const auto f=prepareRegionFill(opposite,{},FillRule::NonZero);EXPECT_DOUBLE_EQ(89,area(f));
    EXPECT_FALSE(covers(f,{5,5}));EXPECT_TRUE(covers(f,{22,2}));
}
TEST(CpuRegionRecorderTest, RealArcsAndFullEllipsesKeepCurvesAndExplicitClosedSeams) {
    constexpr double pi=3.14159265358979323846;
    auto r=region({std::make_shared<lc::entity::Arc>(Coordinate{},2,0,pi,true,nullptr),line({-2,0},{2,0})});
    const auto captured=recordRegion(r);unsigned conics=0;for (const auto& e:captured.boundary.path) conics+=e.verb==PathVerb::Conic;
    EXPECT_EQ(2u,conics);const auto half=prepareRegionFill(r);EXPECT_NEAR(2*pi,area(half),.4);
    EXPECT_TRUE(covers(half,{0,1}));EXPECT_FALSE(covers(half,{0,-1}));
    auto full=region({std::make_shared<lc::entity::Ellipse>(Coordinate{},Coordinate{4,0},2,0,0,false,nullptr)});
    const auto f=prepareRegionFill(full);EXPECT_NEAR(8*pi,area(f),1);EXPECT_TRUE(covers(f,{0,0}));EXPECT_FALSE(covers(f,{4,2}));
    EXPECT_EQ(PathVerb::Close,recordRegion(full).boundary.path.back().verb);
}
TEST(CpuRegionRecorderTest, BoundaryBulgesIgnoreStrokeDecorationWithoutEnablingPatternWidths) {
    auto p=std::make_shared<lc::entity::LWPolyline>(std::vector<lc::entity::LWVertex2D>{{{-2,0},1,2,3},{{2,0}},{{2,3}},{{-2,3}}},
        4,0,0,true,Coordinate{0,0,1},nullptr);
    auto r=region({p});const auto f=prepareRegionFill(r);EXPECT_NEAR(12+2*3.14159265358979323846,area(f),.5);
    EXPECT_TRUE(covers(f,{0,-1}));EXPECT_TRUE(covers(f,{0,2}));EXPECT_FALSE(covers(f,{3,0}));
    lc::objects::Pattern tile;tile.boundingBox=lc::geo::Area({-2,-2},{2,3});tile.entities={p};
    EXPECT_EQ("unsupported-hatch-width",error([&]{recordHatchTile(tile);}));
}
TEST(CpuRegionRecorderTest, EllipseAxesValidateBeforeAnyRegionBoundaryOrPreparedInkEscapes) {
    const double nan=std::numeric_limits<double>::quiet_NaN(),inf=std::numeric_limits<double>::infinity();
    const double tiny=std::numeric_limits<double>::denorm_min();
    auto ellipse=[](Coordinate axis,bool partial,bool reverse) {
        return std::make_shared<lc::entity::Ellipse>(Coordinate{},axis,2,0,partial?1.5707963267948966:0,reverse,nullptr);
    };
    const auto good=recordRegion(region());const auto ink=pattern();
    for (auto axis:std::vector<Coordinate>{{3,0,4},{3,0,-4},{3,0,tiny},{3,0,-tiny},
             {nan,0,0},{3,nan,0},{3,0,nan},{inf,0,0},{3,-inf,0},{3,0,inf},{3,0,-inf}}) {
        for (bool partial:{false,true}) for (bool reverse:{false,true}) {
            SCOPED_TRACE(::testing::Message()<<axis.x()<<","<<axis.y()<<","<<axis.z()<<" "<<partial<<" "<<reverse);
            auto malformed=region();malformed.addLoop(lc::geo::Loop({ellipse({3,4},false,false)}));
            // Inject malformed provider data without making the CAD bounding-box
            // constructor responsible for validating it before the adapter runs.
            auto& entities=const_cast<std::vector<Entity>&>(malformed.loopList()[1].entities());
            entities[0]=ellipse(axis,partial,reverse);auto saved=good;
            EXPECT_EQ("invalid-hatch-coordinate",error([&]{saved=recordRegion(malformed);}));
            EXPECT_EQ(good.boundary.path.size(),saved.boundary.path.size());
            EXPECT_EQ("invalid-hatch-coordinate",error([&]{prepareRegionFill(malformed);}));
            EXPECT_EQ("invalid-hatch-coordinate",error([&]{prepareRegionHatch(malformed,ink);}));
        }
    }
    PaintState state;state.transform={-1,.25,0,2,7,11,{}};
    for (double z:{0.,-0.}) for (bool partial:{false,true}) for (bool reverse:{false,true}) {
        std::vector<Entity> edges{ellipse({3,4,z},partial,reverse)};
        if (partial) edges.push_back(line({-1.6,1.2},{3,4}));
        const auto out=recordRegion(region(edges),state);ASSERT_FALSE(out.boundary.path.empty());
        EXPECT_NEAR(5,out.boundary.path.front().point.x,1e-12);EXPECT_NEAR(19,out.boundary.path.front().point.y,1e-12);
        EXPECT_EQ(PathVerb::Close,out.boundary.path.back().verb);
        if (!partial) {
            const auto fill=prepareRegionFill(region(edges),state);EXPECT_NEAR(20*3.14159265358979323846,area(fill),2);
            EXPECT_TRUE(covers(fill,{7,11}));EXPECT_FALSE(covers(fill,{50,50}));
        }
    }
    EXPECT_DOUBLE_EQ(100,area(prepareRegionFill(region())));
}
TEST(CpuRegionRecorderTest, ReversedQuadraticAndCubicSplineEdgesSwapControlsNotTheirShapes) {
    for (unsigned degree:{2u,3u}) {
        auto s=degree==2?spline(2,{{0,0},{3,4},{6,0}}):spline(3,{{0,0},{2,3},{4,3},{6,0}});
        auto r=region({line({0,0},{6,0}),s});const auto c=recordRegion(r);
        ASSERT_EQ(4u,c.boundary.path.size());const auto& curve=c.boundary.path[2];
        EXPECT_EQ(degree==2?PathVerb::Quadratic:PathVerb::Cubic,curve.verb);EXPECT_DOUBLE_EQ(0,curve.point.x);
        EXPECT_DOUBLE_EQ(degree==2?3:4,curve.control1.x);if (degree==3) EXPECT_DOUBLE_EQ(2,curve.control2.x);
        const auto f=prepareRegionFill(r);EXPECT_NEAR(degree==2?8:9,area(f),.7);EXPECT_TRUE(covers(f,{3,1}));
    }
}
TEST(CpuRegionRecorderTest, MultipleCurrentSplineProviderSpansAreStitchedWithoutConnectorLines) {
    auto s=spline(3,{{0,0},{1,2},{2,3},{3,4},{4,3},{5,2},{6,0}});ASSERT_GT(s->bezierCount(),1u);
    const auto a=s->bezierAt(0)->getCP().front(),b=s->bezierAt(s->bezierCount()-1)->getCP().back();
    const auto captured=recordRegion(region({s,line(b,a)}));unsigned moves=0,cubics=0;
    for (const auto& e:captured.boundary.path) {moves+=e.verb==PathVerb::Move;cubics+=e.verb==PathVerb::Cubic;}
    EXPECT_EQ(1u,moves);EXPECT_EQ(s->bezierCount(),cubics);EXPECT_EQ(PathVerb::Close,captured.boundary.path.back().verb);
    EXPECT_FALSE(prepareFill(captured.boundary,FillRule::EvenOdd,{},nullptr,captured.positionBound).triangles.empty());
}
TEST(CpuRegionRecorderTest, RoundoffOnlyJoinsAreCertifiedAndPassedIntoFillAndHatchPreparation) {
    auto r=region({line({0,0},{10,0}),line({10+1e-13,0},{10,10}),line({10,10},{0,0})});const auto input=recordRegion(r);
    EXPECT_GT(input.positionBound,0);EXPECT_LE(input.positionBound,1.0/64);
    const auto raw=prepareFill(input.boundary);const auto f=prepareRegionFill(r);
    EXPECT_NEAR(50,area(f),1e-10);EXPECT_GE(f.positionBound,raw.positionBound+input.positionBound);
    const auto rawHatch=prepareTiledHatch(input.boundary,pattern());const auto h=prepareRegionHatch(r,pattern());
    EXPECT_GE(h.mask.positionBound,rawHatch.mask.positionBound+input.positionBound);EXPECT_LE(h.positionBound,.25);
    RegionLimits exact;exact.joinError=0;EXPECT_EQ("disconnected-region-loop",error([&]{recordRegion(r,{},exact);}));
}
TEST(CpuRegionRecorderTest, SourceBoundaryErrorParticipatesInTopologyClearanceNotJustOutputMetadata) {
    PathRecorder r;r.rectangle({0,0},10,10);r.rectangle({10.001,0},10,10);r.fill();const auto c=r.commands()[0];
    EXPECT_FALSE(prepareFill(c).triangles.empty());EXPECT_EQ("precision-limit",error([&]{prepareFill(c,FillRule::EvenOdd,{},nullptr,.01);}));
    for (double bound:{-.01,.02,std::numeric_limits<double>::quiet_NaN()}) {
        EXPECT_EQ("invalid-boundary-error",error([&]{prepareFill(c,FillRule::EvenOdd,{},nullptr,bound);}));
        EXPECT_EQ("invalid-boundary-error",error([&]{prepareTiledHatch(c,pattern(),1,0,FillRule::EvenOdd,{}, {},bound);}));
    }
}
TEST(CpuRegionRecorderTest, RealGapsBranchesAndMultipleCyclesInOneLoopNeverCreateInventedBridges) {
    auto gap=region({line({0,0},{10,0}),line({10+1e-7,0},{10,10}),line({10,10},{0,0})});
    EXPECT_EQ("disconnected-region-loop",error([&]{recordRegion(gap);}));
    auto edges=box();edges.push_back(line({10,0},{20,0}));EXPECT_EQ("ambiguous-region-loop",error([&]{recordRegion(region(edges));}));
    edges=box();const auto second=box(20,0);edges.insert(edges.end(),second.begin(),second.end());
    EXPECT_EQ("disconnected-region-loop",error([&]{recordRegion(region(edges));}));
}
TEST(CpuRegionRecorderTest, CrossingAndTouchingCadLoopsReachTheExistingFailClosedTopologyChecks) {
    auto cross=region({line({0,0},{10,10}),line({10,10},{0,10}),line({0,10},{10,0}),line({10,0},{0,0})});
    EXPECT_NO_THROW(recordRegion(cross));EXPECT_EQ("unsupported-fill-topology",error([&]{prepareRegionFill(cross);}));
    auto touch=region();touch.addLoop(lc::geo::Loop(box(10,0)));EXPECT_EQ("unsupported-fill-topology",error([&]{prepareRegionFill(touch);}));
}
TEST(CpuRegionRecorderTest, LargeOriginsAffineReflectionsAndPixelScaleApplyOnceBeforePreparation) {
    auto r=region(box(1e12,-1e12,4,4));PaintState state;state.transform={-2,.5,.25,3,10,20,{1e12,-1e12}};
    const auto input=recordRegion(r,state);EXPECT_DOUBLE_EQ(10,input.boundary.path[0].point.x);EXPECT_DOUBLE_EQ(20,input.boundary.path[0].point.y);
    const auto f=prepareRegionFill(r,state,FillRule::NonZero);EXPECT_DOUBLE_EQ(98,area(f));
    EXPECT_TRUE(covers(f,{7,26.5}));EXPECT_FALSE(covers(f,{33,33}));EXPECT_LE(f.positionBound,1.0/8);
}
TEST(CpuRegionRecorderTest, EndToEndCadRegionAndFinitePatternKeepHoleMaskDashPhaseAndInheritedPaint) {
    auto r=region(box(0,0,30,10));r.addLoop(lc::geo::Loop(box(10,1,10,8)));
    PaintState state;state.transform.xx=state.transform.yy=2;state.dash={2,2};state.rgba={{.2,.3,.4,.5}};
    const auto h=prepareRegionHatch(r,pattern(state));EXPECT_TRUE(covers(h.mask,{6,4}));EXPECT_TRUE(covers(h.ink,{6,4}));
    EXPECT_FALSE(covers(h.ink,{10,4}));EXPECT_TRUE(covers(h.ink,{26,4}));EXPECT_FALSE(covers(h.mask,{26,4}));
    EXPECT_EQ(state.rgba,h.ink.state.rgba);EXPECT_EQ(StrokeCoverage::UnionBeforeOver,h.ink.coverage);EXPECT_LE(h.positionBound,.25);
}
TEST(CpuRegionRecorderTest, GlobalRecordingAndPreparationWorkCapsFailAtomicallyAndRecover) {
    auto r=region();const auto input=recordRegion(r);RecordedRegion saved=input;RegionLimits l;l.work=input.visited-1;
    EXPECT_EQ("region-work-limit",error([&]{saved=recordRegion(r,{},l);}));EXPECT_EQ(input.boundary.path.size(),saved.boundary.path.size());
    l={};l.work=input.visited;EXPECT_EQ(input.visited,recordRegion(r,{},l).visited);
    l={};l.recording.elements=7;EXPECT_EQ("vertex-limit",error([&]{recordRegion(r,{},l);}));
    l={};l.recording.commands=0;EXPECT_EQ("command-limit",error([&]{recordRegion(r,{},l);}));
    const auto f=prepareRegionFill(r);FillLimits fill;fill.work=f.visited;EXPECT_DOUBLE_EQ(100,area(prepareRegionFill(r,{},FillRule::EvenOdd,fill)));
    --fill.work;EXPECT_FALSE(error([&]{prepareRegionFill(r,{},FillRule::EvenOdd,fill);}).empty());
    const auto h=prepareRegionHatch(r,pattern());TiledHatchLimits hatch;hatch.geometry.work=h.visited;
    EXPECT_EQ(h.visited,prepareRegionHatch(r,pattern(),1,0,FillRule::EvenOdd,{},hatch).visited);--hatch.geometry.work;
    EXPECT_FALSE(error([&]{prepareRegionHatch(r,pattern(),1,0,FillRule::EvenOdd,{},hatch);}).empty());
}
TEST(CpuRegionRecorderTest, InvalidNullUnsupportedNonplanarAndDegenerateInputsDoNotDisappear) {
    auto r=region();auto& entities=const_cast<std::vector<Entity>&>(r.loopList()[0].entities());entities[0]=nullptr;
    EXPECT_EQ("invalid-hatch-entity",error([&]{recordRegion(r);}));entities.clear();EXPECT_EQ("invalid-region-loop",error([&]{recordRegion(r);}));
    EXPECT_EQ("unsupported-region-entity",error([&]{recordRegion(region({std::make_shared<lc::entity::Point>(Coordinate{},nullptr)}));}));
    EXPECT_EQ("invalid-hatch-coordinate",error([&]{recordRegion(region({line({0,0,1},{1,1,1})}));}));
    EXPECT_EQ("invalid-region-loop",error([&]{recordRegion(region({line({0,0},{0,0})}));}));
    RegionLimits l;l.joinError=.02;EXPECT_EQ("invalid-limits",error([&]{recordRegion(region(),{},l);}));
}
TEST(CpuRegionRecorderTest, EmptyRegionsValidatePaintAndGradientSnapshotsStayOwned) {
    lc::geo::Region empty;EXPECT_TRUE(recordRegion(empty).boundary.path.empty());EXPECT_TRUE(prepareRegionFill(empty).triangles.empty());
    EXPECT_TRUE(prepareRegionHatch(empty,pattern()).ink.triangles.empty());PaintState bad;bad.rgba[0]=2;
    EXPECT_EQ("invalid-color",error([&]{recordRegion(empty,bad);}));
    LinearGradient g;g.end={10,0};g.stops={{0,{{1,0,0,1}}},{1,{{0,0,1,1}}}};
    const auto f=prepareRegionFill(region(),{},FillRule::EvenOdd,{},&g);g.stops.clear();
    EXPECT_TRUE(f.hasGradient);EXPECT_EQ((std::array<double,4>{{.5,0,.5,1}}),sampleGradient(f.gradient,{5,5}));
}
