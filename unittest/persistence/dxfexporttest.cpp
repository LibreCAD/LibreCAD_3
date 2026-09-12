// dxfexporttest.cpp — issue #412 phase-0 exit criterion.
//
// Purpose: expose the writer's silent data loss.  DXFimpl reads 14 entity
// kinds but writes only 8; opening a supplier's DXF and saving overwrites
// dimensions, polylines, hatches, splines, points and images with nothing.
// LibreCAD's own DXF reader masks the defect (it recomputes dimension
// geometry from the block reference), so the file on disk MUST be inspected
// directly rather than round-tripped through DXFimpl.
//
// This test parses the produced DXF as raw ASCII group-code pairs and
// counts occurrences of each entity keyword between ENTITIES...ENDSEC.
// That deliberately bypasses libdxfrw so failures are attributable to the
// writer, not to a symmetric reader/writer bug hiding the defect.
//
// Expected failure surface before phase 1 lands: POINT, SPLINE, HATCH,
// LWPOLYLINE, IMAGE, and every DIMENSION variant are absent from the file.

#include <gtest/gtest.h>

#include <fstream>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <boost/filesystem.hpp>

#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>

#include <cad/meta/block.h>
#include <cad/meta/color.h>
#include <cad/meta/layer.h>
#include <cad/meta/metalinewidth.h>

#include <cad/operations/blockops.h>
#include <cad/operations/builder.h>
#include <cad/operations/entitybuilder.h>
#include <cad/operations/layerops.h>

#include <cad/primitive/arc.h>
#include <cad/primitive/circle.h>
#include <cad/primitive/dimaligned.h>
#include <cad/primitive/dimangular.h>
#include <cad/primitive/dimdiametric.h>
#include <cad/primitive/dimlinear.h>
#include <cad/primitive/dimradial.h>
#include <cad/primitive/ellipse.h>
#include <cad/primitive/hatch.h>
#include <cad/primitive/image.h>
#include <cad/primitive/insert.h>
#include <cad/primitive/line.h>
#include <cad/primitive/lwpolyline.h>
#include <cad/primitive/mtext.h>
#include <cad/primitive/point.h>
#include <cad/primitive/spline.h>
#include <cad/primitive/text.h>
#include <cad/primitive/textconst.h>

#include <cad/builders/insert.h>

#include "persistence/file.h"

namespace {

// ---- Minimal DXF group-code scanner.
//
// A DXF file is a sequence of (code, value) pairs, one per two lines.
// Between an ENTITIES section marker and its matching ENDSEC every "0/<name>"
// pair introduces an entity.  Counting those pairs per entity keyword tells
// us exactly what the writer emitted — no libdxfrw parse loop involved.
class EntityCounter {
public:
    explicit EntityCounter(const std::string& path) {
        std::ifstream in(path);
        // Cheap trim: DXF group-code lines are ASCII with trailing \r on
        // Windows outputs and often surrounded by leading spaces.
        std::string codeLine, valueLine;
        bool inEntities = false;
        while (std::getline(in, codeLine) && std::getline(in, valueLine)) {
            trim(codeLine);
            trim(valueLine);
            if (codeLine != "0") {
                continue;
            }
            if (valueLine == "SECTION") {
                // Peek: the very next pair should be (2, sectionName).
                std::string c, v;
                if (std::getline(in, c) && std::getline(in, v)) {
                    trim(c);
                    trim(v);
                    if (c == "2" && v == "ENTITIES") {
                        inEntities = true;
                    }
                }
                continue;
            }
            if (valueLine == "ENDSEC") {
                inEntities = false;
                continue;
            }
            if (inEntities) {
                ++_counts[valueLine];
            }
        }
    }

    int count(const std::string& keyword) const {
        auto it = _counts.find(keyword);
        return it == _counts.end() ? 0 : it->second;
    }

    std::string summary() const {
        std::ostringstream os;
        for (const auto& kv : _counts) {
            os << "  " << kv.first << " x " << kv.second << "\n";
        }
        return os.str();
    }

private:
    static void trim(std::string& s) {
        auto notSpace = [](unsigned char c) { return !std::isspace(c); };
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
        s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    }

    std::map<std::string, int> _counts;
};

std::string uniqueTmpDxf(const char* tag) {
    auto pid = std::to_string(::getpid());
    return (boost::filesystem::temp_directory_path()
            / (std::string("lc3-issue412-") + tag + "-" + pid + ".dxf")).string();
}

// Populate a document with one entity of every reader-producible kind.
// The exhaustive coverage is the point: any gap in writeEntity() surfaces
// as an EXPECT_GE failure below, naming the missing keyword.
void populateOneOfEach(const std::shared_ptr<lc::storage::DocumentImpl>& doc) {
    auto builder = std::make_shared<lc::operation::Builder>(doc, "populate");
    auto layer = std::make_shared<lc::meta::Layer>(
        "0", lc::meta::MetaLineWidthByValue(0.25),
        lc::Color(255, 255, 255, 255));

    auto eb = std::make_shared<lc::operation::EntityBuilder>(doc);

    // Straightforward primitives that already dispatch correctly (guards
    // against regression in the currently-working branches).
    eb->appendEntity(std::make_shared<lc::entity::Line>(
        lc::geo::Coordinate(0, 0), lc::geo::Coordinate(10, 0), layer));

    eb->appendEntity(std::make_shared<lc::entity::Circle>(
        lc::geo::Coordinate(20, 0), 5.0, layer));

    eb->appendEntity(std::make_shared<lc::entity::Arc>(
        lc::geo::Coordinate(40, 0), 5.0, 0.0, M_PI, /*ccw=*/true, layer));

    eb->appendEntity(std::make_shared<lc::entity::Ellipse>(
        lc::geo::Coordinate(60, 0), lc::geo::Coordinate(5, 0),
        0.5, 0.0, M_PI, /*reversed=*/false, layer));

    eb->appendEntity(std::make_shared<lc::entity::Text>(
        lc::geo::Coordinate(0, 10), "hello", /*height=*/2.5,
        /*angle=*/0.0, /*style=*/"STANDARD",
        lc::TextConst::DrawingDirection::None,
        lc::TextConst::HAlign::HALeft,
        lc::TextConst::VAlign::VABaseline, layer));

    eb->appendEntity(std::make_shared<lc::entity::MText>(
        lc::geo::Coordinate(0, 15), "multi\nline", /*height=*/2.5,
        /*angle=*/0.0, /*style=*/"STANDARD",
        lc::TextConst::DrawingDirection::None,
        lc::TextConst::HAlign::HALeft,
        lc::TextConst::VAlign::VABaseline,
        /*bold=*/false, /*italic=*/false,
        /*underline=*/false, /*strikethrough=*/false, layer));

    // Entities the writer currently drops.
    eb->appendEntity(std::make_shared<lc::entity::Point>(
        lc::geo::Coordinate(100, 0), layer));

    // Spline: minimum viable arg pack.  Values match a straight linear
    // spline; enough to exercise writeSpline() without asserting geometry.
    std::vector<lc::geo::Coordinate> controls{
        {0, 30}, {5, 35}, {10, 30}};
    std::vector<double> knots{0, 0, 0, 1, 1, 1};
    std::vector<lc::geo::Coordinate> fits;
    eb->appendEntity(std::make_shared<lc::entity::Spline>(
        controls, knots, fits, /*degree=*/2, /*closed=*/false,
        /*fitTolerance=*/0.0,
        /*stan*/0.0, 0.0, 0.0, /*etan*/0.0, 0.0, 0.0,
        /*n*/0.0, 0.0, 1.0,
        static_cast<lc::geo::Spline::splineflag>(8) /*PLANAR*/,
        layer));

    std::vector<lc::entity::LWVertex2D> vertexes{
        lc::entity::LWVertex2D(lc::geo::Coordinate(0, 40)),
        lc::entity::LWVertex2D(lc::geo::Coordinate(5, 42)),
        lc::entity::LWVertex2D(lc::geo::Coordinate(10, 40))};
    eb->appendEntity(std::make_shared<lc::entity::LWPolyline>(
        vertexes, /*width=*/0.0, /*elev=*/0.0, /*thick=*/0.0,
        /*closed=*/false, lc::geo::Coordinate(0, 0, 1), layer));

    eb->appendEntity(std::make_shared<lc::entity::Image>(
        "smoke.png",
        lc::geo::Coordinate(50, 40), lc::geo::Coordinate(1, 0),
        lc::geo::Coordinate(0, 1), /*w=*/10.0, /*h=*/10.0,
        /*bright=*/50.0, /*contrast=*/50.0, /*fade=*/0.0, layer));

    // Hatch needs at least one boundary loop, otherwise
    // lc::geo::Region::boundingBox() dereferences _loopList[0] and crashes.
    // That crash is tracked as a separate P0 in the issue-412 plan (its own
    // PR, not this branch) — give the Hatch a single closed LWPolyline loop
    // to sidestep it while still exercising writeHatch().
    {
        std::vector<lc::entity::LWVertex2D> loopVerts{
            lc::entity::LWVertex2D(lc::geo::Coordinate(30, 40)),
            lc::entity::LWVertex2D(lc::geo::Coordinate(35, 40)),
            lc::entity::LWVertex2D(lc::geo::Coordinate(35, 45)),
            lc::entity::LWVertex2D(lc::geo::Coordinate(30, 45))};
        auto boundary = std::make_shared<lc::entity::LWPolyline>(
            loopVerts, 0.0, 0.0, 0.0, /*closed=*/true,
            lc::geo::Coordinate(0, 0, 1), layer);
        auto hatch = std::make_shared<lc::entity::Hatch>(layer);
        lc::geo::Region reg;
        reg.addLoop(lc::geo::Loop({boundary}));
        hatch->setRegion(reg);
        hatch->setPatternName("SOLID");
        hatch->setSolid(1);
        eb->appendEntity(hatch);
    }

    // Dimensions — one of each subtype; the writer must dispatch to the
    // matching DRW_Dim* struct with the DXF-70 subtype flag set.
    eb->appendEntity(std::make_shared<lc::entity::DimLinear>(
        lc::geo::Coordinate(0, 60), lc::geo::Coordinate(5, 62),
        lc::TextConst::AttachmentPoint::Middle_center,
        /*textAngle=*/0.0, /*lsf=*/1.0,
        lc::TextConst::LineSpacingStyle::Exact, "",
        lc::geo::Coordinate(0, 60), lc::geo::Coordinate(10, 60),
        /*angle=*/0.0, /*oblique=*/0.0, layer));

    eb->appendEntity(std::make_shared<lc::entity::DimAligned>(
        lc::geo::Coordinate(0, 70), lc::geo::Coordinate(5, 72),
        lc::TextConst::AttachmentPoint::Middle_center,
        /*textAngle=*/0.0, /*lsf=*/1.0,
        lc::TextConst::LineSpacingStyle::Exact, "",
        lc::geo::Coordinate(0, 70), lc::geo::Coordinate(10, 70), layer));

    eb->appendEntity(std::make_shared<lc::entity::DimRadial>(
        lc::geo::Coordinate(20, 70), lc::geo::Coordinate(22, 72),
        lc::TextConst::AttachmentPoint::Middle_center,
        /*textAngle=*/0.0, /*lsf=*/1.0,
        lc::TextConst::LineSpacingStyle::Exact, "",
        lc::geo::Coordinate(25, 70), /*leader=*/1.0, layer));

    eb->appendEntity(std::make_shared<lc::entity::DimDiametric>(
        lc::geo::Coordinate(40, 70), lc::geo::Coordinate(42, 72),
        lc::TextConst::AttachmentPoint::Middle_center,
        /*textAngle=*/0.0, /*lsf=*/1.0,
        lc::TextConst::LineSpacingStyle::Exact, "",
        lc::geo::Coordinate(50, 70), /*leader=*/1.0, layer));

    eb->appendEntity(std::make_shared<lc::entity::DimAngular>(
        lc::geo::Coordinate(60, 70), lc::geo::Coordinate(62, 72),
        lc::TextConst::AttachmentPoint::Middle_center,
        /*textAngle=*/0.0, /*lsf=*/1.0,
        lc::TextConst::LineSpacingStyle::Exact, "",
        lc::geo::Coordinate(60, 70), lc::geo::Coordinate(70, 70),
        lc::geo::Coordinate(60, 70), lc::geo::Coordinate(60, 80), layer));

    // Insert requires a block; add an empty one.  Its presence in the
    // ENTITIES section verifies writeEntity() dispatches Insert correctly.
    auto block = std::make_shared<lc::meta::Block>(
        "smoke_block", lc::geo::Coordinate(0, 0));
    auto addBlock = std::make_shared<lc::operation::AddBlock>(doc, block);
    builder->append(addBlock);

    lc::builder::InsertBuilder ib;
    ib.setDocument(doc);
    ib.setLayer(layer);
    ib.setDisplayBlock(block);
    ib.setCoordinate(lc::geo::Coordinate(80, 0));
    eb->appendEntity(ib.build());

    builder->append(eb);
    builder->execute();
}

} // namespace

namespace {

// Issue #412 phase 2: helpers to grep specific DXF group codes.  A DXF entry
// looks like a sequence of alternating "code" / "value" lines; find every
// occurrence of a (code, value) pair inside an entity of a given keyword.
struct DxfPairsInEntities {
    // pairsByEntity["LINE"] = { {"10", "1.0"}, {"20", "2.0"}, {"30", "3.0"}, ... }
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> pairs;

    explicit DxfPairsInEntities(const std::string& path) {
        std::ifstream in(path);
        std::string codeLine, valueLine;
        bool inEntities = false;
        std::string current;
        auto trim = [](std::string& s) {
            auto notSpace = [](unsigned char c) { return !std::isspace(c); };
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
            s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
        };
        while (std::getline(in, codeLine) && std::getline(in, valueLine)) {
            trim(codeLine);
            trim(valueLine);
            if (codeLine == "0") {
                if (valueLine == "SECTION") {
                    std::string c, v;
                    if (std::getline(in, c) && std::getline(in, v)) {
                        trim(c); trim(v);
                        if (c == "2" && v == "ENTITIES") inEntities = true;
                    }
                    current.clear();
                    continue;
                }
                if (valueLine == "ENDSEC") { inEntities = false; current.clear(); continue; }
                current = inEntities ? valueLine : std::string();
                continue;
            }
            if (!current.empty()) {
                pairs[current].emplace_back(codeLine, valueLine);
            }
        }
    }

    // Return every value for a specific code inside the first N entities of
    // the given keyword.  Useful for asserting "code 30 exists and equals 7".
    std::vector<std::string> valuesFor(const std::string& keyword,
                                       const std::string& code) const {
        std::vector<std::string> out;
        auto it = pairs.find(keyword);
        if (it == pairs.end()) return out;
        for (const auto& p : it->second) {
            if (p.first == code) out.push_back(p.second);
        }
        return out;
    }
};

}  // namespace

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfExportTest, EveryReaderProducibleKindReachesTheFile) {
    const std::string dxfPath = uniqueTmpDxf("one-of-each");
    boost::filesystem::remove(dxfPath);

    auto sm = std::make_shared<lc::storage::StorageManagerImpl>();
    auto doc = std::make_shared<lc::storage::DocumentImpl>(sm);

    populateOneOfEach(doc);

    lc::persistence::File::save(
        doc, dxfPath, lc::persistence::File::Type::LIBDXFRW_DXF_R2000);

    ASSERT_TRUE(boost::filesystem::exists(dxfPath));
    ASSERT_GT(boost::filesystem::file_size(dxfPath), 0u);

    EntityCounter counted(dxfPath);
    const std::string dbg = "Entities the writer actually emitted:\n"
                            + counted.summary();

    // Currently-working branches — regression tripwire.
    EXPECT_GE(counted.count("LINE"),    1) << dbg;
    EXPECT_GE(counted.count("CIRCLE"),  1) << dbg;
    EXPECT_GE(counted.count("ARC"),     1) << dbg;
    EXPECT_GE(counted.count("ELLIPSE"), 1) << dbg;
    EXPECT_GE(counted.count("TEXT"),    1) << dbg;
    EXPECT_GE(counted.count("MTEXT"),   1) << dbg;
    EXPECT_GE(counted.count("INSERT"),  1) << dbg;

    // The eight kinds the issue reports as silently dropped.  Every one
    // of these is expected to FAIL at phase-0 completion; phase 1 makes
    // them pass.
    EXPECT_GE(counted.count("POINT"),     1) << "POINT dropped. " << dbg;
    EXPECT_GE(counted.count("SPLINE"),    1) << "SPLINE dropped. " << dbg;
    EXPECT_GE(counted.count("LWPOLYLINE"),1) << "LWPOLYLINE dropped. " << dbg;
    EXPECT_GE(counted.count("HATCH"),     1) << "HATCH dropped. " << dbg;
    EXPECT_GE(counted.count("IMAGE"),     1) << "IMAGE dropped. " << dbg;
    EXPECT_GE(counted.count("DIMENSION"), 5) << "DIMENSION dropped. " << dbg;

    boost::filesystem::remove(dxfPath);
}

// Issue #412 phase 2: correctness of writers that already ran.  Line, Circle,
// Arc, Ellipse and Text used to drop the Z coordinate of every point; Text
// never emitted its style; MText corrupted alignment on round-trip because
// codes 71/72/73 mean different things for MTEXT than for TEXT and the writer
// used the TEXT interpretation.  Regression tripwire for those fixes.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfExportTest, Phase2PreservesZAndTextAttributes) {
    const std::string dxfPath = uniqueTmpDxf("phase2");
    boost::filesystem::remove(dxfPath);

    auto sm = std::make_shared<lc::storage::StorageManagerImpl>();
    auto doc = std::make_shared<lc::storage::DocumentImpl>(sm);
    auto layer = std::make_shared<lc::meta::Layer>(
        "0", lc::meta::MetaLineWidthByValue(0.25),
        lc::Color(255, 255, 255, 255));

    auto eb = std::make_shared<lc::operation::EntityBuilder>(doc);

    // Z=7 on line start, Z=8 on line end.
    eb->appendEntity(std::make_shared<lc::entity::Line>(
        lc::geo::Coordinate(0, 0, 7), lc::geo::Coordinate(10, 0, 8), layer));

    // Circle Z=9.
    eb->appendEntity(std::make_shared<lc::entity::Circle>(
        lc::geo::Coordinate(20, 0, 9), 5.0, layer));

    // Arc Z=11.
    eb->appendEntity(std::make_shared<lc::entity::Arc>(
        lc::geo::Coordinate(40, 0, 11), 5.0, 0.0, M_PI, true, layer));

    // Text at Z=13, with custom style "MyStyle".
    eb->appendEntity(std::make_shared<lc::entity::Text>(
        lc::geo::Coordinate(0, 10, 13), "hello", 2.5, 0.0, "MyStyle",
        lc::TextConst::DrawingDirection::None,
        lc::TextConst::HAlign::HALeft,
        lc::TextConst::VAlign::VABaseline, layer));

    // MText at Z=15, HAlign=Center + VAlign=Middle => attachment 5 (Middle
    // center).  Textgen=Backward => drawing direction 1.
    eb->appendEntity(std::make_shared<lc::entity::MText>(
        lc::geo::Coordinate(0, 15, 15), "hi", 2.5, 0.0, "STANDARD",
        lc::TextConst::DrawingDirection::Backward,
        lc::TextConst::HAlign::HACenter,
        lc::TextConst::VAlign::VAMiddle,
        false, false, false, false, layer));

    eb->execute();

    lc::persistence::File::save(
        doc, dxfPath, lc::persistence::File::Type::LIBDXFRW_DXF_R2000);

    ASSERT_TRUE(boost::filesystem::exists(dxfPath));

    DxfPairsInEntities parsed(dxfPath);

    // Line — code 30 is basePoint.z, code 31 is secPoint.z.
    {
        auto z10 = parsed.valuesFor("LINE", "30");   // basePoint.z
        auto z11 = parsed.valuesFor("LINE", "31");   // secPoint.z
        ASSERT_FALSE(z10.empty()) << "LINE has no code 30 (start z)";
        ASSERT_FALSE(z11.empty()) << "LINE has no code 31 (end z)";
        EXPECT_DOUBLE_EQ(std::stod(z10.front()), 7.0);
        EXPECT_DOUBLE_EQ(std::stod(z11.front()), 8.0);
    }
    // Circle — code 30 is center.z.
    {
        auto z = parsed.valuesFor("CIRCLE", "30");
        ASSERT_FALSE(z.empty());
        EXPECT_DOUBLE_EQ(std::stod(z.front()), 9.0);
    }
    // Arc — code 30 is center.z.
    {
        auto z = parsed.valuesFor("ARC", "30");
        ASSERT_FALSE(z.empty());
        EXPECT_DOUBLE_EQ(std::stod(z.front()), 11.0);
    }
    // Text — code 30 is basePoint.z, code 7 is style name.
    {
        auto z = parsed.valuesFor("TEXT", "30");
        ASSERT_FALSE(z.empty());
        EXPECT_DOUBLE_EQ(std::stod(z.front()), 13.0);

        auto style = parsed.valuesFor("TEXT", "7");
        ASSERT_FALSE(style.empty()) << "TEXT missing style (code 7)";
        EXPECT_EQ(style.front(), "MyStyle")
            << "Text style was dropped before phase 2.";
    }
    // MText — code 30 is basePoint.z, code 71 is attachment point (5 for
    // middle-center), code 72 is drawing direction (1 for backward).
    {
        auto z = parsed.valuesFor("MTEXT", "30");
        ASSERT_FALSE(z.empty());
        EXPECT_DOUBLE_EQ(std::stod(z.front()), 15.0);

        auto attach = parsed.valuesFor("MTEXT", "71");
        ASSERT_FALSE(attach.empty()) << "MTEXT missing attachment point";
        EXPECT_EQ(std::stoi(attach.front()), 5)
            << "MTEXT attachment (Middle center) should be 5, was "
            << attach.front();

        auto dir = parsed.valuesFor("MTEXT", "72");
        ASSERT_FALSE(dir.empty());
        EXPECT_EQ(std::stoi(dir.front()), 1)
            << "MTEXT drawing direction (Backward) should be 1, was "
            << dir.front();
    }

    boost::filesystem::remove(dxfPath);
}
