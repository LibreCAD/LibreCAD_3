// dxfroundtriptest.cpp — the `lc-test-vehicle` slice of the libdxfrw
// integration plan.
//
// Purpose: guard the three kernel crashes that made `lc::persistence::File`
// unusable on real DXF input, and the save-result plumbing that made every
// write failure invisible.  Before the fixes these were not partial data loss
// — they were `std::terminate` / `EXC_BAD_ACCESS` inside `File::open`, so 25
// of the 43 files in the review corpus could not be opened at all.
//
//   * `geo::Area`'s two-coordinate constructor threw a bare `const char*`
//     ("Points describe a volume, not a area.") whenever the two corners
//     differed in x AND y AND z.  `lc::entity::Line::boundingBox`
//     (lckernel/cad/primitive/line.cpp:149) hands it a segment's endpoints
//     unguarded, and `LWPolyline::boundingBox` reaches it through the Line
//     entities the polyline generates — so any 3-D segment aborted the
//     program the moment `DocumentImpl::insertEntity` put it in the quad
//     tree.  Because the throw was a `const char*`, `catch (const
//     std::exception&)` never saw it.                       → 4c7e438b
//   * `geo::Loop`'s constructor and `Region::boundingBox` indexed element 0
//     of containers a DXF hatch may legally leave empty (91=1 93=0).
//   * `Spline::calculateBoundingBox` indexed `controlPoints()[0]` on a
//     fit-points-only SPLINE (72=0 73=0 74=n), which is legal DXF and which
//     the pinned libdxfrw accepts.                          → 983ee034
//   * `File::save` returned void and discarded `DXFimpl::writeDXF`'s result;
//     an unsupported type was a silent no-op.               → 7b5cbc7b
//
// Discipline, identical to dxfexporttest.cpp: this file includes **no**
// libdxfrw header.  `persistence` links libdxfrw PRIVATE precisely so that
// its C++17 requirement does not propagate; linking it into `lcunittest`
// would raise the whole binary — including the kaguya scripting tests, which
// are gnu++14 and break on kaguya's missing FunctionSignature specialisation
// for noexcept members.  Everything here goes through the public
// `lc::persistence::File` surface and the kernel.

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include <cad/const.h>

#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>

#include <cad/meta/color.h>
#include <cad/meta/layer.h>
#include <cad/meta/metalinewidth.h>
#include <cad/meta/customentitystorage.h>

#include <cad/operations/builder.h>
#include <cad/operations/blockops.h>
#include <cad/operations/entitybuilder.h>

#include <cad/geometry/geoarea.h>
#include <cad/geometry/geocoordinate.h>
#include <cad/geometry/georegion.h>

#include <cad/primitive/hatch.h>
#include <cad/primitive/line.h>
#include <cad/primitive/insert.h>
#include <cad/primitive/mtext.h>
#include <cad/primitive/lwpolyline.h>
#include <cad/primitive/spline.h>

#include "persistence/file.h"
#include "persistence/format.h"
#include "persistence/readguard.h"

namespace {

std::string uniqueTmpDxf(const char* tag) {
    return (boost::filesystem::temp_directory_path()
            / (std::string("lc3-roundtrip-") + tag + "-"
               + std::to_string(::getpid()) + ".dxf")).string();
}

std::shared_ptr<lc::storage::DocumentImpl> newDocument() {
    return std::make_shared<lc::storage::DocumentImpl>(
        std::make_shared<lc::storage::StorageManagerImpl>());
}

lc::meta::Layer_CSPtr defaultLayer() {
    return std::make_shared<lc::meta::Layer>(
        "0", lc::meta::MetaLineWidthByValue(0.25),
        lc::Color(255, 255, 255, 255));
}

// Insert through exactly the path the DXF reader uses.  `DXFimpl` holds an
// `EntityBuilder` appended to the `operation::Builder` that `File::open`
// creates (persistence/libdxfrw/dxfimpl.cpp:72-75), and `File::open` ends
// with `builder->execute()` (persistence/file.cpp:98).  Executing the builder
// is what reaches `DocumentImpl::insertEntity` → `StorageManager` →
// `QuadTree::insert` → `CADEntity::boundingBox()`, which is where all three
// crashes lived.  Calling `boundingBox()` directly would not prove the open
// path is safe.
void insertThroughBuilder(
    const std::shared_ptr<lc::storage::DocumentImpl>& doc,
    const std::vector<lc::entity::CADEntity_CSPtr>& entities) {
    auto builder = std::make_shared<lc::operation::Builder>(doc, "insert");
    auto entityBuilder = std::make_shared<lc::operation::EntityBuilder>(doc);
    for (const auto& entity : entities) {
        entityBuilder->appendEntity(entity);
    }
    builder->append(entityBuilder);
    builder->execute();
}

// "No throw" alone is not a guard: an entity that is silently dropped also
// fails to throw.  Every case below asserts the entity reached the document's
// container as well.
bool documentHolds(const std::shared_ptr<lc::storage::DocumentImpl>& doc,
                   const lc::entity::CADEntity_CSPtr& entity) {
    return doc->entityContainer().entityByID(entity->id()) != nullptr;
}

/** The document's entities by kernel kind. */
std::map<std::string, std::size_t> entitiesInDocument(
    const std::shared_ptr<lc::storage::DocumentImpl>& doc) {
    std::map<std::string, std::size_t> census;

    for (const auto& entity : doc->entityContainer().asVector()) {
        if (std::dynamic_pointer_cast<const lc::entity::Line>(entity)) {
            census["Line"]++;
        } else if (std::dynamic_pointer_cast<const lc::entity::Circle>(entity)) {
            census["Circle"]++;
        } else if (std::dynamic_pointer_cast<const lc::entity::Arc>(entity)) {
            census["Arc"]++;
        } else if (std::dynamic_pointer_cast<const lc::entity::Ellipse>(entity)) {
            census["Ellipse"]++;
        } else if (std::dynamic_pointer_cast<const lc::entity::Text>(entity)) {
            census["Text"]++;
        } else if (std::dynamic_pointer_cast<const lc::entity::MText>(entity)) {
            census["MText"]++;
        } else if (std::dynamic_pointer_cast<const lc::entity::Point>(entity)) {
            census["Point"]++;
        } else if (std::dynamic_pointer_cast<const lc::entity::LWPolyline>(entity)) {
            census["LWPolyline"]++;
        } else if (std::dynamic_pointer_cast<const lc::entity::Spline>(entity)) {
            census["Spline"]++;
        } else if (std::dynamic_pointer_cast<const lc::entity::Hatch>(entity)) {
            census["Hatch"]++;
        } else if (std::dynamic_pointer_cast<const lc::entity::Insert>(entity)) {
            census["Insert"]++;
        } else {
            census["(unclassified)"]++;
        }
    }

    return census;
}


}  // namespace

// The filed task: "LibreCAD 3 crashes opening DXFs with 3D segments".
//
// A LINE whose endpoints differ in x AND y AND z, and an LWPOLYLINE
// containing such a segment, must both survive insertion, and the resulting
// bounding box must be the axis-aligned box of the endpoints — including z,
// which is the behaviour commit 4c7e438b chose when it deleted the throw
// rather than projecting the pair onto a plane.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, ThreeDimensionalSegmentInsertsWithoutThrowing) {
    auto doc = newDocument();
    auto layer = defaultLayer();

    // Every axis differs — the exact predicate the deleted throw tested.
    const lc::geo::Coordinate start(1.0, 2.0, 3.0);
    const lc::geo::Coordinate end(7.0, 11.0, 13.0);
    auto line = std::make_shared<lc::entity::Line>(start, end, layer);

    // A polyline whose consecutive vertices differ on every axis reaches the
    // same constructor through the Line entities LWPolyline generates
    // (lckernel/cad/primitive/lwpolyline.cpp:195) and its boundingBox at :147.
    std::vector<lc::entity::LWVertex2D> vertexes{
        lc::entity::LWVertex2D(lc::geo::Coordinate(0.0, 0.0, 0.0)),
        lc::entity::LWVertex2D(lc::geo::Coordinate(3.0, 4.0, 5.0)),
        lc::entity::LWVertex2D(lc::geo::Coordinate(6.0, 1.0, 2.0))};
    auto polyline = std::make_shared<lc::entity::LWPolyline>(
        vertexes, /*width=*/0.0, /*elevation=*/0.0, /*thickness=*/0.0,
        /*closed=*/false, lc::geo::Coordinate(0, 0, 1), layer);

    ASSERT_NO_THROW(insertThroughBuilder(
        doc, {line, polyline}))
        << "A 3-D segment must not abort the insert path; before 4c7e438b "
           "geo::Area threw a bare const char* from Line::boundingBox.";

    // Not silently dropped.
    EXPECT_TRUE(documentHolds(doc, line));
    EXPECT_TRUE(documentHolds(doc, polyline));
    EXPECT_EQ(doc->entityContainer().asVector().size(), 2u);

    // The line's own box is the 3-D AABB of its endpoints.
    const auto lineBox = line->boundingBox();
    EXPECT_DOUBLE_EQ(lineBox.minP().x(), 1.0);
    EXPECT_DOUBLE_EQ(lineBox.minP().y(), 2.0);
    EXPECT_DOUBLE_EQ(lineBox.minP().z(), 3.0);
    EXPECT_DOUBLE_EQ(lineBox.maxP().x(), 7.0);
    EXPECT_DOUBLE_EQ(lineBox.maxP().y(), 11.0);
    EXPECT_DOUBLE_EQ(lineBox.maxP().z(), 13.0);
    EXPECT_DOUBLE_EQ(lineBox.width(), 6.0);
    EXPECT_DOUBLE_EQ(lineBox.height(), 9.0);

    // The polyline's box is assembled with geo::Area::merge, which is
    // deliberately 2-D (geoarea.h:154-170 builds its corners from x and y
    // only), so only x and y are asserted here.  Asserting z would pin an
    // implementation detail of merge rather than the behaviour under test.
    const auto polyBox = polyline->boundingBox();
    EXPECT_DOUBLE_EQ(polyBox.minP().x(), 0.0);
    EXPECT_DOUBLE_EQ(polyBox.minP().y(), 0.0);
    EXPECT_DOUBLE_EQ(polyBox.maxP().x(), 6.0);
    EXPECT_DOUBLE_EQ(polyBox.maxP().y(), 4.0);
}

// A HATCH boundary loop may declare zero edges (91=1 93=0), and LibreCAD
// writes such loops itself today.  Before 983ee034, geo::Loop's constructor
// read loop[0] on the empty vector and Region::boundingBox read _loopList[0]
// on a region with no loops — two out-of-bounds reads on the open path.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, EmptyHatchLoopDoesNotCrash) {
    auto doc = newDocument();
    auto layer = defaultLayer();

    // (a) A region carrying one loop that has no edges — geo::Loop's ctor.
    auto emptyLoopHatch = std::make_shared<lc::entity::Hatch>(layer);
    {
        lc::geo::Region region;
        region.addLoop(lc::geo::Loop(std::vector<lc::entity::CADEntity_CSPtr>{}));
        ASSERT_EQ(region.numLoops(), 1u);
        emptyLoopHatch->setRegion(region);
    }
    emptyLoopHatch->setPatternName("SOLID");
    emptyLoopHatch->setSolid(1);

    // (b) A hatch whose region has no loops at all — Region::boundingBox.
    auto noLoopHatch = std::make_shared<lc::entity::Hatch>(layer);
    noLoopHatch->setPatternName("SOLID");
    noLoopHatch->setSolid(1);
    ASSERT_EQ(noLoopHatch->getRegion().numLoops(), 0u);

    ASSERT_NO_THROW(insertThroughBuilder(doc, {emptyLoopHatch, noLoopHatch}));

    EXPECT_TRUE(documentHolds(doc, emptyLoopHatch));
    EXPECT_TRUE(documentHolds(doc, noLoopHatch));

    // Both degenerate hatches fall back to the default, empty Area.
    for (const auto& hatch : {emptyLoopHatch, noLoopHatch}) {
        const auto box = hatch->boundingBox();
        EXPECT_DOUBLE_EQ(box.minP().x(), 0.0);
        EXPECT_DOUBLE_EQ(box.minP().y(), 0.0);
        EXPECT_DOUBLE_EQ(box.maxP().x(), 0.0);
        EXPECT_DOUBLE_EQ(box.maxP().y(), 0.0);
        EXPECT_DOUBLE_EQ(box.width(), 0.0);
        EXPECT_DOUBLE_EQ(box.height(), 0.0);
    }
}

// A SPLINE may carry fit points and no control points (72=0 73=0 74=n).  That
// is legal DXF, the pinned libdxfrw accepts it (isValidFitSplineLayout needs
// only fitCount >= 2), and it reaches the kernel through the DXF reader.
// Spline::calculateBoundingBox runs from the constructor, so before 983ee034
// this crashed before the entity ever reached the document.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, FitPointsOnlySplineDoesNotCrash) {
    auto doc = newDocument();
    auto layer = defaultLayer();

    const std::vector<lc::geo::Coordinate> noControlPoints;
    const std::vector<double> noKnots;
    const std::vector<lc::geo::Coordinate> fitPoints{
        lc::geo::Coordinate(-2.0, 1.0, 0.0),
        lc::geo::Coordinate(4.0, 7.0, 0.0),
        lc::geo::Coordinate(9.0, -3.0, 0.0)};

    std::shared_ptr<lc::entity::Spline> spline;
    ASSERT_NO_THROW(
        spline = std::make_shared<lc::entity::Spline>(
            noControlPoints, noKnots, fitPoints,
            /*degree=*/3, /*closed=*/false, /*fitTolerance=*/0.0,
            /*startTangent=*/0.0, 0.0, 0.0,
            /*endTangent=*/0.0, 0.0, 0.0,
            /*normal=*/0.0, 0.0, 1.0,
            static_cast<lc::geo::Spline::splineflag>(8) /*PLANAR*/,
            layer));
    ASSERT_NE(spline, nullptr);
    ASSERT_TRUE(spline->controlPoints().empty());
    ASSERT_EQ(spline->fitPoints().size(), 3u);

    ASSERT_NO_THROW(insertThroughBuilder(doc, {spline}));
    EXPECT_TRUE(documentHolds(doc, spline));

    // With no control points the box must cover the fit points, not collapse
    // to the origin — a guard that merely returned an empty Area for this
    // shape would hide the spline from every area query.
    const auto box = spline->boundingBox();
    EXPECT_DOUBLE_EQ(box.minP().x(), -2.0);
    EXPECT_DOUBLE_EQ(box.minP().y(), -3.0);
    EXPECT_DOUBLE_EQ(box.maxP().x(), 9.0);
    EXPECT_DOUBLE_EQ(box.maxP().y(), 7.0);

    // A spline with neither list is the remaining degenerate shape; it takes
    // the empty-Area fallback and must still not crash.
    std::shared_ptr<lc::entity::Spline> emptySpline;
    ASSERT_NO_THROW(
        emptySpline = std::make_shared<lc::entity::Spline>(
            noControlPoints, noKnots, std::vector<lc::geo::Coordinate>{},
            /*degree=*/3, /*closed=*/false, /*fitTolerance=*/0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0,
            static_cast<lc::geo::Spline::splineflag>(8), layer));
    EXPECT_DOUBLE_EQ(emptySpline->boundingBox().width(), 0.0);
    EXPECT_DOUBLE_EQ(emptySpline->boundingBox().height(), 0.0);
}

// `File::save` used to return void and throw `DXFimpl::writeDXF`'s result
// away, and an unsupported `File::Type` fell through a silent `else`: the
// user pressed Save, no file appeared, and nothing said so.  It now returns
// bool (persistence/file.cpp:101-109).
//
// This case is the one guard here that the pre-fix tree fails at *compile*
// time rather than at run time: against `static void save(...)` the
// EXPECT_FALSE below is "invalid argument type 'void' to unary expression".
// That is the strongest form the guard can take — the signature cannot
// regress without this file failing to build.
//
// Two independent refusals are asserted, each paired with a control so that a
// `save` that returned false unconditionally would not pass:
//   * a *format* with no writer behind it (LIBOPENCAD_DWG, outside the
//     libdxfrw range persistence/file.cpp:102 tests), and
//   * a *document* the library itself rejects for the requested version —
//     measured: an LWPOLYLINE at R12 gives "DRW::error 2, emit-failure: DXF
//     output generation failed" and no file, while the same document at
//     R2000, and a plain LINE at R12, both write.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, SaveReportsFailure) {
    auto layer = defaultLayer();

    auto lineDoc = newDocument();
    insertThroughBuilder(lineDoc, {std::make_shared<lc::entity::Line>(
        lc::geo::Coordinate(0.0, 0.0, 0.0),
        lc::geo::Coordinate(10.0, 5.0, 0.0), layer)});

    const std::vector<lc::entity::LWVertex2D> vertexes{
        lc::entity::LWVertex2D(lc::geo::Coordinate(0.0, 0.0)),
        lc::entity::LWVertex2D(lc::geo::Coordinate(5.0, 2.0)),
        lc::entity::LWVertex2D(lc::geo::Coordinate(9.0, 0.0))};
    auto makePolylineDoc = [&]() {
        auto doc = newDocument();
        insertThroughBuilder(doc, {std::make_shared<lc::entity::LWPolyline>(
            vertexes, 0.0, 0.0, 0.0, /*closed=*/false,
            lc::geo::Coordinate(0, 0, 1), layer)});
        return doc;
    };

    // (a) A File::Type with no writer at all.
    const std::string noWriterPath = uniqueTmpDxf("no-writer");
    boost::filesystem::remove(noWriterPath);
    EXPECT_FALSE(lc::persistence::File::save(
        lineDoc, noWriterPath, lc::persistence::File::Type::LIBOPENCAD_DWG))
        << "A File::Type with no writer must return false, not do nothing.";
    EXPECT_FALSE(boost::filesystem::exists(noWriterPath))
        << "A refused save must leave no file behind.";

    // (b) A document holding a record the target revision has no room for.
    //
    // It is not the LWPOLYLINE: those are written as R12's own POLYLINE, which
    // carries the same geometry (see the round trip below).  A SPLINE has no
    // R12 record at all -- but that is a reason to lose the spline, not the
    // drawing.  save() returns false because the whole drawing was not
    // written, and the file holds everything else.
    auto splineDoc = newDocument();
    insertThroughBuilder(splineDoc, {
        std::make_shared<lc::entity::Line>(
            lc::geo::Coordinate(0.0, 0.0, 0.0), lc::geo::Coordinate(10.0, 5.0, 0.0), layer),
        std::make_shared<lc::entity::Spline>(
            std::vector<lc::geo::Coordinate>{{0.0, 0.0}, {3.0, 8.0}, {7.0, -4.0}, {10.0, 0.0}},
            std::vector<double>{0, 0, 0, 0, 1, 1, 1, 1},
            std::vector<lc::geo::Coordinate>{},
            3, false, 0.0,
            0, 0, 0, 0, 0, 0, 0, 0, 1,
            lc::geo::Spline::splineflag::PERIODIC, layer)});

    const std::string lossyPath = uniqueTmpDxf("lossy-r12");
    boost::filesystem::remove(lossyPath);
    EXPECT_FALSE(lc::persistence::File::save(
        splineDoc, lossyPath, lc::persistence::File::Type::LIBDXFRW_DXF_R12))
        << "R12 cannot carry a SPLINE, so this is not a complete save.";
    ASSERT_TRUE(boost::filesystem::exists(lossyPath))
        << "The rest of the drawing must still be written.";

    {
        auto reopened = newDocument();
        ASSERT_NO_THROW(lc::persistence::File::open(
            reopened, lossyPath, lc::persistence::File::Library::LIBDXFRW));
        int lines = 0;
        int splines = 0;
        for (const auto& entity : reopened->entityContainer().asVector()) {
            if (std::dynamic_pointer_cast<const lc::entity::Line>(entity)) {
                lines++;
            }
            if (std::dynamic_pointer_cast<const lc::entity::Spline>(entity)) {
                splines++;
            }
        }
        EXPECT_EQ(lines, 1) << "Losing one record must not cost the others.";
        EXPECT_EQ(splines, 0) << "R12 has no SPLINE record.";
    }
    boost::filesystem::remove(lossyPath);

    // (c) The LWPOLYLINE that used to be refused is now written as a POLYLINE
    // and comes back as the same polyline.
    const std::string downConvertedPath = uniqueTmpDxf("r12-polyline");
    boost::filesystem::remove(downConvertedPath);
    EXPECT_TRUE(lc::persistence::File::save(
        makePolylineDoc(), downConvertedPath,
        lc::persistence::File::Type::LIBDXFRW_DXF_R12))
        << "R12 carries an LWPOLYLINE as POLYLINE; the save must not fail.";
    {
        std::ifstream written(downConvertedPath);
        const std::string body((std::istreambuf_iterator<char>(written)),
                               std::istreambuf_iterator<char>());
        EXPECT_NE(body.find("\nPOLYLINE\n"), std::string::npos)
            << "R12 must get a POLYLINE record.";
        EXPECT_EQ(body.find("\nLWPOLYLINE\n"), std::string::npos)
            << "R12 has no LWPOLYLINE record.";

        auto reopened = newDocument();
        ASSERT_NO_THROW(lc::persistence::File::open(
            reopened, downConvertedPath, lc::persistence::File::Library::LIBDXFRW));
        int polylines = 0;
        for (const auto& entity : reopened->entityContainer().asVector()) {
            if (auto pl = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(entity)) {
                polylines++;
                EXPECT_EQ(pl->vertex().size(), vertexes.size())
                    << "The down-convert must keep every vertex.";
            }
        }
        EXPECT_EQ(polylines, 1) << "The polyline must survive the down-convert.";
    }
    boost::filesystem::remove(downConvertedPath);

    // Control 1: the same document at a version that can carry it.
    const std::string acceptedPath = uniqueTmpDxf("accepted-r2000");
    boost::filesystem::remove(acceptedPath);
    EXPECT_TRUE(lc::persistence::File::save(
        makePolylineDoc(), acceptedPath,
        lc::persistence::File::Type::LIBDXFRW_DXF_R2000))
        << "The refusal must be attributable to the version, not the writer.";
    EXPECT_TRUE(boost::filesystem::exists(acceptedPath));
    EXPECT_GT(boost::filesystem::file_size(acceptedPath), 0u);

    // Control 2: the same version with a document it can carry.
    const std::string r12Path = uniqueTmpDxf("accepted-r12");
    boost::filesystem::remove(r12Path);
    EXPECT_TRUE(lc::persistence::File::save(
        lineDoc, r12Path, lc::persistence::File::Type::LIBDXFRW_DXF_R12))
        << "The refusal must be attributable to the entity, not to R12.";
    EXPECT_TRUE(boost::filesystem::exists(r12Path));
    EXPECT_GT(boost::filesystem::file_size(r12Path), 0u);

    boost::filesystem::remove(acceptedPath);
    boost::filesystem::remove(r12Path);
}

// The round trip the suite is named for: a 3-D segment must survive
// open→save→open through the real `File` API, with its z intact.  This is the
// first blocking test in the repository that calls `File::open` at all.
//
// A DXF file can legally contain records the kernel cannot represent: an ARC
// with radius 0, a CIRCLE with a negative radius, or an LWPOLYLINE declaring
// zero vertices. geo::Arc and geo::Circle throw std::runtime_error for those,
// and the throw used to travel out through libdxfrw's callback and out of
// File::open, which no caller guards -- so one bad record cost the whole file.
// They must be skipped individually, and the rest of the file must still load.
// A spline read from a file must be writable again. The reader used to trim the
// first and last knot, leaving knotCount != controlCount + degree + 1, which the
// library refuses -- so the SECOND save produced no file at all and the drawing
// could not be saved after being opened.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, SplineSurvivesReopenAndResave) {
    const std::string first = uniqueTmpDxf("spline-1");
    const std::string second = uniqueTmpDxf("spline-2");
    boost::filesystem::remove(first);
    boost::filesystem::remove(second);

    const std::vector<lc::geo::Coordinate> controlPoints{
        lc::geo::Coordinate(0.0, 0.0, 0.0), lc::geo::Coordinate(3.0, 5.0, 0.0),
        lc::geo::Coordinate(7.0, -2.0, 0.0), lc::geo::Coordinate(11.0, 4.0, 0.0)};
    // Clamped cubic: controlCount + degree + 1 == 8.
    const std::vector<double> knots{0., 0., 0., 0., 1., 1., 1., 1.};

    {
        auto doc = newDocument();
        auto spline = std::make_shared<lc::entity::Spline>(
            controlPoints, knots, std::vector<lc::geo::Coordinate>{},
            /*degree=*/3, /*closed=*/false, /*fitTolerance=*/0.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0,
            static_cast<lc::geo::Spline::splineflag>(8), defaultLayer());
        ASSERT_NO_THROW(insertThroughBuilder(doc, {spline}));
        ASSERT_TRUE(lc::persistence::File::save(
            doc, first, lc::persistence::File::Type::LIBDXFRW_DXF_R2000));
    }

    auto reopened = newDocument();
    ASSERT_NO_THROW(lc::persistence::File::open(
        reopened, first, lc::persistence::File::Library::LIBDXFRW));

    lc::entity::Spline_CSPtr readBack;
    for (const auto& entity : reopened->entityContainer().asVector()) {
        if (auto sp = std::dynamic_pointer_cast<const lc::entity::Spline>(entity)) {
            readBack = sp;
        }
    }
    ASSERT_NE(readBack, nullptr) << "The spline did not survive the first read.";
    EXPECT_EQ(readBack->degree(), 3u);
    EXPECT_EQ(readBack->controlPoints().size(), controlPoints.size());
    EXPECT_EQ(readBack->knotPoints().size(), knots.size())
        << "The knot vector was trimmed on read; the re-save below cannot succeed.";

    // The regression: this is the save that used to produce nothing.
    EXPECT_TRUE(lc::persistence::File::save(
        reopened, second, lc::persistence::File::Type::LIBDXFRW_DXF_R2000));
    EXPECT_TRUE(boost::filesystem::exists(second));
    EXPECT_GT(boost::filesystem::file_size(second), 0u);

    boost::filesystem::remove(first);
    boost::filesystem::remove(second);
}

// A block carrying custom-entity metadata is written as a 102 application group.
// Without its closing marker the group is unbalanced, dxfRW::writeAppData fails,
// and the whole file is abandoned -- no file, no message.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, CustomEntityBlockRoundTrip) {
    const std::string path = uniqueTmpDxf("custom-entity");
    boost::filesystem::remove(path);

    {
        auto doc = newDocument();
        auto storage = std::make_shared<lc::meta::CustomEntityStorage>(
            "testplugin", "testentity", lc::geo::Coordinate(0.0, 0.0, 0.0),
            std::map<std::string, std::string>{{"width", "42"}});
        auto builder = std::make_shared<lc::operation::Builder>(doc, "add block");
        builder->append(std::make_shared<lc::operation::AddBlock>(doc, storage));
        builder->execute();

        ASSERT_TRUE(lc::persistence::File::save(
            doc, path, lc::persistence::File::Type::LIBDXFRW_DXF_R2000))
            << "Saving a document with a custom entity produced no file.";
    }

    ASSERT_TRUE(boost::filesystem::exists(path));
    ASSERT_GT(boost::filesystem::file_size(path), 0u);

    auto reopened = newDocument();
    ASSERT_NO_THROW(lc::persistence::File::open(
        reopened, path, lc::persistence::File::Library::LIBDXFRW));

    lc::meta::CustomEntityStorage_CSPtr found;
    for (const auto& block : reopened->blocks()) {
        if (auto ce = std::dynamic_pointer_cast<const lc::meta::CustomEntityStorage>(block)) {
            found = ce;
        }
    }
    ASSERT_NE(found, nullptr) << "The custom entity block did not survive the round trip.";
    EXPECT_EQ(found->pluginName(), "testplugin");
    EXPECT_EQ(found->entityName(), "testentity");
    EXPECT_EQ(found->param("width"), "42");

    boost::filesystem::remove(path);
}

// Every type the Save As dialog offers must either produce a real file or
// report failure -- and a type named "binary" must actually write binary.
// LIBDXFRW_DXB_R2013 wrote ASCII because the encoding test was an exclusive
// range that stopped one short of the last binary target.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, EveryAdvertisedSaveTypeProducesAFile) {
    const char* kBinarySentinel = "AutoCAD Binary DXF";

    for (const auto& advertised : lc::persistence::File::getAvailableFileTypes()) {
        const auto type = advertised.first;
        const std::string path = uniqueTmpDxf(
            ("target-" + std::to_string(static_cast<int>(type))).c_str());
        boost::filesystem::remove(path);

        // Representative rather than minimal: a lone LINE is carried by every
        // revision back to R10, so it could not tell a working target from one
        // that refuses the drawing.  The polyline is the interesting one --
        // LWPOLYLINE arrived with R13, and the R12 targets have to write it as
        // POLYLINE instead of refusing the file.
        auto layer = defaultLayer();
        auto doc = newDocument();
        std::vector<lc::entity::LWVertex2D> vertexes{
            lc::entity::LWVertex2D(lc::geo::Coordinate(0.0, 0.0)),
            lc::entity::LWVertex2D(lc::geo::Coordinate(5.0, 2.0), 0.4),
            lc::entity::LWVertex2D(lc::geo::Coordinate(9.0, 0.0))};
        ASSERT_NO_THROW(insertThroughBuilder(doc, {
            std::make_shared<lc::entity::Line>(
                lc::geo::Coordinate(0.0, 0.0, 0.0), lc::geo::Coordinate(4.0, 3.0, 0.0), layer),
            std::make_shared<lc::entity::Circle>(
                lc::geo::Coordinate(2.0, 2.0, 0.0), 1.5, layer),
            std::make_shared<lc::entity::LWPolyline>(
                vertexes, 0.0, 0.0, 0.0, /*closed=*/false,
                lc::geo::Coordinate(0, 0, 1), layer)}));

        const bool saved = lc::persistence::File::save(doc, path, type);

        if (!saved) {
            // Only LIBOPENCAD_DWG may refuse: every DXF target must carry this
            // drawing, R12 included.  A refusal must also leave nothing
            // behind, not a truncated file.
            EXPECT_EQ(type, lc::persistence::File::LIBOPENCAD_DWG)
                << advertised.second << " refused a drawing it should carry.";
            EXPECT_FALSE(boost::filesystem::exists(path))
                << advertised.second << " reported failure but left a file.";
            continue;
        }

        ASSERT_TRUE(boost::filesystem::exists(path)) << advertised.second;
        ASSERT_GT(boost::filesystem::file_size(path), 0u) << advertised.second;

        std::ifstream written(path, std::ios::binary);
        const std::string head(
            (std::istreambuf_iterator<char>(written)), std::istreambuf_iterator<char>());
        const bool isBinary = head.find(kBinarySentinel) != std::string::npos;

        EXPECT_EQ(isBinary, lc::persistence::File::isBinaryType(type))
            << advertised.second << " wrote the wrong encoding.";

        // Writing is half of it. A binary target that produces a file nothing
        // can read is not a save target, and the encodings must not disagree
        // about what the drawing contains.
        auto reopened = newDocument();
        lc::persistence::ImportResult result;
        ASSERT_NO_THROW(result = lc::persistence::File::importFile(
            reopened, path, lc::persistence::File::Library::LIBDXFRW))
            << advertised.second;

        EXPECT_TRUE(result.ok) << advertised.second << " wrote a file it cannot read back.";
        EXPECT_FALSE(result.partial) << advertised.second;
        EXPECT_TRUE(result.failures.empty()) << advertised.second;
        EXPECT_EQ(entitiesInDocument(reopened), entitiesInDocument(doc))
            << advertised.second << " changed the drawing.";

        boost::filesystem::remove(path);
    }
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, DegenerateRecordsAreSkippedNotFatal) {
    const std::string path = uniqueTmpDxf("degenerate");
    boost::filesystem::remove(path);

    // Hand-written R12 ASCII: a zero-radius ARC, a negative-radius CIRCLE, an
    // empty LWPOLYLINE, and one good LINE that must survive all three.
    {
        std::ofstream dxf(path);
        dxf << "0\nSECTION\n2\nENTITIES\n"
            << "0\nARC\n8\n0\n10\n0.0\n20\n0.0\n30\n0.0\n40\n0.0\n50\n0.0\n51\n90.0\n"
            << "0\nCIRCLE\n8\n0\n10\n5.0\n20\n5.0\n30\n0.0\n40\n-5.0\n"
            << "0\nLWPOLYLINE\n8\n0\n90\n0\n70\n0\n"
            << "0\nLINE\n8\n0\n10\n1.0\n20\n2.0\n30\n0.0\n11\n7.0\n21\n11.0\n31\n0.0\n"
            << "0\nENDSEC\n0\nEOF\n";
    }

    auto doc = newDocument();
    ASSERT_NO_THROW(lc::persistence::File::open(
        doc, path, lc::persistence::File::Library::LIBDXFRW))
        << "A degenerate record must not abort the open.";

    bool foundLine = false;
    for (const auto& entity : doc->entityContainer().asVector()) {
        if (std::dynamic_pointer_cast<const lc::entity::Line>(entity)) {
            foundLine = true;
        }
    }
    EXPECT_TRUE(foundLine)
        << "The good LINE must still be imported after the skipped records.";

    boost::filesystem::remove(path);
}

// LWPolyline::generateEntities() walked off an empty vertex vector: begin() was
// end(), and the closing segment read lastPoint->bulge() past the allocation.
// A bulge between coincident points is the same class -- createArcBulge yields
// a radius geo::Arc rejects -- and must degrade to the chord.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, DegenerateLwPolylinesInsertWithoutCrashing) {
    auto doc = newDocument();

    auto empty = std::make_shared<lc::entity::LWPolyline>(
        std::vector<lc::entity::LWVertex2D>{}, 0., 0., 0., true,
        lc::geo::Coordinate(0., 0., 1.), defaultLayer());

    // One vertex, closed: there is no second point to close against.
    auto single = std::make_shared<lc::entity::LWPolyline>(
        std::vector<lc::entity::LWVertex2D>{
            lc::entity::LWVertex2D(lc::geo::Coordinate(1., 1.), 0.5)},
        0., 0., 0., true, lc::geo::Coordinate(0., 0., 1.), defaultLayer());

    // Two coincident points with a bulge: createArcBulge computes a radius that
    // geo::Arc rejects.
    auto coincidentBulge = std::make_shared<lc::entity::LWPolyline>(
        std::vector<lc::entity::LWVertex2D>{
            lc::entity::LWVertex2D(lc::geo::Coordinate(4., 4.), 0.5),
            lc::entity::LWVertex2D(lc::geo::Coordinate(4., 4.), 0.)},
        0., 0., 0., false, lc::geo::Coordinate(0., 0., 1.), defaultLayer());

    ASSERT_NO_THROW(insertThroughBuilder(doc, {empty, single, coincidentBulge}));

    EXPECT_TRUE(documentHolds(doc, empty));
    EXPECT_TRUE(documentHolds(doc, single));
    EXPECT_TRUE(documentHolds(doc, coincidentBulge));
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, ThreeDimensionalSegmentSurvivesRoundTrip) {
    const std::string path = uniqueTmpDxf("3d-line");
    boost::filesystem::remove(path);

    const lc::geo::Coordinate start(1.0, 2.0, 3.0);
    const lc::geo::Coordinate end(7.0, 11.0, 13.0);

    {
        auto doc = newDocument();
        auto line = std::make_shared<lc::entity::Line>(
            start, end, defaultLayer());
        // Guarded: on a pre-fix kernel the throw lands HERE, in setup, so an
        // unguarded call would make this case a duplicate of
        // ThreeDimensionalSegmentInsertsWithoutThrowing instead of a test of
        // the read path below.
        ASSERT_NO_THROW(insertThroughBuilder(doc, {line}));
        ASSERT_TRUE(lc::persistence::File::save(
            doc, path, lc::persistence::File::Type::LIBDXFRW_DXF_R2000));
    }

    ASSERT_TRUE(boost::filesystem::exists(path));
    ASSERT_GT(boost::filesystem::file_size(path), 0u);

    auto reopened = newDocument();
    // The read path: the reader builds the Line, the builder inserts it, and the
    // quad tree asks for its box. On a pre-fix kernel this is where the throw
    // would land for a file produced elsewhere; here the guarded setup above
    // fails first, which is why that assertion is separate.
    ASSERT_NO_THROW(lc::persistence::File::open(
        reopened, path, lc::persistence::File::Library::LIBDXFRW));

    const auto entities = reopened->entityContainer().asVector();
    ASSERT_FALSE(entities.empty()) << "The reopened document is empty.";

    bool found = false;
    for (const auto& entity : entities) {
        auto line = std::dynamic_pointer_cast<const lc::entity::Line>(entity);
        if (line == nullptr) {
            continue;
        }
        if (std::abs(line->start().x() - start.x()) < LCTOLERANCE &&
            std::abs(line->start().y() - start.y()) < LCTOLERANCE &&
            std::abs(line->start().z() - start.z()) < LCTOLERANCE &&
            std::abs(line->end().x() - end.x()) < LCTOLERANCE &&
            std::abs(line->end().y() - end.y()) < LCTOLERANCE &&
            std::abs(line->end().z() - end.z()) < LCTOLERANCE) {
            found = true;
            // The box that used to abort, now computed on a document that
            // came off disk.
            const auto box = line->boundingBox();
            EXPECT_DOUBLE_EQ(box.minP().z(), 3.0);
            EXPECT_DOUBLE_EQ(box.maxP().z(), 13.0);
            break;
        }
    }
    EXPECT_TRUE(found)
        << "The 3-D LINE did not survive save/open with its z intact.";

    boost::filesystem::remove(path);
}

// DXF symbol-table names are case-insensitive, and writers disagree on how to
// spell model space: AutoCAD writes `*Model_Space`, ODA's File Converter
// writes `*MODEL_SPACE` for R13 and R14.  `DXFimpl::getBlock` compared the
// block name to `DEFAULT_VIEWPORT` exactly, so with the uppercase spelling
// model space was treated as an ordinary named block: every entity in the
// drawing was filed inside that block and the document's own container came
// back empty.  The two ODA sample files in the review corpus opened with 0
// entities for exactly this reason.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, ModelSpaceNameIsCaseInsensitive) {
    const std::string path = uniqueTmpDxf("modelspace-case");
    boost::filesystem::remove(path);

    // Model space spelled the way ODA spells it, plus a genuinely named block:
    // the LINE must land in the document, the CIRCLE must stay in NOTSPACE.
    {
        std::ofstream dxf(path);
        dxf << "0\nSECTION\n2\nBLOCKS\n"
            << "0\nBLOCK\n8\n0\n2\n*MODEL_SPACE\n70\n0\n"
            << "10\n0.0\n20\n0.0\n30\n0.0\n3\n*MODEL_SPACE\n1\n\n"
            << "0\nLINE\n8\n0\n10\n1.0\n20\n2.0\n30\n0.0\n11\n7.0\n21\n11.0\n31\n0.0\n"
            << "0\nENDBLK\n8\n0\n"
            << "0\nBLOCK\n8\n0\n2\nNOTSPACE\n70\n0\n"
            << "10\n0.0\n20\n0.0\n30\n0.0\n3\nNOTSPACE\n1\n\n"
            << "0\nCIRCLE\n8\n0\n10\n5.0\n20\n5.0\n30\n0.0\n40\n3.0\n"
            << "0\nENDBLK\n8\n0\n"
            << "0\nENDSEC\n"
            << "0\nSECTION\n2\nENTITIES\n0\nENDSEC\n0\nEOF\n";
    }

    auto doc = newDocument();
    ASSERT_NO_THROW(lc::persistence::File::open(
        doc, path, lc::persistence::File::Library::LIBDXFRW));

    bool foundLine = false;
    bool foundCircle = false;
    for (const auto& entity : doc->entityContainer().asVector()) {
        if (std::dynamic_pointer_cast<const lc::entity::Line>(entity)) {
            foundLine = true;
            EXPECT_EQ(entity->block(), nullptr)
                << "A *MODEL_SPACE entity belongs to the drawing, not to a block.";
        }
        if (std::dynamic_pointer_cast<const lc::entity::Circle>(entity)) {
            foundCircle = true;
        }
    }

    EXPECT_TRUE(foundLine)
        << "The entity inside *MODEL_SPACE must reach the document container.";
    EXPECT_FALSE(foundCircle)
        << "An entity inside a named block must stay in that block.";

    boost::filesystem::remove(path);
}

// File::open used to take the revision from dxfRW::getVersion(), which is the
// *text codec* bucket rather than the drawing's revision: it collapses R2004
// onto R2000 and R2010/R2013 onto R2007, maps AC1009 and AC1014 onto binary
// save targets, and -- because DRW_TextCodec's constructor defaults to AC1021
// -- reports R2007 for a file with no header at all. 33 of the 43 files in the
// review corpus were recorded as the wrong revision, 8 of them as a binary
// target, and that recorded value is what Save reuses.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, AcadVersionMapsToVariant) {
    const struct {
        const char* acadVersion;
        lc::persistence::File::Type expected;
        bool recognised;
    } cases[] = {
        {"AC1006", lc::persistence::File::LIBDXFRW_DXF_R12, true},
        {"AC1009", lc::persistence::File::LIBDXFRW_DXF_R12, true},
        {"AC1012", lc::persistence::File::LIBDXFRW_DXF_R14, true},   // R13 rounds up
        {"AC1014", lc::persistence::File::LIBDXFRW_DXF_R14, true},
        {"AC1015", lc::persistence::File::LIBDXFRW_DXF_R2000, true},
        {"AC1018", lc::persistence::File::LIBDXFRW_DXF_R2004, true},
        {"AC1021", lc::persistence::File::LIBDXFRW_DXF_R2007, true},
        {"AC1024", lc::persistence::File::LIBDXFRW_DXF_R2010, true},
        {"AC1027", lc::persistence::File::LIBDXFRW_DXF_R2013, true},
        {"AC1032", lc::persistence::File::LIBDXFRW_DXF_R2013, true}, // R2018 rounds down
        {"AC9999", lc::persistence::File::LIBDXFRW_DXF_R12, false},  // unsupported-version
        {"", lc::persistence::File::LIBDXFRW_DXF_R12, false},        // no $ACADVER
    };

    for (const auto& c : cases) {
        bool recognised = true;
        EXPECT_EQ(lc::persistence::File::typeForAcadVersion(c.acadVersion, &recognised),
                  c.expected) << "$ACADVER " << c.acadVersion;
        EXPECT_EQ(recognised, c.recognised) << "$ACADVER " << c.acadVersion;
        EXPECT_FALSE(lc::persistence::File::isBinaryType(
            lc::persistence::File::typeForAcadVersion(c.acadVersion)))
            << "Reading a file must never select a binary save target: " << c.acadVersion;
    }
}

// The same mapping, reached the way the application reaches it: through a real
// file.  This is what the empty addHeader override cost -- the revision was
// there in the HEADER section the whole time and nothing read it.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, OpenRecordsTheDrawingsOwnRevision) {
    const struct {
        const char* acadVersion;   // nullptr = write no HEADER section at all
        lc::persistence::File::Type expected;
    } cases[] = {
        {"AC1009", lc::persistence::File::LIBDXFRW_DXF_R12},
        {"AC1015", lc::persistence::File::LIBDXFRW_DXF_R2000},
        {"AC1018", lc::persistence::File::LIBDXFRW_DXF_R2004},
        {"AC1024", lc::persistence::File::LIBDXFRW_DXF_R2010},
        {"AC1027", lc::persistence::File::LIBDXFRW_DXF_R2013},
        {nullptr, lc::persistence::File::LIBDXFRW_DXF_R12},
    };

    for (const auto& c : cases) {
        const std::string path = uniqueTmpDxf(c.acadVersion ? c.acadVersion : "noheader");
        boost::filesystem::remove(path);
        {
            std::ofstream dxf(path);
            if (c.acadVersion != nullptr) {
                dxf << "0\nSECTION\n2\nHEADER\n"
                    << "9\n$ACADVER\n1\n" << c.acadVersion << "\n"
                    << "9\n$INSUNITS\n70\n4\n"
                    << "9\n$MEASUREMENT\n70\n1\n"
                    << "0\nENDSEC\n";
            }
            dxf << "0\nSECTION\n2\nENTITIES\n"
                << "0\nLINE\n8\n0\n10\n0.0\n20\n0.0\n30\n0.0\n11\n1.0\n21\n1.0\n31\n0.0\n"
                << "0\nENDSEC\n0\nEOF\n";
        }

        auto doc = newDocument();
        EXPECT_EQ(lc::persistence::File::open(
                      doc, path, lc::persistence::File::Library::LIBDXFRW),
                  c.expected)
            << "$ACADVER " << (c.acadVersion ? c.acadVersion : "(absent)");

        boost::filesystem::remove(path);
    }
}

namespace {

// BLOCKS containing `inner` and `outer` in the order given, plus one INSERT of
// OUTER at (10,10) in ENTITIES. INNER holds a line from (0,0) to (5,5); OUTER
// holds an INSERT of INNER at its own origin. The drawing is the same either
// way round -- only the order the reader meets the definitions in differs, and
// DWG systematically delivers the INSERT first.
std::string nestedBlockDxf(bool innerFirst) {
    const std::string inner =
        "0\nBLOCK\n8\n0\n2\nINNER\n70\n0\n10\n0.0\n20\n0.0\n30\n0.0\n3\nINNER\n1\n\n"
        "0\nLINE\n8\n0\n10\n0.0\n20\n0.0\n30\n0.0\n11\n5.0\n21\n5.0\n31\n0.0\n"
        "0\nENDBLK\n8\n0\n";
    const std::string outer =
        "0\nBLOCK\n8\n0\n2\nOUTER\n70\n0\n10\n0.0\n20\n0.0\n30\n0.0\n3\nOUTER\n1\n\n"
        "0\nINSERT\n8\n0\n2\nINNER\n10\n0.0\n20\n0.0\n30\n0.0\n"
        "0\nENDBLK\n8\n0\n";

    return "0\nSECTION\n2\nBLOCKS\n" + (innerFirst ? inner + outer : outer + inner)
           + "0\nENDSEC\n"
           + "0\nSECTION\n2\nENTITIES\n"
             "0\nINSERT\n8\n0\n2\nOUTER\n10\n10.0\n20\n10.0\n30\n0.0\n"
             "0\nENDSEC\n0\nEOF\n";
}

}  // namespace

// An INSERT's bounding box is the union of the boxes of the entities in the
// block it displays. DXFimpl built the Insert inside the read callback, where
// nothing has reached the document yet -- every entity the callbacks make is
// queued in an operation::Builder that runs afterwards -- so the block always
// measured empty and the box came out as a degenerate point at the insertion
// point. Worse, blockByName() misses during the read too, so the entity was
// pointed at a fabricated stand-in Block rather than the one the file defines,
// and the "block gained an entity" event never matched it by identity.
//
// The nesting is not decoration: OUTER's box can only be right if the INSERT
// inside it was built first, which is what forces the dependency ordering.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, InsertMeasuresTheBlockItDisplays) {
    for (const bool innerFirst : {true, false}) {
        const std::string path = uniqueTmpDxf(innerFirst ? "blocks-inner-first" : "blocks-insert-first");
        boost::filesystem::remove(path);
        {
            std::ofstream dxf(path);
            dxf << nestedBlockDxf(innerFirst);
        }

        auto doc = newDocument();
        ASSERT_NO_THROW(lc::persistence::File::open(
            doc, path, lc::persistence::File::Library::LIBDXFRW));

        // One INSERT of OUTER at (10,10); OUTER draws INNER at its origin;
        // INNER draws (0,0)-(5,5). So the box is (10,10)-(15,15).
        int inserts = 0;
        for (const auto& entity : doc->entityContainer().asVector()) {
            auto insert = std::dynamic_pointer_cast<const lc::entity::Insert>(entity);
            if (insert == nullptr) {
                continue;
            }

            inserts++;
            ASSERT_NE(insert->displayBlock(), nullptr);
            EXPECT_EQ(insert->displayBlock()->name(), "OUTER");
            // The block the INSERT points at must be the document's own, not a
            // stand-in that happens to share its name.
            EXPECT_EQ(insert->displayBlock(), doc->blockByName("OUTER"));

            const auto box = insert->boundingBox();
            EXPECT_DOUBLE_EQ(box.minP().x(), 10.0);
            EXPECT_DOUBLE_EQ(box.minP().y(), 10.0);
            EXPECT_DOUBLE_EQ(box.maxP().x(), 15.0);
            EXPECT_DOUBLE_EQ(box.maxP().y(), 15.0);
        }

        EXPECT_EQ(inserts, 1) << (innerFirst ? "inner block first" : "INSERT before its block");

        // Each block is defined once; no same-named stand-in survives.
        std::map<std::string, int> byName;
        for (const auto& block : doc->blocks()) {
            byName[block->name()]++;
        }
        EXPECT_EQ(byName["INNER"], 1);
        EXPECT_EQ(byName["OUTER"], 1);

        boost::filesystem::remove(path);
    }
}

// writeHatch only knew how to emit LINE, ARC and ELLIPSE boundary edges. A
// loop made of an LWPOLYLINE or a SPLINE -- both of which addHatch reads back
// -- produced a loop with no edges at all, so the hatch came back with empty
// geo::Loops (the crash the kernel guards had to be taught to survive) or
// vanished. libdxfrw itself writes both; nothing in LibreCAD asked it to.
//
// The three loops cover the boundary path rule that makes this more than a
// transcription: DXF group 92 bit 1 declares the loop to *be* one polyline,
// and dxfRW::writeHatch refuses -- losing the whole file, not just the hatch
// -- if such a loop holds anything else. A polyline sharing its loop with
// another edge must therefore go in decomposed.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, HatchBoundariesSurviveARoundTrip) {
    const std::string path = uniqueTmpDxf("hatch-boundaries");
    boost::filesystem::remove(path);

    auto layer = defaultLayer();

    // Loop 1: one closed polyline, one of whose segments bulges.
    std::vector<lc::entity::LWVertex2D> square{
        {lc::geo::Coordinate(0.0, 0.0), 0.0, 0.0, 0.0},
        {lc::geo::Coordinate(10.0, 0.0), 0.5, 0.0, 0.0},
        {lc::geo::Coordinate(10.0, 10.0), 0.0, 0.0, 0.0},
        {lc::geo::Coordinate(0.0, 10.0), 0.0, 0.0, 0.0}};
    auto polylineLoop = std::make_shared<lc::entity::LWPolyline>(
        square, 0.0, 0.0, 0.0, true, lc::geo::Coordinate(0.0, 0.0, 1.0), layer);

    // Loop 2: one spline.
    auto spline = std::make_shared<lc::entity::Spline>(
        std::vector<lc::geo::Coordinate>{{20.0, 0.0}, {23.0, 8.0}, {27.0, -4.0}, {30.0, 0.0}},
        std::vector<double>{0, 0, 0, 0, 1, 1, 1, 1},
        std::vector<lc::geo::Coordinate>{},
        3, false, 0.0,
        0, 0, 0, 0, 0, 0, 0, 0, 1,
        lc::geo::Spline::splineflag::PERIODIC, layer);

    // Loop 3: a polyline that shares its loop with a plain line.
    std::vector<lc::entity::LWVertex2D> leg{
        {lc::geo::Coordinate(40.0, 0.0), 0.0, 0.0, 0.0},
        {lc::geo::Coordinate(50.0, 0.0), 0.0, 0.0, 0.0}};
    auto mixedPolyline = std::make_shared<lc::entity::LWPolyline>(
        leg, 0.0, 0.0, 0.0, false, lc::geo::Coordinate(0.0, 0.0, 1.0), layer);
    lc::builder::LineBuilder lineBuilder;
    lineBuilder.setStart(lc::geo::Coordinate(50.0, 0.0));
    lineBuilder.setEnd(lc::geo::Coordinate(40.0, 0.0));
    lineBuilder.setLayer(layer);

    lc::geo::Region region;
    region.addLoop(lc::geo::Loop({polylineLoop}));
    region.addLoop(lc::geo::Loop({spline}));
    region.addLoop(lc::geo::Loop({mixedPolyline, lineBuilder.build()}));

    auto hatch = std::make_shared<lc::entity::Hatch>(layer);
    hatch->setPatternName("SOLID");
    hatch->setSolid(true);
    hatch->setAngle(0.0);
    hatch->setScale(1.0);
    hatch->setRegion(region);

    auto doc = newDocument();
    insertThroughBuilder(doc, {hatch});
    ASSERT_TRUE(lc::persistence::File::save(doc, path, lc::persistence::File::LIBDXFRW_DXF_R2000))
        << "The hatch must be writable at all.";

    auto reopened = newDocument();
    ASSERT_NO_THROW(lc::persistence::File::open(
        reopened, path, lc::persistence::File::Library::LIBDXFRW));

    lc::entity::Hatch_CSPtr reloaded;
    for (const auto& entity : reopened->entityContainer().asVector()) {
        if (auto found = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity)) {
            reloaded = found;
        }
    }
    ASSERT_NE(reloaded, nullptr) << "The hatch did not survive the round trip.";

    const auto& loops = reloaded->getRegion().loopList();
    ASSERT_EQ(loops.size(), 3u) << "Every boundary loop must come back.";

    int polylineLoops = 0;
    int splineLoops = 0;
    int decomposedLoops = 0;
    for (const auto& loop : loops) {
        const auto& entities = loop.entities();
        EXPECT_FALSE(entities.empty()) << "A boundary loop with no edges is not a boundary.";

        if (entities.size() == 1
            && std::dynamic_pointer_cast<const lc::entity::LWPolyline>(entities.front())) {
            polylineLoops++;
        } else if (entities.size() == 1
                   && std::dynamic_pointer_cast<const lc::entity::Spline>(entities.front())) {
            splineLoops++;
        } else {
            // The mixed loop: the polyline's one segment plus the line, as two
            // LINE edges.
            decomposedLoops++;
            EXPECT_EQ(entities.size(), 2u);
            for (const auto& edge : entities) {
                EXPECT_NE(std::dynamic_pointer_cast<const lc::entity::Line>(edge), nullptr)
                    << "A polyline sharing a loop must arrive as plain edges.";
            }
        }
    }

    EXPECT_EQ(polylineLoops, 1) << "The all-polyline loop must stay a polyline boundary.";
    EXPECT_EQ(splineLoops, 1) << "The spline boundary must survive.";
    EXPECT_EQ(decomposedLoops, 1);

    boost::filesystem::remove(path);
}


// The encoding is a property of the file, not a preference: a user who opens an
// ASCII DXF and presses Save must get an ASCII DXF back. File::open used to
// answer AC1009 and AC1014 with the DXB_* variants, which write *binary*, so
// saving an R12 or R14 drawing under the type its own open reported silently
// changed the file's encoding. Nothing in the format required that -- it came
// from reading the revision out of the text codec.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, AsciiRoundTripStaysAscii) {
    const char* kBinarySentinel = "AutoCAD Binary DXF";

    for (const char* acadVersion : {"AC1009", "AC1014", "AC1015", "AC1027"}) {
        const std::string source = uniqueTmpDxf((std::string("ascii-") + acadVersion).c_str());
        const std::string resaved = uniqueTmpDxf((std::string("resaved-") + acadVersion).c_str());
        boost::filesystem::remove(source);
        boost::filesystem::remove(resaved);

        {
            std::ofstream dxf(source);
            dxf << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\n" << acadVersion << "\n0\nENDSEC\n"
                << "0\nSECTION\n2\nENTITIES\n"
                << "0\nLINE\n8\n0\n10\n0.0\n20\n0.0\n30\n0.0\n11\n4.0\n21\n3.0\n31\n0.0\n"
                << "0\nENDSEC\n0\nEOF\n";
        }

        auto doc = newDocument();
        const auto recorded = lc::persistence::File::open(
            doc, source, lc::persistence::File::Library::LIBDXFRW);

        EXPECT_FALSE(lc::persistence::File::isBinaryType(recorded))
            << acadVersion << " is an ASCII file; opening it must not record a binary target.";

        ASSERT_TRUE(lc::persistence::File::save(doc, resaved, recorded)) << acadVersion;

        std::ifstream written(resaved, std::ios::binary);
        const std::string body((std::istreambuf_iterator<char>(written)),
                               std::istreambuf_iterator<char>());
        EXPECT_EQ(body.find(kBinarySentinel), std::string::npos)
            << acadVersion << " came back binary after a plain Save.";

        boost::filesystem::remove(source);
        boost::filesystem::remove(resaved);
    }
}

#ifndef UNITTEST_FIXTURES_DIR
#error "UNITTEST_FIXTURES_DIR must name unittest/persistence/fixtures (see unittest/CMakeLists.txt)"
#endif

namespace {

/**
 * The DXF records in a file's ENTITIES section, by type name.
 *
 * Counting the file itself is the point: every other case in this suite
 * compares one import against another, which cannot see what the first import
 * already lost. VERTEX and SEQEND are parts of the POLYLINE that precedes
 * them, not records of their own.
 */
std::map<std::string, std::size_t> recordsInFile(const std::string& path) {
    std::map<std::string, std::size_t> census;
    std::ifstream file(path);
    std::string code;
    std::string value;
    bool inEntities = false;

    while (std::getline(file, code) && std::getline(file, value)) {
        while (!value.empty() && (value.back() == '\r' || value.back() == ' ')) {
            value.pop_back();
        }
        if (code.find('0') == std::string::npos || code.find_first_not_of(" \t0") != std::string::npos) {
            continue;  // not group code 0
        }

        if (value == "SECTION") {
            std::string sectionCode;
            std::string sectionName;
            if (std::getline(file, sectionCode) && std::getline(file, sectionName)) {
                while (!sectionName.empty() && (sectionName.back() == '\r' || sectionName.back() == ' ')) {
                    sectionName.pop_back();
                }
                inEntities = sectionName == "ENTITIES";
            }
            continue;
        }
        if (value == "ENDSEC" || value == "EOF") {
            inEntities = false;
            continue;
        }

        if (inEntities && value != "VERTEX" && value != "SEQEND") {
            census[value]++;
        }
    }

    return census;
}

std::string fixture(const char* name) {
    return std::string(UNITTEST_FIXTURES_DIR) + "/" + name;
}

}  // namespace

// The absolute import oracle.
//
// Every other case here asserts that a second import agrees with the first,
// which is satisfied by losing the same thing twice. This one compares the
// document against the records in the file, and accounts for the difference
// exactly: a record is either imported or on the list of kinds LibreCAD has no
// entity for. Nothing may fall between the two.
//
// The known-loss list is meant to shrink. When addSolid or addRay stops being
// an empty override this test fails and the table is updated -- which is the
// notification, not an inconvenience.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, AbsoluteImportOracle) {
    const struct {
        const char* name;
        lc::persistence::File::Type expectedType;
        std::map<std::string, std::size_t> records;   // what the file holds
        std::map<std::string, std::size_t> imported;  // what the document holds
        std::map<std::string, std::size_t> dropped;   // no kernel entity for these
    } cases[] = {
        {"oracle_r2000.dxf",
         lc::persistence::File::LIBDXFRW_DXF_R2000,
         {{"LINE", 2}, {"CIRCLE", 1}, {"ARC", 1}, {"LWPOLYLINE", 1}, {"TEXT", 1},
          {"POINT", 1}, {"ELLIPSE", 1}, {"SOLID", 1}, {"3DFACE", 1}, {"RAY", 1},
          {"XLINE", 1}},
         // A SOLID is a filled quadrilateral, and arrives as the solid-filled
         // Hatch that draws the same thing. 3DFACE, RAY and XLINE have no
         // LibreCAD equivalent at all and are still counted as losses.
         {{"Line", 2}, {"Circle", 1}, {"Arc", 1}, {"LWPolyline", 1}, {"Text", 1},
          {"Point", 1}, {"Ellipse", 1}, {"Hatch", 1}},
         {{"3DFACE", 1}, {"RAY", 1}, {"XLINE", 1}}},
        {"oracle_r12.dxf",
         lc::persistence::File::LIBDXFRW_DXF_R12,
         {{"LINE", 1}, {"CIRCLE", 1}, {"TEXT", 1}, {"POLYLINE", 1}},
         // R12's POLYLINE arrives as the same kernel entity as an LWPOLYLINE.
         {{"Line", 1}, {"Circle", 1}, {"Text", 1}, {"LWPolyline", 1}},
         {}},
    };

    for (const auto& c : cases) {
        const std::string path = fixture(c.name);
        ASSERT_TRUE(boost::filesystem::exists(path)) << path;

        // The fixture itself is part of the assertion: if it changes, the
        // expected numbers below stop meaning anything.
        EXPECT_EQ(recordsInFile(path), c.records) << c.name << ": the fixture changed.";

        auto doc = newDocument();
        lc::persistence::File::Type recorded = lc::persistence::File::LIBOPENCAD_DWG;
        ASSERT_NO_THROW(recorded = lc::persistence::File::open(
            doc, path, lc::persistence::File::Library::LIBDXFRW));

        // The fixture states its own revision in $ACADVER, so the recorded
        // variant is checkable against the bytes rather than against another
        // import.
        EXPECT_EQ(recorded, c.expectedType) << c.name;
        EXPECT_EQ(entitiesInDocument(doc), c.imported) << c.name;

        std::size_t recordCount = 0;
        for (const auto& record : c.records) {
            recordCount += record.second;
        }
        std::size_t droppedCount = 0;
        for (const auto& loss : c.dropped) {
            droppedCount += loss.second;
            EXPECT_GT(c.records.count(loss.first), 0u)
                << c.name << ": " << loss.first << " is listed as lost but is not in the file.";
        }

        EXPECT_EQ(doc->entityContainer().asVector().size() + droppedCount, recordCount)
            << c.name << ": every record must be imported or accounted for as a known loss.";

        // And the importer must say the same thing on its own, without this
        // table: a loss LibreCAD does not report is one the user cannot know
        // about.
        auto reported = newDocument();
        const auto result = lc::persistence::File::importFile(
            reported, path, lc::persistence::File::Library::LIBDXFRW);
        EXPECT_EQ(result.loss.droppedByType, c.dropped) << c.name;
    }
}

// Round-tripping must reach a fixed point. Comparing the first import with the
// second is not enough on its own -- see AbsoluteImportOracle -- but a
// difference between the second and the third is a writer that keeps changing
// the drawing under a user who keeps pressing Save.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, RoundTripIsStable) {
    for (const char* name : {"oracle_r2000.dxf", "oracle_r12.dxf"}) {
        const std::string source = fixture(name);
        ASSERT_TRUE(boost::filesystem::exists(source)) << source;

        auto opened = newDocument();
        const auto type = lc::persistence::File::open(
            opened, source, lc::persistence::File::Library::LIBDXFRW);
        const auto census0 = entitiesInDocument(opened);

        std::map<std::string, std::size_t> census[2];
        std::shared_ptr<lc::storage::DocumentImpl> carried = opened;

        for (int hop = 0; hop < 2; hop++) {
            const std::string path = uniqueTmpDxf(
                (std::string("stable-") + name + "-" + std::to_string(hop)).c_str());
            boost::filesystem::remove(path);

            ASSERT_TRUE(lc::persistence::File::save(carried, path, type))
                << name << " hop " << hop;

            auto reopened = newDocument();
            ASSERT_NO_THROW(lc::persistence::File::open(
                reopened, path, lc::persistence::File::Library::LIBDXFRW));
            census[hop] = entitiesInDocument(reopened);
            carried = reopened;

            boost::filesystem::remove(path);
        }

        EXPECT_EQ(census[0], census[1]) << name << ": saving twice must not keep changing it.";
        EXPECT_EQ(census0, census[0]) << name << ": the first save must not lose anything.";
    }
}


// The import result is what the layers above persistence act on: whether to
// enable Save, what to put in the title bar, what to tell the user. Each field
// is measured against a file chosen to produce it.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, ImportResultDescribesACleanRead) {
    auto doc = newDocument();
    const auto result = lc::persistence::File::importFile(
        doc, fixture("oracle_r2000.dxf"), lc::persistence::File::Library::LIBDXFRW);

    EXPECT_TRUE(result.ok);
    EXPECT_FALSE(result.partial) << "A clean read is not a partial one.";
    EXPECT_EQ(result.variantId, "dxf.ac1015.ascii");
    EXPECT_EQ(result.sourceVersionTag, "AC1015") << "The file's own $ACADVER, not a guess at it.";
    EXPECT_EQ(result.entitiesDelivered, 9u);
    EXPECT_TRUE(result.diagnostics.empty()) << "Nothing went wrong; nothing should be reported.";

    // The kinds LibreCAD has no entity for -- 3DFACE, RAY, XLINE -- are counted
    // rather than dropped in silence. The fixture's SOLID is not among them: it
    // arrives as the solid Hatch that draws the same thing.
    EXPECT_EQ(result.loss.total(), 3u);
    EXPECT_FALSE(result.loss.empty());
    EXPECT_EQ(result.loss.droppedByType.count("SOLID"), 0u);
}

// A failed read is not an empty one. libdxfrw stops where it fails and
// LibreCAD keeps what arrived before that point -- a drawing the user can see,
// and must not be allowed to silently save back over the original. That is the
// case File::open could not express at all: it returned a revision and nothing
// else.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, ImportResultDescribesAPartialRead) {
    const std::string path = uniqueTmpDxf("partial");
    boost::filesystem::remove(path);
    {
        // Two complete LINEs, then a record that stops mid-way: no ENDSEC, no
        // EOF, no closing group codes.
        std::ofstream dxf(path);
        dxf << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1015\n0\nENDSEC\n"
            << "0\nSECTION\n2\nENTITIES\n"
            << "0\nLINE\n8\n0\n10\n0.0\n20\n0.0\n30\n0.0\n11\n1.0\n21\n1.0\n31\n0.0\n"
            << "0\nLINE\n8\n0\n10\n2.0\n20\n2.0\n30\n0.0\n11\n3.0\n21\n3.0\n31\n0.0\n"
            << "0\nLINE\n8\n0\n10\n4.0\n";
    }

    auto doc = newDocument();
    lc::persistence::ImportResult result;
    ASSERT_NO_THROW(result = lc::persistence::File::importFile(
        doc, path, lc::persistence::File::Library::LIBDXFRW));

    EXPECT_FALSE(result.ok);
    EXPECT_TRUE(result.partial) << "Entities arrived before the failure; that is a partial read.";
    EXPECT_EQ(result.entitiesDelivered, 2u) << "The two complete records must be kept.";
    EXPECT_EQ(doc->entityContainer().asVector().size(), 2u);
    EXPECT_EQ(result.sourceVersionTag, "AC1015")
        << "The revision was read before the failure and must survive it.";
    ASSERT_FALSE(result.diagnostics.empty());
    EXPECT_EQ(result.diagnostics.front().severity, lc::persistence::Severity::Error);
    EXPECT_FALSE(result.diagnostics.front().code.empty())
        << "A diagnostic without a code cannot be matched on.";

    boost::filesystem::remove(path);
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, ImportResultDescribesAnUnopenableFile) {
    auto doc = newDocument();
    const auto result = lc::persistence::File::importFile(
        doc, uniqueTmpDxf("does-not-exist"), lc::persistence::File::Library::LIBDXFRW);

    EXPECT_FALSE(result.ok);
    EXPECT_FALSE(result.partial) << "Nothing arrived, so there is nothing partial about it.";
    EXPECT_EQ(result.entitiesDelivered, 0u);
    EXPECT_TRUE(result.sourceVersionTag.empty());
    ASSERT_FALSE(result.diagnostics.empty());
    EXPECT_EQ(result.diagnostics.front().severity, lc::persistence::Severity::Error);

    // Even with nothing read, the recorded variant is definite: a caller that
    // ignores `ok` must not read an uninitialised save target.
    EXPECT_FALSE(result.variantId.empty());
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, ExportResultDescribesWhatItLeftBehind) {
    auto layer = defaultLayer();
    auto doc = newDocument();
    insertThroughBuilder(doc, {
        std::make_shared<lc::entity::Line>(
            lc::geo::Coordinate(0.0, 0.0, 0.0), lc::geo::Coordinate(10.0, 5.0, 0.0), layer),
        std::make_shared<lc::entity::Spline>(
            std::vector<lc::geo::Coordinate>{{0.0, 0.0}, {3.0, 8.0}, {7.0, -4.0}, {10.0, 0.0}},
            std::vector<double>{0, 0, 0, 0, 1, 1, 1, 1},
            std::vector<lc::geo::Coordinate>{},
            3, false, 0.0,
            0, 0, 0, 0, 0, 0, 0, 0, 1,
            lc::geo::Spline::splineflag::PERIODIC, layer)});

    // R12: the file is written, the spline is named as left behind.
    const std::string lossy = uniqueTmpDxf("export-lossy");
    boost::filesystem::remove(lossy);
    const auto left = lc::persistence::File::exportFile(
        doc, lossy, lc::persistence::File::LIBDXFRW_DXF_R12);
    EXPECT_TRUE(left.ok) << "The file was written; that is not a failure.";
    EXPECT_EQ(left.variantId, "dxf.ac1009.ascii");
    EXPECT_EQ(left.loss.droppedByType.at("SPLINE"), 1u);
    ASSERT_FALSE(left.diagnostics.empty());
    EXPECT_EQ(left.diagnostics.front().severity, lc::persistence::Severity::Warning);
    EXPECT_EQ(left.diagnostics.front().code, "record-not-in-revision");
    EXPECT_TRUE(boost::filesystem::exists(lossy));
    boost::filesystem::remove(lossy);

    // R2000: nothing to leave behind.
    const std::string whole = uniqueTmpDxf("export-whole");
    boost::filesystem::remove(whole);
    const auto complete = lc::persistence::File::exportFile(
        doc, whole, lc::persistence::File::LIBDXFRW_DXF_R2000);
    EXPECT_TRUE(complete.ok);
    EXPECT_TRUE(complete.loss.empty());
    EXPECT_TRUE(complete.diagnostics.empty());
    boost::filesystem::remove(whole);

    // A type with no writer at all is a different failure, with its own code
    // and no file.
    const std::string refused = uniqueTmpDxf("export-no-writer");
    boost::filesystem::remove(refused);
    const auto noWriter = lc::persistence::File::exportFile(
        doc, refused, lc::persistence::File::LIBOPENCAD_DWG);
    EXPECT_FALSE(noWriter.ok);
    ASSERT_FALSE(noWriter.diagnostics.empty());
    EXPECT_EQ(noWriter.diagnostics.front().code, "no-writer");
    EXPECT_FALSE(boost::filesystem::exists(refused));
}

// The skip and the report must come from one rule.
//
// Two copies -- one deciding what to leave out, one explaining what was left
// out -- drift, and then LibreCAD either writes a record it said it dropped or
// reports a loss that never happened. Both paths ask variantCarriesRecord, and
// this feeds the same entity to both.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, WhatIsSkippedIsWhatIsReported) {
    const char* const preR13[] = {"dxf.ac1009.ascii", "dxf.ac1009.binary"};
    const char* const r13AndLater[] = {
        "dxf.ac1014.ascii", "dxf.ac1015.ascii", "dxf.ac1018.ascii",
        "dxf.ac1021.ascii", "dxf.ac1024.ascii", "dxf.ac1027.ascii",
        "dxf.ac1027.binary"};
    const char* const absentBeforeR13[] = {"SPLINE", "MTEXT", "HATCH", "IMAGE"};

    for (const char* variantId : preR13) {
        for (const char* kind : absentBeforeR13) {
            EXPECT_FALSE(lc::persistence::variantCarriesRecord(variantId, kind))
                << variantId << " cannot carry " << kind;
        }
        // Not on the list: R12 has no LWPOLYLINE either, but LibreCAD writes
        // POLYLINE for it rather than dropping it.
        EXPECT_TRUE(lc::persistence::variantCarriesRecord(variantId, "LWPOLYLINE"));
        EXPECT_TRUE(lc::persistence::variantCarriesRecord(variantId, "LINE"));
    }

    for (const char* variantId : r13AndLater) {
        for (const char* kind : absentBeforeR13) {
            EXPECT_TRUE(lc::persistence::variantCarriesRecord(variantId, kind))
                << variantId << " carries " << kind;
        }
    }

    // An unknown record kind is carried, not dropped: a new entity type is the
    // library's to judge, and silently dropping it here would be invisible.
    EXPECT_TRUE(lc::persistence::variantCarriesRecord("dxf.ac1009.ascii", "WIPEOUT"));
    // An unknown variant carries nothing, because nothing is known about it.
    EXPECT_FALSE(lc::persistence::variantCarriesRecord("dxf.nope", "LINE"));

    // And the rule as applied: the same spline, at both revisions.
    auto layer = defaultLayer();
    auto doc = newDocument();
    insertThroughBuilder(doc, {std::make_shared<lc::entity::Spline>(
        std::vector<lc::geo::Coordinate>{{0.0, 0.0}, {3.0, 8.0}, {7.0, -4.0}, {10.0, 0.0}},
        std::vector<double>{0, 0, 0, 0, 1, 1, 1, 1},
        std::vector<lc::geo::Coordinate>{},
        3, false, 0.0,
        0, 0, 0, 0, 0, 0, 0, 0, 1,
        lc::geo::Spline::splineflag::PERIODIC, layer)});

    const std::string r12 = uniqueTmpDxf("mirror-r12");
    const std::string r2000 = uniqueTmpDxf("mirror-r2000");
    boost::filesystem::remove(r12);
    boost::filesystem::remove(r2000);

    const auto dropped = lc::persistence::File::exportFile(
        doc, r12, lc::persistence::File::LIBDXFRW_DXF_R12);
    const auto kept = lc::persistence::File::exportFile(
        doc, r2000, lc::persistence::File::LIBDXFRW_DXF_R2000);

    EXPECT_EQ(dropped.loss.droppedByType.count("SPLINE"), 1u);
    EXPECT_TRUE(kept.loss.empty());

    boost::filesystem::remove(r12);
    boost::filesystem::remove(r2000);
}

// File::open is re-entrant, and has been for as long as hatches have had
// patterns: reading a non-solid HATCH calls PatternProvider, which opens the
// pattern's own DXF through File::open while the outer read is still in
// progress. Anything File::open keeps in a member, a static, or a shared
// builder has to survive that, which is why importFile builds its reader and
// its operation::Builder per call.
//
// It is also the caller that reaches getAvailableLibrariesForFormat's empty-map
// case first if DXF ever loses its reader.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, ReadingAPatternedHatchReentersOpen) {
    const std::string path = uniqueTmpDxf("patterned-hatch");
    boost::filesystem::remove(path);
    {
        // A square boundary of four LINE edges, hatched with a named pattern
        // that ships in res/hatch -- so the nested read has a real file.
        std::ofstream dxf(path);
        dxf << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1015\n0\nENDSEC\n"
            << "0\nSECTION\n2\nENTITIES\n"
            << "0\nHATCH\n8\n0\n100\nAcDbHatch\n10\n0.0\n20\n0.0\n30\n0.0\n"
            << "210\n0.0\n220\n0.0\n230\n1.0\n"
            << "2\nANSI31\n70\n0\n71\n0\n91\n1\n"
            << "92\n0\n93\n4\n"
            << "72\n1\n10\n0.0\n20\n0.0\n11\n10.0\n21\n0.0\n"
            << "72\n1\n10\n10.0\n20\n0.0\n11\n10.0\n21\n10.0\n"
            << "72\n1\n10\n10.0\n20\n10.0\n11\n0.0\n21\n10.0\n"
            << "72\n1\n10\n0.0\n20\n10.0\n11\n0.0\n21\n0.0\n"
            << "97\n0\n"
            << "75\n0\n76\n1\n52\n0.0\n41\n1.0\n77\n0\n78\n0\n47\n0.1\n98\n0\n"
            << "0\nENDSEC\n0\nEOF\n";
    }

    auto doc = newDocument();
    lc::persistence::ImportResult result;
    ASSERT_NO_THROW(result = lc::persistence::File::importFile(
        doc, path, lc::persistence::File::Library::LIBDXFRW));

    EXPECT_TRUE(result.ok) << "The nested read must not disturb the outer one.";
    EXPECT_FALSE(result.partial);
    EXPECT_EQ(result.entitiesDelivered, 1u)
        << "The pattern's own entities belong to the pattern, not to this drawing.";

    lc::entity::Hatch_CSPtr hatch;
    for (const auto& entity : doc->entityContainer().asVector()) {
        if (auto found = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity)) {
            hatch = found;
        }
    }
    ASSERT_NE(hatch, nullptr);
    EXPECT_EQ(hatch->getPatternName(), "ANSI31");
    EXPECT_FALSE(hatch->isSolid());
    EXPECT_EQ(hatch->getRegion().loopList().size(), 1u);

    boost::filesystem::remove(path);
}

// The read-callback guard.
//
// libdxfrw's read path is not a function-try-block the way its write path is,
// so an exception from a callback leaves dxfRW::read() reporting
// getError() == BAD_NONE -- the caller is told the file read cleanly while the
// process is already unwinding. Every read callback now runs inside
// runGuarded(), and DXFimpl records the record kind, handle, layer and block
// before carrying on with the next record.
//
// The mechanism is tested here rather than through a poisoned file because
// there is no longer a DXF record that reaches a throw: the three kernel
// crashes are fixed and degenerate records are skipped before they reach a
// constructor. That is exactly the ordering the plan insisted on -- this guard
// must not land while those crashes are live, or it turns them into silent
// holes. That every callback is actually wrapped is checked by
// scripts/ci/check-read-callbacks.sh, which no test can do.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, AReadCallbackThatThrowsIsRecordedNotFatal) {
    std::string reason;

    // The ordinary case.
    EXPECT_FALSE(lc::persistence::runGuarded(
        [] { throw std::runtime_error("bad line"); }, reason));
    EXPECT_EQ(reason, "bad line");

    // A bare const char*, which is what lc::geo::Area threw for years and what
    // a `catch (const std::exception&)` walks straight past. This is the
    // assertion that matters: the obvious handler would have been wrong.
    reason.clear();
    EXPECT_FALSE(lc::persistence::runGuarded(
        [] { throw "points describe a volume, not a area."; }, reason));
    EXPECT_EQ(reason, "points describe a volume, not a area.");

    // Something else entirely still has to be survivable, with a reason the
    // user can be shown.
    reason.clear();
    EXPECT_FALSE(lc::persistence::runGuarded([] { throw 42; }, reason));
    EXPECT_FALSE(reason.empty());

    // A callback that does not throw reports success and leaves reason alone.
    reason = "untouched";
    bool ran = false;
    EXPECT_TRUE(lc::persistence::runGuarded([&] { ran = true; }, reason));
    EXPECT_TRUE(ran);
    EXPECT_EQ(reason, "untouched");
}

// And the whole corpus of fixtures still reads with nothing recorded: the guard
// must not be quietly swallowing work that used to succeed.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, GuardedReadsRecordNothingForGoodFiles) {
    for (const char* name : {"oracle_r2000.dxf", "oracle_r12.dxf"}) {
        auto doc = newDocument();
        const auto result = lc::persistence::File::importFile(
            doc, fixture(name), lc::persistence::File::Library::LIBDXFRW);

        EXPECT_TRUE(result.ok) << name;
        EXPECT_TRUE(result.failures.empty()) << name;
    }
}


// Binary DXF is the same drawing, not a lossy sibling.
//
// The encodings differ in how a group code's value is laid out, which is
// exactly the kind of difference that truncates or extends a string by one
// character without anyone noticing -- the DXF text records were the suspected
// case. TEXT and MTEXT are therefore compared character for character rather
// than by presence.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, BinaryAndAsciiCarryTheSameText) {
    auto layer = defaultLayer();
    const std::string plain = "plain text";
    const std::string multi = "multi line text";

    auto doc = newDocument();
    ASSERT_NO_THROW(insertThroughBuilder(doc, {
        std::make_shared<lc::entity::Text>(
            lc::geo::Coordinate(1.0, 9.0, 0.0), plain, 0.5, 0.0, "STANDARD",
            lc::TextConst::DrawingDirection::None, lc::TextConst::HAlign::HALeft,
            lc::TextConst::VAlign::VABaseline, layer),
        std::make_shared<lc::entity::MText>(
            lc::geo::Coordinate(1.0, 7.0, 0.0), multi, 0.5, 0.0, "STANDARD",
            lc::TextConst::DrawingDirection::None, lc::TextConst::HAlign::HALeft,
            lc::TextConst::VAlign::VABaseline, false, false, false, false, layer)}));

    const struct {
        lc::persistence::File::Type type;
        const char* label;
    } targets[] = {
        {lc::persistence::File::LIBDXFRW_DXF_R2000, "R2000 ASCII"},
        {lc::persistence::File::LIBDXFRW_DXB_R2000, "R2000 binary"},
        {lc::persistence::File::LIBDXFRW_DXF_R2013, "R2013 ASCII"},
        {lc::persistence::File::LIBDXFRW_DXB_R2013, "R2013 binary"},
    };

    for (const auto& target : targets) {
        const std::string path = uniqueTmpDxf(target.label);
        boost::filesystem::remove(path);

        ASSERT_TRUE(lc::persistence::File::save(doc, path, target.type)) << target.label;

        auto reopened = newDocument();
        ASSERT_NO_THROW(lc::persistence::File::open(
            reopened, path, lc::persistence::File::Library::LIBDXFRW)) << target.label;

        int texts = 0;
        int mtexts = 0;
        for (const auto& entity : reopened->entityContainer().asVector()) {
            if (auto mtext = std::dynamic_pointer_cast<const lc::entity::MText>(entity)) {
                mtexts++;
                EXPECT_EQ(mtext->text_value(), multi)
                    << target.label << " changed the MTEXT, length "
                    << mtext->text_value().size() << " instead of " << multi.size();
            } else if (auto text = std::dynamic_pointer_cast<const lc::entity::Text>(entity)) {
                texts++;
                EXPECT_EQ(text->text_value(), plain) << target.label;
            }
        }

        EXPECT_EQ(texts, 1) << target.label;
        EXPECT_EQ(mtexts, 1) << target.label;

        boost::filesystem::remove(path);
    }
}

// A block's attributes are the values filled into its placeholders -- a title
// block's drawing number, a door's width. They were dropped in silence, so an
// attributed drawing came back with its labels missing and looked merely wrong
// rather than incomplete.
//
// LibreCAD has no attribute model: nothing ties a value to the INSERT it
// belongs to, or keeps its tag. What it has is text at a position, which is
// what an ATTRIB draws. So the values arrive as Text, the tag does not survive,
// and the import says so rather than leaving it to be discovered on the next
// save. This test pins that bargain in both directions -- what is kept and what
// is not -- so that a later attribute model changes a stated contract rather
// than an accident.
//
// The fixture is authored here, not taken from a corpus file, so the expected
// tags and values are known independently of any reader.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, AttributedInsertSurfacesItsValues) {
    auto doc = newDocument();
    lc::persistence::ImportResult result;
    ASSERT_NO_THROW(result = lc::persistence::File::importFile(
        doc, fixture("attributed_insert.dxf"), lc::persistence::File::Library::LIBDXFRW));

    ASSERT_TRUE(result.ok);

    std::vector<std::string> values;
    int inserts = 0;
    for (const auto& entity : doc->entityContainer().asVector()) {
        if (auto text = std::dynamic_pointer_cast<const lc::entity::Text>(entity)) {
            values.push_back(text->text_value());
        }
        if (std::dynamic_pointer_cast<const lc::entity::Insert>(entity)) {
            inserts++;
        }
    }
    std::sort(values.begin(), values.end());

    EXPECT_EQ(inserts, 1);
    ASSERT_EQ(values.size(), 2u) << "Both visible attributes must arrive.";
    EXPECT_EQ(values[0], "C");
    EXPECT_EQ(values[1], "LC-2026-014");

    // The invisible one is not drawn by AutoCAD either, so it is counted rather
    // than placed on the drawing.
    EXPECT_EQ(result.loss.droppedByType.at("ATTRIB (invisible)"), 1u);

    // And the user is told what the import could not model, once.
    ASSERT_EQ(result.loss.notes.size(), 1u);
    EXPECT_NE(result.loss.notes.front().find("tags are not kept"), std::string::npos)
        << result.loss.notes.front();

    // Round trip: the values survive, as text. The tags do not -- that is the
    // half LibreCAD cannot yet carry, and asserting it keeps the claim honest.
    const std::string path = uniqueTmpDxf("attributed");
    boost::filesystem::remove(path);
    ASSERT_TRUE(lc::persistence::File::save(
        doc, path, lc::persistence::File::LIBDXFRW_DXF_R2000));

    auto reopened = newDocument();
    ASSERT_NO_THROW(lc::persistence::File::open(
        reopened, path, lc::persistence::File::Library::LIBDXFRW));

    std::vector<std::string> again;
    for (const auto& entity : reopened->entityContainer().asVector()) {
        if (auto text = std::dynamic_pointer_cast<const lc::entity::Text>(entity)) {
            again.push_back(text->text_value());
        }
    }
    std::sort(again.begin(), again.end());
    EXPECT_EQ(again, values) << "The attribute values must survive a save and reopen.";

    std::ifstream written(path);
    const std::string body((std::istreambuf_iterator<char>(written)),
                           std::istreambuf_iterator<char>());
    EXPECT_EQ(body.find("\nATTRIB\n"), std::string::npos)
        << "Without an attribute model there is nothing to write an ATTRIB from; "
           "they go back as TEXT, which is what the import note warns about.";

    boost::filesystem::remove(path);
}

namespace {

/** Every record in one section of a DXF file, by type name. */
std::map<std::string, std::size_t> recordsInSection(const std::string& path,
                                                    const std::string& section) {
    std::map<std::string, std::size_t> census;
    std::ifstream file(path);
    std::string code;
    std::string value;
    bool inSection = false;

    while (std::getline(file, code) && std::getline(file, value)) {
        while (!value.empty() && (value.back() == '\r' || value.back() == ' ')) {
            value.pop_back();
        }
        if (code.find_first_not_of(" \t0") != std::string::npos || code.find('0') == std::string::npos) {
            continue;
        }

        if (value == "SECTION") {
            std::string sectionCode;
            std::string sectionName;
            if (std::getline(file, sectionCode) && std::getline(file, sectionName)) {
                while (!sectionName.empty()
                       && (sectionName.back() == '\r' || sectionName.back() == ' ')) {
                    sectionName.pop_back();
                }
                inSection = sectionName == section;
            }
            continue;
        }
        if (value == "ENDSEC" || value == "EOF") {
            inSection = false;
            continue;
        }

        if (inSection) {
            census[value]++;
        }
    }

    return census;
}

/** Every code-5 handle in a file, with how many times it appears. */
std::map<std::string, std::size_t> handlesInFile(const std::string& path) {
    std::map<std::string, std::size_t> handles;
    std::ifstream file(path);
    std::string code;
    std::string value;

    while (std::getline(file, code) && std::getline(file, value)) {
        while (!value.empty() && (value.back() == '\r' || value.back() == ' ')) {
            value.pop_back();
        }
        const auto trimmed = code.find_first_not_of(" \t");
        if (trimmed != std::string::npos && code.substr(trimmed) == "5") {
            for (auto& c : value) {
                c = static_cast<char>(::toupper(static_cast<unsigned char>(c)));
            }
            handles[value]++;
        }
    }

    return handles;
}

}  // namespace

// A DXF holds far more than geometry: layouts, plot settings, table styles,
// dictionaries, and whatever a vertical application stored under its own class.
// LibreCAD reads none of it and wrote none of it back, so opening a drawing and
// pressing Save destroyed every one of those records -- silently, while the
// drawing still looked right, which is what made it dangerous. Across the
// review corpus that is 4,931 OBJECTS records over 25 files.
//
// libdxfrw hands them over verbatim and takes them back; nothing was asking.
//
// NOLINTNEXTLINE(readability-identifier-naming)
// The ENTITIES half of the same promise. These are a separate code path from
// the OBJECTS records above -- they arrive through addRawDxfEntity and are
// replayed from writeEntities() -- and they were dropped silently while their
// handles were still being reserved, so the write looked like it was putting
// them back.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, UnmodelledEntitiesSurviveASave) {
    const std::string source = fixture("raw_entities.dxf");
    ASSERT_TRUE(boost::filesystem::exists(source));

    const auto before = recordsInSection(source, "ENTITIES");
    ASSERT_EQ(before.at("GEOPOSITIONMARKER"), 1u) << "the fixture changed";
    ASSERT_EQ(before.at("LINE"), 1u) << "the fixture changed";

    auto doc = newDocument();
    const auto result = lc::persistence::File::importFile(
        doc, source, lc::persistence::File::Library::LIBDXFRW);
    ASSERT_TRUE(result.ok);

    lc::persistence::File::Type type = lc::persistence::File::LIBDXFRW_DXF_R2000;
    ASSERT_TRUE(lc::persistence::File::typeForVariantId(result.variantId, type));

    const std::string saved = uniqueTmpDxf("raw-entities");
    boost::filesystem::remove(saved);
    const auto written = lc::persistence::File::exportFile(doc, saved, type);
    ASSERT_TRUE(written.ok);
    ASSERT_TRUE(boost::filesystem::exists(saved));

    auto after = recordsInSection(saved, "ENTITIES");
    EXPECT_EQ(after["GEOPOSITIONMARKER"], 1u)
        << "An ENTITIES record LibreCAD does not model was destroyed by a save.";
    EXPECT_EQ(after["LINE"], 1u) << "The modelled entity must still be there too.";

    // Same invariant the OBJECTS case pins: a preserved handle re-emitted beside
    // freshly minted ones must not collide.
    for (const auto& handle : handlesInFile(saved)) {
        EXPECT_EQ(handle.second, 1u) << "handle " << handle.first << " is used twice";
    }

    boost::filesystem::remove(saved);
}

TEST(DxfRoundTripTest, UnmodelledRecordsSurviveASave) {
    const std::string source = fixture("raw_objects.dxf");
    ASSERT_TRUE(boost::filesystem::exists(source));

    const auto before = recordsInSection(source, "OBJECTS");
    ASSERT_EQ(before.at("DICTIONARYVAR"), 1u) << "the fixture changed";
    ASSERT_EQ(before.at("ACDBPLACEHOLDER"), 1u) << "the fixture changed";

    auto doc = newDocument();
    const auto result = lc::persistence::File::importFile(
        doc, source, lc::persistence::File::Library::LIBDXFRW);
    ASSERT_TRUE(result.ok);

    lc::persistence::File::Type type = lc::persistence::File::LIBDXFRW_DXF_R2000;
    ASSERT_TRUE(lc::persistence::File::typeForVariantId(result.variantId, type));

    const std::string saved = uniqueTmpDxf("raw-objects");
    boost::filesystem::remove(saved);
    const auto written = lc::persistence::File::exportFile(doc, saved, type);
    ASSERT_TRUE(written.ok);
    ASSERT_TRUE(boost::filesystem::exists(saved));

    auto after = recordsInSection(saved, "OBJECTS");
    EXPECT_EQ(after["DICTIONARYVAR"], 1u) << "A record LibreCAD does not model was destroyed.";
    EXPECT_EQ(after["ACDBPLACEHOLDER"], 1u) << "A record LibreCAD does not model was destroyed.";
    EXPECT_GE(after["DICTIONARY"], before.at("DICTIONARY"))
        << "The source's dictionaries must all still be there.";

    // Handles are what every reference in the file is resolved through. Two
    // records sharing one is a corrupt file, and re-emitting preserved handles
    // beside freshly minted ones is exactly how that would happen.
    for (const auto& handle : handlesInFile(saved)) {
        EXPECT_EQ(handle.second, 1u) << "handle " << handle.first << " is used twice";
    }

    boost::filesystem::remove(saved);
}

// Replay is attempted only where it is faithful. A different revision, or a
// binary target, and the records are dropped rather than written somewhere they
// have no defined meaning -- and the count reaches the user either way.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, UnmodelledRecordsAreNotForcedIntoAnotherRevision) {
    auto doc = newDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(
        doc, fixture("raw_objects.dxf"), lc::persistence::File::Library::LIBDXFRW).ok);

    const struct {
        lc::persistence::File::Type type;
        const char* label;
    } elsewhere[] = {
        {lc::persistence::File::LIBDXFRW_DXF_R12, "an older revision"},
        {lc::persistence::File::LIBDXFRW_DXF_R2013, "a newer revision"},
        {lc::persistence::File::LIBDXFRW_DXB_R2000, "the same revision, binary"},
    };

    for (const auto& target : elsewhere) {
        const std::string path = uniqueTmpDxf(target.label);
        boost::filesystem::remove(path);

        const auto written = lc::persistence::File::exportFile(doc, path, target.type);
        ASSERT_TRUE(written.ok) << target.label << ": the drawing itself must still be written";
        EXPECT_EQ(written.loss.droppedByType.count("unmodelled records"), 1u)
            << target.label << ": dropping them silently is what this replaces";

        if (!lc::persistence::File::isBinaryType(target.type)) {
            const auto after = recordsInSection(path, "OBJECTS");
            EXPECT_EQ(after.count("DICTIONARYVAR"), 0u)
                << target.label << ": a record with no meaning here must not be written";
        }

        boost::filesystem::remove(path);
    }
}

// A SOLID is a filled triangle or quadrilateral, and LibreCAD had no entity for
// one: 33 of them in a single corpus file, dropped. It has a solid-filled
// Hatch, which draws the same thing.
//
// The corner order is the part worth a test. DXF numbers a SOLID's points 10,
// 11, 12, 13 in a bow-tie, so the boundary runs first, second, fourth, third.
// Taking them in numeric order yields a crossed quadrilateral -- still four
// edges, still one loop, still every count a census would check, and visibly
// wrong on screen.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, SolidArrivesAsTheShapeItDraws) {
    const std::string path = uniqueTmpDxf("solid");
    boost::filesystem::remove(path);
    {
        // A unit square: (0,0) (10,0) as the first edge, then (0,10) (10,10) in
        // DXF's third/fourth slots.
        std::ofstream dxf(path);
        dxf << "0\nSECTION\n2\nENTITIES\n"
            << "0\nSOLID\n8\n0\n"
            << "10\n0.0\n20\n0.0\n30\n0.0\n"
            << "11\n10.0\n21\n0.0\n31\n0.0\n"
            << "12\n0.0\n22\n10.0\n32\n0.0\n"
            << "13\n10.0\n23\n10.0\n33\n0.0\n"
            << "0\nENDSEC\n0\nEOF\n";
    }

    auto doc = newDocument();
    const auto result = lc::persistence::File::importFile(
        doc, path, lc::persistence::File::Library::LIBDXFRW);
    ASSERT_TRUE(result.ok);
    EXPECT_EQ(result.loss.droppedByType.count("SOLID"), 0u) << "The SOLID was dropped again.";

    lc::entity::Hatch_CSPtr hatch;
    for (const auto& entity : doc->entityContainer().asVector()) {
        if (auto found = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity)) {
            hatch = found;
        }
    }
    ASSERT_NE(hatch, nullptr) << "A SOLID must arrive as something drawable.";
    EXPECT_TRUE(hatch->isSolid());
    ASSERT_EQ(hatch->getRegion().loopList().size(), 1u);

    const auto& edges = hatch->getRegion().loopList().front().entities();
    ASSERT_EQ(edges.size(), 4u);

    // first -> second -> fourth -> third -> first, which is the square. In
    // numeric order it would be first -> second -> third -> fourth, whose
    // second edge runs diagonally from (10,0) to (0,10).
    const lc::geo::Coordinate expected[4][2] = {
        {{0.0, 0.0}, {10.0, 0.0}},
        {{10.0, 0.0}, {10.0, 10.0}},
        {{10.0, 10.0}, {0.0, 10.0}},
        {{0.0, 10.0}, {0.0, 0.0}},
    };

    for (std::size_t i = 0; i < 4; i++) {
        auto line = std::dynamic_pointer_cast<const lc::entity::Line>(edges[i]);
        ASSERT_NE(line, nullptr) << "edge " << i;
        EXPECT_DOUBLE_EQ(line->start().x(), expected[i][0].x()) << "edge " << i;
        EXPECT_DOUBLE_EQ(line->start().y(), expected[i][0].y()) << "edge " << i;
        EXPECT_DOUBLE_EQ(line->end().x(), expected[i][1].x()) << "edge " << i;
        EXPECT_DOUBLE_EQ(line->end().y(), expected[i][1].y()) << "edge " << i;
    }

    boost::filesystem::remove(path);
}

// A triangular SOLID repeats its last corner, which would otherwise become a
// zero-length edge and a degenerate boundary.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, TriangularSolidLosesItsRepeatedCorner) {
    const std::string path = uniqueTmpDxf("solid-triangle");
    boost::filesystem::remove(path);
    {
        std::ofstream dxf(path);
        dxf << "0\nSECTION\n2\nENTITIES\n"
            << "0\nSOLID\n8\n0\n"
            << "10\n0.0\n20\n0.0\n30\n0.0\n"
            << "11\n10.0\n21\n0.0\n31\n0.0\n"
            << "12\n5.0\n22\n8.0\n32\n0.0\n"
            << "13\n5.0\n23\n8.0\n33\n0.0\n"
            << "0\nENDSEC\n0\nEOF\n";
    }

    auto doc = newDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(
        doc, path, lc::persistence::File::Library::LIBDXFRW).ok);

    lc::entity::Hatch_CSPtr hatch;
    for (const auto& entity : doc->entityContainer().asVector()) {
        if (auto found = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity)) {
            hatch = found;
        }
    }
    ASSERT_NE(hatch, nullptr);
    ASSERT_EQ(hatch->getRegion().loopList().size(), 1u);
    EXPECT_EQ(hatch->getRegion().loopList().front().entities().size(), 3u)
        << "A triangle has three edges, not four with one of zero length.";

    boost::filesystem::remove(path);
}

// A drawing's units are a property of the drawing, and lckernel has nowhere to
// keep them: no units, no measurement system, no global line-type scale. So a
// drawing in inches was read, written back as millimetres, and silently
// rescaled for whoever opened it next -- with nothing on screen to say
// LibreCAD had done it.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, DrawingUnitsSurviveASave) {
    const struct {
        int insUnits;
        int measurement;
        const char* what;
    } cases[] = {
        {1, 0, "inches, imperial"},
        {4, 1, "millimetres, metric"},
        {6, 1, "metres, metric"},
    };

    for (const auto& c : cases) {
        const std::string source = uniqueTmpDxf(c.what);
        const std::string resaved = uniqueTmpDxf((std::string(c.what) + "-again").c_str());
        boost::filesystem::remove(source);
        boost::filesystem::remove(resaved);

        {
            std::ofstream dxf(source);
            dxf << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1015\n"
                << "9\n$INSUNITS\n70\n" << c.insUnits << "\n"
                << "9\n$MEASUREMENT\n70\n" << c.measurement << "\n"
                << "0\nENDSEC\n"
                << "0\nSECTION\n2\nENTITIES\n"
                << "0\nLINE\n8\n0\n10\n0.0\n20\n0.0\n30\n0.0\n11\n1.0\n21\n1.0\n31\n0.0\n"
                << "0\nENDSEC\n0\nEOF\n";
        }

        auto doc = newDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(
            doc, source, lc::persistence::File::Library::LIBDXFRW).ok) << c.what;
        ASSERT_TRUE(lc::persistence::File::save(
            doc, resaved, lc::persistence::File::LIBDXFRW_DXF_R2000)) << c.what;

        // Read the header back out of the file rather than through the
        // importer, so this cannot pass by agreeing with itself.
        std::ifstream written(resaved);
        std::string code;
        std::string value;
        std::string pending;
        int insUnits = -1;
        int measurement = -1;
        while (std::getline(written, code) && std::getline(written, value)) {
            while (!value.empty() && (value.back() == '\r' || value.back() == ' ')) {
                value.pop_back();
            }
            const auto trimmed = code.find_first_not_of(" \t");
            const std::string groupCode = trimmed == std::string::npos ? code : code.substr(trimmed);

            if (groupCode == "9") {
                pending = value;
            } else if (groupCode == "70" && pending == "$INSUNITS") {
                insUnits = std::stoi(value);
            } else if (groupCode == "70" && pending == "$MEASUREMENT") {
                measurement = std::stoi(value);
            }
        }

        EXPECT_EQ(insUnits, c.insUnits) << c.what << ": the drawing was rescaled by saving it";
        EXPECT_EQ(measurement, c.measurement) << c.what;

        boost::filesystem::remove(source);
        boost::filesystem::remove(resaved);
    }
}
