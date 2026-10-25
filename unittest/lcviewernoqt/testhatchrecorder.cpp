#include <gtest/gtest.h>
#include "painters/pathgeometry.h"
#include "cad/primitive/line.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/circle.h"
#include "cad/primitive/ellipse.h"
#include "cad/primitive/point.h"
#include "cad/primitive/lwpolyline.h"
#include "cad/primitive/spline.h"
#include "cad/primitive/hatch.h"
#include "cad/objects/pattern.h"
#include <limits>
#include <type_traits>

namespace {
using namespace lc::viewer::render;
using Entity=lc::entity::CADEntity_CSPtr;
lc::objects::Pattern tile(std::vector<Entity> entities) {
    lc::objects::Pattern p;p.name="owned test";p.boundingBox=lc::geo::Area({-10,-10},{10,10});p.entities=std::move(entities);return p;
}
Entity line(lc::geo::Coordinate a={0,0},lc::geo::Coordinate b={2,1}) {return std::make_shared<lc::entity::Line>(a,b,nullptr);}
lc::entity::Spline_CSPtr spline(lc::geo::Spline::splineflag flags=lc::geo::Spline::PLANAR) {
    lc::builder::SplineBuilder b;b.setLayer(std::make_shared<lc::meta::Layer>("pattern"));b.setDegree(3);b.setClosed(false);b.setFlags(flags);
    b.setControlPoints({{0,0},{1,3},{3,3},{4,0}});return b.build();
}
template<class Call> std::string error(Call call) {
    try {call();} catch (const std::invalid_argument& e) {return e.what();}
    ADD_FAILURE()<<"Expected an atomic diagnostic";return {};
}
}
TEST(CpuHatchRecorderTest, RealProviderGeometryTypesKeepOrderAndOwnedModelCoordinates) {
    auto p=tile({line(),std::make_shared<lc::entity::Arc>(lc::geo::Coordinate{},3,0,1.5707963267948966,true,nullptr),
        std::make_shared<lc::entity::Point>(lc::geo::Coordinate{1,2},nullptr),
        std::make_shared<lc::entity::LWPolyline>(std::vector<lc::entity::LWVertex2D>{{{0,0},.5},{{2,0},0},{{2,2},0}},
            0,0,0,false,lc::geo::Coordinate{0,0,1},nullptr)});
    const auto recorded=recordHatchTile(p);ASSERT_EQ(4u,recorded.commands.size());
    EXPECT_EQ(PathVerb::Line,recorded.commands[0].path[1].verb);
    EXPECT_EQ(PathVerb::Conic,recorded.commands[1].path[1].verb);
    EXPECT_NEAR(3,recorded.commands[1].path.back().point.y,1e-12);
    EXPECT_EQ(PaintOperation::Point,recorded.commands[2].operation);EXPECT_DOUBLE_EQ(3,recorded.commands[2].pointDiameter);
    EXPECT_EQ(PointUnits::Physical,recorded.commands[2].pointUnits);
    EXPECT_EQ(PathVerb::Conic,recorded.commands[3].path[1].verb);EXPECT_EQ(PathVerb::Line,recorded.commands[3].path.back().verb);
    p.entities.clear();p.boundingBox=lc::geo::Area({100,100},{200,200});
    EXPECT_DOUBLE_EQ(-10,recorded.minimum.x);EXPECT_DOUBLE_EQ(10,recorded.maximum.y);
    EXPECT_DOUBLE_EQ(2,recorded.commands[0].path.back().point.x);EXPECT_DOUBLE_EQ(1,recorded.commands[2].path[0].point.x);
}
TEST(CpuHatchRecorderTest, OuterTransformAndInheritedStyleAreSnapshotsNotPrematureGeometryTransforms) {
    auto p=tile({line(),line()});PaintState state;state.transform.origin={1e12,-1e12};state.transform.xx=-2;state.transform.xy=.5;
    state.rgba={{.2,.4,.6,.5}};state.lineWidth=3;state.lineWidthCompensation=-1;state.dash={2,1};state.dashOffset=-.5;state.dashScaled=true;state.antialias=false;
    const auto result=recordHatchTile(p,state);state.dash.clear();state.rgba[0]=1;
    ASSERT_EQ(2u,result.commands.size());EXPECT_DOUBLE_EQ(1e12,result.state.transform.origin.x);EXPECT_DOUBLE_EQ(-2,result.state.transform.xx);
    for (const auto& c:result.commands) {
        EXPECT_DOUBLE_EQ(1,c.state.transform.xx);EXPECT_DOUBLE_EQ(0,c.state.transform.origin.x);
        EXPECT_DOUBLE_EQ(2,c.path.back().point.x);EXPECT_DOUBLE_EQ(1,c.path.back().point.y);
        EXPECT_DOUBLE_EQ(.2,c.state.rgba[0]);EXPECT_DOUBLE_EQ(.5,c.state.rgba[3]);EXPECT_DOUBLE_EQ(3,c.state.lineWidth);
        EXPECT_DOUBLE_EQ(-1,c.state.lineWidthCompensation);EXPECT_EQ((std::vector<double>{2,1}),c.state.dash);
        EXPECT_DOUBLE_EQ(-.5,c.state.dashOffset);EXPECT_TRUE(c.state.dashScaled);EXPECT_FALSE(c.state.antialias);
    }
}
TEST(CpuHatchRecorderTest, ExactCirclesEllipsesAndCurrentPolynomialSplineSpansRemainCurves) {
    auto s=spline();static_assert(std::is_same<decltype(s->bezierAt(0)),const lc::geo::BB_CSPtr&>::value,"borrowed const provider span");
    ASSERT_EQ(1u,s->bezierCount());EXPECT_THROW(s->bezierAt(1),std::out_of_range);
    auto p=tile({std::make_shared<lc::entity::Circle>(lc::geo::Coordinate{},2,nullptr),
        std::make_shared<lc::entity::Ellipse>(lc::geo::Coordinate{},lc::geo::Coordinate{3,0},2,0,0,false,nullptr),s});
    const auto r=recordHatchTile(p);ASSERT_EQ(3u,r.commands.size());
    for (unsigned i=0;i<2;++i) {
        unsigned conics=0;for (const auto& e:r.commands[i].path) if (e.verb==PathVerb::Conic) ++conics;
        EXPECT_EQ(4u,conics);EXPECT_DOUBLE_EQ(r.commands[i].path.front().point.x,r.commands[i].path.back().point.x);
        EXPECT_DOUBLE_EQ(r.commands[i].path.front().point.y,r.commands[i].path.back().point.y);
        EXPECT_EQ(PathVerb::Close,r.commands[i].path.back().verb);
    }
    EXPECT_EQ(PathVerb::Cubic,r.commands[2].path.back().verb);EXPECT_DOUBLE_EQ(4,r.commands[2].path.back().point.x);
    const auto near=std::make_shared<lc::entity::Ellipse>(lc::geo::Coordinate{},lc::geo::Coordinate{3,0},2,0,-std::numeric_limits<double>::epsilon(),false,nullptr);
    ASSERT_FALSE(near->isArc());const auto nearCommand=recordHatchTile(tile({near})).commands[0];
    EXPECT_EQ(6u,nearCommand.path.size());EXPECT_EQ(PathVerb::Close,nearCommand.path.back().verb);
}
TEST(CpuHatchRecorderTest, BothArcDirectionsUseModelOrientationWithoutLegacyYFlip) {
    for (bool ccw:{false,true}) {
        const auto p=tile({std::make_shared<lc::entity::Arc>(lc::geo::Coordinate{},3,0,1.5707963267948966,ccw,nullptr)});
        const auto command=recordHatchTile(p).commands[0];
        EXPECT_EQ(ccw?2u:4u,command.path.size());EXPECT_EQ(ccw?1:-1,command.path[1].control1.y>0?1:-1);
        EXPECT_NEAR(0,command.path.back().point.x,1e-12);EXPECT_NEAR(3,command.path.back().point.y,1e-12);
    }
}
TEST(CpuHatchRecorderTest, AggregateRecorderCapsFailAtomicallyAndRecover) {
    const auto p=tile({line(),line()});RecordingLimits limits;limits.elements=3;HatchTile out;
    EXPECT_EQ("vertex-limit",error([&]{out=recordHatchTile(p,{},limits);}));EXPECT_TRUE(out.commands.empty());
    limits={};limits.commands=1;EXPECT_EQ("command-limit",error([&]{out=recordHatchTile(p,{},limits);}));
    limits={};limits.elements=4;limits.dashElements=3;PaintState state;state.dash={1,1,1};
    EXPECT_EQ("state-limit",error([&]{out=recordHatchTile(p,state,limits);}));EXPECT_TRUE(out.commands.empty());
    limits.elements=6;EXPECT_EQ(2u,recordHatchTile(p,state,limits).commands.size());
    EXPECT_EQ(2u,recordHatchTile(p).commands.size());
}
TEST(CpuHatchRecorderTest, InvalidNullUnknownAndNonplanarEntitiesCannotDisappearSilently) {
    HatchTile out;
    EXPECT_EQ("invalid-hatch-entity",error([&]{out=recordHatchTile(tile({nullptr}));}));
    EXPECT_EQ("unsupported-hatch-entity",error([&]{out=recordHatchTile(tile({std::make_shared<lc::entity::Hatch>(lc::meta::Layer_CSPtr{})}));}));
    EXPECT_EQ("invalid-hatch-coordinate",error([&]{out=recordHatchTile(tile({line({0,0,1},{1,1,1})}));}));
    EXPECT_TRUE(out.commands.empty());EXPECT_EQ(1u,recordHatchTile(tile({line()})).commands.size());
}
TEST(CpuHatchRecorderTest, UnsupportedWidthsPlanesAndRationalProvidersAreExplicitDiagnostics) {
    auto poly=[](double width,lc::geo::Coordinate normal){return std::make_shared<lc::entity::LWPolyline>(
        std::vector<lc::entity::LWVertex2D>{{{0,0}},{{2,2}}},width,0,0,false,normal,nullptr);};
    EXPECT_EQ("unsupported-hatch-width",error([&]{recordHatchTile(tile({poly(2,{0,0,1})}));}));
    EXPECT_EQ("unsupported-hatch-plane",error([&]{recordHatchTile(tile({poly(0,{0,1,0})}));}));
    EXPECT_EQ("unsupported-rational-provider",error([&]{recordHatchTile(tile({spline(lc::geo::Spline::RATIONAL)}));}));
}
TEST(CpuHatchRecorderTest, EmptyTileAndDegeneratePeriodsValidateWithoutExpanding) {
    lc::objects::Pattern empty;EXPECT_TRUE(recordHatchTile(empty).commands.empty());
    auto p=tile({line()});p.boundingBox=lc::geo::Area({0,0},{3,0});HatchTile out;
    EXPECT_EQ("invalid-hatch-tile",error([&]{out=recordHatchTile(p);}));EXPECT_TRUE(out.commands.empty());
    PaintState state;state.rgba[0]=std::numeric_limits<double>::quiet_NaN();
    EXPECT_EQ("invalid-color",error([&]{recordHatchTile(empty,state);}));
}
