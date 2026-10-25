#include <gtest/gtest.h>
#include "patternLoader/patternProvider.h"
#include "painters/pathgeometry.h"
#include "buildcontracts_config.h"
#include <boost/filesystem.hpp>
#include <algorithm>
#include <cctype>
#include <set>
#include <string>

TEST(CpuHatchCatalogTest, EveryBundledDxfPatternLoadsAndRecordsWithoutDroppingItsEntities) {
    namespace fs=boost::filesystem;
    using namespace lc::viewer::render;
    const auto directory=fs::path(build_contract::source)/"res/hatch";
    std::set<std::string> names;
    for (fs::directory_iterator i(directory),end;i!=end;++i) if (i->path().extension()==".dxf") {
        auto name=i->path().stem().string();std::transform(name.begin(),name.end(),name.begin(),[](unsigned char c){return char(std::tolower(c));});
        ASSERT_TRUE(names.insert(name).second);
    }
    ASSERT_EQ(63u,names.size());std::size_t entities=0,commands=0,points=0;
    auto provider=lc::persistence::PatternProvider::Instance();
    for (const auto& name:names) {
        SCOPED_TRACE(name);const auto& pattern=provider->getPattern(name);
        ASSERT_EQ(name,pattern.name);ASSERT_FALSE(pattern.entities.empty());
        HatchTile recorded;ASSERT_NO_THROW(recorded=recordHatchTile(pattern));
        EXPECT_EQ(pattern.entities.size(),recorded.commands.size());
        entities+=pattern.entities.size();commands+=recorded.commands.size();
        for (const auto& c:recorded.commands) if (c.operation==PaintOperation::Point) {
            ++points;EXPECT_DOUBLE_EQ(3,c.pointDiameter);EXPECT_EQ(PointUnits::Physical,c.pointUnits);
        }
    }
    // Independent ASCII ENTITIES inventory in the retained continuation packet:
    // 11588 LINE +40 ARC +one LWPOLYLINE +two POINT, not a generated renderer list.
    EXPECT_EQ(11631u,entities);EXPECT_EQ(entities,commands);EXPECT_EQ(2u,points);
    RecordProperty("patterns",int(names.size()));RecordProperty("entities",int(entities));RecordProperty("points",int(points));
}
TEST(CpuHatchCatalogTest, EveryBundledPatternPreparesActualFiniteTileInkWithinGlobalCaps) {
    namespace fs=boost::filesystem;using namespace lc::viewer::render;
    auto provider=lc::persistence::PatternProvider::Instance();std::size_t patterns=0,triangles=0;
    const auto directory=fs::path(build_contract::source)/"res/hatch";
    for (fs::directory_iterator i(directory),end;i!=end;++i) if (i->path().extension()==".dxf") {
        auto name=i->path().stem().string();std::transform(name.begin(),name.end(),name.begin(),[](unsigned char c){return char(std::tolower(c));});
        SCOPED_TRACE(name);const auto& pattern=provider->getPattern(name);auto captured=recordHatchTile(pattern);
        const double extent=std::max(captured.maximum.x-captured.minimum.x,captured.maximum.y-captured.minimum.y);
        ASSERT_GT(extent,0);PaintState state;state.transform.origin=captured.minimum;
        state.transform.xx=state.transform.yy=64/extent;state.transform.tx=state.transform.ty=32;
        captured=recordHatchTile(pattern,state);PathRecorder r;r.rectangle({32,32},64,64);r.fill();PreparedTiledHatch prepared;
        ASSERT_NO_THROW(prepared=prepareTiledHatch(r.commands()[0],captured));
        EXPECT_FALSE(prepared.mask.triangles.empty());EXPECT_FALSE(prepared.ink.triangles.empty());
        EXPECT_LE(prepared.positionBound,.25);EXPECT_LE(prepared.cells,65536u);EXPECT_LE(prepared.visited,4*MaxFrameElements);
        EXPECT_LE(3*(prepared.mask.triangles.size()+prepared.ink.triangles.size()),MaxFrameElements);
        triangles+=prepared.ink.triangles.size();++patterns;
    }
    EXPECT_EQ(63u,patterns);EXPECT_GT(triangles,0u);RecordProperty("prepared_patterns",int(patterns));
    RecordProperty("finite_ink_triangles",int(triangles)); // normalized CPU preparation, NOT pixel/hardware acceptance
}
