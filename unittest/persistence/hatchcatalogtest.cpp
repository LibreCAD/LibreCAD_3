#include <gtest/gtest.h>
#include "patternLoader/patternProvider.h"
#include "painters/pathgeometry.h"
#include "buildcontracts_config.h"
#include "persistence/file.h"
#include "cad/storage/documentimpl.h"
#include "cad/storage/storagemanagerimpl.h"
#include "cad/primitive/hatch.h"
#include <boost/filesystem.hpp>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <set>
#include <string>
#include <fstream>

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
TEST(CpuHatchCatalogTest, ClayKeepsItsOriginBasedCellDespiteTheEmptyInkMargin) {
    using namespace lc::viewer::render;
    const auto& pattern=lc::persistence::PatternProvider::Instance()->getPattern("clay");
    ASSERT_EQ("clay",pattern.name);const auto tile=recordHatchTile(pattern);
    EXPECT_NEAR(6.667,tile.minimum.y,1e-12);EXPECT_DOUBLE_EQ(100,tile.maximum.y);
    EXPECT_DOUBLE_EQ(100,tile.period.x);EXPECT_DOUBLE_EQ(100,tile.period.y);
    auto cross=[](Point a,Point b,Point p){return (b.x-a.x)*(p.y-a.y)-(b.y-a.y)*(p.x-a.x);};
    auto inside=[&](const PreparedStroke& ink,Point p) {
        for (const auto& t:ink.triangles) if (cross(t[0],t[1],p)>=0 && cross(t[1],t[2],p)>=0 && cross(t[2],t[0],p)>=0) return true;
        return false;
    };
    // This full horizontal source segment is independent evidence for its
    // 100-unit repeat; deriving the expected phase from tile.period would hide
    // the original tight-bounds defect.
    bool fullLine=false;
    for (const auto& c:tile.commands) if (c.path.size()==2) {
        const auto a=c.path[0].point,b=c.path[1].point;
        fullLine|=a.x==0 && a.y==20 && b.x==100 && b.y==20;
    }
    ASSERT_TRUE(fullLine);
    for (double scale:{.5,1.,2.}) for (bool reflected:{false,true}) for (double angle:{0.,.37}) {
        auto placed=tile;placed.state.transform.xx=reflected?-1:1;
        PathRecorder r;r.rectangle({reflected?-400.:0,0},400,400);r.fill();
        const auto prepared=prepareTiledHatch(r.commands()[0],placed,scale,angle);
        auto mapped=[&](Point p) {return placed.state.transform.point({scale*(std::cos(angle)*p.x-std::sin(angle)*p.y),
            scale*(std::sin(angle)*p.x+std::cos(angle)*p.y)});};
        EXPECT_TRUE(inside(prepared.ink,mapped({90,120})));
        EXPECT_FALSE(inside(prepared.ink,mapped({90,126.666})));EXPECT_LE(prepared.positionBound,.25);
    }
}
TEST(CpuHatchCatalogTest, RawDxfHatchImportsRealRegionLoopsAndPreparesItsHoleWithoutWriterSymmetry) {
    namespace fs=boost::filesystem;using namespace lc::viewer::render;
    const auto path=(fs::temp_directory_path()/fs::unique_path("lc3-region-%%%%%%%%.dxf")).string();
    struct Cleanup {std::string path;~Cleanup(){boost::system::error_code ec;fs::remove(path,ec);}} cleanup{path};
    // Independently authored raw group-code pairs: NOT a DXFimpl writer/reader
    // round trip that could conceal a symmetric boundary-import defect. R2000
    // table records carry their handle and subclass markers; libdxfrw rejects
    // the whole file without them.
    const char* dxf="0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1015\n0\nENDSEC\n"
        "0\nSECTION\n2\nTABLES\n0\nTABLE\n2\nLAYER\n5\n2\n100\nAcDbSymbolTable\n70\n1\n"
        "0\nLAYER\n5\n10\n100\nAcDbSymbolTableRecord\n100\nAcDbLayerTableRecord\n2\n0\n70\n0\n62\n7\n6\nCONTINUOUS\n0\nENDTAB\n0\nENDSEC\n"
        "0\nSECTION\n2\nENTITIES\n0\nHATCH\n100\nAcDbEntity\n8\n0\n100\nAcDbHatch\n"
        "10\n0\n20\n0\n30\n0\n210\n0\n220\n0\n230\n1\n2\nSOLID\n70\n1\n71\n0\n91\n2\n"
        "92\n2\n72\n0\n73\n1\n93\n4\n10\n0\n20\n0\n10\n20\n20\n0\n10\n20\n20\n20\n10\n0\n20\n20\n97\n0\n"
        "92\n2\n72\n0\n73\n1\n93\n4\n10\n5\n20\n5\n10\n5\n20\n15\n10\n15\n20\n15\n10\n15\n20\n5\n97\n0\n"
        "75\n0\n76\n1\n98\n0\n0\nENDSEC\n0\nEOF\n";
    {std::ofstream stream(path);ASSERT_TRUE(stream.good());stream<<dxf;ASSERT_TRUE(stream.good());}
    auto storage=std::make_shared<lc::storage::StorageManagerImpl>();auto document=std::make_shared<lc::storage::DocumentImpl>(storage);
    ASSERT_NO_THROW(lc::persistence::File::open(document,path,lc::persistence::File::LIBDXFRW));
    const auto entities=document->entitiesByBlock(nullptr).asVector();ASSERT_EQ(1u,entities.size());
    const auto hatch=std::dynamic_pointer_cast<const lc::entity::Hatch>(entities[0]);ASSERT_TRUE(hatch);ASSERT_TRUE(hatch->isSolid());
    const auto& region=hatch->getRegion();ASSERT_EQ(2u,region.loopList().size());
    const auto captured=recordRegion(region);EXPECT_EQ(12u,captured.boundary.path.size());EXPECT_DOUBLE_EQ(0,captured.positionBound);
    const auto filled=prepareRegionFill(region,{},FillRule::NonZero);double area=0;
    auto cross=[](Point a,Point b,Point p){return (b.x-a.x)*(p.y-a.y)-(b.y-a.y)*(p.x-a.x);};
    auto inside=[&](Point p){for (const auto& t:filled.triangles) if (cross(t[0],t[1],p)>=0 && cross(t[1],t[2],p)>=0 && cross(t[2],t[0],p)>=0) return true;return false;};
    for (const auto& t:filled.triangles) area+=cross(t[0],t[1],t[2])/2;
    EXPECT_DOUBLE_EQ(300,area);EXPECT_TRUE(inside({2,2}));EXPECT_FALSE(inside({10,10}));EXPECT_FALSE(inside({25,2}));
    RecordProperty("imported_region_loops",2);RecordProperty("independent_region_area",300);
}
