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
// do not link libdxfrw and cannot reach its headers
// for noexcept members.  Everything here goes through the public
// `lc::persistence::File` surface and the kernel.

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include <cad/const.h>

#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>

#include <cad/meta/color.h>
#include <cad/meta/metacolor.h>
#include <cad/meta/icolor.h>
#include <cad/meta/layer.h>
#include <cad/meta/metalinewidth.h>
#include <cad/meta/customentitystorage.h>

#include <cad/operations/builder.h>
#include <cad/operations/blockops.h>
#include <cad/operations/layerops.h>
#include <cad/operations/entitybuilder.h>

#include <cad/geometry/geoarea.h>
#include <cad/geometry/geocoordinate.h>
#include <cad/geometry/georegion.h>

#include <cad/primitive/dimension.h>
#include <cad/primitive/image.h>
#include <cad/primitive/dimlinear.h>
#include <cad/builders/dimlinear.h>
#include <cad/primitive/dimaligned.h>
#include <cad/primitive/dimradial.h>
#include <cad/primitive/dimdiametric.h>
#include <cad/primitive/dimangular.h>
#include <cad/primitive/dimordinate.h>
#include <cad/primitive/hatch.h>
#include <cad/primitive/line.h>
#include <cad/primitive/point.h>
#include <cad/primitive/insert.h>
#include <cad/primitive/mtext.h>
#include <cad/primitive/lwpolyline.h>
#include <cad/primitive/spline.h>

#include "persistence/file.h"
#include "persistence/documentsource.h"
#include "persistence/format.h"
#include "persistence/readguard.h"
#include "persistence/libdxfrw/dxfimpl.h"
#include "persistence/libdxfrw/preservedrecords.h"

namespace {

std::string uniqueTmpDxf(const char* tag) {
    return (boost::filesystem::temp_directory_path()
            / boost::filesystem::unique_path(
                std::string("lc3-roundtrip-") + tag + "-%%%%%%%%-%%%%%%%%.dxf"))
        .string();
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

TEST(DxfRoundTripTest, TransformedImageRetainsSpatialGeometry) {
    for (auto type : {lc::persistence::File::LIBDXFRW_DXF_R2010,
                      lc::persistence::File::LIBDXFRW_DXB_R2010}) {
        SCOPED_TRACE(static_cast<int>(type));
        const std::string path = uniqueTmpDxf("image-spatial");
        auto doc = newDocument();
        auto image = std::make_shared<lc::entity::Image>("scratch.png",
            lc::geo::Coordinate(10, 20, 30), lc::geo::Coordinate(1, 0, 0.5),
            lc::geo::Coordinate(0, 1, -0.25), 2, 3, 40, 60, 10, doc->layerByName("0"));
        auto transformed = std::dynamic_pointer_cast<const lc::entity::Image>(
            image->scale(lc::geo::Coordinate(), lc::geo::Coordinate(2, 3, 1)));
        ASSERT_TRUE(transformed);
        insertThroughBuilder(doc, {transformed});
        ASSERT_TRUE(lc::persistence::File::save(doc, path, type));
        auto reopened = newDocument();
        const auto result = lc::persistence::File::importFile(reopened, path, lc::persistence::File::LIBDXFRW);
        ASSERT_TRUE(result.ok);
        std::vector<std::shared_ptr<const lc::entity::Image>> images;
        for (const auto& entity : reopened->entityContainer().asVector())
            if (auto found = std::dynamic_pointer_cast<const lc::entity::Image>(entity)) images.push_back(found);
        ASSERT_EQ(images.size(), 1u);
        EXPECT_EQ(images[0]->base(), lc::geo::Coordinate(20, 60, 30));
        EXPECT_EQ(images[0]->uv(), lc::geo::Coordinate(2, 0, 0.5));
        EXPECT_EQ(images[0]->vv(), lc::geo::Coordinate(0, 3, -0.25));
        EXPECT_DOUBLE_EQ(images[0]->width(), 2); EXPECT_DOUBLE_EQ(images[0]->height(), 3);
        EXPECT_DOUBLE_EQ(images[0]->brightness(), 40); EXPECT_DOUBLE_EQ(images[0]->contrast(), 60);
        EXPECT_DOUBLE_EQ(images[0]->fade(), 10);
        EXPECT_EQ(images[0]->boundingBox().minP(), lc::geo::Coordinate(20, 60, 29.25));
        EXPECT_EQ(images[0]->boundingBox().maxP(), lc::geo::Coordinate(24, 69, 31));
        const std::string second = uniqueTmpDxf("image-spatial-resave");
        // Source IMAGE dictionary identity is still incomplete. A codec that
        // cannot replay this graph must report its raw-record loss instead.
        const auto resave = lc::persistence::File::exportFile(reopened, second, type);
        ASSERT_TRUE(resave.ok);
        EXPECT_TRUE(resave.loss.droppedByType.count("unreferenced dictionaries") ||
                    resave.loss.droppedByType.count("unmodelled records"));
        if (type == lc::persistence::File::LIBDXFRW_DXB_R2010) {
            const std::string ascii = uniqueTmpDxf("image-spatial-binary-to-ascii");
            const auto converted = lc::persistence::File::exportFile(reopened, ascii,
                lc::persistence::File::LIBDXFRW_DXF_R2010);
            ASSERT_TRUE(converted.ok);
            EXPECT_EQ(converted.loss.droppedByType.count("unmodelled records"), 0u);
            auto convertedDocument = newDocument();
            ASSERT_TRUE(lc::persistence::File::importFile(convertedDocument, ascii,
                lc::persistence::File::LIBDXFRW).ok);
            ASSERT_EQ(convertedDocument->entityContainer().asVector().size(), 1u);
            const auto convertedImage = std::dynamic_pointer_cast<const lc::entity::Image>(
                convertedDocument->entityContainer().asVector().front());
            ASSERT_TRUE(convertedImage);
            EXPECT_EQ(convertedImage->base(), images[0]->base());
            EXPECT_EQ(convertedImage->uv(), images[0]->uv());
            EXPECT_EQ(convertedImage->vv(), images[0]->vv());
            boost::filesystem::remove(ascii);
        }
        auto reread = newDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(reread, second, lc::persistence::File::LIBDXFRW).ok);
        std::vector<std::shared_ptr<const lc::entity::Image>> rereadImages;
        for (const auto& entity : reread->entityContainer().asVector())
            if (auto found = std::dynamic_pointer_cast<const lc::entity::Image>(entity)) rereadImages.push_back(found);
        ASSERT_EQ(rereadImages.size(), 1u);
        EXPECT_EQ(rereadImages[0]->base(), images[0]->base());
        EXPECT_EQ(rereadImages[0]->uv(), images[0]->uv());
        EXPECT_EQ(rereadImages[0]->vv(), images[0]->vv());
        boost::filesystem::remove(path);
        boost::filesystem::remove(second);
    }
}

// The filed task: "LibreCAD 3 crashes opening DXFs with 3D segments".
//
// A LINE whose endpoints differ in x AND y AND z, and an LWPOLYLINE
// containing such a segment, must both survive insertion, and the resulting
// bounding box must be the axis-aligned box of the endpoints — including z,
// which is the behaviour commit 4c7e438b chose when it deleted the throw
// rather than projecting the pair onto a plane.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, ExplicitTrueColorSurvivesImportEditAndExport) {
    const auto input = uniqueTmpDxf("true-color-source");
    { std::ofstream f(input); f << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1018\n0\nENDSEC\n"
        "0\nSECTION\n2\nENTITIES\n0\nLINE\n5\n55\n100\nAcDbEntity\n8\n0\n62\n1\n"
        "420\n13132850\n430\nTest$Copper\n100\nAcDbLine\n10\n1\n20\n2\n30\n0\n11\n4\n21\n6\n31\n0\n"
        "0\nENDSEC\n0\nEOF\n"; }
    auto doc = newDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(doc, input, lc::persistence::File::LIBDXFRW).ok);
    auto entities = doc->entityContainer().asVector(); ASSERT_EQ(entities.size(), 1u);
    auto color = entities[0]->metaInfo<lc::meta::MetaColorByValue>(lc::meta::MetaColor::LCMETANAME());
    ASSERT_TRUE(color); EXPECT_EQ(color->redI(), 200); EXPECT_EQ(color->greenI(), 100); EXPECT_EQ(color->blueI(), 50);
    auto moved = entities[0]->move(lc::geo::Coordinate(10, 0, 0)); ASSERT_TRUE(moved);
    doc->removeEntity(entities[0]); doc->insertEntity(moved);
    for (const auto type : {lc::persistence::File::LIBDXFRW_DXF_R2004,
                            lc::persistence::File::LIBDXFRW_DXB_R2004}) {
        const auto output = uniqueTmpDxf("true-color-output");
        const auto written = lc::persistence::File::exportFile(doc, output, type);
        ASSERT_TRUE(written.ok); EXPECT_TRUE(written.loss.notes.empty());
        auto reopened = newDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(reopened, output, lc::persistence::File::LIBDXFRW).ok);
        auto read = reopened->entityContainer().asVector(); ASSERT_EQ(read.size(), 1u);
        auto rgb = read[0]->metaInfo<lc::meta::MetaColorByValue>(lc::meta::MetaColor::LCMETANAME());
        ASSERT_TRUE(rgb); EXPECT_EQ(rgb->redI(), 200); EXPECT_EQ(rgb->greenI(), 100); EXPECT_EQ(rgb->blueI(), 50);
        const auto named = read[0]->metaInfo<lc::meta::MetaColorTrueColor>(lc::meta::MetaColor::LCMETANAME());
        ASSERT_TRUE(named); EXPECT_EQ(named->sourceAci(), 1); EXPECT_EQ(named->name(), "Test$Copper");
        auto line = std::dynamic_pointer_cast<const lc::entity::Line>(read[0]); ASSERT_TRUE(line);
        EXPECT_DOUBLE_EQ(line->start().x(), 11); EXPECT_DOUBLE_EQ(line->end().x(), 14);
        boost::filesystem::remove(output);
    }
    const auto legacy = uniqueTmpDxf("true-color-legacy");
    const auto converted = lc::persistence::File::exportFile(doc, legacy, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(converted.ok);
    EXPECT_TRUE(std::any_of(converted.loss.notes.begin(), converted.loss.notes.end(), [](const std::string& note) {
        return note.find("True colors were approximated") != std::string::npos;
    }));
    boost::filesystem::remove(legacy); boost::filesystem::remove(input);
}

TEST(DxfRoundTripTest, TrueBlackDowngradeDoesNotBecomeByBlock) {
    const auto input = uniqueTmpDxf("true-black-source"), output = uniqueTmpDxf("true-black-r2000");
    { std::ofstream f(input); f << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1018\n0\nENDSEC\n"
        "0\nSECTION\n2\nENTITIES\n0\nLINE\n5\n55\n100\nAcDbEntity\n8\n0\n62\n256\n420\n0\n"
        "100\nAcDbLine\n10\n1\n20\n2\n30\n0\n11\n4\n21\n6\n31\n0\n0\nENDSEC\n0\nEOF\n"; }
    auto doc = newDocument(); ASSERT_TRUE(lc::persistence::File::importFile(doc, input, lc::persistence::File::LIBDXFRW).ok);
    const auto result = lc::persistence::File::exportFile(doc, output, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(result.ok);
    std::ifstream f(output); ASSERT_TRUE(f.good());
    std::string code, value; bool line = false, found = false;
    while (std::getline(f, code) && std::getline(f, value)) {
        if (std::stoi(code) == 0) { if (line) break; line = value == "LINE"; }
        else if (line && std::stoi(code) == 62) { found = true; EXPECT_GT(std::stoi(value), 0); EXPECT_LE(std::stoi(value), 255); }
    }
    EXPECT_TRUE(found);
    f.close(); boost::filesystem::remove(input); boost::filesystem::remove(output);
}

TEST(DxfRoundTripTest, LayerTrueColorAndByLayerEntitiesSurviveRoundTrip) {
    const auto input = uniqueTmpDxf("layer-rgb-source");
    { std::ofstream f(input); f << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1018\n0\nENDSEC\n"
        "0\nSECTION\n2\nTABLES\n0\nTABLE\n2\nLAYER\n5\n2\n70\n2\n"
        "0\nLAYER\n5\n10\n100\nAcDbSymbolTableRecord\n100\nAcDbLayerTableRecord\n2\n0\n70\n0\n62\n1\n420\n13132850\n430\nTest$Copper\n6\nCONTINUOUS\n"
        "0\nLAYER\n5\n60\n100\nAcDbSymbolTableRecord\n100\nAcDbLayerTableRecord\n2\nCOLOR_LAYER\n70\n0\n62\n-3\n420\n0\n6\nCONTINUOUS\n"
        "0\nENDTAB\n0\nENDSEC\n0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n5\n70\n100\nAcDbEntity\n8\n0\n100\nAcDbLine\n10\n1\n20\n2\n30\n0\n11\n4\n21\n6\n31\n0\n"
        "0\nLINE\n5\n71\n100\nAcDbEntity\n8\ncolor_layer\n100\nAcDbLine\n10\n3\n20\n2\n30\n0\n11\n7\n21\n6\n31\n0\n"
        "0\nENDSEC\n0\nEOF\n"; }
    auto verify = [](const std::shared_ptr<lc::storage::DocumentImpl>& doc) {
        const auto zero = doc->layerByName("0"), black = doc->layerByName("COLOR_LAYER");
        ASSERT_TRUE(zero); ASSERT_TRUE(black); EXPECT_FALSE(zero->isOff()); EXPECT_TRUE(black->isOff());
        EXPECT_EQ(zero->color().redI(), 200); EXPECT_EQ(zero->color().greenI(), 100); EXPECT_EQ(zero->color().blueI(), 50);
        EXPECT_EQ(black->color().redI(), 0); EXPECT_EQ(black->color().greenI(), 0); EXPECT_EQ(black->color().blueI(), 0);
        const auto entities = doc->entityContainer().asVector(); ASSERT_EQ(entities.size(), 2u);
        for (const auto& e : entities) {
            const auto layer = doc->layerByName(e->layer()->name()); ASSERT_TRUE(layer);
            EXPECT_EQ(e->layer(), layer);
        }
    };
    auto doc = newDocument(); const auto imported = lc::persistence::File::importFile(doc,input,lc::persistence::File::LIBDXFRW);
    ASSERT_TRUE(imported.ok); verify(doc);
    for (const auto type : {lc::persistence::File::LIBDXFRW_DXF_R2004, lc::persistence::File::LIBDXFRW_DXB_R2004}) {
        const auto output=uniqueTmpDxf("layer-rgb-output");
        ASSERT_TRUE(lc::persistence::File::exportFile(doc,output,type).ok);
        auto reopened=newDocument(); const auto reread = lc::persistence::File::importFile(reopened,output,lc::persistence::File::LIBDXFRW);
        ASSERT_TRUE(reread.ok);
        verify(reopened); boost::filesystem::remove(output);
    }
    // Turn the imported off layer on and off through normal layer replacement.
    const auto offLayer = doc->layerByName("COLOR_LAYER"); ASSERT_TRUE(offLayer); ASSERT_TRUE(offLayer->isOff());
    auto onLayer = std::make_shared<lc::meta::Layer>(offLayer->name(), offLayer->lineWidth(), offLayer->color(),
                                                     offLayer->linePattern(), offLayer->isFrozen(), false);
    auto toggle = std::make_shared<lc::operation::Builder>(doc, "turn on");
    toggle->append(std::make_shared<lc::operation::ReplaceLayer>(doc, offLayer, onLayer)); toggle->execute();
    const auto onOutput = uniqueTmpDxf("layer-on-output");
    ASSERT_TRUE(lc::persistence::File::exportFile(doc, onOutput, lc::persistence::File::LIBDXFRW_DXF_R2004).ok);
    auto onDoc = newDocument(); ASSERT_TRUE(lc::persistence::File::importFile(onDoc, onOutput, lc::persistence::File::LIBDXFRW).ok);
    ASSERT_TRUE(onDoc->layerByName("COLOR_LAYER")); EXPECT_FALSE(onDoc->layerByName("COLOR_LAYER")->isOff());
    boost::filesystem::remove(onOutput);
    auto againOff = std::make_shared<lc::meta::Layer>(onLayer->name(), onLayer->lineWidth(), onLayer->color(),
                                                      onLayer->linePattern(), onLayer->isFrozen(), true);
    auto toggleAgain = std::make_shared<lc::operation::Builder>(doc, "turn off");
    toggleAgain->append(std::make_shared<lc::operation::ReplaceLayer>(doc, onLayer, againOff)); toggleAgain->execute();
    const auto legacy = uniqueTmpDxf("layer-rgb-legacy");
    const auto downgraded = lc::persistence::File::exportFile(doc, legacy, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(downgraded.ok);
    EXPECT_TRUE(std::any_of(downgraded.loss.notes.begin(), downgraded.loss.notes.end(), [](const std::string& note) {
        return note.find("Layer true colors or color names") != std::string::npos;
    }));
    auto oldDoc = newDocument(); ASSERT_TRUE(lc::persistence::File::importFile(oldDoc, legacy, lc::persistence::File::LIBDXFRW).ok);
    lc::iColor palette; auto nearestBlack = palette.intToColor(palette.colorToInt(lc::Color(0, 0, 0), 1));
    ASSERT_TRUE(nearestBlack); ASSERT_TRUE(oldDoc->layerByName("COLOR_LAYER"));
    auto oldColor = oldDoc->layerByName("COLOR_LAYER")->color(); EXPECT_TRUE(oldColor == nearestBlack->color());
    boost::filesystem::remove(legacy);

    // Replacing a layer with a new RGB must not reattach its stale source name.
    const auto previous = doc->layerByName("0"); ASSERT_TRUE(previous);
    const auto replacement = std::make_shared<lc::meta::Layer>("0", previous->lineWidth(), lc::Color(17, 34, 51), previous->linePattern(), previous->isFrozen());
    auto edit = std::make_shared<lc::operation::Builder>(doc, "change layer color");
    edit->append(std::make_shared<lc::operation::ReplaceLayer>(doc, previous, replacement)); edit->execute();
    const auto changed = uniqueTmpDxf("layer-rgb-changed");
    const auto written = lc::persistence::File::exportFile(doc, changed, lc::persistence::File::LIBDXFRW_DXF_R2004);
    ASSERT_TRUE(written.ok);
    EXPECT_TRUE(std::any_of(written.loss.notes.begin(), written.loss.notes.end(), [](const std::string& note) {
        return note.find("source color name") != std::string::npos;
    }));
    auto changedDoc = newDocument(); ASSERT_TRUE(lc::persistence::File::importFile(changedDoc, changed, lc::persistence::File::LIBDXFRW).ok);
    ASSERT_TRUE(changedDoc->layerByName("0"));
    EXPECT_EQ(changedDoc->layerByName("0")->color().redI(), 17);
    EXPECT_EQ(changedDoc->layerByName("0")->color().greenI(), 34);
    EXPECT_EQ(changedDoc->layerByName("0")->color().blueI(), 51);
    boost::filesystem::remove(changed); boost::filesystem::remove(input);
}

TEST(DxfRoundTripTest, MissingLayerUsesOneSharedFallback) {
    const auto input = uniqueTmpDxf("missing-layer");
    { std::ofstream f(input); f << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1018\n0\nENDSEC\n"
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLINE\n5\n70\n100\nAcDbEntity\n8\nMISSING\n100\nAcDbLine\n10\n1\n20\n2\n11\n4\n21\n6\n"
        "0\nLINE\n5\n71\n100\nAcDbEntity\n8\nmissing\n100\nAcDbLine\n10\n3\n20\n2\n11\n7\n21\n6\n"
        "0\nENDSEC\n0\nEOF\n"; }
    auto doc = newDocument(); ASSERT_TRUE(lc::persistence::File::importFile(doc, input, lc::persistence::File::LIBDXFRW).ok);
    const auto layer = doc->layerByName("MISSING"); ASSERT_TRUE(layer);
    const auto entities = doc->entityContainer().asVector(); ASSERT_EQ(entities.size(), 2u);
    for (const auto& entity : entities) EXPECT_EQ(entity->layer(), layer);
    boost::filesystem::remove(input);
}

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

        const auto exportResult = lc::persistence::File::exportFile(doc, path, type);
        const bool saved = exportResult.ok;

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
        if (type == lc::persistence::File::LIBDXFRW_DXF_R12 || type == lc::persistence::File::LIBDXFRW_DXF_R14 ||
            type == lc::persistence::File::LIBDXFRW_DXB_R12 || type == lc::persistence::File::LIBDXFRW_DXB_R14)
            EXPECT_EQ(exportResult.loss.droppedByType.count("layer lineweight"), 0u);

        std::ifstream written(path, std::ios::binary);
        const std::string head(
            (std::istreambuf_iterator<char>(written)), std::istreambuf_iterator<char>());
        written.close();
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
TEST(DxfRoundTripTest, MalformedGeometryRejectsImportAndProtectsSource) {
    using File = lc::persistence::File;
    struct RemoveFile {
        std::string path;
        ~RemoveFile() { boost::system::error_code ignored; boost::filesystem::remove(path, ignored); }
    };
    const std::string goodLine =
        "0\nLINE\n8\n0\n10\n1.0\n20\n2.0\n30\n0.0\n11\n7.0\n21\n11.0\n31\n0.0\n";

    // Retain upstream's strict validation: no invalid geometry is published,
    // and reaching an invalid record does not silently recover later records.
    // Exercise both radius failures independently, with/without a valid prefix.
    for (const char* kind : {"ARC", "CIRCLE"}) {
        for (const char* radius : {"0.0", "-5.0"}) {
            for (bool prefix : {false, true}) {
                SCOPED_TRACE(::testing::Message() << kind << " radius=" << radius << " prefix=" << prefix);
                const std::string path = uniqueTmpDxf("strict-radius");
                RemoveFile cleanup{path};
                const std::string payload =
                    "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1015\n0\nENDSEC\n"
                    "0\nSECTION\n2\nENTITIES\n" + (prefix ? goodLine : std::string())
                    + "0\n" + kind + "\n8\n0\n10\n0.0\n20\n0.0\n30\n0.0\n40\n" + radius
                    + "\n50\n0.0\n51\n90.0\n" + goodLine + "0\nENDSEC\n0\nEOF\n";
                { std::ofstream dxf(path, std::ios::binary); ASSERT_TRUE(dxf << payload); }

                auto doc = newDocument();
                lc::persistence::ImportResult result;
                ASSERT_NO_THROW(result = File::importFile(doc, path, File::LIBDXFRW));
                EXPECT_FALSE(result.ok);
                EXPECT_EQ(result.partial, prefix);
                EXPECT_EQ(result.entitiesDelivered, prefix ? 1u : 0u);
                const std::map<std::string, std::size_t> expected = prefix
                    ? std::map<std::string, std::size_t>{{"Line", 1}}
                    : std::map<std::string, std::size_t>{};
                EXPECT_EQ(entitiesInDocument(doc), expected);
                EXPECT_TRUE(result.failures.empty()) << "parser rejection precedes entity callbacks";
                EXPECT_EQ(result.variantId, "dxf.ac1015.ascii");
                ASSERT_FALSE(result.diagnostics.empty());
                EXPECT_EQ(result.diagnostics.front().severity, lc::persistence::Severity::Error);
                EXPECT_EQ(result.diagnostics.front().code, "code-parsed");
                EXPECT_FALSE(result.diagnostics.front().message.empty());

                const auto source = lc::persistence::sourceFromImport(path, result);
                EXPECT_TRUE(source.partial) << "even an empty failed import is unsafe to overwrite";
                const auto save = lc::persistence::decideSave(source);
                EXPECT_EQ(save.action, lc::persistence::SaveAction::AskForTarget);
                EXPECT_TRUE(save.path.empty());
                EXPECT_FALSE(save.reason.empty());

                auto legacy = newDocument();
                File::Type legacyType = File::LIBDXFRW_DXF_R12;
                ASSERT_NO_THROW(legacyType = File::open(legacy, path, File::LIBDXFRW));
                EXPECT_EQ(legacyType, File::LIBDXFRW_DXF_R2000);
                EXPECT_EQ(entitiesInDocument(legacy), expected);
                if (prefix) {
                    const std::string recoveredPath = uniqueTmpDxf("recovered-prefix");
                    RemoveFile recoveredCleanup{recoveredPath};
                    ASSERT_TRUE(File::exportFile(doc, recoveredPath, File::LIBDXFRW_DXF_R2000).ok);
                    auto recovered = newDocument();
                    ASSERT_TRUE(File::importFile(recovered, recoveredPath, File::LIBDXFRW).ok);
                    EXPECT_EQ(entitiesInDocument(recovered), expected);
                }
                std::ifstream original(path, std::ios::binary);
                const std::string bytes((std::istreambuf_iterator<char>(original)),
                                        std::istreambuf_iterator<char>());
                EXPECT_EQ(bytes, payload) << "import/recovery must leave the source byte-identical";
            }
        }
    }
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
        written.close();
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

// The historical raw-object controls start OBJECTS with an owned dictionary.
// Give that dictionary a real root in a temporary copy so the test exercises
// passthrough without relying on acceptance of a malformed root position.
struct RootedObjectFixture {
    std::string path = uniqueTmpDxf("rooted-objects");
    bool valid = false;
    explicit RootedObjectFixture(const char* name) {
        std::ifstream in(fixture(name), std::ios::binary);
        if (!in) return;
        std::string text((std::istreambuf_iterator<char>(in)), {});
        const std::string section = "0\nSECTION\n2\nOBJECTS\n";
        const auto sectionPos = text.find(section);
        if (sectionPos == std::string::npos) return;
        const auto start = sectionPos + section.size();
        const std::string first = "0\nDICTIONARY\n5\n";
        if (text.compare(start, first.size(), first) != 0) return;
        const auto handleStart = start + first.size();
        const auto handleEnd = text.find('\n', handleStart);
        const auto ownerCode = text.find("330\n", handleEnd);
        if (handleEnd == std::string::npos || ownerCode == std::string::npos) return;
        const auto ownerEnd = text.find('\n', ownerCode + 4);
        if (ownerEnd == std::string::npos) return;
        const auto handle = text.substr(handleStart, handleEnd - handleStart);
        text.replace(ownerCode + 4, ownerEnd - ownerCode - 4, "C");
        text.insert(start, "0\nDICTIONARY\n5\nC\n330\n0\n100\nAcDbDictionary\n281\n1\n3\nLC_ROOT\n350\n" + handle + "\n");
        std::ofstream out(path, std::ios::binary);
        out << text;
        valid = static_cast<bool>(out);
    }
    ~RootedObjectFixture() { boost::system::error_code ignored; boost::filesystem::remove(path, ignored); }
};

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
    const char* const absentBeforeR13[] = {"SPLINE", "MTEXT", "HATCH", "IMAGE",
                                           "DIMENSION"};

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
    // A real newline, and a backslash. The fixture used to be "multi line
    // text" -- spaces, not line breaks -- so the one test that compares MText
    // content character for character never exercised the paragraph-break
    // encoding it was named for, and never noticed that \P was written on the
    // way out and never read on the way back in.
    const std::string multi = "multi\nline C:\\text";

    auto doc = newDocument();
    ASSERT_NO_THROW(insertThroughBuilder(doc, {
        std::make_shared<lc::entity::Text>(
            lc::geo::Coordinate(1.0, 9.0, 0.0), plain, 0.5, 0.0, "STANDARD",
            lc::TextConst::DrawingDirection::None, lc::TextConst::HAlign::HALeft,
            lc::TextConst::VAlign::VABaseline, layer),
        std::make_shared<lc::entity::MText>(
            lc::geo::Coordinate(1.0, 7.0, 0.0), multi, 0.5, 0.0, "STANDARD",
            lc::TextConst::DrawingDirection::None, lc::TextConst::HAlign::HALeft,
            lc::TextConst::VAlign::VABaseline, false, false, false, false,
            /*width=*/0.0, lc::TextConst::MTextDrawingDirection::ByStyle,
            /*lineSpacingFactor=*/1.0, lc::TextConst::LineSpacingStyle::AtLeast,
            layer)}));

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
    written.close();
    EXPECT_EQ(body.find("\nATTRIB\n"), std::string::npos)
        << "Without an attribute model there is nothing to write an ATTRIB from; "
           "they go back as TEXT, which is what the import note warns about.";

    boost::filesystem::remove(path);
}

// R2018 multiline ATTRIB stores its value in an embedded MText object rather
// than DRW_Attrib::text. It must not become an empty Text when inserted.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, MultilineInsertAttributeRetainsItsLines) {
    auto doc = newDocument();
    const auto imported = lc::persistence::File::importFile(
        doc, fixture("multiline_attributed_insert.dxf"),
        lc::persistence::File::Library::LIBDXFRW);
    ASSERT_TRUE(imported.ok);

    std::vector<lc::entity::MText_CSPtr> values;
    for (const auto& entity : doc->entityContainer().asVector()) {
        if (auto text = std::dynamic_pointer_cast<const lc::entity::MText>(entity)) {
            values.push_back(text);
        }
    }
    ASSERT_EQ(values.size(), 1u) << "The embedded MText value must be delivered.";
    EXPECT_EQ(values.front()->text_value(), "FIRST LINE\nSECOND LINE");
    EXPECT_DOUBLE_EQ(values.front()->insertion_point().x(), 101.0);
    EXPECT_DOUBLE_EQ(values.front()->insertion_point().y(), 202.0);
    ASSERT_EQ(imported.loss.notes.size(), 1u);
    EXPECT_NE(imported.loss.notes.front().find("tags are not kept"), std::string::npos);

    const std::string path = uniqueTmpDxf("multiline-attributed-insert");
    boost::filesystem::remove(path);
    ASSERT_TRUE(lc::persistence::File::save(
        doc, path, lc::persistence::File::LIBDXFRW_DXF_R2000));

    auto reopened = newDocument();
    const auto roundTrip = lc::persistence::File::importFile(
        reopened, path, lc::persistence::File::Library::LIBDXFRW);
    ASSERT_TRUE(roundTrip.ok);
    std::vector<lc::entity::MText_CSPtr> reloadedValues;
    for (const auto& entity : reopened->entityContainer().asVector()) {
        if (auto text = std::dynamic_pointer_cast<const lc::entity::MText>(entity)) {
            reloadedValues.push_back(text);
        }
    }
    ASSERT_EQ(reloadedValues.size(), 1u);
    EXPECT_EQ(reloadedValues.front()->text_value(), values.front()->text_value());
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

/** The code-5 handle and code-330 owner of each named record, last one wins. */
struct RecordIdentity {
    std::string handle;
    std::string owner;
};
std::vector<RecordIdentity> identitiesOfType(const std::string& path, const std::string& type) {
    std::vector<RecordIdentity> identities;
    std::ifstream file(path);
    std::string code;
    std::string value;
    std::string record;
    RecordIdentity identity;

    while (std::getline(file, code) && std::getline(file, value)) {
        while (!value.empty() && (value.back() == '\r' || value.back() == ' ')) {
            value.pop_back();
        }
        const auto trimmed = code.find_first_not_of(" \t");
        const std::string group = trimmed == std::string::npos ? code : code.substr(trimmed);

        for (auto& c : value) {
            c = static_cast<char>(::toupper(static_cast<unsigned char>(c)));
        }

        if (group == "0") {
            if (record == type) identities.push_back(identity);
            record = value;
            identity = {};
        } else if (record != type) {
            continue;
        } else if (group == "5") {
            identity.handle = value;
        } else if (group == "330" && identity.owner.empty()) {
            identity.owner = value;
        }
    }
    if (record == type) identities.push_back(identity);
    return identities;
}


/** Every group of one record kind, as (code -> values) in file order. */
std::map<int, std::vector<std::string>> groupsOfRecord(const std::string& path,
                                                      const std::string& recordName) {
    std::map<int, std::vector<std::string>> groups;
    std::ifstream file(path);
    std::string code;
    std::string value;
    std::string record;

    while (std::getline(file, code) && std::getline(file, value)) {
        while (!value.empty() && (value.back() == '\r' || value.back() == ' ')) {
            value.pop_back();
        }
        const auto start = code.find_first_not_of(" \t");
        const std::string group = start == std::string::npos ? code : code.substr(start);

        if (group == "0") {
            record = value;
            continue;
        }
        if (record != recordName) {
            continue;
        }
        try {
            groups[std::stoi(group)].push_back(value);
        } catch (const std::exception&) {
            // not a numeric group code; nothing to record
        }
    }

    return groups;
}

}  // namespace

// Three group codes that every MTEXT LibreCAD_3 writes got wrong, and that no
// test looked at because the round trip reads them back through the same
// mistaken assumptions that wrote them. These assertions read the file.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, AnMTextIsWrittenWithValidGroupCodes) {
    auto doc = newDocument();
    ASSERT_NO_THROW(insertThroughBuilder(doc, {
        std::make_shared<lc::entity::MText>(
            lc::geo::Coordinate(1.0, 1.0, 0.0), "text", 100.0, 0.0, "STANDARD",
            lc::TextConst::DrawingDirection::None, lc::TextConst::HAlign::HALeft,
            lc::TextConst::VAlign::VATop, false, false, false, false,
            /*width=*/0.0, lc::TextConst::MTextDrawingDirection::ByStyle,
            /*lineSpacingFactor=*/1.0, lc::TextConst::LineSpacingStyle::AtLeast,
            defaultLayer())}));

    const std::string saved = uniqueTmpDxf("mtext-codes");
    boost::filesystem::remove(saved);
    ASSERT_TRUE(lc::persistence::File::save(
        doc, saved, lc::persistence::File::LIBDXFRW_DXF_R2000));

    const auto groups = groupsOfRecord(saved, "MTEXT");

    // 41 is the reference rectangle width. It used to come out as 1.0 --
    // DRW_Text's constructor default, emitted unconditionally -- which tells
    // every conforming reader to wrap this 100-unit-high text at one drawing
    // unit. Zero means "no reference rectangle".
    ASSERT_EQ(groups.count(41), 1u) << "libdxfrw always emits code 41";
    ASSERT_EQ(groups.at(41).size(), 1u);
    EXPECT_DOUBLE_EQ(std::stod(groups.at(41).front()), 0.0)
        << "a one-unit wrap column was declared on every MTEXT ever written";

    // 72 is the drawing direction: 1, 3 or 5 are the defined values. It used
    // to come out as 0 for every MText the dialog creates.
    ASSERT_EQ(groups.count(72), 1u);
    const int direction = std::stoi(groups.at(72).front());
    EXPECT_TRUE(direction == 1 || direction == 3 || direction == 5)
        << "code 72 = " << direction << " is not a defined drawing direction";
    EXPECT_EQ(direction, 5) << "a default MText means 'by style'";

    // And reopening must not reinterpret that as a mirrored MText, which is
    // what writing 1 here would have done.
    auto reopened = newDocument();
    ASSERT_NO_THROW(lc::persistence::File::open(
        reopened, saved, lc::persistence::File::Library::LIBDXFRW));
    lc::entity::MText_CSPtr readBack;
    for (const auto& entity : reopened->entityContainer().asVector()) {
        if (auto m = std::dynamic_pointer_cast<const lc::entity::MText>(entity)) {
            readBack = m;
        }
    }
    ASSERT_NE(readBack, nullptr);
    EXPECT_EQ(readBack->textgeneration(), lc::TextConst::DrawingDirection::None)
        << "the drawing direction changed by being written and read";

    boost::filesystem::remove(saved);
}

namespace {

/// The single MText in a document, or null.
lc::entity::MText_CSPtr onlyMText(
    const std::shared_ptr<lc::storage::DocumentImpl>& doc) {
    lc::entity::MText_CSPtr found;
    for (const auto& entity : doc->entityContainer().asVector()) {
        if (auto m = std::dynamic_pointer_cast<const lc::entity::MText>(entity)) {
            found = m;
        }
    }
    return found;
}

/// A one-MTEXT ENTITIES section, with whatever trailing groups are passed.
void writeOneMText(const std::string& path, const std::string& tail) {
    std::ofstream dxf(path);
    dxf << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1015\n0\nENDSEC\n"
        << "0\nSECTION\n2\nENTITIES\n"
        << "0\nMTEXT\n8\n0\n"
        << "10\n1.0\n20\n2.0\n30\n0.0\n"
        << "40\n2.5\n"
        << "1\ntext\n"
        << "71\n1\n"
        << tail
        << "0\nENDSEC\n0\nEOF\n";
}

}  // namespace

// Codes 41, 44, 72 and 73 arrived on every MTEXT libdxfrw handed over and were
// read by nothing: the entity had nowhere to keep them, so a file's reference
// rectangle and line spacing were discarded on open and replaced by a default
// on the next save.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, AnMTextReadsItsWidthSpacingAndDirection) {
    const std::string path = uniqueTmpDxf("mtext-fields");
    boost::filesystem::remove(path);
    writeOneMText(path, "41\n250.0\n72\n3\n73\n2\n44\n1.75\n");

    auto doc = newDocument();
    ASSERT_NO_THROW(lc::persistence::File::open(
        doc, path, lc::persistence::File::Library::LIBDXFRW));

    auto mtext = onlyMText(doc);
    ASSERT_NE(mtext, nullptr) << "the MTEXT did not arrive";

    EXPECT_DOUBLE_EQ(mtext->width(), 250.0) << "group 41, the reference rectangle";
    EXPECT_EQ(mtext->drawingDirection(),
              lc::TextConst::MTextDrawingDirection::TopToBottom) << "group 72";
    EXPECT_DOUBLE_EQ(mtext->lineSpacingFactor(), 1.75) << "group 44";
    EXPECT_EQ(mtext->lineSpacingStyle(), lc::TextConst::LineSpacingStyle::Exact)
        << "group 73 -- the enum numbers these 0 and 1, the file numbers them 1 and 2";

    boost::filesystem::remove(path);
}

// Real files do not stay inside the defined values. Across 150 drawings here,
// 456 of 877 MTEXT records carry group 72 = 2, which is not one of the three
// directions the specification defines, and 269 carry group 73 = 0, which is
// neither of its two styles. A cast would turn those into an entity claiming a
// direction no renderer has, so they take the documented default instead.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, AnMTextWithUndefinedGroupCodesTakesTheDefaults) {
    const std::string path = uniqueTmpDxf("mtext-undefined");
    boost::filesystem::remove(path);
    // 72 = 2 and 73 = 0 are the values the corpus actually carries; 44 = 9.0 is
    // outside the documented 0.25 to 4.
    writeOneMText(path, "41\n100.0\n72\n2\n73\n0\n44\n9.0\n");

    auto doc = newDocument();
    ASSERT_NO_THROW(lc::persistence::File::open(
        doc, path, lc::persistence::File::Library::LIBDXFRW));

    auto mtext = onlyMText(doc);
    ASSERT_NE(mtext, nullptr) << "the MTEXT did not arrive";

    EXPECT_EQ(mtext->drawingDirection(),
              lc::TextConst::MTextDrawingDirection::ByStyle)
        << "an undefined group 72 must not become an undefined direction";
    EXPECT_EQ(mtext->lineSpacingStyle(), lc::TextConst::LineSpacingStyle::AtLeast)
        << "an undefined group 73 takes the DXF default";
    EXPECT_DOUBLE_EQ(mtext->lineSpacingFactor(), 1.0)
        << "a spacing factor outside 0.25 to 4 takes the DXF default";
    EXPECT_DOUBLE_EQ(mtext->width(), 100.0) << "group 41 was in range and stands";

    boost::filesystem::remove(path);
}

// And the write side: what the entity carries is what the file says. Before
// this the writer emitted three constants -- 0, 5 and 1 -- so a drawing opened
// and saved came back with its reference rectangle and spacing replaced.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, AnMTextWritesTheWidthAndSpacingItCarries) {
    const std::string source = uniqueTmpDxf("mtext-fields-source");
    boost::filesystem::remove(source);
    writeOneMText(source, "41\n250.0\n72\n3\n73\n2\n44\n1.75\n");

    auto doc = newDocument();
    ASSERT_NO_THROW(lc::persistence::File::open(
        doc, source, lc::persistence::File::Library::LIBDXFRW));
    ASSERT_NE(onlyMText(doc), nullptr);

    const std::string saved = uniqueTmpDxf("mtext-fields-saved");
    boost::filesystem::remove(saved);
    ASSERT_TRUE(lc::persistence::File::save(
        doc, saved, lc::persistence::File::LIBDXFRW_DXF_R2000));

    const auto groups = groupsOfRecord(saved, "MTEXT");

    ASSERT_EQ(groups.count(41), 1u);
    EXPECT_DOUBLE_EQ(std::stod(groups.at(41).front()), 250.0)
        << "the reference rectangle was replaced on save";
    ASSERT_EQ(groups.count(72), 1u);
    EXPECT_EQ(std::stoi(groups.at(72).front()), 3) << "the drawing direction was replaced";
    ASSERT_EQ(groups.count(73), 1u);
    EXPECT_EQ(std::stoi(groups.at(73).front()), 2) << "the line spacing style was replaced";
    ASSERT_EQ(groups.count(44), 1u);
    EXPECT_DOUBLE_EQ(std::stod(groups.at(44).front()), 1.75)
        << "the line spacing factor was replaced";

    // The whole point: open, save, open again and nothing moved.
    auto reopened = newDocument();
    ASSERT_NO_THROW(lc::persistence::File::open(
        reopened, saved, lc::persistence::File::Library::LIBDXFRW));
    auto readBack = onlyMText(reopened);
    ASSERT_NE(readBack, nullptr);
    EXPECT_DOUBLE_EQ(readBack->width(), 250.0);
    EXPECT_EQ(readBack->drawingDirection(),
              lc::TextConst::MTextDrawingDirection::TopToBottom);
    EXPECT_DOUBLE_EQ(readBack->lineSpacingFactor(), 1.75);
    EXPECT_EQ(readBack->lineSpacingStyle(), lc::TextConst::LineSpacingStyle::Exact);

    boost::filesystem::remove(source);
    boost::filesystem::remove(saved);
}

namespace {

/// The two-column layout of a real MTEXT, as XDATA.
///
/// MTEXT columns are not AcDbMText group codes: they are 1070 keys inside an
/// ACAD_MTEXT_COLUMN_INFO group under AppID ACAD. 75 is the column type, 79
/// auto-height, 76 the count, 78 flow-reversed, 48 the width and 49 the
/// gutter. Nothing in LibreCAD reads any of it; that is the point.
const char* const kColumnXData =
    "1001\nACAD\n"
    "1000\nACAD_MTEXT_COLUMN_INFO_BEGIN\n"
    "1070\n75\n1070\n1\n"
    "1070\n79\n1070\n0\n"
    "1070\n76\n1070\n2\n"
    "1070\n78\n1070\n0\n"
    "1070\n48\n1040\n50.0\n"
    "1070\n49\n1040\n5.0\n"
    "1000\nACAD_MTEXT_COLUMN_INFO_END\n";

/// The same, plus a handle reference -- what linked columns carry.
const char* const kLinkedColumnXData =
    "1001\nACAD\n"
    "1000\nACAD_MTEXT_COLUMNS\n"
    "1005\n2F\n";

}  // namespace

// Columns survive a DXF-to-DXF pass through libdxfrw alone: DRW_Entity
// captures 1000-1071 into extData and dxfRW::writeMText writes it back, with
// no MTEXT-specific code on either side. They did not survive LibreCAD,
// because writeMText builds a fresh DRW_MText and the capture went nowhere --
// `grep -c extData persistence/libdxfrw/dxfimpl.cpp` was 0. Open a columned
// drawing, press Save, and the columns were gone.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, AnMTextKeepsItsColumnsAcrossASave) {
    const std::string source = uniqueTmpDxf("mtext-columns-source");
    boost::filesystem::remove(source);
    writeOneMText(source, std::string("41\n100.0\n72\n5\n73\n1\n44\n1.0\n") + kColumnXData);

    auto doc = newDocument();
    ASSERT_NO_THROW(lc::persistence::File::open(
        doc, source, lc::persistence::File::Library::LIBDXFRW));
    ASSERT_NE(onlyMText(doc), nullptr);

    const std::string saved = uniqueTmpDxf("mtext-columns-saved");
    boost::filesystem::remove(saved);
    ASSERT_TRUE(lc::persistence::File::save(
        doc, saved, lc::persistence::File::LIBDXFRW_DXF_R2000));

    const auto groups = groupsOfRecord(saved, "MTEXT");

    ASSERT_EQ(groups.count(1001), 1u) << "the MTEXT went out with no XDATA at all";
    EXPECT_EQ(groups.at(1001).front(), "ACAD");

    ASSERT_EQ(groups.count(1000), 1u);
    EXPECT_EQ(groups.at(1000).front(), "ACAD_MTEXT_COLUMN_INFO_BEGIN");
    EXPECT_EQ(groups.at(1000).back(), "ACAD_MTEXT_COLUMN_INFO_END");

    // The keys and their values, in order, exactly as they arrived. DXF pads
    // an integer group to a fixed width, so compare the numbers.
    ASSERT_EQ(groups.count(1070), 1u);
    std::vector<int> keys;
    for (const std::string& value : groups.at(1070)) {
        keys.push_back(std::stoi(value));
    }
    const std::vector<int> expected{75, 1, 79, 0, 76, 2, 78, 0, 48, 49};
    EXPECT_EQ(keys, expected) << "the column keys were reordered or lost";

    ASSERT_EQ(groups.count(1040), 1u);
    ASSERT_EQ(groups.at(1040).size(), 2u);
    EXPECT_DOUBLE_EQ(std::stod(groups.at(1040)[0]), 50.0) << "column width";
    EXPECT_DOUBLE_EQ(std::stod(groups.at(1040)[1]), 5.0) << "column gutter";

    boost::filesystem::remove(source);
    boost::filesystem::remove(saved);
}

// A column layout describes a particular block of text. Re-emitting it beside
// different words is worse than dropping it: the reader would lay the new text
// out to the old measurements, and nothing would say so. The text the capture
// belonged to is kept beside it, and an edit is noticed by comparison rather
// than by trusting that an edit produces a new entity -- setProperties keeps
// the id.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, AnEditedMTextDropsTheColumnsThatDescribedItsOldText) {
    const std::string source = uniqueTmpDxf("mtext-columns-edit");
    boost::filesystem::remove(source);
    writeOneMText(source, std::string("41\n100.0\n") + kColumnXData);

    auto doc = newDocument();
    ASSERT_NO_THROW(lc::persistence::File::open(
        doc, source, lc::persistence::File::Library::LIBDXFRW));
    auto original = onlyMText(doc);
    ASSERT_NE(original, nullptr);

    // Same entity id, different words.
    lc::entity::PropertiesMap edited;
    edited["textValue"] = std::string("something else entirely");
    auto changed = std::dynamic_pointer_cast<const lc::entity::MText>(
        original->setProperties(edited));
    ASSERT_NE(changed, nullptr);
    ASSERT_EQ(changed->id(), original->id()) << "the premise of this test";
    ASSERT_NE(changed->text_value(), original->text_value())
        << "setProperties did not change the text; the test proves nothing";

    // The shelf travels with the document the file was read into, so the edit
    // has to replace the entity in THAT document for the lookup to happen at
    // all: take the original out and put the edited one in.
    {
        auto remove = std::make_shared<lc::operation::EntityBuilder>(doc);
        remove->appendEntity(original);
        remove->appendOperation(std::make_shared<lc::operation::Push>());
        remove->appendOperation(std::make_shared<lc::operation::Remove>());
        remove->execute();

        auto add = std::make_shared<lc::operation::EntityBuilder>(doc);
        add->appendEntity(changed);
        add->execute();
    }
    ASSERT_NE(onlyMText(doc), nullptr);
    EXPECT_EQ(onlyMText(doc)->text_value(), changed->text_value());

    const std::string saved = uniqueTmpDxf("mtext-columns-edited");
    boost::filesystem::remove(saved);
    ASSERT_TRUE(lc::persistence::File::save(
        doc, saved, lc::persistence::File::LIBDXFRW_DXF_R2000));

    const auto groups = groupsOfRecord(saved, "MTEXT");
    EXPECT_EQ(groups.count(1001), 0u)
        << "a column layout for text this MText no longer holds was written out";
}

// A 1005 names another record by its code 5, and every handle in the written
// file was either minted fresh or moved by the reservation pass -- so the
// handle the variant carries names whatever now happens to hold it, or
// nothing. A dangling reference is worse than a lost one.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, XDataThatNamesAnotherRecordByHandleIsNotWrittenBack) {
    const std::string source = uniqueTmpDxf("mtext-columns-linked");
    boost::filesystem::remove(source);
    writeOneMText(source, std::string("41\n100.0\n") + kLinkedColumnXData);

    auto doc = newDocument();
    ASSERT_NO_THROW(lc::persistence::File::open(
        doc, source, lc::persistence::File::Library::LIBDXFRW));
    ASSERT_NE(onlyMText(doc), nullptr);

    const std::string saved = uniqueTmpDxf("mtext-columns-linked-saved");
    boost::filesystem::remove(saved);
    ASSERT_TRUE(lc::persistence::File::save(
        doc, saved, lc::persistence::File::LIBDXFRW_DXF_R2000));

    const auto groups = groupsOfRecord(saved, "MTEXT");
    EXPECT_EQ(groups.count(1005), 0u) << "a handle reference was replayed";
    EXPECT_EQ(groups.count(1001), 0u)
        << "half a capture is not a capture: the whole group goes or none of it";

    boost::filesystem::remove(source);
    boost::filesystem::remove(saved);
}

// And nothing is invented. An MText this session created has no XDATA, and
// must not acquire any.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, AnMTextThisSessionCreatedGetsNoExtendedData) {
    auto doc = newDocument();
    ASSERT_NO_THROW(insertThroughBuilder(doc, {
        std::make_shared<lc::entity::MText>(
            lc::geo::Coordinate(1.0, 1.0, 0.0), "text", 10.0, 0.0, "STANDARD",
            lc::TextConst::DrawingDirection::None, lc::TextConst::HAlign::HALeft,
            lc::TextConst::VAlign::VATop, false, false, false, false,
            /*width=*/0.0, lc::TextConst::MTextDrawingDirection::ByStyle,
            /*lineSpacingFactor=*/1.0, lc::TextConst::LineSpacingStyle::AtLeast,
            defaultLayer())}));

    const std::string saved = uniqueTmpDxf("mtext-no-xdata");
    boost::filesystem::remove(saved);
    ASSERT_TRUE(lc::persistence::File::save(
        doc, saved, lc::persistence::File::LIBDXFRW_DXF_R2000));

    const auto groups = groupsOfRecord(saved, "MTEXT");
    EXPECT_EQ(groups.count(1001), 0u);
    EXPECT_EQ(groups.count(1000), 0u);

    boost::filesystem::remove(saved);
}


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
// A drawing with a dimension has to remain saveable at R12. libdxfrw's
// writeDimension refuses the record for AC1009 and fails the whole emit, so
// before DIMENSION was named as absent-before-R13 the save produced no file at
// all -- File > Save on an R12 drawing, since the save reuses the source
// revision. The dimension is dropped, which R12 forces, but the drawing is
// written and the drop is reported.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, ADimensionDoesNotCostTheWholeR12File) {
    auto doc = newDocument();
    auto builder = std::make_shared<lc::operation::Builder>(doc, "seed");
    auto entities = std::make_shared<lc::operation::EntityBuilder>(doc);

    lc::builder::LineBuilder line;
    line.setLayer(doc->layerByName("0"));
    line.setStart({0.0, 0.0});
    line.setEnd({100.0, 0.0});
    entities->appendEntity(line.build());

    lc::builder::DimLinearBuilder dimension;
    dimension.setLayer(doc->layerByName("0"));
    dimension.setDefinitionPoint({0.0, 0.0});
    dimension.setDefinitionPoint2({0.0, 20.0});
    dimension.setDefinitionPoint3({100.0, 20.0});
    dimension.setMiddleOfText({50.0, 25.0});
    dimension.setOblique(0.0);
    dimension.setAngle(0.0);
    entities->appendEntity(dimension.build());

    builder->append(entities);
    builder->execute();

    for (const auto type : {lc::persistence::File::LIBDXFRW_DXF_R12,
                            lc::persistence::File::LIBDXFRW_DXB_R12}) {
        const std::string saved = uniqueTmpDxf("r12-dimension");
        boost::filesystem::remove(saved);

        const auto written = lc::persistence::File::exportFile(doc, saved, type);
        EXPECT_TRUE(written.ok)
            << "a dimension must not cost the whole R12 file";
        EXPECT_TRUE(boost::filesystem::exists(saved)) << "no file was written";
        EXPECT_EQ(written.loss.droppedByType.count("DIMENSION"), 1u)
            << "the dropped dimension has to be reported, not silent";

        // And nothing anonymous is left behind referencing a record that was
        // never written.
        if (type == lc::persistence::File::LIBDXFRW_DXF_R12
            && boost::filesystem::exists(saved)) {
            const auto blocks = recordsInSection(saved, "BLOCKS");
            for (const auto& block : blocks) {
                EXPECT_NE(block.first.rfind("*D", 0), 0u)
                    << "orphan anonymous dimension block " << block.first;
            }
        }
        boost::filesystem::remove(saved);
    }
}

// A save through a symlink has to land on the file the link points at. The
// writer publishes through a temporary and renameat(), which replaces the name
// it is handed -- so without resolving first, saving to drawing.dxf -> the real
// file turned the link into a regular file, left the real file holding the old
// drawing, and reported success. The user's next open of the real path showed
// none of their edits.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, ASaveThroughALinkLandsOnTheRealFile) {
    // boost::filesystem throughout, which is what the rest of this file uses
    // -- mixing it with std::filesystem in one translation unit buys nothing.
    // The link variable is not called "link" because <unistd.h> already has
    // one, and `link.string()` then resolves to a member of a function type.
    const boost::filesystem::path directory =
        boost::filesystem::temp_directory_path()
        / boost::filesystem::unique_path("libdxfrw-link-save-%%%%%%%%-%%%%%%%%");
    boost::system::error_code ignored;
    boost::filesystem::remove_all(directory, ignored);
    boost::filesystem::create_directories(directory, ignored);

    const boost::filesystem::path real = directory / "real.dxf";
    const boost::filesystem::path linkPath = directory / "link.dxf";
    {
        std::ofstream seed(real.string());
        seed << "0\nSECTION\n2\nENTITIES\n0\nENDSEC\n0\nEOF\n";
    }
    boost::system::error_code linked;
    boost::filesystem::create_symlink(real, linkPath, linked);
    if (linked) {
        GTEST_SKIP() << "symlinks unavailable here";
    }

    auto doc = newDocument();
    auto builder = std::make_shared<lc::operation::Builder>(doc, "seed");
    auto entities = std::make_shared<lc::operation::EntityBuilder>(doc);
    lc::builder::LineBuilder line;
    line.setLayer(doc->layerByName("0"));
    line.setStart({0.0, 0.0});
    line.setEnd({42.0, 0.0});
    entities->appendEntity(line.build());
    builder->append(entities);
    builder->execute();

    const auto written = lc::persistence::File::exportFile(
        doc, linkPath.string(), lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(written.ok);

    EXPECT_TRUE(boost::filesystem::is_symlink(boost::filesystem::symlink_status(linkPath)))
        << "the link itself was replaced by a regular file";

    // The drawing has to be in the file the link points at, not beside it.
    auto reopened = newDocument();
    const auto reread = lc::persistence::File::importFile(
        reopened, real.string(), lc::persistence::File::LIBDXFRW);
    ASSERT_TRUE(reread.ok);
    EXPECT_EQ(reopened->entityContainer().asVector().size(), 1u)
        << "the real file did not receive the save";

    boost::filesystem::remove_all(directory, ignored);
}

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
    const RootedObjectFixture rooted("raw_objects.dxf"); ASSERT_TRUE(rooted.valid);
    const std::string& source = rooted.path;
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
    const auto variables = identitiesOfType(saved, "DICTIONARYVAR");
    const auto placeholders = identitiesOfType(saved, "ACDBPLACEHOLDER");
    ASSERT_EQ(variables.size(), 1u);
    EXPECT_EQ(std::count_if(placeholders.begin(), placeholders.end(), [&](const auto& item) {
        return !item.owner.empty() && item.owner == variables.front().owner;
    }), 1) << "the source placeholder no longer belongs to its dictionary";
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

// Reserving a preserved handle stops the allocator minting it again. It does
// not stop the codec writing its own fixed structural handles -- the table
// heads, LAYER "0", the *Model_Space block, the root dictionary -- because
// those go out as literals whatever is reserved. A preserved record that
// arrived carrying one of them was re-emitted under the same code 5 as the
// codec's record, so the file went out with two objects claiming one identity
// and every reference to it ambiguous, and the save reported success.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, APreservedHandleDoesNotCollideWithTheCodecsOwn) {
    const RootedObjectFixture rooted("raw_objects_low_handles.dxf"); ASSERT_TRUE(rooted.valid);
    const std::string& source = rooted.path;
    ASSERT_TRUE(boost::filesystem::exists(source));

    // The fixture is raw_objects.dxf with its three preserved handles moved
    // onto codec literals: 1F is BLOCK_RECORD *Model_Space, 10 is LAYER "0",
    // 12 is APPID "ACAD".
    const auto sourceHandles = handlesInFile(source);
    ASSERT_EQ(sourceHandles.count("10"), 1u) << "the fixture changed";
    ASSERT_EQ(sourceHandles.at("10"), 1u) << "the fixture changed";

    auto doc = newDocument();
    const auto result = lc::persistence::File::importFile(
        doc, source, lc::persistence::File::Library::LIBDXFRW);
    ASSERT_TRUE(result.ok);

    lc::persistence::File::Type type = lc::persistence::File::LIBDXFRW_DXF_R2000;
    ASSERT_TRUE(lc::persistence::File::typeForVariantId(result.variantId, type));

    const std::string saved = uniqueTmpDxf("low-handles");
    boost::filesystem::remove(saved);
    ASSERT_TRUE(lc::persistence::File::exportFile(doc, saved, type).ok);

    for (const auto& handle : handlesInFile(saved)) {
        EXPECT_EQ(handle.second, 1u)
            << "handle " << handle.first << " is used twice: a preserved record "
            << "kept a handle the codec writes itself";
    }

    // The records are still there, and still point at each other: remapping is
    // only correct if every reference moved with the handle it names.
    const auto after = recordsInSection(saved, "OBJECTS");
    EXPECT_EQ(after.count("DICTIONARYVAR"), 1u)
        << "a record LibreCAD does not model was destroyed";
    EXPECT_EQ(after.count("ACDBPLACEHOLDER"), 1u)
        << "a record LibreCAD does not model was destroyed";

    const auto variables = identitiesOfType(saved, "DICTIONARYVAR");
    const auto placeholders = identitiesOfType(saved, "ACDBPLACEHOLDER");
    ASSERT_EQ(variables.size(), 1u);

    // Both were owned by the fixture's own DICTIONARY. Whatever handle that
    // dictionary ended up with, the two 330s must still name it and each
    // other's -- a remap is only correct if every reference moved with the
    // handle it names.
    const auto placeholder = std::find_if(placeholders.begin(), placeholders.end(), [&](const auto& item) {
        return !item.owner.empty() && item.owner == variables.front().owner;
    });
    ASSERT_NE(placeholder, placeholders.end()) << "the preserved records no longer agree on their owner";
    EXPECT_NE(variables.front().handle, placeholder->handle) << "two preserved records were given one handle";

    boost::filesystem::remove(saved);
}

/** The names a DXF's root NamedObjectsDictionary (handle C) lists. */
std::set<std::string> rootDictNames(const std::string& path) {
    std::set<std::string> names;
    std::ifstream file(path);
    std::string code;
    std::string value;
    std::string record;
    bool inRoot = false;

    while (std::getline(file, code) && std::getline(file, value)) {
        while (!value.empty() && (value.back() == '\r' || value.back() == ' ')) {
            value.pop_back();
        }
        const auto trimmed = code.find_first_not_of(" \t");
        const std::string group = trimmed == std::string::npos ? code : code.substr(trimmed);

        if (group == "0") {
            if (inRoot) {
                break;  // the root dictionary ended
            }
            record = value;
        } else if (record == "DICTIONARY" && group == "5") {
            std::string handle = value;
            for (auto& c : handle) {
                c = static_cast<char>(::toupper(static_cast<unsigned char>(c)));
            }
            inRoot = handle == "C";
        } else if (inRoot && group == "3") {
            names.insert(value);
        }
    }

    return names;
}

// libdxfrw routes every named dictionary through the raw net and regenerates
// the root holding only ACAD_GROUP, so a preserved dictionary went into the
// file with nothing naming it. Readers prune orphans, and everything hanging
// off them goes too. Splicing the source's entries back is the other half of
// that contract -- but only for the dictionaries this save really does
// re-emit: naming one whose own children are missing hands the reader a
// dangling reference, which is worse than leaving it detached.
//
// NOLINTNEXTLINE(readability-identifier-naming)
// Code 330 is optional on a DICTIONARY, so "carried no owner group" does not
// mean "is the root" -- 284 of 1840 real drawings hold an owner-less
// dictionary that is nothing of the kind. libdxfrw decides by handle and by
// position, and this side has to decide the same way: a dictionary the library
// preserves is re-emitted carrying its own entries, so harvesting those entries
// here as well names its children a second time, directly from the regenerated
// root, and the child's own owner points at only one of the two.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, AnOwnerLessDictionaryIsNotMistakenForTheRoot) {
    const std::string source = fixture("root_dict_owner_less.dxf");
    ASSERT_TRUE(boost::filesystem::exists(source));
    ASSERT_EQ(rootDictNames(source), (std::set<std::string>{"ACAD_MYAPP"}))
        << "the fixture changed";

    auto doc = newDocument();
    const auto result = lc::persistence::File::importFile(
        doc, source, lc::persistence::File::Library::LIBDXFRW);
    ASSERT_TRUE(result.ok);

    lc::persistence::File::Type type = lc::persistence::File::LIBDXFRW_DXF_R2000;
    ASSERT_TRUE(lc::persistence::File::typeForVariantId(result.variantId, type));

    const std::string saved = uniqueTmpDxf("owner-less-dict");
    boost::filesystem::remove(saved);
    const auto written = lc::persistence::File::exportFile(doc, saved, type);
    ASSERT_TRUE(written.ok);

    const auto names = rootDictNames(saved);
    EXPECT_EQ(names.count("ACAD_MYAPP"), 1u)
        << "the dictionary the root really did name is an orphan";
    EXPECT_EQ(names.count("LC_CHILD"), 0u)
        << "an owner-less dictionary was harvested as the root, so its child "
        << "is named from the root as well as from the dictionary that owns it";

    // The codec writes its own ACAD_GROUP entry; the source's copy of it must
    // not be carried across on top, nor counted as a dictionary left detached.
    EXPECT_EQ(names.count("ACAD_GROUP"), 1u);
    EXPECT_EQ(written.loss.droppedByType.count("unreferenced dictionaries"), 0u)
        << "nothing here is unreachable, so nothing should be reported as such";

    boost::filesystem::remove(saved);
}

TEST(DxfRoundTripTest, PreservedDictionariesAreNamedFromTheRootAgain) {
    const std::string source = fixture("root_dict_entries.dxf");
    ASSERT_TRUE(boost::filesystem::exists(source));
    ASSERT_EQ(rootDictNames(source), (std::set<std::string>{"LC_REACHABLE", "LC_DANGLING"}))
        << "the fixture changed";

    auto doc = newDocument();
    const auto result = lc::persistence::File::importFile(
        doc, source, lc::persistence::File::Library::LIBDXFRW);
    ASSERT_TRUE(result.ok);

    lc::persistence::File::Type type = lc::persistence::File::LIBDXFRW_DXF_R2000;
    ASSERT_TRUE(lc::persistence::File::typeForVariantId(result.variantId, type));

    const std::string saved = uniqueTmpDxf("root-dict");
    boost::filesystem::remove(saved);
    const auto written = lc::persistence::File::exportFile(doc, saved, type);
    ASSERT_TRUE(written.ok);

    const auto names = rootDictNames(saved);
    EXPECT_EQ(names.count("LC_REACHABLE"), 1u)
        << "the dictionary this save re-emitted is still an orphan";
    EXPECT_EQ(names.count("LC_DANGLING"), 0u)
        << "a dictionary whose own entry is missing was named from the root anyway";
    EXPECT_EQ(names.count("ACAD_GROUP"), 1u)
        << "the codec's own entry was displaced";

    // And the one that could not be re-attached is reported rather than left
    // to be discovered by whoever opens the file next.
    EXPECT_EQ(written.loss.droppedByType.count("unreferenced dictionaries"), 1u)
        << "a dictionary was left detached without saying so";

    boost::filesystem::remove(saved);
}

TEST(DxfRoundTripTest, CyclicDictionariesCannotHideMissingDescendants) {
    const std::string source = uniqueTmpDxf("cyclic-dictionaries");
    const std::string saved = uniqueTmpDxf("cyclic-dictionaries-out");
    {
        std::ofstream file(source, std::ios::binary);
        file << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1015\n0\nENDSEC\n"
                "0\nSECTION\n2\nENTITIES\n0\nENDSEC\n0\nSECTION\n2\nOBJECTS\n"
                "0\nDICTIONARY\n5\nC\n330\n0\n100\nAcDbDictionary\n281\n1\n"
                "3\nLC_A\n350\n90\n3\nLC_B\n350\n91\n3\nLC_GOOD\n350\n92\n"
                "3\nLC_BFIRST\n350\n96\n3\nLC_AFOLLOW\n350\n95\n"
                "3\nLC_UPSTREAM\n350\n94\n"
                "0\nDICTIONARY\n5\n90\n330\nC\n100\nAcDbDictionary\n281\n1\n"
                "3\nB\n350\n91\n3\nMISSING\n350\n99\n"
                "0\nDICTIONARY\n5\n91\n330\nC\n100\nAcDbDictionary\n281\n1\n3\nA\n350\n90\n"
                "0\nDICTIONARY\n5\n92\n330\nC\n100\nAcDbDictionary\n281\n1\n3\nD\n350\n93\n"
                "0\nDICTIONARY\n5\n93\n330\nC\n100\nAcDbDictionary\n281\n1\n3\nC\n350\n92\n"
                "0\nDICTIONARY\n5\n94\n330\nC\n100\nAcDbDictionary\n281\n1\n3\nB\n350\n91\n"
                "0\nDICTIONARY\n5\n95\n330\nC\n100\nAcDbDictionary\n281\n1\n3\nB\n350\n96\n3\nMISSING\n350\n99\n"
                "0\nDICTIONARY\n5\n96\n330\nC\n100\nAcDbDictionary\n281\n1\n3\nA\n350\n95\n"
                "0\nENDSEC\n0\nEOF\n";
        ASSERT_TRUE(file.good());
    }
    auto doc = newDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(doc, source, lc::persistence::File::LIBDXFRW).ok);
    const auto result = lc::persistence::File::exportFile(doc, saved, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(result.ok);
    const auto names = rootDictNames(saved);
    EXPECT_EQ(names.count("LC_A"), 0u);
    EXPECT_EQ(names.count("LC_B"), 0u);
    EXPECT_EQ(names.count("LC_UPSTREAM"), 0u);
    EXPECT_EQ(names.count("LC_BFIRST"), 0u);
    EXPECT_EQ(names.count("LC_AFOLLOW"), 0u);
    EXPECT_EQ(names.count("LC_GOOD"), 1u);
    const auto loss = result.loss.droppedByType.find("unreferenced dictionaries");
    ASSERT_NE(loss, result.loss.droppedByType.end());
    EXPECT_EQ(loss->second, 5u);
    boost::filesystem::remove(source);
    boost::filesystem::remove(saved);
}

TEST(DxfRoundTripTest, DeepSharedDictionaryCyclePropagatesMissingTarget) {
    const std::string source = uniqueTmpDxf("deep-dictionary-cycle");
    const std::string saved = uniqueTmpDxf("deep-dictionary-cycle-out");
    {
        std::ofstream file(source, std::ios::binary);
        file << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1015\n0\nENDSEC\n"
                "0\nSECTION\n2\nENTITIES\n0\nENDSEC\n0\nSECTION\n2\nOBJECTS\n"
                "0\nDICTIONARY\n5\nC\n330\n0\n100\nAcDbDictionary\n281\n1\n"
                "3\nLC_FIRST\n350\n100\n3\nLC_MIDDLE\n350\n300\n3\nLC_SAFE\n350\n800\n";
        for (unsigned index = 0; index < 1024; ++index) {
            file << "0\nDICTIONARY\n5\n" << std::hex << 0x100u + index
                 << "\n330\nC\n100\nAcDbDictionary\n281\n1\n3\nNEXT\n350\n"
                 << (index == 1023 ? 0x100u : 0x101u + index) << '\n';
            if (index == 1023) file << "3\nMISSING\n350\n900\n";
        }
        file << "0\nDICTIONARY\n5\n800\n330\nC\n100\nAcDbDictionary\n281\n1\n"
                "0\nENDSEC\n0\nEOF\n";
        ASSERT_TRUE(file.good());
    }
    auto doc = newDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(doc, source, lc::persistence::File::LIBDXFRW).ok);
    const auto result = lc::persistence::File::exportFile(doc, saved, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(result.ok);
    const auto names = rootDictNames(saved);
    EXPECT_EQ(names.count("LC_FIRST"), 0u);
    EXPECT_EQ(names.count("LC_MIDDLE"), 0u);
    EXPECT_EQ(names.count("LC_SAFE"), 1u);
    const auto loss = result.loss.droppedByType.find("unreferenced dictionaries");
    ASSERT_NE(loss, result.loss.droppedByType.end());
    EXPECT_EQ(loss->second, 2u);
    boost::filesystem::remove(source);
    boost::filesystem::remove(saved);
}

TEST(DxfRoundTripTest, RegeneratedImageReactorDoesNotCloseRawDictionary) {
  for (int mode : {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27}) {
    const int space = mode >= 6 ? 0 : mode % 3;
    const bool modern = mode >= 3 && mode < 6;
    const bool extendedDictionary = mode == 6;
    const bool foreignImage = mode == 7;
    const bool invalidOwner = mode == 8;
    const bool unmodeledChild = mode == 9;
    const bool invalidDictionaryOwner = mode == 10;
    const bool differentRoot = mode == 11;
    const bool nested = mode == 12 || mode == 13 || mode == 14;
    const bool invalidNestedOwner = mode == 13;
    const bool invalidNestedRootOwner = mode == 14;
    const bool sourcePaper = mode == 15;
    const bool duplicateTarget = mode == 16;
    const bool definitionMetadata = mode == 19;
    const bool unsafeDefinitionMetadata = mode >= 22 && mode != 26;
    const bool escapedText = mode == 20;
    const bool longText = mode == 21;
    const int extraRecords = mode == 17 ? 252 : mode == 18 ? 253 : 0;
    const char* rootHandle = differentRoot ? "50" : "C";
    const bool inBlock = space != 0;
    const char* blockName = space == 2 ? "*U1" : "LC_IMG";
    SCOPED_TRACE(mode);
    const std::string source = uniqueTmpDxf("image-reactor-reference");
    const std::string saved = uniqueTmpDxf("image-reactor-reference-out");
    {
        std::ofstream file(source, std::ios::binary);
        file << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\n" << (modern ? "AC1024" : "AC1015") << "\n0\nENDSEC\n";
        if (sourcePaper)
            file << "0\nSECTION\n2\nBLOCKS\n0\nBLOCK\n5\n75\n8\n0\n2\n*Paper_Space0\n"
                    "70\n0\n10\n0\n20\n0\n30\n0\n3\n*Paper_Space0\n1\n\n"
                    "0\nENDBLK\n5\n76\n8\n0\n0\nENDSEC\n";
        if (inBlock)
            file << "0\nSECTION\n2\nBLOCKS\n0\nBLOCK\n5\n61\n330\n60\n8\n0\n2\n"
                 << blockName << "\n70\n" << (space == 2 ? 1 : 0)
                 << "\n10\n0\n20\n0\n30\n0\n3\n" << blockName << "\n1\n\n";
        else file << "0\nSECTION\n2\nENTITIES\n";
        file << "0\nIMAGE\n5\n70\n";
        if (inBlock) file << "330\n60\n";
        file << "8\n0\n100\nAcDbRasterImage\n90\n0\n"
                "10\n1\n20\n2\n30\n3\n11\n1\n21\n0\n31\n0\n12\n0\n22\n1\n32\n0\n"
                "13\n2\n23\n3\n340\n81\n360\n82\n70\n3\n280\n1\n281\n50\n282\n50\n283\n0\n"
                "71\n2\n91\n3\n14\n0\n24\n0\n14\n1\n24\n0\n14\n1\n24\n2\n";
        if (modern) file << "290\n1\n";
        if (inBlock) file << "0\nENDBLK\n5\n62\n330\n60\n8\n0\n";
        file << "0\nENDSEC\n"
                "0\nSECTION\n2\nOBJECTS\n0\nDICTIONARY\n5\n" << rootHandle << "\n330\n0\n100\nAcDbDictionary\n281\n1\n"
                "3\nACAD_IMAGE_DICT\n350\n80\n3\nLC_REACTOR\n350\n" << (nested ? "92" : "90") << "\n"
                "0\nDICTIONARY\n5\n80\n330\n" << rootHandle << "\n100\nAcDbDictionary\n281\n1\n3\nscratch\n350\n81\n"
                "0\nDICTIONARY\n5\n90\n330\n" << (invalidDictionaryOwner ? "92" : invalidNestedOwner ? "93" : nested ? "92" : rootHandle) << "\n";
        if (extendedDictionary) file << "102\n{LC_TEST\n1\ncustom\n102\n}\n";
        file << "100\nAcDbDictionary\n281\n1\n3\nOBSERVE\n350\n82\n3\nKEEP\n350\n91\n";
        if (duplicateTarget) file << "3\nALIAS\n350\n91\n";
        for (int i = 0; i < extraRecords; ++i)
            file << "3\nEXTRA_" << i << "\n350\n" << std::hex << 0x200 + i << std::dec << "\n";
        if (nested) file << "0\nDICTIONARY\n5\n92\n330\n" << (invalidNestedRootOwner ? "93" : "C") << "\n100\nAcDbDictionary\n281\n1\n3\nCHILD\n350\n90\n";
        file << "0\nXRECORD\n5\n91\n330\n" << (invalidOwner ? "92" : "90")
             << "\n100\nAcDbXrecord\n280\n1\n1\n"
             << (escapedText ? "keep\\value" : longText ? std::string(256, 'A') : "keep-value") << '\n';
        if (unmodeledChild) file << "340\n92\n";
        for (int i = 0; i < extraRecords; ++i)
            file << "0\nXRECORD\n5\n" << std::hex << 0x200 + i << std::dec
                 << "\n330\n90\n100\nAcDbXrecord\n280\n1\n1\nextra\n";
        file << "0\nIMAGEDEF\n5\n81\n330\n80\n";
        if (definitionMetadata || unsafeDefinitionMetadata || mode == 26) {
            file << "102\n{LC_DEF\n1\n" << (mode == 23 ? std::string(256, 'A') : "custom") << '\n';
            if (mode == 22) file << "330\n90\n";
            if (mode == 24) file << "102\n{NESTED\n1\nvalue\n102\n}\n";
            for (int i = 0; i < (mode == 26 ? 125 : mode == 27 ? 126 : 0); ++i) file << "1\nx\n";
            file << "102\n}\n";
            if (mode == 25) file << "102\n{LC_OTHER\n330\n90\n102\n}\n";
        }
        file << "100\nAcDbRasterImageDef\n90\n2\n1\nscratch.png\n"
                "10\n2\n20\n3\n11\n0.25\n21\n0.5\n280\n1\n281\n5\n"
                "0\nIMAGEDEF_REACTOR\n5\n82\n102\n{LC_TEST\n1\nreactor-metadata\n102\n}\n"
                "330\n70\n100\nAcDbRasterImageDefReactor\n90\n2\n"
                "0\nENDSEC\n0\nEOF\n";
        ASSERT_TRUE(file.good());
    }
    auto doc = newDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(doc, source, lc::persistence::File::LIBDXFRW).ok);
    if (foreignImage) {
        auto foreign = newDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(foreign, source, lc::persistence::File::LIBDXFRW).ok);
        const auto current = doc->entityContainer().asVector();
        const auto replacement = foreign->entityContainer().asVector();
        ASSERT_EQ(current.size(), 1u);
        ASSERT_EQ(replacement.size(), 1u);
        doc->removeEntity(current.front());
        doc->insertEntity(replacement.front());
    }
    if (inBlock) {
        auto block = doc->blockByName(blockName);
        ASSERT_TRUE(block);
        const auto items = doc->entitiesByBlock(block).asVector();
        ASSERT_EQ(items.size(), 1u);
        ASSERT_TRUE(std::dynamic_pointer_cast<const lc::entity::Image>(items.front()));
        EXPECT_TRUE(doc->entityContainer().asVector().empty());
    } else {
        const auto items = doc->entityContainer().asVector();
        ASSERT_EQ(items.size(), 1u);
        auto image = std::dynamic_pointer_cast<const lc::entity::Image>(items.front());
        ASSERT_TRUE(image);
        doc->removeEntity(image);
        auto moved = image->move(lc::geo::Coordinate(10, 0, 0));
        ASSERT_TRUE(moved);
        // A style editor may replace the complete metadata map.
        auto style = lc::meta::MetaInfo::create();
        style->add(std::make_shared<lc::meta::MetaColorByValue>(17, 34, 51));
        auto edited = moved->modify(moved->layer(), modern ? nullptr : style, moved->block());
        ASSERT_TRUE(edited);
        EXPECT_EQ(style->size(), 1u);
        if (!modern) {
            const auto color = edited->metaInfo<lc::meta::MetaColorByValue>(lc::meta::MetaColor::LCMETANAME());
            ASSERT_TRUE(color);
            EXPECT_EQ(color->redI(), 17);
            EXPECT_EQ(color->greenI(), 34);
            EXPECT_EQ(color->blueI(), 51);
        }
        doc->insertEntity(edited);
    }
    const auto result = lc::persistence::File::exportFile(doc, saved, modern ?
        lc::persistence::File::LIBDXFRW_DXF_R2010 : lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(result.ok);
    {
        std::ifstream output(saved, std::ios::binary);
        std::string code, value;
        bool blockRecord = false;
        std::size_t paper0 = 0;
        while (std::getline(output, code) && std::getline(output, value)) {
            if (!value.empty() && value.back() == '\r') value.pop_back();
            const int group = std::stoi(code);
            if (group == 0) blockRecord = value == "BLOCK_RECORD";
            if (blockRecord && group == 2 && value == "*Paper_Space0") ++paper0;
        }
        EXPECT_EQ(paper0, mode <= 5 || mode == 17 || mode >= 19 ? 0u : 1u);
    }
    const auto names = rootDictNames(saved);
    EXPECT_EQ(names.count("ACAD_IMAGE_DICT"), 1u);
    EXPECT_EQ(names.count("LC_REACTOR"), extendedDictionary || invalidOwner || unmodeledChild || invalidDictionaryOwner || differentRoot || invalidNestedOwner || invalidNestedRootOwner ? 0u : 1u);
    const auto loss = result.loss.droppedByType.find("unreferenced dictionaries");
    ASSERT_NE(loss, result.loss.droppedByType.end());
    EXPECT_EQ(loss->second, extendedDictionary || invalidOwner || unmodeledChild || invalidDictionaryOwner || differentRoot || invalidNestedOwner || invalidNestedRootOwner ? 2u : 1u);
    const auto skipped = result.loss.droppedByType.find("IMAGEDEF_REACTOR");
    ASSERT_NE(skipped, result.loss.droppedByType.end());
    EXPECT_EQ(skipped->second, 1u);
    const auto definitionLoss = result.loss.droppedByType.find("IMAGEDEF metadata");
    if (unsafeDefinitionMetadata) {
        ASSERT_NE(definitionLoss, result.loss.droppedByType.end());
        EXPECT_EQ(definitionLoss->second, 1u);
    } else EXPECT_EQ(definitionLoss, result.loss.droppedByType.end());
    // Omitted source IMAGE objects must not leave dangling dictionary entries.
    // A handle from a different import must not authorize a removal.
    const auto droppedEntries = result.loss.droppedByType.find("dictionary entries");
    ASSERT_NE(droppedEntries, result.loss.droppedByType.end());
    EXPECT_EQ(droppedEntries->second, extendedDictionary ? 1u : 2u);
    {
        std::ifstream output(saved, std::ios::binary);
        std::string code, value;
        bool dictionary = false;
        std::size_t scratchCount = 0;
        std::size_t keepCount = 0;
        std::size_t observeCount = 0;
        while (std::getline(output, code) && std::getline(output, value)) {
            if (!value.empty() && value.back() == '\r') value.pop_back();
            const int group = std::stoi(code);
            if (group == 0) dictionary = value == "DICTIONARY";
            if (dictionary && group == 3) {
                if (value == "scratch") ++scratchCount;
                if (value == "KEEP") ++keepCount;
                if (value == "OBSERVE") ++observeCount;
            }
        }
        EXPECT_EQ(scratchCount, 1u);
        EXPECT_EQ(keepCount, 1u);
        EXPECT_EQ(observeCount, extendedDictionary ? 1u : 0u);
    }
    const auto recordGroups = [](const std::string& path, const std::string& type) {
        std::ifstream file(path, std::ios::binary);
        std::map<int, std::vector<std::string>> groups;
        std::string code, value;
        bool inImage = false;
        while (std::getline(file, code) && std::getline(file, value)) {
            if (!value.empty() && value.back() == '\r') value.pop_back();
            const int number = std::stoi(code);
            if (number == 0) {
                if (inImage) break;
                inImage = value == type;
            } else if (inImage) groups[number].push_back(value);
        }
        return groups;
    };
    const auto imageGroups = [&](const std::string& path) { return recordGroups(path, "IMAGE"); };
    auto groups = imageGroups(saved);
    auto definition = recordGroups(saved, "IMAGEDEF");
    for (int key : {90, 10, 20, 11, 21, 280, 281}) ASSERT_EQ(definition[key].size(), 1u);
    EXPECT_EQ(std::stoi(definition[90][0]), 2);
    EXPECT_DOUBLE_EQ(std::stod(definition[10][0]), 2);
    EXPECT_DOUBLE_EQ(std::stod(definition[20][0]), 3);
    EXPECT_DOUBLE_EQ(std::stod(definition[11][0]), 0.25);
    EXPECT_DOUBLE_EQ(std::stod(definition[21][0]), 0.5);
    EXPECT_EQ(std::stoi(definition[280][0]), 1);
    EXPECT_EQ(std::stoi(definition[281][0]), 5);
    if (definitionMetadata || mode == 25 || mode == 26) {
        EXPECT_NE(std::find(definition[102].begin(), definition[102].end(), "{LC_DEF"), definition[102].end());
        ASSERT_EQ(definition[1].size(), mode == 26 ? 127u : 2u);
        EXPECT_EQ(definition[1][0], "custom");
        EXPECT_EQ(definition[1].back(), "scratch.png");
        if (mode == 25)
            EXPECT_EQ(std::find(definition[102].begin(), definition[102].end(), "{LC_OTHER"), definition[102].end());
    } else if (unsafeDefinitionMetadata) {
        EXPECT_EQ(std::find(definition[102].begin(), definition[102].end(), "{LC_DEF"), definition[102].end());
    }
    ASSERT_EQ(groups[70].size(), 1u);
    EXPECT_EQ(std::stoi(groups[70][0]), 3);
    ASSERT_EQ(groups[280].size(), 1u);
    EXPECT_EQ(std::stoi(groups[280][0]), 1);
    ASSERT_EQ(groups[71].size(), 1u);
    EXPECT_EQ(std::stoi(groups[71][0]), 2);
    ASSERT_EQ(groups[91].size(), 1u);
    EXPECT_EQ(std::stoi(groups[91][0]), 3);
    ASSERT_EQ(groups[14].size(), 3u);
    ASSERT_EQ(groups[24].size(), 3u);
    EXPECT_DOUBLE_EQ(std::stod(groups[14][0]), 0);
    EXPECT_DOUBLE_EQ(std::stod(groups[24][0]), 0);
    EXPECT_DOUBLE_EQ(std::stod(groups[14][1]), 1);
    EXPECT_DOUBLE_EQ(std::stod(groups[24][1]), 0);
    EXPECT_DOUBLE_EQ(std::stod(groups[14][2]), 1);
    EXPECT_DOUBLE_EQ(std::stod(groups[24][2]), 2);
    const std::string resaved = uniqueTmpDxf("image-clipping-resaved");
    auto reopened = newDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(reopened, saved, lc::persistence::File::LIBDXFRW).ok);
    ASSERT_TRUE(lc::persistence::File::exportFile(reopened, resaved, modern ?
        lc::persistence::File::LIBDXFRW_DXF_R2010 : lc::persistence::File::LIBDXFRW_DXF_R2000).ok);
    const auto rereadGroups = imageGroups(resaved);
    if (definitionMetadata) {
        const auto resavedDefinition = recordGroups(resaved, "IMAGEDEF");
        ASSERT_TRUE(resavedDefinition.count(102));
        const auto& appGroups = resavedDefinition.at(102);
        EXPECT_NE(std::find(appGroups.begin(), appGroups.end(), "{LC_DEF"), appGroups.end());
        ASSERT_TRUE(resavedDefinition.count(1));
        EXPECT_NE(std::find(resavedDefinition.at(1).begin(), resavedDefinition.at(1).end(), "custom"), resavedDefinition.at(1).end());

        const auto items = doc->entityContainer().asVector();
        ASSERT_EQ(items.size(), 1u);
        const auto sourceImage = std::dynamic_pointer_cast<const lc::entity::Image>(items.front());
        ASSERT_TRUE(sourceImage);
        const auto copied = sourceImage->copy(lc::geo::Coordinate(5, 0, 0));
        doc->insertEntity(copied);
        const std::string metadataBinary = uniqueTmpDxf("image-definition-appdata-binary");
        const auto binaryResult = lc::persistence::File::exportFile(doc, metadataBinary,
            lc::persistence::File::LIBDXFRW_DXB_R2000);
        ASSERT_TRUE(binaryResult.ok);
        EXPECT_EQ(binaryResult.loss.droppedByType.count("IMAGEDEF metadata"), 0u);
        auto binaryDocument = newDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(binaryDocument, metadataBinary,
            lc::persistence::File::LIBDXFRW).ok);
        EXPECT_EQ(binaryDocument->entityContainer().asVector().size(), 2u);
        const std::string metadataReopened = uniqueTmpDxf("image-definition-appdata-reopened");
        ASSERT_TRUE(lc::persistence::File::exportFile(binaryDocument, metadataReopened,
            lc::persistence::File::LIBDXFRW_DXF_R2000).ok);
        const auto binaryDefinition = recordGroups(metadataReopened, "IMAGEDEF");
        ASSERT_TRUE(binaryDefinition.count(102));
        EXPECT_NE(std::find(binaryDefinition.at(102).begin(), binaryDefinition.at(102).end(), "{LC_DEF"), binaryDefinition.at(102).end());
        ASSERT_TRUE(binaryDefinition.count(1));
        EXPECT_NE(std::find(binaryDefinition.at(1).begin(), binaryDefinition.at(1).end(), "custom"), binaryDefinition.at(1).end());
        doc->removeEntity(copied);
        auto renamed = std::make_shared<lc::entity::Image>("renamed.png", sourceImage->base(), sourceImage->uv(),
            sourceImage->vv(), sourceImage->width(), sourceImage->height(), sourceImage->brightness(),
            sourceImage->contrast(), sourceImage->fade(), sourceImage->layer(), sourceImage->metaInfo(), sourceImage->block());
        doc->removeEntity(sourceImage);
        doc->insertEntity(renamed);
        lc::persistence::DXFimpl retryWriter(doc);
        ASSERT_TRUE(retryWriter.writeDXF(metadataReopened, lc::persistence::File::LIBDXFRW_DXF_R2000));
        EXPECT_EQ(retryWriter.loss().droppedByType.at("IMAGEDEF fields"), 1u);
        EXPECT_EQ(retryWriter.loss().droppedByType.at("IMAGEDEF metadata"), 1u);
        doc->removeEntity(renamed);
        doc->insertEntity(sourceImage);
        ASSERT_TRUE(retryWriter.writeDXF(metadataReopened, lc::persistence::File::LIBDXFRW_DXF_R2000));
        EXPECT_EQ(retryWriter.loss().droppedByType.count("IMAGEDEF metadata"), 0u);
        const auto retriedDefinition = recordGroups(metadataReopened, "IMAGEDEF");
        ASSERT_TRUE(retriedDefinition.count(1));
        EXPECT_NE(std::find(retriedDefinition.at(1).begin(), retriedDefinition.at(1).end(), "custom"), retriedDefinition.at(1).end());
        boost::filesystem::remove(metadataBinary);
        boost::filesystem::remove(metadataReopened);
    }
    for (const int key : {70, 280, 71, 91, 14, 24, 290}) {
        const auto before = groups.find(key);
        const auto after = rereadGroups.find(key);
        ASSERT_EQ(before == groups.end(), after == rereadGroups.end());
        if (before != groups.end()) EXPECT_EQ(before->second, after->second);
    }
    boost::filesystem::remove(resaved);
    if (modern) {
        ASSERT_EQ(groups[290].size(), 1u);
        EXPECT_EQ(std::stoi(groups[290][0]), 1);
        const std::string downgraded = uniqueTmpDxf("image-clip-mode-downgraded");
        const auto legacy = lc::persistence::File::exportFile(doc, downgraded, lc::persistence::File::LIBDXFRW_DXF_R2000);
        ASSERT_TRUE(legacy.ok);
        const auto omitted = legacy.loss.droppedByType.find("IMAGE clip mode");
        ASSERT_NE(omitted, legacy.loss.droppedByType.end());
        EXPECT_EQ(omitted->second, 1u);
        boost::filesystem::remove(downgraded);
    }
    if (mode == 0) {
        const auto items = doc->entityContainer().asVector();
        ASSERT_EQ(items.size(), 1u);
        const auto image = std::dynamic_pointer_cast<const lc::entity::Image>(items.front());
        ASSERT_TRUE(image);
        auto resized = std::make_shared<lc::entity::Image>(image->name(), image->base(), image->uv(), image->vv(),
            image->width() + 1, image->height(), image->brightness(), image->contrast(), image->fade(),
            image->layer(), image->metaInfo(), image->block());
        doc->removeEntity(image);
        doc->insertEntity(resized);
        const std::string changed = uniqueTmpDxf("image-clipping-size-change");
        const auto changedResult = lc::persistence::File::exportFile(doc, changed, lc::persistence::File::LIBDXFRW_DXF_R2000);
        ASSERT_TRUE(changedResult.ok);
        const auto dropped = changedResult.loss.droppedByType.find("IMAGE clipping");
        ASSERT_NE(dropped, changedResult.loss.droppedByType.end());
        EXPECT_EQ(dropped->second, 1u);
        doc->removeEntity(resized);
        doc->insertEntity(image);
        const auto retry = lc::persistence::File::exportFile(doc, changed, lc::persistence::File::LIBDXFRW_DXF_R2000);
        ASSERT_TRUE(retry.ok);
        EXPECT_EQ(retry.loss.droppedByType.count("IMAGE clipping"), 0u);
        EXPECT_EQ(imageGroups(changed)[91], groups[91]);
        EXPECT_EQ(imageGroups(changed)[14], groups[14]);
        EXPECT_EQ(imageGroups(changed)[24], groups[24]);
        boost::filesystem::remove(changed);

        auto renamed = std::make_shared<lc::entity::Image>("different.png", image->base(), image->uv(), image->vv(),
            image->width(), image->height(), image->brightness(), image->contrast(), image->fade(),
            image->layer(), image->metaInfo(), image->block());
        doc->removeEntity(image);
        doc->insertEntity(renamed);
        const std::string renamedPath = uniqueTmpDxf("image-definition-path-change");
        const auto renamedResult = lc::persistence::File::exportFile(doc, renamedPath, lc::persistence::File::LIBDXFRW_DXF_R2000);
        ASSERT_TRUE(renamedResult.ok);
        const auto fields = renamedResult.loss.droppedByType.find("IMAGEDEF fields");
        ASSERT_NE(fields, renamedResult.loss.droppedByType.end());
        EXPECT_EQ(fields->second, 1u);
        auto renamedDefinition = recordGroups(renamedPath, "IMAGEDEF");
        ASSERT_EQ(renamedDefinition[11].size(), 1u);
        EXPECT_DOUBLE_EQ(std::stod(renamedDefinition[11][0]), 1);
        doc->removeEntity(renamed);
        doc->insertEntity(image);
        boost::filesystem::remove(renamedPath);

        auto paper = doc->blockByName("*Paper_Space0");
        ASSERT_TRUE(paper);
        auto line = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(1, 2), lc::geo::Coordinate(3, 4),
            doc->layerByName("0"), nullptr, paper);
        doc->insertEntity(line);
        const std::string protectedPath = uniqueTmpDxf("image-paper-refusal");
        { std::ofstream output(protectedPath, std::ios::binary); output << "existing destination"; }
        EXPECT_FALSE(lc::persistence::File::exportFile(doc, protectedPath, lc::persistence::File::LIBDXFRW_DXF_R2000).ok);
        { std::ifstream input(protectedPath, std::ios::binary);
          EXPECT_EQ(std::string((std::istreambuf_iterator<char>(input)), {}), "existing destination"); }
        doc->removeEntity(line);
        ASSERT_TRUE(lc::persistence::File::exportFile(doc, protectedPath, lc::persistence::File::LIBDXFRW_DXF_R2000).ok);
        auto retryDoc = newDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(retryDoc, protectedPath, lc::persistence::File::LIBDXFRW).ok);
        ASSERT_EQ(retryDoc->entityContainer().asVector().size(), 1u);
        boost::filesystem::remove(protectedPath);

        // Deleting the last IMAGE must not leave its source reactor or a
        // dictionary entry that can bind to an unrelated generated handle.
        doc->removeEntity(image);
        const std::string emptyPath = uniqueTmpDxf("image-last-removal");
        const auto empty = lc::persistence::File::exportFile(doc, emptyPath, lc::persistence::File::LIBDXFRW_DXF_R2000);
        ASSERT_TRUE(empty.ok);
        const auto removedReactor = empty.loss.droppedByType.find("IMAGEDEF_REACTOR");
        ASSERT_NE(removedReactor, empty.loss.droppedByType.end());
        EXPECT_EQ(removedReactor->second, 1u);
        const auto removedDefinition = empty.loss.droppedByType.find("IMAGEDEF");
        ASSERT_NE(removedDefinition, empty.loss.droppedByType.end());
        EXPECT_EQ(removedDefinition->second, 1u);
        const auto removedEntries = empty.loss.droppedByType.find("dictionary entries");
        ASSERT_NE(removedEntries, empty.loss.droppedByType.end());
        EXPECT_EQ(removedEntries->second, 2u);
        std::ifstream emptyFile(emptyPath, std::ios::binary);
        std::string emptyCode, emptyValue;
        while (std::getline(emptyFile, emptyCode) && std::getline(emptyFile, emptyValue)) {
            if (!emptyValue.empty() && emptyValue.back() == '\r') emptyValue.pop_back();
            if (std::stoi(emptyCode) == 2) EXPECT_NE(emptyValue, "*Paper_Space0");
            if (std::stoi(emptyCode) == 0) {
                EXPECT_NE(emptyValue, "IMAGE");
                EXPECT_NE(emptyValue, "IMAGEDEF");
                EXPECT_NE(emptyValue, "IMAGEDEF_REACTOR");
            }
        }
        emptyFile.close();
        const std::string binaryEmptyPath = uniqueTmpDxf("image-last-removal-binary");
        const auto binaryEmpty = lc::persistence::File::exportFile(doc, binaryEmptyPath,
            lc::persistence::File::LIBDXFRW_DXB_R2000);
        ASSERT_TRUE(binaryEmpty.ok);
        EXPECT_EQ(binaryEmpty.loss.droppedByType.count("unmodelled records"), 0u);
        std::ifstream binaryFile(binaryEmptyPath, std::ios::binary);
        const std::string binaryBytes((std::istreambuf_iterator<char>(binaryFile)), {});
        EXPECT_EQ(binaryBytes.find("AutoCAD Binary DXF"), 0u);
        EXPECT_EQ(binaryBytes.find("*Paper_Space0"), std::string::npos);
        auto binaryReopened = newDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(binaryReopened, binaryEmptyPath,
            lc::persistence::File::LIBDXFRW).ok);
        EXPECT_TRUE(binaryReopened->entityContainer().asVector().empty());
        const auto metadata = binaryReopened->allMetaTypes();
        const auto shelfEntry = metadata.find(lc::persistence::PreservedRecords::kId);
        ASSERT_NE(shelfEntry, metadata.end());
        const auto shelf = std::dynamic_pointer_cast<const lc::persistence::PreservedRecords>(shelfEntry->second);
        ASSERT_TRUE(shelf);
        const auto root = std::find_if(shelf->rootDictEntries.begin(), shelf->rootDictEntries.end(),
            [](const auto& entry) { return entry.first == "LC_REACTOR"; });
        ASSERT_NE(root, shelf->rootDictEntries.end());
        const auto record = std::find_if(shelf->objects.begin(), shelf->objects.end(),
            [root](const auto& object) { return object.name == "XRECORD" && object.parentHandle == root->second; });
        ASSERT_NE(record, shelf->objects.end());
        EXPECT_TRUE(std::any_of(record->groups.begin(), record->groups.end(), [](const auto& value) {
            return value.code() == 1 && value.type() == DRW_Variant::STRING && value.content.s &&
                *value.content.s == "keep-value";
        }));
        binaryFile.close();
        auto modelLine = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0), lc::geo::Coordinate(1, 2),
            doc->layerByName("0"), nullptr, nullptr);
        doc->insertEntity(modelLine);
        ASSERT_TRUE(lc::persistence::File::exportFile(doc, binaryEmptyPath,
            lc::persistence::File::LIBDXFRW_DXB_R2000).ok);
        std::ifstream withLine(binaryEmptyPath, std::ios::binary);
        const std::string lineBytes((std::istreambuf_iterator<char>(withLine)), {});
        EXPECT_EQ(lineBytes.find("*Paper_Space0"), std::string::npos);
        withLine.close();
        auto lineReopened = newDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(lineReopened, binaryEmptyPath,
            lc::persistence::File::LIBDXFRW).ok);
        ASSERT_EQ(lineReopened->entityContainer().asVector().size(), 1u);
        EXPECT_TRUE(std::dynamic_pointer_cast<const lc::entity::Line>(lineReopened->entityContainer().asVector().front()));
        auto point = std::make_shared<lc::entity::Point>(lc::geo::Coordinate(4, 5, 6), doc->layerByName("0"));
        doc->insertEntity(point);
        ASSERT_TRUE(lc::persistence::File::exportFile(doc, binaryEmptyPath,
            lc::persistence::File::LIBDXFRW_DXB_R2000).ok);
        std::ifstream withPoint(binaryEmptyPath, std::ios::binary);
        const std::string pointBytes((std::istreambuf_iterator<char>(withPoint)), {});
        EXPECT_NE(pointBytes.find("*Paper_Space0"), std::string::npos);
        withPoint.close();
        doc->removeEntity(point);
        doc->removeEntity(modelLine);
        ASSERT_TRUE(lc::persistence::File::exportFile(doc, binaryEmptyPath,
            lc::persistence::File::LIBDXFRW_DXB_R2000).ok);
        std::ifstream retryBinary(binaryEmptyPath, std::ios::binary);
        const std::string retryBytes((std::istreambuf_iterator<char>(retryBinary)), {});
        EXPECT_EQ(retryBytes.find("*Paper_Space0"), std::string::npos);
        retryBinary.close();
        ASSERT_TRUE(lc::persistence::File::exportFile(doc, binaryEmptyPath,
            lc::persistence::File::LIBDXFRW_DXB_R2004).ok);
        std::ifstream changedVersion(binaryEmptyPath, std::ios::binary);
        const std::string changedVersionBytes((std::istreambuf_iterator<char>(changedVersion)), {});
        EXPECT_NE(changedVersionBytes.find("*Paper_Space0"), std::string::npos);
        changedVersion.close();
        auto paperLine = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(0, 0), lc::geo::Coordinate(1, 2),
            doc->layerByName("0"), nullptr, paper);
        doc->insertEntity(paperLine);
        { std::ofstream protectedOutput(binaryEmptyPath, std::ios::binary); protectedOutput << "keep destination"; }
        lc::persistence::DXFimpl reusable(doc);
        EXPECT_FALSE(reusable.writeDXF(binaryEmptyPath, lc::persistence::File::LIBDXFRW_DXB_R2000));
        { std::ifstream protectedInput(binaryEmptyPath, std::ios::binary);
          EXPECT_EQ(std::string((std::istreambuf_iterator<char>(protectedInput)), {}), "keep destination"); }
        doc->removeEntity(paperLine);
        ASSERT_TRUE(reusable.writeDXF(binaryEmptyPath, lc::persistence::File::LIBDXFRW_DXB_R2000));
        EXPECT_EQ(reusable.loss().droppedByType.count("unmodelled records"), 0u);
        boost::filesystem::remove(binaryEmptyPath);
        doc->insertEntity(image);
        const auto restored = lc::persistence::File::exportFile(doc, emptyPath, lc::persistence::File::LIBDXFRW_DXF_R2000);
        ASSERT_TRUE(restored.ok);
        EXPECT_EQ(imageGroups(emptyPath)[14], groups[14]);
        const std::string restoredBinary = uniqueTmpDxf("image-restored-binary");
        const auto restoredResult = lc::persistence::File::exportFile(doc, restoredBinary,
            lc::persistence::File::LIBDXFRW_DXB_R2000);
        ASSERT_TRUE(restoredResult.ok);
        EXPECT_EQ(restoredResult.loss.droppedByType.count("unmodelled records"), 0u);
        std::ifstream restoredFile(restoredBinary, std::ios::binary);
        const std::string restoredBytes((std::istreambuf_iterator<char>(restoredFile)), {});
        EXPECT_EQ(restoredBytes.find("*Paper_Space0"), std::string::npos);
        restoredFile.close();
        auto restoredDocument = newDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(restoredDocument, restoredBinary,
            lc::persistence::File::LIBDXFRW).ok);
        EXPECT_EQ(restoredDocument->entityContainer().asVector().size(), 1u);
        const std::string asciiAfterBinary = uniqueTmpDxf("image-binary-to-ascii");
        const auto binaryRoundtrip = lc::persistence::File::exportFile(restoredDocument, asciiAfterBinary,
            lc::persistence::File::LIBDXFRW_DXF_R2000);
        ASSERT_TRUE(binaryRoundtrip.ok);
        EXPECT_EQ(binaryRoundtrip.loss.droppedByType.count("unmodelled records"), 0u);
        EXPECT_EQ(rootDictNames(asciiAfterBinary).count("LC_REACTOR"), 1u);
        const auto roundtripRecord = recordGroups(asciiAfterBinary, "XRECORD");
        ASSERT_EQ(roundtripRecord.at(1).size(), 1u);
        EXPECT_EQ(roundtripRecord.at(1).front(), "keep-value");
        const std::string binaryAfterBinary = uniqueTmpDxf("image-binary-to-binary");
        const auto binaryAgain = lc::persistence::File::exportFile(restoredDocument, binaryAfterBinary,
            lc::persistence::File::LIBDXFRW_DXB_R2000);
        ASSERT_TRUE(binaryAgain.ok);
        EXPECT_EQ(binaryAgain.loss.droppedByType.count("unmodelled records"), 0u);
        auto twiceReopened = newDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(twiceReopened, binaryAfterBinary,
            lc::persistence::File::LIBDXFRW).ok);
        EXPECT_EQ(twiceReopened->entityContainer().asVector().size(), 1u);
        boost::filesystem::remove(binaryAfterBinary);

        const auto restoredMeta = restoredDocument->allMetaTypes();
        const auto restoredShelfIt = restoredMeta.find(lc::persistence::PreservedRecords::kId);
        ASSERT_NE(restoredShelfIt, restoredMeta.end());
        const auto originalShelf = std::dynamic_pointer_cast<const lc::persistence::PreservedRecords>(restoredShelfIt->second);
        ASSERT_TRUE(originalShelf);
        lc::persistence::DXFimpl reverseWriter(restoredDocument);
        for (int mutation = 0; mutation < 3; ++mutation) {
            auto altered = std::make_shared<lc::persistence::PreservedRecords>(*originalShelf);
            if (mutation == 0) {
                ASSERT_EQ(altered->classes.size(), 2u);
                altered->classes.front().className = "unexpected";
            } else {
                const auto object = std::find_if(altered->objects.begin(), altered->objects.end(),
                    [mutation](const auto& item) { return item.name == (mutation == 1 ? "ACDBDICTIONARYWDFLT" : "XRECORD"); });
                ASSERT_NE(object, altered->objects.end());
                if (mutation == 1) object->groups.back() = DRW_Variant(340, std::string("ABC"));
                else object->groups.back() = DRW_Variant(1, std::string("unsafe\\text"));
            }
            restoredDocument->replaceDocumentMetaType(originalShelf, altered);
            ASSERT_TRUE(reverseWriter.writeDXF(asciiAfterBinary, lc::persistence::File::LIBDXFRW_DXF_R2000));
            EXPECT_GT(reverseWriter.loss().droppedByType.count("unmodelled records"), 0u);
            restoredDocument->replaceDocumentMetaType(altered, originalShelf);
            ASSERT_TRUE(reverseWriter.writeDXF(asciiAfterBinary, lc::persistence::File::LIBDXFRW_DXF_R2000));
            EXPECT_EQ(reverseWriter.loss().droppedByType.count("unmodelled records"), 0u);
            EXPECT_EQ(rootDictNames(asciiAfterBinary).count("LC_REACTOR"), 1u);
        }
        boost::filesystem::remove(asciiAfterBinary);
        doc->insertEntity(modelLine);
        ASSERT_TRUE(lc::persistence::File::exportFile(doc, restoredBinary,
            lc::persistence::File::LIBDXFRW_DXB_R2000).ok);
        std::ifstream mixedFile(restoredBinary, std::ios::binary);
        const std::string mixedBytes((std::istreambuf_iterator<char>(mixedFile)), {});
        EXPECT_EQ(mixedBytes.find("*Paper_Space0"), std::string::npos);
        mixedFile.close();
        auto mixedDocument = newDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(mixedDocument, restoredBinary,
            lc::persistence::File::LIBDXFRW).ok);
        EXPECT_EQ(mixedDocument->entityContainer().asVector().size(), 2u);
        doc->removeEntity(modelLine);
        boost::filesystem::remove(restoredBinary);
        boost::filesystem::remove(emptyPath);
    }
    if (sourcePaper) {
        const auto items = doc->entityContainer().asVector();
        ASSERT_EQ(items.size(), 1u);
        doc->removeEntity(items.front());
        const std::string noImagePath = uniqueTmpDxf("image-source-paper-no-image");
        ASSERT_TRUE(lc::persistence::File::exportFile(doc, noImagePath,
            lc::persistence::File::LIBDXFRW_DXF_R2000).ok);
        std::ifstream output(noImagePath, std::ios::binary);
        std::string code, value;
        bool hasPaper = false;
        while (std::getline(output, code) && std::getline(output, value)) {
            if (!value.empty() && value.back() == '\r') value.pop_back();
            if (std::stoi(code) == 2 && value == "*Paper_Space0") hasPaper = true;
        }
        EXPECT_TRUE(hasPaper);
        boost::filesystem::remove(noImagePath);
        const std::string binaryPath = uniqueTmpDxf("image-source-paper-no-image-binary");
        ASSERT_TRUE(lc::persistence::File::exportFile(doc, binaryPath,
            lc::persistence::File::LIBDXFRW_DXB_R2000).ok);
        std::ifstream binaryOutput(binaryPath, std::ios::binary);
        const std::string binary((std::istreambuf_iterator<char>(binaryOutput)), {});
        EXPECT_NE(binary.find("*Paper_Space0"), std::string::npos);
        binaryOutput.close();
        boost::filesystem::remove(binaryPath);
    }
    if (foreignImage || unmodeledChild || escapedText || longText) {
        const std::string guardedBinary = uniqueTmpDxf("image-guarded-source-binary");
        const auto guarded = lc::persistence::File::exportFile(doc, guardedBinary,
            lc::persistence::File::LIBDXFRW_DXB_R2000);
        ASSERT_TRUE(guarded.ok);
        EXPECT_GT(guarded.loss.droppedByType.count("unmodelled records"), 0u);
        std::ifstream in(guardedBinary, std::ios::binary);
        const std::string bytes((std::istreambuf_iterator<char>(in)), {});
        if (foreignImage || unmodeledChild) EXPECT_NE(bytes.find("*Paper_Space0"), std::string::npos);
        in.close();
        const auto asciiRetry = lc::persistence::File::exportFile(doc, saved,
            lc::persistence::File::LIBDXFRW_DXF_R2000);
        ASSERT_TRUE(asciiRetry.ok);
        EXPECT_EQ(asciiRetry.loss.droppedByType.count("unmodelled records"), 0u);
        boost::filesystem::remove(guardedBinary);
    }
    boost::filesystem::remove(source);
    boost::filesystem::remove(saved);
  }
}

TEST(DxfRoundTripTest, ImageClipStorageLimitReportsDroppedBoundary) {
    const std::string source = uniqueTmpDxf("image-clip-budget");
    const std::string saved = uniqueTmpDxf("image-clip-budget-out");
    {
        std::ofstream file(source, std::ios::binary);
        file << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1015\n0\nENDSEC\n"
                "0\nSECTION\n2\nENTITIES\n";
        for (int n = 0; n < 2; ++n) {
            file << "0\nIMAGE\n5\n" << (n == 0 ? "70" : "71")
                 << "\n8\n0\n100\nAcDbRasterImage\n90\n0\n"
                    "10\n0\n20\n0\n30\n0\n11\n1\n21\n0\n31\n0\n12\n0\n22\n1\n32\n0\n"
                    "13\n2\n23\n3\n340\n81\n70\n3\n280\n1\n281\n50\n282\n50\n283\n0\n"
                    "71\n2\n91\n90000\n";
            for (int vertex = 0; vertex < 90000; ++vertex)
                file << "14\n" << vertex % 2 << "\n24\n" << (vertex / 2) % 2 << "\n";
        }
        file << "0\nENDSEC\n0\nSECTION\n2\nOBJECTS\n"
                "0\nIMAGEDEF\n5\n81\n100\nAcDbRasterImageDef\n90\n0\n1\nscratch.png\n"
                "10\n2\n20\n3\n11\n1\n21\n1\n280\n1\n281\n0\n"
                "0\nENDSEC\n0\nEOF\n";
        ASSERT_TRUE(file.good());
    }
    auto doc = newDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(doc, source, lc::persistence::File::LIBDXFRW).ok);
    ASSERT_EQ(doc->entityContainer().asVector().size(), 2u);
    const auto result = lc::persistence::File::exportFile(doc, saved, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(result.ok);
    const auto dropped = result.loss.droppedByType.find("IMAGE clipping");
    ASSERT_NE(dropped, result.loss.droppedByType.end());
    EXPECT_EQ(dropped->second, 1u);
    boost::filesystem::remove(source);
    boost::filesystem::remove(saved);
}

TEST(DxfRoundTripTest, RemovingLastImageDoesNotDiscardUnrelatedRawReactor) {
    const std::string source = uniqueTmpDxf("image-unrelated-reactor");
    const std::string saved = uniqueTmpDxf("image-unrelated-reactor-out");
    {
        std::ofstream file(source, std::ios::binary);
        file << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1015\n0\nENDSEC\n"
                "0\nSECTION\n2\nENTITIES\n0\nIMAGE\n5\n70\n8\n0\n100\nAcDbRasterImage\n90\n0\n"
                "10\n0\n20\n0\n30\n0\n11\n1\n21\n0\n31\n0\n12\n0\n22\n1\n32\n0\n"
                "13\n2\n23\n3\n340\n81\n360\n82\n70\n3\n280\n0\n281\n50\n282\n50\n283\n0\n"
                "71\n1\n91\n2\n14\n-0.5\n24\n-0.5\n14\n1.5\n24\n2.5\n0\nENDSEC\n"
                "0\nSECTION\n2\nOBJECTS\n0\nDICTIONARY\n5\nC\n330\n0\n100\nAcDbDictionary\n281\n1\n"
                "3\nACAD_IMAGE_DICT\n350\n80\n3\nLC_OTHER\n350\n90\n"
                "0\nDICTIONARY\n5\n80\n330\nC\n100\nAcDbDictionary\n281\n1\n3\nFIRST\n350\n81\n"
                "0\nDICTIONARY\n5\n90\n330\nC\n100\nAcDbDictionary\n281\n1\n"
                "3\nIMAGE_LINK\n350\n82\n3\nOTHER_LINK\n350\n84\n"
                "0\nIMAGEDEF\n5\n81\n330\n80\n100\nAcDbRasterImageDef\n90\n0\n1\nscratch.png\n"
                "10\n2\n20\n3\n11\n1\n21\n1\n280\n1\n281\n0\n"
                "0\nIMAGEDEF_REACTOR\n5\n82\n330\n70\n100\nAcDbRasterImageDefReactor\n90\n2\n"
                "0\nIMAGEDEF_REACTOR\n5\n84\n330\n72\n100\nAcDbRasterImageDefReactor\n90\n2\n"
                "0\nENDSEC\n0\nEOF\n";
        ASSERT_TRUE(file.good());
    }
    auto doc = newDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(doc, source, lc::persistence::File::LIBDXFRW).ok);
    const auto images = doc->entityContainer().asVector();
    ASSERT_EQ(images.size(), 1u);
    doc->removeEntity(images.front());
    const auto result = lc::persistence::File::exportFile(doc, saved, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(result.ok);
    const auto lost = result.loss.droppedByType.find("IMAGEDEF_REACTOR");
    ASSERT_NE(lost, result.loss.droppedByType.end());
    EXPECT_EQ(lost->second, 1u);
    std::ifstream file(saved, std::ios::binary);
    std::string code, value;
    bool reactor = false;
    bool hasPaper = false;
    std::vector<std::string> handles;
    while (std::getline(file, code) && std::getline(file, value)) {
        if (!value.empty() && value.back() == '\r') value.pop_back();
        const int group = std::stoi(code);
        if (group == 2 && value == "*Paper_Space0") hasPaper = true;
        if (group == 0) reactor = value == "IMAGEDEF_REACTOR";
        else if (reactor && group == 5) handles.push_back(value);
    }
    ASSERT_EQ(handles.size(), 1u);
    EXPECT_EQ(handles.front(), "84");
    EXPECT_TRUE(hasPaper);
    file.close();
    const auto binaryPath = uniqueTmpDxf("image-unrelated-reactor-binary");
    ASSERT_TRUE(lc::persistence::File::exportFile(doc, binaryPath,
        lc::persistence::File::LIBDXFRW_DXB_R2000).ok);
    std::ifstream binaryFile(binaryPath, std::ios::binary);
    const std::string binary((std::istreambuf_iterator<char>(binaryFile)), {});
    EXPECT_NE(binary.find("*Paper_Space0"), std::string::npos);
    binaryFile.close();
    boost::filesystem::remove(binaryPath);
    boost::filesystem::remove(source);
    boost::filesystem::remove(saved);
}

TEST(DxfRoundTripTest, DistinctImageDefinitionsWithSamePathRemainDistinct) {
    const std::string source = uniqueTmpDxf("image-definition-identities");
    const std::string saved = uniqueTmpDxf("image-definition-identities-out");
    {
        std::ofstream file(source, std::ios::binary);
        file << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1015\n0\nENDSEC\n"
                "0\nSECTION\n2\nENTITIES\n";
        for (int n = 0; n < 2; ++n) {
            file << "0\nIMAGE\n5\n" << (n ? "71" : "70") << "\n8\n0\n100\nAcDbRasterImage\n90\n0\n"
                    "10\n0\n20\n0\n30\n0\n11\n1\n21\n0\n31\n0\n12\n0\n22\n1\n32\n0\n"
                    "13\n" << (n ? 8 : 2) << "\n23\n" << (n ? 9 : 3)
                 << "\n340\n" << (n ? "83" : "81")
                 << "\n70\n3\n280\n0\n281\n50\n282\n50\n283\n0\n71\n1\n91\n2\n"
                    "14\n-0.5\n24\n-0.5\n14\n1.5\n24\n2.5\n";
        }
        file << "0\nENDSEC\n0\nSECTION\n2\nOBJECTS\n"
                "0\nDICTIONARY\n5\nC\n330\n0\n100\nAcDbDictionary\n281\n1\n3\nACAD_IMAGE_DICT\n350\n80\n"
                "0\nDICTIONARY\n5\n80\n330\nC\n100\nAcDbDictionary\n281\n1\n3\nFIRST\n350\n81\n3\nSECOND\n350\n83\n";
        for (int n = 0; n < 2; ++n)
            file << "0\nIMAGEDEF\n5\n" << (n ? "83" : "81")
                 << "\n330\n80\n100\nAcDbRasterImageDef\n90\n0\n1\nsame.png\n10\n" << (n ? 8 : 2)
                 << "\n20\n" << (n ? 9 : 3) << "\n11\n" << (n ? 0.75 : 0.25)
                 << "\n21\n" << (n ? 0.5 : 0.125) << "\n280\n1\n281\n5\n";
        file << "0\nENDSEC\n0\nEOF\n";
        ASSERT_TRUE(file.good());
    }
    auto doc = newDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(doc, source, lc::persistence::File::LIBDXFRW).ok);
    ASSERT_EQ(doc->entityContainer().asVector().size(), 2u);
    const auto result = lc::persistence::File::exportFile(doc, saved, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(result.ok);
    EXPECT_EQ(result.loss.droppedByType.count("IMAGEDEF identity"), 0u);
    std::ifstream file(saved, std::ios::binary);
    std::string code, value;
    bool inDefinition = false;
    bool inImage = false;
    std::vector<std::pair<double, double>> sizes;
    std::map<std::string, double> definitionWidths;
    std::vector<std::pair<double, std::string>> imageReferences;
    std::string handle, reference;
    double imageWidth = 0;
    double u = 0, v = 0;
    while (std::getline(file, code) && std::getline(file, value)) {
        if (!value.empty() && value.back() == '\r') value.pop_back();
        const int group = std::stoi(code);
        if (group == 0) {
            if (inDefinition) { sizes.emplace_back(u, v); definitionWidths[handle] = u; }
            if (inImage) imageReferences.emplace_back(imageWidth, reference);
            inDefinition = value == "IMAGEDEF";
            inImage = value == "IMAGE";
            handle.clear(); reference.clear(); imageWidth = 0;
            u = v = 0;
        } else if (inDefinition && group == 5) handle = value;
        else if (inDefinition && group == 10) u = std::stod(value);
        else if (inDefinition && group == 20) v = std::stod(value);
        else if (inImage && group == 13) imageWidth = std::stod(value);
        else if (inImage && group == 340) reference = value;
    }
    std::sort(sizes.begin(), sizes.end());
    ASSERT_EQ(sizes.size(), 2u);
    EXPECT_EQ(sizes[0], std::make_pair(2.0, 3.0));
    EXPECT_EQ(sizes[1], std::make_pair(8.0, 9.0));
    ASSERT_EQ(imageReferences.size(), 2u);
    EXPECT_NE(imageReferences[0].second, imageReferences[1].second);
    for (const auto& ref : imageReferences) {
        ASSERT_EQ(definitionWidths.count(ref.second), 1u);
        EXPECT_DOUBLE_EQ(definitionWidths.at(ref.second), ref.first);
    }
    file.close();
    EXPECT_EQ(result.loss.droppedByType.count("unreferenced dictionaries"), 0u);
    std::ifstream dictionaryFile(saved, std::ios::binary);
    bool dictionary = false;
    std::string pendingKey;
    std::map<std::string, std::string> sourceKeys;
    while (std::getline(dictionaryFile, code) && std::getline(dictionaryFile, value)) {
        if (!value.empty() && value.back() == '\r') value.pop_back();
        const int group = std::stoi(code);
        if (group == 0) { dictionary = value == "DICTIONARY"; pendingKey.clear(); }
        else if (dictionary && group == 3) pendingKey = value;
        else if (dictionary && group == 350 && (pendingKey == "FIRST" || pendingKey == "SECOND")) {
            EXPECT_TRUE(sourceKeys.emplace(pendingKey, value).second);
            pendingKey.clear();
        }
    }
    ASSERT_EQ(sourceKeys.size(), 2u);
    EXPECT_DOUBLE_EQ(definitionWidths.at(sourceKeys.at("FIRST")), 2);
    EXPECT_DOUBLE_EQ(definitionWidths.at(sourceKeys.at("SECOND")), 8);
    dictionaryFile.close();

    const std::string binary = uniqueTmpDxf("image-definition-identities-binary");
    const std::string binaryResaved = uniqueTmpDxf("image-definition-identities-binary-resaved");
    const auto binaryResult = lc::persistence::File::exportFile(doc, binary, lc::persistence::File::LIBDXFRW_DXB_R2000);
    ASSERT_TRUE(binaryResult.ok);
    EXPECT_EQ(binaryResult.loss.droppedByType.count("unmodelled records"), 0u);
    { std::ifstream in(binary, std::ios::binary);
      const std::string bytes((std::istreambuf_iterator<char>(in)), {});
      EXPECT_EQ(bytes.find("*Paper_Space0"), std::string::npos); }
    auto binaryDoc = newDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(binaryDoc, binary, lc::persistence::File::LIBDXFRW).ok);
    ASSERT_EQ(binaryDoc->entityContainer().asVector().size(), 2u);
    const auto binaryResaveResult = lc::persistence::File::exportFile(binaryDoc, binaryResaved, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(binaryResaveResult.ok);
    EXPECT_EQ(binaryResaveResult.loss.droppedByType.count("unmodelled records"), 0u);
    std::ifstream binaryOutput(binaryResaved, std::ios::binary);
    inDefinition = false;
    std::vector<double> binaryWidths;
    while (std::getline(binaryOutput, code) && std::getline(binaryOutput, value)) {
        if (!value.empty() && value.back() == '\r') value.pop_back();
        const int group = std::stoi(code);
        if (group == 0) inDefinition = value == "IMAGEDEF";
        else if (inDefinition && group == 10) binaryWidths.push_back(std::stod(value));
    }
    std::sort(binaryWidths.begin(), binaryWidths.end());
    ASSERT_EQ(binaryWidths.size(), 2u);
    EXPECT_DOUBLE_EQ(binaryWidths[0], 2);
    EXPECT_DOUBLE_EQ(binaryWidths[1], 8);
    binaryOutput.close();
    boost::filesystem::remove(binary);
    boost::filesystem::remove(binaryResaved);

    std::ifstream originalFile(source, std::ios::binary);
    const std::string original((std::istreambuf_iterator<char>(originalFile)), {});
    originalFile.close();
    auto lowHandles = original;
    const auto replaceAll = [](std::string& text, const std::string& from, const std::string& to) {
        for (std::size_t pos = 0; (pos = text.find(from, pos)) != std::string::npos; pos += to.size())
            text.replace(pos, from.size(), to);
    };
    replaceAll(lowHandles, "\n81\n", "\nD\n");
    replaceAll(lowHandles, "\n83\n", "\nE\n");
    const std::string lowPath = uniqueTmpDxf("image-definition-low-handles");
    { std::ofstream out(lowPath, std::ios::binary); out << lowHandles; }
    auto lowDoc = newDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(lowDoc, lowPath, lc::persistence::File::LIBDXFRW).ok);
    ASSERT_TRUE(lc::persistence::File::exportFile(lowDoc, saved, lc::persistence::File::LIBDXFRW_DXF_R2000).ok);
    auto lowReopened = newDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(lowReopened, saved, lc::persistence::File::LIBDXFRW).ok);
    std::vector<double> lowWidths;
    for (const auto& item : lowReopened->entityContainer().asVector()) {
        const auto image = std::dynamic_pointer_cast<const lc::entity::Image>(item);
        ASSERT_TRUE(image);
        lowWidths.push_back(image->width());
    }
    std::sort(lowWidths.begin(), lowWidths.end());
    ASSERT_EQ(lowWidths.size(), 2u);
    EXPECT_DOUBLE_EQ(lowWidths[0], 2);
    EXPECT_DOUBLE_EQ(lowWidths[1], 8);
    const std::string lowBinary = uniqueTmpDxf("image-definition-low-handles-binary");
    const auto lowBinaryResult = lc::persistence::File::exportFile(lowDoc, lowBinary,
        lc::persistence::File::LIBDXFRW_DXB_R2000);
    ASSERT_TRUE(lowBinaryResult.ok);
    EXPECT_EQ(lowBinaryResult.loss.droppedByType.count("unmodelled records"), 0u);
    auto lowBinaryReopened = newDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(lowBinaryReopened, lowBinary,
        lc::persistence::File::LIBDXFRW).ok);
    EXPECT_EQ(lowBinaryReopened->entityContainer().asVector().size(), 2u);
    boost::filesystem::remove(lowBinary);
    boost::filesystem::remove(lowPath);

    auto collision = original;
    const std::string end = "0\nENDSEC\n0\nEOF\n";
    const auto endPos = collision.rfind(end);
    ASSERT_NE(endPos, std::string::npos);
    collision.insert(endPos, "0\nXRECORD\n5\n83\n330\nC\n100\nAcDbXrecord\n280\n1\n1\ncollision\n");
    const std::string collisionPath = uniqueTmpDxf("image-definition-handle-collision");
    { std::ofstream out(collisionPath, std::ios::binary); out << collision; }
    auto collisionDoc = newDocument();
    { std::ofstream out(saved, std::ios::binary); out << "existing destination"; }
    EXPECT_FALSE(lc::persistence::File::importFile(collisionDoc, collisionPath, lc::persistence::File::LIBDXFRW).ok);
    { std::ifstream in(saved, std::ios::binary);
      EXPECT_EQ(std::string((std::istreambuf_iterator<char>(in)), {}), "existing destination"); }
    ASSERT_TRUE(lc::persistence::File::exportFile(doc, saved, lc::persistence::File::LIBDXFRW_DXF_R2000).ok);
    boost::filesystem::remove(collisionPath);

    // Retry after removing the second source definition and copying the first.
    lc::persistence::DXFimpl reusable(doc);
    ASSERT_TRUE(reusable.writeDXF(saved, lc::persistence::File::LIBDXFRW_DXF_R2000));
    EXPECT_EQ(reusable.loss().droppedByType.count("unreferenced dictionaries"), 0u);
    lc::entity::Image_CSPtr kept, removed;
    for (const auto& item : doc->entityContainer().asVector()) {
        auto image = std::dynamic_pointer_cast<const lc::entity::Image>(item);
        ASSERT_TRUE(image);
        if (image->width() == 2) kept = image;
        else { removed = image; doc->removeEntity(image); }
    }
    ASSERT_TRUE(kept);
    doc->insertEntity(kept->copy(lc::geo::Coordinate(10, 0, 0)));
    const auto retried = lc::persistence::File::exportFile(doc, saved, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(retried.ok);
    EXPECT_EQ(retried.loss.droppedByType.count("IMAGEDEF identity"), 0u);
    const auto removedDefinition = retried.loss.droppedByType.find("IMAGEDEF");
    ASSERT_NE(removedDefinition, retried.loss.droppedByType.end());
    EXPECT_EQ(removedDefinition->second, 1u);
    const auto dropped = retried.loss.droppedByType.find("dictionary entries");
    ASSERT_NE(dropped, retried.loss.droppedByType.end());
    EXPECT_EQ(dropped->second, 2u);
    std::ifstream reread(saved, std::ios::binary);
    inDefinition = inImage = false;
    std::size_t definitionCount = 0;
    std::vector<std::string> references;
    while (std::getline(reread, code) && std::getline(reread, value)) {
        if (!value.empty() && value.back() == '\r') value.pop_back();
        const int group = std::stoi(code);
        if (group == 0) {
            inDefinition = value == "IMAGEDEF";
            inImage = value == "IMAGE";
            if (inDefinition) ++definitionCount;
        } else if (inImage && group == 340) references.push_back(value);
        if (group == 5) EXPECT_NE(value, "83");
    }
    EXPECT_EQ(definitionCount, 1u);
    ASSERT_EQ(references.size(), 2u);
    EXPECT_EQ(references[0], references[1]);
    reread.close();
    ASSERT_TRUE(removed);
    ASSERT_TRUE(reusable.writeDXF(saved, lc::persistence::File::LIBDXFRW_DXF_R2000));
    EXPECT_EQ(reusable.loss().droppedByType.at("unreferenced dictionaries"), 1u);
    doc->insertEntity(removed);
    ASSERT_TRUE(reusable.writeDXF(saved, lc::persistence::File::LIBDXFRW_DXF_R2000));
    EXPECT_EQ(reusable.loss().droppedByType.count("unreferenced dictionaries"), 0u);
    std::ifstream restored(saved, std::ios::binary);
    const std::string restoredText((std::istreambuf_iterator<char>(restored)), {});
    EXPECT_NE(restoredText.find("FIRST\n"), std::string::npos);
    EXPECT_NE(restoredText.find("SECOND\n"), std::string::npos);
    boost::filesystem::remove(source);
    boost::filesystem::remove(saved);
}

TEST(DxfRoundTripTest, ImageDefinitionXdataRetainsValuesAndRegistersAppIds) {
    const auto readGroups = [](const std::string& path, const std::string& kind) {
        std::ifstream in(path, std::ios::binary);
        std::map<int, std::vector<std::string>> fields;
        std::string code, value;
        bool active = false;
        while (std::getline(in, code) && std::getline(in, value)) {
            if (!value.empty() && value.back() == '\r') value.pop_back();
            const int number = std::stoi(code);
            if (number == 0) {
                if (active && kind == "IMAGEDEF") break;
                active = value == kind;
            } else if (active) fields[number].push_back(value);
        }
        return fields;
    };
    for (int mode : {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}) {
        SCOPED_TRACE(mode);
        const std::string source = uniqueTmpDxf("image-definition-xdata-source");
        const std::string output = uniqueTmpDxf("image-definition-xdata-output");
        {
            std::ofstream file(source, std::ios::binary);
            file << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1015\n0\nENDSEC\n"
                    "0\nSECTION\n2\nTABLES\n0\nTABLE\n2\nAPPID\n5\n9\n70\n" << (mode == 9 ? 2 : (mode == 5 || mode == 13 ? 4 : 3)) << '\n' <<
                    "0\nAPPID\n5\n12\n330\n9\n100\nAcDbSymbolTableRecord\n100\nAcDbRegAppTableRecord\n2\nACAD\n70\n0\n"
                    "0\nAPPID\n5\n40\n330\n9\n100\nAcDbSymbolTableRecord\n100\nAcDbRegAppTableRecord\n2\nLibreCad\n70\n0\n";
            if (mode != 9) {
                file << "0\nAPPID\n5\n41\n";
                if (mode == 12) file << "102\n{APP_META\n1\ncustom\n102\n}\n";
                file << "330\n9\n100\nAcDbSymbolTableRecord\n100\nAcDbRegAppTableRecord\n2\nLC_META\n70\n"
                     << (mode == 8 ? 16 : 0) << '\n';
            }
            if (mode == 5 || mode == 13)
                file << "0\nAPPID\n5\n42\n330\n9\n100\nAcDbSymbolTableRecord\n100\nAcDbRegAppTableRecord\n2\nLAYERREF\n70\n0\n";
            file << "0\nENDTAB\n";
            if (mode == 5)
                file << "0\nTABLE\n2\nLAYER\n5\n2\n70\n1\n0\nLAYER\n5\n20\n330\n2\n"
                        "100\nAcDbSymbolTableRecord\n100\nAcDbLayerTableRecord\n2\nREF\n70\n0\n62\n7\n6\nCONTINUOUS\n0\nENDTAB\n";
            file << "0\nENDSEC\n0\nSECTION\n2\nENTITIES\n";
            for (int n = 0; n < 2; ++n)
                file << "0\nIMAGE\n5\n" << (n ? "71" : "70") << "\n100\nAcDbEntity\n8\n0\n100\nAcDbRasterImage\n90\n0\n"
                        "10\n0\n20\n0\n30\n0\n11\n1\n21\n0\n31\n0\n12\n0\n22\n1\n32\n0\n"
                        "13\n2\n23\n3\n340\n" << (mode == 7 && n == 1 ? "83" : "81") <<
                        "\n70\n3\n280\n0\n281\n50\n282\n50\n283\n0\n"
                        "71\n1\n91\n2\n14\n-0.5\n24\n-0.5\n14\n1.5\n24\n2.5\n";
            file << "0\nENDSEC\n0\nSECTION\n2\nOBJECTS\n"
                    "0\nDICTIONARY\n5\nC\n330\n0\n100\nAcDbDictionary\n281\n1\n3\nACAD_IMAGE_DICT\n350\n80\n"
                    "0\nDICTIONARY\n5\n80\n330\nC\n100\nAcDbDictionary\n281\n1\n3\nFIRST\n350\n81\n";
            if (mode == 7) file << "3\nSECOND\n350\n83\n";
            file << "0\nIMAGEDEF\n5\n81\n330\n80\n100\nAcDbRasterImageDef\n90\n0\n1\nscratch.png\n"
                    "10\n2\n20\n3\n11\n1\n21\n1\n280\n1\n281\n0\n";
            if (mode == 1) file << "1001\nBADREF\n1005\n90\n";
            if (mode == 2) file << "1001\nLONG\n1000\n" << std::string(256, 'A') << '\n';
            if (mode == 4) file << "1001\nWIDE\n1070\n32768\n";
            if (mode == 5 || mode == 13) file << "1001\nLAYERREF\n1003\n" << (mode == 5 ? "ref" : "MISSING") << '\n';
            if (mode == 6) file << "1001\nUNBALANCED\n1002\n{\n1000\nincomplete\n";
            if (mode == 10) file << "1001\nbad name\n1000\nvalue\n";
            if (mode == 11) file << "1001\n" << std::string(32, 'A') << "\n1000\nvalue\n";
            file << "1001\nLC_META\n1000\npayload\n1070\n-123\n1071\n-456789\n1040\n1.25\n"
                    "1002\n{\n1000\nnested\n1002\n}\n"
                    "1001\nACAD\n1000\nbuiltin\n1001\nLibreCad\n1000\nown\n";
            if (mode == 3) file << "1001\nlc_meta\n1000\nduplicate\n";
            if (mode == 7) file << "0\nIMAGEDEF\n5\n83\n330\n80\n100\nAcDbRasterImageDef\n90\n0\n1\nscratch.png\n"
                                   "10\n2\n20\n3\n11\n1\n21\n1\n280\n1\n281\n0\n"
                                   "1001\nlc_meta\n1000\nsecond\n";
            file << "0\nENDSEC\n0\nEOF\n";
        }
        auto doc = newDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(doc, source, lc::persistence::File::LIBDXFRW).ok);
        ASSERT_EQ(doc->entityContainer().asVector().size(), 2u);
        const auto result = lc::persistence::File::exportFile(doc, output, lc::persistence::File::LIBDXFRW_DXF_R2000);
        ASSERT_TRUE(result.ok);
        EXPECT_EQ(result.loss.droppedByType.count("IMAGEDEF metadata"), mode == 0 || mode == 5 || mode == 7 ? 0u : 1u);
        const auto data = readGroups(output, "IMAGEDEF");
        ASSERT_TRUE(data.count(1001));
        EXPECT_EQ(data.at(1001), (mode == 5 ? std::vector<std::string>{"LAYERREF", "LC_META", "ACAD", "LibreCad"} :
                                             std::vector<std::string>{"LC_META", "ACAD", "LibreCad"}));
        EXPECT_EQ(data.at(1000), (std::vector<std::string>{"payload", "nested", "builtin", "own"}));
        EXPECT_EQ(data.at(1002), (std::vector<std::string>{"{", "}"}));
        EXPECT_EQ(std::stoi(data.at(1070).at(0)), -123);
        EXPECT_EQ(std::stoi(data.at(1071).at(0)), -456789);
        EXPECT_DOUBLE_EQ(std::stod(data.at(1040).at(0)), 1.25);
        const auto appids = readGroups(output, "APPID").at(2);
        EXPECT_EQ(std::count(appids.begin(), appids.end(), "ACAD"), 1);
        EXPECT_EQ(std::count(appids.begin(), appids.end(), "LibreCad"), 1);
        EXPECT_EQ(std::count(appids.begin(), appids.end(), "LC_META"), 1);
        EXPECT_EQ(std::count(appids.begin(), appids.end(), "BADREF"), 0);
        EXPECT_EQ(std::count(appids.begin(), appids.end(), "LONG"), 0);
        EXPECT_EQ(std::count(appids.begin(), appids.end(), "WIDE"), 0);
        EXPECT_EQ(std::count(appids.begin(), appids.end(), "LAYERREF"), mode == 5 ? 1 : 0);
        EXPECT_EQ(std::count(appids.begin(), appids.end(), "UNBALANCED"), 0);
        EXPECT_EQ(std::count(appids.begin(), appids.end(), "bad name"), 0);
        EXPECT_EQ(std::count(appids.begin(), appids.end(), std::string(32, 'A')), 0);
        if (mode == 5) {
            EXPECT_EQ(data.at(1003), (std::vector<std::string>{"ref"}));
            const auto originalLayer = doc->layerByName("REF");
            ASSERT_TRUE(originalLayer);
            lc::persistence::DXFimpl retry(doc);
            const auto assertLayerGroup = [&](bool expected) {
                ASSERT_TRUE(retry.writeDXF(output, lc::persistence::File::LIBDXFRW_DXF_R2000));
                EXPECT_EQ(retry.loss().droppedByType.count("IMAGEDEF metadata"), expected ? 0u : 1u);
                const auto fields = readGroups(output, "IMAGEDEF");
                EXPECT_EQ(fields.count(1003), expected ? 1u : 0u);
                if (expected) EXPECT_EQ(fields.at(1003), (std::vector<std::string>{"ref"}));
                EXPECT_EQ(fields.at(1001), (expected ? std::vector<std::string>{"LAYERREF", "LC_META", "ACAD", "LibreCad"} :
                                                        std::vector<std::string>{"LC_META", "ACAD", "LibreCad"}));
                const auto currentAppIds = readGroups(output, "APPID").at(2);
                EXPECT_EQ(std::count(currentAppIds.begin(), currentAppIds.end(), "LAYERREF"), expected ? 1 : 0);
            };
            doc->removeDocumentMetaType(originalLayer);
            assertLayerGroup(false);
            auto replacement = std::make_shared<lc::meta::Layer>("REF");
            doc->addDocumentMetaType(replacement);
            assertLayerGroup(false);
            doc->removeDocumentMetaType(replacement);
            auto renamedLayer = std::make_shared<lc::meta::Layer>("RENAMED");
            doc->addDocumentMetaType(renamedLayer);
            assertLayerGroup(false);
            doc->removeDocumentMetaType(renamedLayer);
            doc->addDocumentMetaType(originalLayer);
            assertLayerGroup(true);
            const auto stateChange = lc::meta::Layer::withState(originalLayer, false, true, false);
            doc->replaceDocumentMetaType(originalLayer, stateChange);
            assertLayerGroup(true);
            doc->replaceDocumentMetaType(stateChange, originalLayer);
            auto foreign = newDocument();
            ASSERT_TRUE(lc::persistence::File::importFile(foreign, source, lc::persistence::File::LIBDXFRW).ok);
            const auto foreignImages = foreign->entityContainer().asVector();
            const auto originalImages = doc->entityContainer().asVector();
            ASSERT_EQ(foreignImages.size(), 2u);
            ASSERT_EQ(originalImages.size(), 2u);
            for (const auto& image : originalImages) doc->removeEntity(image);
            doc->insertEntity(foreignImages.front());
            assertLayerGroup(false);
            doc->removeEntity(foreignImages.front());
            for (const auto& image : originalImages) doc->insertEntity(image);
            assertLayerGroup(true);
            const std::string layerBinary = uniqueTmpDxf("image-definition-xdata-layer-binary");
            const auto layerResult = lc::persistence::File::exportFile(doc, layerBinary,
                lc::persistence::File::LIBDXFRW_DXB_R2000);
            ASSERT_TRUE(layerResult.ok);
            EXPECT_EQ(layerResult.loss.droppedByType.count("IMAGEDEF metadata"), 0u);
            auto layerReopened = newDocument();
            ASSERT_TRUE(lc::persistence::File::importFile(layerReopened, layerBinary,
                lc::persistence::File::LIBDXFRW).ok);
            const std::string layerAscii = uniqueTmpDxf("image-definition-xdata-layer-ascii");
            ASSERT_TRUE(lc::persistence::File::exportFile(layerReopened, layerAscii,
                lc::persistence::File::LIBDXFRW_DXF_R2000).ok);
            EXPECT_EQ(readGroups(layerAscii, "IMAGEDEF").at(1003), (std::vector<std::string>{"ref"}));
            boost::filesystem::remove(layerBinary);
            boost::filesystem::remove(layerAscii);
        }
        if (mode == 0) {
            const std::string binary = uniqueTmpDxf("image-definition-xdata-binary");
            ASSERT_TRUE(lc::persistence::File::exportFile(doc, binary, lc::persistence::File::LIBDXFRW_DXB_R2000).ok);
            auto reopened = newDocument();
            ASSERT_TRUE(lc::persistence::File::importFile(reopened, binary, lc::persistence::File::LIBDXFRW).ok);
            ASSERT_EQ(reopened->entityContainer().asVector().size(), 2u);
            ASSERT_TRUE(lc::persistence::File::exportFile(reopened, output, lc::persistence::File::LIBDXFRW_DXF_R2000).ok);
            EXPECT_EQ(readGroups(output, "IMAGEDEF").at(1001), data.at(1001));
            EXPECT_EQ(readGroups(output, "IMAGEDEF").at(1000), data.at(1000));
            boost::filesystem::remove(binary);

            const auto items = doc->entityContainer().asVector();
            const auto image = std::dynamic_pointer_cast<const lc::entity::Image>(items.front());
            ASSERT_TRUE(image);
            auto renamed = std::make_shared<lc::entity::Image>("other.png", image->base(), image->uv(), image->vv(),
                image->width(), image->height(), image->brightness(), image->contrast(), image->fade(),
                image->layer(), image->metaInfo(), image->block());
            for (const auto& item : items) doc->removeEntity(item);
            doc->insertEntity(renamed);
            lc::persistence::DXFimpl retry(doc);
            ASSERT_TRUE(retry.writeDXF(output, lc::persistence::File::LIBDXFRW_DXF_R2000));
            EXPECT_EQ(retry.loss().droppedByType.at("IMAGEDEF metadata"), 1u);
            EXPECT_FALSE(readGroups(output, "IMAGEDEF").count(1001));
            doc->removeEntity(renamed);
            for (const auto& item : items) doc->insertEntity(item);
            ASSERT_TRUE(retry.writeDXF(output, lc::persistence::File::LIBDXFRW_DXF_R2000));
            EXPECT_EQ(retry.loss().droppedByType.count("IMAGEDEF metadata"), 0u);
            EXPECT_EQ(readGroups(output, "IMAGEDEF").at(1001), data.at(1001));
        }
        boost::filesystem::remove(source);
        boost::filesystem::remove(output);
    }
}

TEST(DxfRoundTripTest, ImageDefinitionMetadataBudgetReportsExhaustion) {
    const std::string source = uniqueTmpDxf("image-definition-xdata-budget-source");
    const std::string output = uniqueTmpDxf("image-definition-xdata-budget-output");
    constexpr int count = 160;
    {
        std::ofstream file(source, std::ios::binary);
        file << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1015\n0\nENDSEC\n"
                "0\nSECTION\n2\nTABLES\n0\nTABLE\n2\nAPPID\n5\n9\n70\n2\n"
                "0\nAPPID\n5\n12\n330\n9\n100\nAcDbSymbolTableRecord\n100\nAcDbRegAppTableRecord\n2\nACAD\n70\n0\n"
                "0\nAPPID\n5\n41\n330\n9\n100\nAcDbSymbolTableRecord\n100\nAcDbRegAppTableRecord\n2\nLC_META\n70\n0\n0\nENDTAB\n0\nENDSEC\n"
                "0\nSECTION\n2\nENTITIES\n";
        for (int n = 0; n < count; ++n) {
            file << "0\nIMAGE\n5\n" << std::hex << 0x100 + n << std::dec
                 << "\n8\n0\n100\nAcDbRasterImage\n90\n0\n10\n0\n20\n0\n30\n0\n"
                    "11\n1\n21\n0\n31\n0\n12\n0\n22\n1\n32\n0\n13\n2\n23\n3\n340\n"
                 << std::hex << 0x1000 + n << std::dec
                 << "\n70\n3\n280\n0\n281\n50\n282\n50\n283\n0\n71\n1\n91\n2\n14\n-0.5\n24\n-0.5\n14\n1.5\n24\n2.5\n";
        }
        file << "0\nENDSEC\n0\nSECTION\n2\nOBJECTS\n";
        const std::string value(250, 'x');
        for (int n = 0; n < count; ++n) {
            file << "0\nIMAGEDEF\n5\n" << std::hex << 0x1000 + n << std::dec
                 << "\n102\n{LC_EXTRA\n1\n" << value << "\n102\n}\n100\nAcDbRasterImageDef\n90\n0\n1\n"
                 << "image_" << n << ".png\n10\n2\n20\n3\n11\n1\n21\n1\n280\n1\n281\n0\n1001\nLC_META\n";
            for (int item = 0; item < 120; ++item) file << "1000\n" << value << '\n';
        }
        file << "0\nENDSEC\n0\nEOF\n";
        ASSERT_TRUE(file.good());
    }
    auto doc = newDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(doc, source, lc::persistence::File::LIBDXFRW).ok);
    ASSERT_EQ(doc->entityContainer().asVector().size(), static_cast<std::size_t>(count));
    const auto result = lc::persistence::File::exportFile(doc, output, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(result.ok);
    const auto dropped = result.loss.droppedByType.find("IMAGEDEF metadata");
    ASSERT_NE(dropped, result.loss.droppedByType.end());
    EXPECT_GT(dropped->second, 0u);
    EXPECT_LT(dropped->second, static_cast<std::size_t>(count));
    std::ifstream file(output, std::ios::binary);
    std::string code, value;
    bool definition = false;
    int definitions = 0, retained = 0, omitted = 0, itemCount = 0;
    const auto finish = [&]() {
        if (!definition) return;
        ++definitions;
        if (itemCount == 120) ++retained;
        else if (itemCount == 0) ++omitted;
        else ADD_FAILURE() << "XDATA group was only partially emitted: " << itemCount;
    };
    while (std::getline(file, code) && std::getline(file, value)) {
        if (!value.empty() && value.back() == '\r') value.pop_back();
        const int group = std::stoi(code);
        if (group == 0) {
            finish();
            definition = value == "IMAGEDEF";
            itemCount = 0;
        } else if (definition && group == 1000) ++itemCount;
    }
    finish();
    EXPECT_EQ(definitions, count);
    EXPECT_GT(retained, 0);
    EXPECT_GT(omitted, 0);
    file.close();
    boost::filesystem::remove(source);
    boost::filesystem::remove(output);
}

TEST(DxfRoundTripTest, NewImageDefinitionUsesImagePixelDimensions) {
    auto doc = newDocument();
    auto image = std::make_shared<lc::entity::Image>("new-image.png", lc::geo::Coordinate(1, 2, 3),
        lc::geo::Coordinate(1, 0, 0), lc::geo::Coordinate(0, 1, 0), 7, 9, 50, 50, 0,
        doc->layerByName("0"));
    doc->insertEntity(image);
    const auto saved = uniqueTmpDxf("new-image-definition");
    const auto result = lc::persistence::File::exportFile(doc, saved, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(result.ok);
    std::ifstream file(saved, std::ios::binary);
    std::string code, value;
    bool inDefinition = false;
    std::map<int, std::string> fields;
    while (std::getline(file, code) && std::getline(file, value)) {
        if (!value.empty() && value.back() == '\r') value.pop_back();
        const int group = std::stoi(code);
        if (group == 0) {
            if (inDefinition) break;
            inDefinition = value == "IMAGEDEF";
        } else if (inDefinition) fields[group] = value;
    }
    for (int group : {10, 20, 11, 21, 280}) ASSERT_EQ(fields.count(group), 1u);
    EXPECT_DOUBLE_EQ(std::stod(fields[10]), 7);
    EXPECT_DOUBLE_EQ(std::stod(fields[20]), 9);
    EXPECT_DOUBLE_EQ(std::stod(fields[11]), 1);
    EXPECT_DOUBLE_EQ(std::stod(fields[21]), 1);
    EXPECT_EQ(std::stoi(fields[280]), 1);
    file.close();
    boost::filesystem::remove(saved);
}

// A layout is the page a drawing prints from. LibreCAD models none of it, and
// libdxfrw parses LAYOUT and PLOTSETTINGS into typed objects instead of handing
// them to the raw passthrough net, so unlike a record this build has never
// heard of they cannot be kept verbatim and put back -- every save destroys
// them. They were also not counted, so the user was told nothing.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, ALayoutThatCannotBeKeptIsReported) {
    const std::string source = fixture("layout_object.dxf");
    ASSERT_TRUE(boost::filesystem::exists(source));
    ASSERT_EQ(recordsInSection(source, "OBJECTS").count("LAYOUT"), 1u)
        << "the fixture changed";

    auto doc = newDocument();
    const auto result = lc::persistence::File::importFile(
        doc, source, lc::persistence::File::Library::LIBDXFRW);
    ASSERT_TRUE(result.ok);

    EXPECT_EQ(result.loss.droppedByType.count("LAYOUT"), 1u)
        << "the layout was dropped without saying so";

    // And it really is gone from the save, which is what the count is about.
    lc::persistence::File::Type type = lc::persistence::File::LIBDXFRW_DXF_R2000;
    ASSERT_TRUE(lc::persistence::File::typeForVariantId(result.variantId, type));
    const std::string saved = uniqueTmpDxf("layout");
    boost::filesystem::remove(saved);
    ASSERT_TRUE(lc::persistence::File::exportFile(doc, saved, type).ok);
    EXPECT_EQ(recordsInSection(saved, "OBJECTS").count("LAYOUT"), 0u)
        << "the layout survived after all -- the count is now wrong";

    boost::filesystem::remove(saved);
}

// The handle a moved record is moved *to* has to be clear of the handles other
// preserved records are keeping. Handing them out during the same walk that
// reserves the verbatim ones moves a record early in the file onto a handle a
// record later in the file holds for itself -- which is the collision the move
// exists to prevent, inflicted on a different pair.
//
// The fixture is built for it: the first record carries a handle below the
// codec's range so it has to move, and the second carries 0x30, the first
// handle the allocator will ever hand out.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, AMovedHandleDoesNotLandOnAnotherPreservedRecord) {
    const std::string source = fixture("raw_objects_first_minted.dxf");
    ASSERT_TRUE(boost::filesystem::exists(source));
    const auto sourceHandles = handlesInFile(source);
    ASSERT_EQ(sourceHandles.count("10"), 1u) << "the fixture changed";
    ASSERT_EQ(sourceHandles.count("30"), 1u) << "the fixture changed";

    auto doc = newDocument();
    const auto result = lc::persistence::File::importFile(
        doc, source, lc::persistence::File::Library::LIBDXFRW);
    ASSERT_TRUE(result.ok);

    lc::persistence::File::Type type = lc::persistence::File::LIBDXFRW_DXF_R2000;
    ASSERT_TRUE(lc::persistence::File::typeForVariantId(result.variantId, type));

    const std::string saved = uniqueTmpDxf("first-minted");
    boost::filesystem::remove(saved);
    ASSERT_TRUE(lc::persistence::File::exportFile(doc, saved, type).ok);

    for (const auto& handle : handlesInFile(saved)) {
        EXPECT_EQ(handle.second, 1u)
            << "handle " << handle.first << " is used twice: a moved record "
            << "landed on one another preserved record was keeping";
    }

    const auto after = recordsInSection(saved, "OBJECTS");
    EXPECT_EQ(after.count("DICTIONARYVAR"), 1u) << "a preserved record was destroyed";
    EXPECT_EQ(after.count("ACDBPLACEHOLDER"), 1u) << "a preserved record was destroyed";

    boost::filesystem::remove(saved);
}

// Replay is attempted only where it is faithful. A different revision, or a
// binary target, and the records are dropped rather than written somewhere they
// have no defined meaning -- and the count reaches the user either way.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, UnmodelledRecordsAreNotForcedIntoAnotherRevision) {
    auto doc = newDocument();
    const RootedObjectFixture rooted("raw_objects.dxf"); ASSERT_TRUE(rooted.valid);
    ASSERT_TRUE(lc::persistence::File::importFile(
        doc, rooted.path, lc::persistence::File::Library::LIBDXFRW).ok);

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

        written.close();
        EXPECT_EQ(insUnits, c.insUnits) << c.what << ": the drawing was rescaled by saving it";
        EXPECT_EQ(measurement, c.measurement) << c.what;

        boost::filesystem::remove(source);
        boost::filesystem::remove(resaved);
    }
}

TEST(DxfRoundTripTest, LegacyTargetsRefuseAdditionalPaperSpaceWithoutOverwriting) {
    for (const auto target : {lc::persistence::File::LIBDXFRW_DXF_R12,
                              lc::persistence::File::LIBDXFRW_DXF_R14,
                              lc::persistence::File::LIBDXFRW_DXB_R12,
                              lc::persistence::File::LIBDXFRW_DXB_R14}) {
        auto doc = newDocument();
        auto mainPaper = std::make_shared<lc::entity::Line>(
            lc::geo::Coordinate(1, 2, 0), lc::geo::Coordinate(3, 4, 0),
            doc->layerByName("0"), nullptr, doc->blockByName("*Paper_Space"));
        auto extraPaper = std::make_shared<lc::entity::Line>(
            lc::geo::Coordinate(2, 3, 0), lc::geo::Coordinate(7, 11, 0),
            doc->layerByName("0"), nullptr, doc->blockByName("*Paper_Space0"));
        insertThroughBuilder(doc, {mainPaper, extraPaper});
        const auto path = uniqueTmpDxf("legacy-paper-refusal");
        { std::ofstream out(path, std::ios::binary); out << "existing destination"; }
        const auto refused = lc::persistence::File::exportFile(doc, path, target);
        EXPECT_FALSE(refused.ok);
        ASSERT_FALSE(refused.diagnostics.empty());
        EXPECT_NE(refused.diagnostics.front().message.find("paper-space"), std::string::npos);
        { std::ifstream in(path, std::ios::binary);
          EXPECT_EQ(std::string((std::istreambuf_iterator<char>(in)), {}), "existing destination"); }
        doc->removeEntity(extraPaper);
        const auto retry = lc::persistence::File::exportFile(doc, path, target);
        ASSERT_TRUE(retry.ok);
        auto reopened = newDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(reopened, path, lc::persistence::File::LIBDXFRW).ok);
        auto primary = reopened->blockByName("*Paper_Space"); ASSERT_TRUE(primary);
        auto entities = reopened->entitiesByBlock(primary).asVector(); ASSERT_EQ(entities.size(), 1u);
        auto line = std::dynamic_pointer_cast<const lc::entity::Line>(entities.front()); ASSERT_TRUE(line);
        EXPECT_DOUBLE_EQ(line->start().x(), 1); EXPECT_DOUBLE_EQ(line->end().y(), 4);
        boost::filesystem::remove(path);
    }
}

TEST(DxfRoundTripTest, R12ImportedExtraPaperBlockRetainsMetadataAndRefusesUnsafeSave) {
    for (const bool includeHeader : {false, true}) {
        const auto input = uniqueTmpDxf("r12-imported-paper");
        const auto output = uniqueTmpDxf("r12-imported-paper-output");
        {
            std::ofstream out(input);
            if (includeHeader)
                out << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1009\n0\nENDSEC\n";
            out << "0\nSECTION\n2\nBLOCKS\n0\nBLOCK\n8\n0\n2\n*PAPER_SPACE0\n"
                   "70\n0\n10\n1\n20\n0\n30\n0\n3\n*PAPER_SPACE0\n1\n\n"
                   "0\nLINE\n8\n0\n10\n2\n20\n3\n30\n0\n11\n7\n21\n11\n31\n0\n"
                   "0\nENDBLK\n8\n0\n0\nENDSEC\n0\nEOF\n";
        }
        auto doc = newDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(doc, input, lc::persistence::File::LIBDXFRW).ok);
        auto paper = doc->blockByName("*Paper_Space0"); ASSERT_TRUE(paper);
        EXPECT_DOUBLE_EQ(paper->base().x(), 1);
        ASSERT_EQ(doc->entitiesByBlock(paper).asVector().size(), 1u);
        { std::ofstream out(output); out << "retain destination"; }
        const auto refused = lc::persistence::File::exportFile(doc, output, lc::persistence::File::LIBDXFRW_DXF_R12);
        EXPECT_FALSE(refused.ok);
        { std::ifstream in(output); EXPECT_EQ(std::string((std::istreambuf_iterator<char>(in)), {}), "retain destination"); }
        boost::filesystem::remove(input); boost::filesystem::remove(output);
    }
}

TEST(DxfRoundTripTest, R14ImportedExtraPaperBlockRetainsMetadataAndRefusesUnsafeSave) {
    for (const bool anonymous : {false, true}) {
        const auto input = uniqueTmpDxf("r14-imported-paper");
        const auto output = uniqueTmpDxf("r14-imported-paper-output");
        {
            std::ofstream out(input);
            out << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1014\n0\nENDSEC\n"
                   "0\nSECTION\n2\nBLOCKS\n0\nBLOCK\n5\n34\n100\nAcDbEntity\n8\n0\n"
                   "100\nAcDbBlockBegin\n2\n*Paper_Space0\n70\n" << (anonymous ? 1 : 0) <<
                   "\n10\n1\n20\n0\n30\n0\n3\n*Paper_Space0\n1\n\n"
                   "0\nLINE\n5\n52\n100\nAcDbEntity\n8\n0\n100\nAcDbLine\n"
                   "10\n2\n20\n3\n30\n0\n11\n7\n21\n11\n31\n0\n"
                   "0\nENDBLK\n5\n35\n100\nAcDbEntity\n8\n0\n100\nAcDbBlockEnd\n"
                   "0\nENDSEC\n0\nEOF\n";
        }
        auto doc = newDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(doc, input, lc::persistence::File::LIBDXFRW).ok);
        auto paper = doc->blockByName("*Paper_Space0"); ASSERT_TRUE(paper);
        EXPECT_DOUBLE_EQ(paper->base().x(), 1); EXPECT_EQ(paper->anonymous(), anonymous);
        ASSERT_EQ(doc->entitiesByBlock(paper).asVector().size(), 1u);
        { std::ofstream out(output); out << "retain destination"; }
        const auto refused = lc::persistence::File::exportFile(doc, output, lc::persistence::File::LIBDXFRW_DXF_R14);
        EXPECT_FALSE(refused.ok);
        { std::ifstream in(output); EXPECT_EQ(std::string((std::istreambuf_iterator<char>(in)), {}), "retain destination"); }
        boost::filesystem::remove(input); boost::filesystem::remove(output);
    }
}

TEST(DxfRoundTripTest, LegacyTargetsRefuseReferencedOrNondefaultExtraPaperSpace) {
    const auto path = uniqueTmpDxf("legacy-paper-metadata");
    auto doc = newDocument();
    auto paper = doc->blockByName("*Paper_Space0"); ASSERT_TRUE(paper);
    lc::builder::InsertBuilder builder;
    builder.setDocument(doc); builder.setLayer(doc->layerByName("0"));
    builder.setDisplayBlock(paper); builder.setCoordinate(lc::geo::Coordinate(0, 0));
    auto insert = builder.build(); insertThroughBuilder(doc, {insert});
    EXPECT_FALSE(lc::persistence::File::exportFile(
        doc, path, lc::persistence::File::LIBDXFRW_DXF_R14).ok);
    doc->removeEntity(insert);
    doc->removeDocumentMetaType(paper);
    auto movedPaper = std::make_shared<lc::meta::Block>("*Paper_Space0", lc::geo::Coordinate(1, 0, 0));
    doc->addDocumentMetaType(movedPaper);
    EXPECT_FALSE(lc::persistence::File::exportFile(
        doc, path, lc::persistence::File::LIBDXFRW_DXF_R14).ok);
    doc->removeDocumentMetaType(movedPaper); doc->addDocumentMetaType(paper);
    EXPECT_TRUE(lc::persistence::File::exportFile(
        doc, path, lc::persistence::File::LIBDXFRW_DXF_R14).ok);
    boost::filesystem::remove(path);
}

TEST(DxfRoundTripTest, LegacyTargetsReportExplicitLayerWidth) {
    const auto source = uniqueTmpDxf("layer-width-source");
    // DocumentImpl's numeric layer width is written explicitly by R2000.
    // Once imported, it must be reported when an older target cannot carry it.
    ASSERT_TRUE(lc::persistence::File::exportFile(
        newDocument(), source, lc::persistence::File::LIBDXFRW_DXF_R2000).ok);
    auto doc = newDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(doc, source, lc::persistence::File::LIBDXFRW).ok);
    for (const auto target : {lc::persistence::File::LIBDXFRW_DXF_R12, lc::persistence::File::LIBDXFRW_DXF_R14}) {
        const auto path = uniqueTmpDxf("legacy-layer-width");
        const auto written = lc::persistence::File::exportFile(doc, path, target);
        ASSERT_TRUE(written.ok);
        EXPECT_EQ(written.loss.droppedByType.count("layer lineweight"), 1u);
        boost::filesystem::remove(path);
    }
    boost::filesystem::remove(source);
}

TEST(DxfRoundTripTest, LegacyTargetsReportUnrepresentableUnitMetadata) {
    const auto source = uniqueTmpDxf("unit-source");
    {
        std::ofstream dxf(source);
        dxf << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1015\n"
               "9\n$INSUNITS\n70\n1\n9\n$MEASUREMENT\n70\n0\n0\nENDSEC\n"
               "0\nSECTION\n2\nENTITIES\n0\nENDSEC\n0\nEOF\n";
    }
    auto doc = newDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(doc, source, lc::persistence::File::LIBDXFRW).ok);
    for (const auto target : {lc::persistence::File::LIBDXFRW_DXF_R12, lc::persistence::File::LIBDXFRW_DXF_R14}) {
        const auto path = uniqueTmpDxf("legacy-unit");
        const auto written = lc::persistence::File::exportFile(doc, path, target);
        ASSERT_TRUE(written.ok);
        EXPECT_EQ(written.loss.droppedByType.count("$INSUNITS"), 1u);
        EXPECT_EQ(written.loss.droppedByType.count("$MEASUREMENT"),
                  target == lc::persistence::File::LIBDXFRW_DXF_R12 ? 1u : 0u);
        std::ifstream in(path); const std::string text((std::istreambuf_iterator<char>(in)), {});
        EXPECT_EQ(text.find("$INSUNITS"), std::string::npos);
        if (target == lc::persistence::File::LIBDXFRW_DXF_R12)
            EXPECT_EQ(text.find("$MEASUREMENT"), std::string::npos);
        else EXPECT_NE(text.find("$MEASUREMENT"), std::string::npos);
        in.close();
        boost::filesystem::remove(path);
    }
    boost::filesystem::remove(source);
}

namespace {

/** The block names a file's DIMENSION records point at, and the blocks it defines. */
struct DimensionBlocks {
    std::vector<std::string> referenced;  //!< group 2 on each DIMENSION, "" when absent
    std::map<std::string, std::size_t> defined;  //!< block name -> entities inside it
};

DimensionBlocks dimensionBlocksIn(const std::string& path) {
    DimensionBlocks found;
    std::ifstream file(path);
    std::string code;
    std::string value;
    std::string section;
    std::string record;
    std::string openBlock;
    bool dimensionNeedsName = false;

    while (std::getline(file, code) && std::getline(file, value)) {
        while (!value.empty() && (value.back() == '\r' || value.back() == ' ')) {
            value.pop_back();
        }
        const auto start = code.find_first_not_of(" \t");
        const std::string group = start == std::string::npos ? code : code.substr(start);

        if (group == "0") {
            if (dimensionNeedsName) {
                found.referenced.push_back("");  // a DIMENSION that named no block
                dimensionNeedsName = false;
            }

            record = value;
            if (value == "ENDBLK") {
                openBlock.clear();
            } else if (value == "DIMENSION") {
                dimensionNeedsName = true;
            } else if (value != "BLOCK" && value != "SECTION" && value != "ENDSEC"
                       && value != "EOF" && !openBlock.empty()) {
                found.defined[openBlock]++;
            }
            continue;
        }

        if (group == "2") {
            if (record == "SECTION") {
                section = value;
            } else if (record == "BLOCK") {
                openBlock = value;
                found.defined[value];  // defined, possibly empty
            } else if (record == "DIMENSION" && dimensionNeedsName) {
                found.referenced.push_back(value);
                dimensionNeedsName = false;
            }
        }
    }

    if (dimensionNeedsName) {
        found.referenced.push_back("");
    }

    return found;
}

}  // namespace

// DXF group 2 on a DIMENSION names an anonymous block holding the lines,
// arrowheads and text the dimension draws. LibreCAD wrote neither the group nor
// the block, and that is not the harmless omission it looks like: a reader does
// not necessarily regenerate the picture. ezdxf's audit *removes* a DIMENSION
// without a valid geometry block -- so a drawing saved by LibreCAD lost its
// dimensions outright when it passed through anything that audits on load.
//
// Measured with ezdxf 1.4.4 on a file from this same document: before, two
// fixes reading "Removed DIMENSION without valid geometry block" and one entity
// left in modelspace; after, zero such fixes and every dimension intact. That
// check needs a Python package CI does not have, so what is asserted here is
// the structure it was verifying: every DIMENSION names a block, and every
// block it names exists and has geometry in it.
//
// NOLINTNEXTLINE(readability-identifier-naming)
// A block named like a dimension block is not necessarily one. The writer
// regenerates the anonymous *D<n> blocks that hold dimension geometry, and used
// to decide which those were from the name alone -- so a block a file carried
// under a *D name was deleted with everything in it, and the INSERT that placed
// it was left pointing at whichever dimension's geometry got minted over the
// name. The drawing opened, looked plausible, and drew arrowheads where the
// user had put a circle.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, ABlockNamedLikeADimensionBlockIsNotDeleted) {
    const std::string source = uniqueTmpDxf("dimname-source");
    const std::string saved = uniqueTmpDxf("dimname-saved");
    boost::filesystem::remove(source);
    boost::filesystem::remove(saved);

    // *D1 holds a CIRCLE and carries no code-70 flag: user content, by the only
    // signal the file offers. *D9 is flagged anonymous, as a real dimension
    // block is. An INSERT places *D1 out at (50,50).
    {
        std::ofstream dxf(source);
        dxf << "0\nSECTION\n2\nBLOCKS\n"
            << "0\nBLOCK\n8\n0\n2\n*D1\n70\n0\n10\n0.0\n20\n0.0\n30\n0.0\n3\n*D1\n"
            << "0\nCIRCLE\n8\n0\n10\n1.0\n20\n2.0\n30\n0.0\n40\n3.0\n"
            << "0\nENDBLK\n8\n0\n"
            << "0\nBLOCK\n8\n0\n2\n*D9\n70\n1\n10\n0.0\n20\n0.0\n30\n0.0\n3\n*D9\n"
            << "0\nLINE\n8\n0\n10\n0.0\n20\n0.0\n30\n0.0\n11\n5.0\n21\n0.0\n31\n0.0\n"
            << "0\nENDBLK\n8\n0\n"
            << "0\nENDSEC\n"
            << "0\nSECTION\n2\nENTITIES\n"
            << "0\nINSERT\n8\n0\n2\n*D1\n10\n50.0\n20\n50.0\n30\n0.0\n"
            << "0\nENDSEC\n0\nEOF\n";
    }

    auto doc = newDocument();
    ASSERT_NO_THROW(lc::persistence::File::open(
        doc, source, lc::persistence::File::Library::LIBDXFRW));

    // A dimension of our own, so the writer really does mint a *D name.
    ASSERT_NO_THROW(insertThroughBuilder(doc, {
        std::make_shared<lc::entity::DimLinear>(
            lc::geo::Coordinate(25, 10, 0), lc::geo::Coordinate(25, 11, 0),
            lc::TextConst::AttachmentPoint::Bottom_center, 0.0, 1.0,
            lc::TextConst::LineSpacingStyle::AtLeast, "",
            lc::geo::Coordinate(0, 0, 0), lc::geo::Coordinate(50, 0, 0), 0.0, 0.0,
            defaultLayer())}));

    ASSERT_TRUE(lc::persistence::File::save(
        doc, saved, lc::persistence::File::LIBDXFRW_DXF_R2000));

    const auto blocks = dimensionBlocksIn(saved);

    // The user's block is still there, still holding its circle -- and the
    // dimension was given a name of its own rather than this one.
    ASSERT_EQ(blocks.defined.count("*D1"), 1u) << "the user block *D1 was deleted";
    EXPECT_GT(blocks.defined.at("*D1"), 0u) << "the CIRCLE in user block *D1 was dropped";
    ASSERT_EQ(blocks.referenced.size(), 1u);
    EXPECT_NE(blocks.referenced[0], "*D1")
        << "the dimension took over the user's block name";

    // The flagged one is machinery: regenerated, so not carried over.
    EXPECT_EQ(blocks.defined.count("*D9"), 0u)
        << "an anonymous dimension block was carried over as well as regenerated";

    // And the circle survived the round trip as a drawing, not just as text.
    auto reopened = newDocument();
    ASSERT_NO_THROW(lc::persistence::File::open(
        reopened, saved, lc::persistence::File::Library::LIBDXFRW));
    int circles = 0;
    for (const auto& block : reopened->blocks()) {
        if (block->name() != "*D1") {
            continue;
        }
        for (const auto& entity : reopened->entitiesByBlock(block).asVector()) {
            if (std::dynamic_pointer_cast<const lc::entity::Circle>(entity)) {
                circles++;
            }
        }
    }
    EXPECT_EQ(circles, 1) << "the circle did not come back";

    boost::filesystem::remove(source);
    boost::filesystem::remove(saved);
}

TEST(DxfRoundTripTest, EveryDimensionCarriesItsGeometryBlock) {
    auto layer = defaultLayer();
    auto doc = newDocument();

    // One of the seven lives inside a user block. That one used to come out
    // nameless: the geometry blocks were minted after the user blocks were
    // written, so by the time this DIMENSION was emitted there was no name to
    // put in its group 2 -- and the block minted for it a moment later sat in
    // the file with nothing pointing at it.
    auto enclosing = std::make_shared<lc::meta::Block>(
        "DIMENSION_HOLDER", lc::geo::Coordinate(0.0, 0.0, 0.0));
    {
        auto blockBuilder = std::make_shared<lc::operation::Builder>(doc, "add block");
        blockBuilder->append(std::make_shared<lc::operation::AddBlock>(doc, enclosing));
        blockBuilder->execute();
    }

    ASSERT_NO_THROW(insertThroughBuilder(doc, {
        std::make_shared<lc::entity::DimLinear>(
            lc::geo::Coordinate(25, 10, 0), lc::geo::Coordinate(25, 11, 0),
            lc::TextConst::AttachmentPoint::Bottom_center, 0.0, 1.0,
            lc::TextConst::LineSpacingStyle::AtLeast, "",
            lc::geo::Coordinate(0, 0, 0), lc::geo::Coordinate(50, 0, 0), 0.0, 0.0, layer),
        std::make_shared<lc::entity::DimAligned>(
            lc::geo::Coordinate(25, -10, 0), lc::geo::Coordinate(25, -11, 0),
            lc::TextConst::AttachmentPoint::Top_center, 0.0, 1.0,
            lc::TextConst::LineSpacingStyle::AtLeast, "",
            lc::geo::Coordinate(0, 0, 0), lc::geo::Coordinate(50, 0, 0), layer),
        std::make_shared<lc::entity::DimRadial>(
            lc::geo::Coordinate(100, 0, 0), lc::geo::Coordinate(105, 6, 0),
            lc::TextConst::AttachmentPoint::Middle_center, 0.0, 1.0,
            lc::TextConst::LineSpacingStyle::AtLeast, "",
            lc::geo::Coordinate(110, 0, 0), 2.0, layer),
        std::make_shared<lc::entity::DimDiametric>(
            lc::geo::Coordinate(140, 0, 0), lc::geo::Coordinate(160, 6, 0),
            lc::TextConst::AttachmentPoint::Middle_center, 0.0, 1.0,
            lc::TextConst::LineSpacingStyle::AtLeast, "",
            lc::geo::Coordinate(160, 0, 0), 2.0, layer),
        std::make_shared<lc::entity::DimAngular>(
            lc::geo::Coordinate(206, 4, 0), lc::geo::Coordinate(208, 6, 0),
            lc::TextConst::AttachmentPoint::Middle_center, 0.0, 1.0,
            lc::TextConst::LineSpacingStyle::AtLeast, "",
            lc::geo::Coordinate(200, 0, 0), lc::geo::Coordinate(210, 0, 0),
            lc::geo::Coordinate(200, 0, 0), lc::geo::Coordinate(206, 8, 0), layer),
        std::make_shared<lc::entity::DimOrdinate>(
            lc::geo::Coordinate(0, 0, 0), lc::geo::Coordinate(250, 30, 0),
            lc::TextConst::AttachmentPoint::Bottom_center, 0.0, 1.0,
            lc::TextConst::LineSpacingStyle::AtLeast, "",
            lc::geo::Coordinate(250, 0, 0), lc::geo::Coordinate(250, 30, 0), true, layer),
        std::make_shared<lc::entity::DimLinear>(
            lc::geo::Coordinate(25, 30, 0), lc::geo::Coordinate(25, 31, 0),
            lc::TextConst::AttachmentPoint::Bottom_center, 0.0, 1.0,
            lc::TextConst::LineSpacingStyle::AtLeast, "",
            lc::geo::Coordinate(0, 20, 0), lc::geo::Coordinate(50, 20, 0), 0.0, 0.0,
            layer, nullptr, enclosing)}));

    const std::string path = uniqueTmpDxf("dimension-blocks");
    boost::filesystem::remove(path);
    ASSERT_TRUE(lc::persistence::File::save(
        doc, path, lc::persistence::File::LIBDXFRW_DXF_R2000));

    const auto blocks = dimensionBlocksIn(path);
    ASSERT_EQ(blocks.referenced.size(), 7u)
        << "All six dimension kinds must be written, plus the one inside a block.";

    for (std::size_t i = 0; i < blocks.referenced.size(); i++) {
        const auto& name = blocks.referenced[i];
        ASSERT_FALSE(name.empty()) << "dimension " << i << " names no geometry block";
        ASSERT_EQ(blocks.defined.count(name), 1u)
            << "dimension " << i << " names block " << name << ", which the file does not define";
        EXPECT_GT(blocks.defined.at(name), 0u)
            << name << " is empty; a reader that honours it would draw nothing";
    }

    // And the drawing still reads back as itself: the geometry blocks are
    // machinery, not six more things in the drawing.
    auto reopened = newDocument();
    ASSERT_NO_THROW(lc::persistence::File::open(
        reopened, path, lc::persistence::File::Library::LIBDXFRW));

    int dimensions = 0;
    for (const auto& entity : reopened->entityContainer().asVector()) {
        if (std::dynamic_pointer_cast<const lc::entity::Dimension>(entity)) {
            dimensions++;
        }
    }
    EXPECT_EQ(dimensions, 6) << "The dimensions themselves must survive the round trip.";

    int inBlock = 0;
    for (const auto& block : reopened->blocks()) {
        if (block->name() != "DIMENSION_HOLDER") {
            continue;
        }
        for (const auto& entity : reopened->entitiesByBlock(block).asVector()) {
            if (std::dynamic_pointer_cast<const lc::entity::Dimension>(entity)) {
                inBlock++;
            }
        }
    }
    EXPECT_EQ(inBlock, 1) << "The dimension inside a block must survive too.";

    boost::filesystem::remove(path);
}


namespace {

/// Every record of one type in ENTITIES, in file order, as code -> first value.
std::vector<std::map<int, std::string>> recordsOfType(const std::string& path,
                                                      const std::string& wanted) {
    std::vector<std::map<int, std::string>> records;
    std::ifstream file(path);
    std::string code;
    std::string value;
    bool inEntities = false;
    bool collecting = false;

    auto trim = [](std::string& text) {
        while (!text.empty() && (text.back() == '\r' || text.back() == ' ')) {
            text.pop_back();
        }
        while (!text.empty() && (text.front() == ' ' || text.front() == '\t')) {
            text.erase(text.begin());
        }
    };

    while (std::getline(file, code) && std::getline(file, value)) {
        trim(code);
        trim(value);

        if (code == "0") {
            collecting = false;
            if (value == "SECTION") {
                std::string sectionCode;
                std::string sectionName;
                if (std::getline(file, sectionCode) && std::getline(file, sectionName)) {
                    trim(sectionName);
                    inEntities = sectionName == "ENTITIES";
                }
                continue;
            }
            if (value == "ENDSEC" || value == "EOF") {
                inEntities = false;
                continue;
            }
            if (inEntities && value == wanted) {
                records.emplace_back();
                collecting = true;
            }
            continue;
        }

        if (!collecting) {
            continue;
        }
        try {
            records.back().emplace(std::stoi(code), value);
        } catch (const std::exception&) {
            // A group code this scanner cannot read is not what it is looking for.
        }
    }

    return records;
}

lc::entity::MText_CSPtr threeLineMText(double angle,
                                       lc::TextConst::VAlign valign) {
    return std::make_shared<lc::entity::MText>(
        lc::geo::Coordinate(10.0, 100.0, 0.0), "one\ntwo\nthree", /*height=*/3.0,
        angle, /*style=*/"STANDARD",
        lc::TextConst::DrawingDirection::None,
        lc::TextConst::HAlign::HALeft, valign,
        false, false, false, false,
        /*width=*/0.0, lc::TextConst::MTextDrawingDirection::ByStyle,
        /*lineSpacingFactor=*/1.0, lc::TextConst::LineSpacingStyle::AtLeast,
        defaultLayer());
}

}  // namespace

// R12 has no MTEXT, so libdxfrw refuses the record and the whole emit fails
// with it -- which is why writeEntity dropped the entity before offering it.
// Dropping loses the words as well as the block. R12 has had TEXT since the
// beginning, and a TEXT holds one line, so a three-line note becomes three
// records rather than nothing.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, AnMTextBecomesOneTextPerLineAtR12) {
    auto doc = newDocument();
    ASSERT_NO_THROW(insertThroughBuilder(doc, {threeLineMText(
        0.0, lc::TextConst::VAlign::VATop)}));

    const std::string path = uniqueTmpDxf("mtext-r12");
    boost::filesystem::remove(path);
    const auto written = lc::persistence::File::exportFile(
        doc, path, lc::persistence::File::Type::LIBDXFRW_DXF_R12);
    ASSERT_TRUE(written.ok) << "the file must still be written";

    const auto census = recordsInFile(path);
    EXPECT_EQ(census.count("MTEXT"), 0u) << "R12 cannot hold an MTEXT";
    ASSERT_EQ(census.count("TEXT"), 1u) << "the lines were dropped, not converted";
    EXPECT_EQ(census.at("TEXT"), 3u) << "one TEXT per line";

    const auto texts = recordsOfType(path, "TEXT");
    ASSERT_EQ(texts.size(), 3u);
    EXPECT_EQ(texts[0].at(1), "one");
    EXPECT_EQ(texts[1].at(1), "two");
    EXPECT_EQ(texts[2].at(1), "three");

    // Reopening gives three Texts back. Nothing claims they are an MText
    // again -- R12 cannot say that, and pretending otherwise would be worse.
    auto reopened = newDocument();
    ASSERT_NO_THROW(lc::persistence::File::open(
        reopened, path, lc::persistence::File::Library::LIBDXFRW));
    std::size_t texts_back = 0;
    for (const auto& entity : reopened->entityContainer().asVector()) {
        if (std::dynamic_pointer_cast<const lc::entity::Text>(entity)) {
            texts_back++;
        }
    }
    EXPECT_EQ(texts_back, 3u);

    boost::filesystem::remove(path);
}

// A TEXT has no line spacing, so the spacing has to become geometry. Each line
// carries the block's own alignment, which is what lets the anchors be stepped
// by one pitch: the alignment decides only where the FIRST line's anchor goes.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, TheR12TextLinesStackWhereTheBlockHadThem) {
    const double pitch = 3.0 * lc::TextConst::MTextLinePitchRatio;

    struct {
        lc::TextConst::VAlign valign;
        double firstLineY;
        const char* label;
    } cases[] = {
        // Top-aligned: the block hangs below its anchor, so line one is at it.
        {lc::TextConst::VAlign::VATop, 100.0, "top"},
        // Bottom-aligned: the block stands on its anchor, so line one is two
        // pitches above it.
        {lc::TextConst::VAlign::VABottom, 100.0 + 2 * pitch, "bottom"},
        // Middle: half the block above, half below.
        {lc::TextConst::VAlign::VAMiddle, 100.0 + pitch, "middle"},
    };

    for (const auto& item : cases) {
        auto doc = newDocument();
        ASSERT_NO_THROW(insertThroughBuilder(doc, {threeLineMText(0.0, item.valign)}))
            << item.label;

        const std::string path = uniqueTmpDxf("mtext-r12-stack");
        boost::filesystem::remove(path);
        ASSERT_TRUE(lc::persistence::File::exportFile(
            doc, path, lc::persistence::File::Type::LIBDXFRW_DXF_R12).ok) << item.label;

        const auto texts = recordsOfType(path, "TEXT");
        ASSERT_EQ(texts.size(), 3u) << item.label;

        for (std::size_t i = 0; i < texts.size(); i++) {
            ASSERT_EQ(texts[i].count(10), 1u) << item.label;
            ASSERT_EQ(texts[i].count(20), 1u) << item.label;
            EXPECT_NEAR(std::stod(texts[i].at(10)), 10.0, 1e-9)
                << item.label << " line " << i << " moved sideways";
            EXPECT_NEAR(std::stod(texts[i].at(20)),
                        item.firstLineY - static_cast<double>(i) * pitch, 1e-9)
                << item.label << " line " << i << " is not one pitch below the last";
        }

        boost::filesystem::remove(path);
    }
}

// The lines stack along the text's own up direction, not along world Y: turn
// the MText a quarter turn and they must step sideways with it.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, TheR12TextLinesTurnWithTheMText) {
    auto doc = newDocument();
    ASSERT_NO_THROW(insertThroughBuilder(doc, {threeLineMText(
        M_PI / 2, lc::TextConst::VAlign::VATop)}));

    const std::string path = uniqueTmpDxf("mtext-r12-rotated");
    boost::filesystem::remove(path);
    ASSERT_TRUE(lc::persistence::File::exportFile(
        doc, path, lc::persistence::File::Type::LIBDXFRW_DXF_R12).ok);

    const auto texts = recordsOfType(path, "TEXT");
    ASSERT_EQ(texts.size(), 3u);

    const double pitch = 3.0 * lc::TextConst::MTextLinePitchRatio;
    for (std::size_t i = 0; i < texts.size(); i++) {
        // Turned a quarter turn, "down the page" is +x in world terms.
        EXPECT_NEAR(std::stod(texts[i].at(10)),
                    10.0 + static_cast<double>(i) * pitch, 1e-9)
            << "line " << i << " did not turn with the block";
        EXPECT_NEAR(std::stod(texts[i].at(20)), 100.0, 1e-9)
            << "line " << i << " stepped down the page instead of across it";
        EXPECT_NEAR(std::stod(texts[i].at(50)), 90.0, 1e-9)
            << "line " << i << " was not rotated";
    }

    boost::filesystem::remove(path);
}

// Converting is still a loss, and a save that loses something has to say what.
// The note carries it: there is no dropped count to report, and reporting only
// counts printed "written without :" with the explanation missing.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, AnR12SaveSaysTheMultilineTextWasConverted) {
    auto doc = newDocument();
    ASSERT_NO_THROW(insertThroughBuilder(doc, {threeLineMText(
        0.0, lc::TextConst::VAlign::VATop)}));

    const std::string path = uniqueTmpDxf("mtext-r12-reported");
    boost::filesystem::remove(path);
    const auto written = lc::persistence::File::exportFile(
        doc, path, lc::persistence::File::Type::LIBDXFRW_DXF_R12);

    EXPECT_TRUE(written.ok);
    EXPECT_EQ(written.loss.droppedByType.count("MTEXT"), 0u)
        << "nothing was dropped; it was written another way";
    ASSERT_EQ(written.loss.notes.size(), 1u) << "the conversion was not reported";
    EXPECT_NE(written.loss.notes.front().find("one TEXT record per line"),
              std::string::npos) << written.loss.notes.front();

    ASSERT_FALSE(written.diagnostics.empty()) << "a note that reaches no caller is not a report";
    const auto explanation = std::find_if(written.diagnostics.begin(), written.diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.message.find("TEXT record per line") != std::string::npos;
    });
    ASSERT_NE(explanation, written.diagnostics.end()) << "the conversion explanation did not reach the caller";
    EXPECT_TRUE(explanation->code == "record-converted" || explanation->code == "record-not-in-revision");

    // And the bool overload still says the drawing was not written whole.
    EXPECT_FALSE(lc::persistence::File::save(
        doc, path, lc::persistence::File::Type::LIBDXFRW_DXF_R12))
        << "an MTEXT that came out as TEXT is not a lossless save";

    boost::filesystem::remove(path);
}

// ---------------------------------------------------------------------------
// Where the separate MText changes meet.
//
// Each of the changes below was written on its own and could only be tested on
// its own. These are the cases that need two of them at once, which is to say
// the cases nobody could write until now -- and they are the ones a user
// actually performs: open a drawing somebody else made, and save it.
// ---------------------------------------------------------------------------

// Decoding the escape language and keeping the column layout have to agree
// about what "the text" is. The layout is only replayed when the MText still
// holds the words it was read with, and that comparison is made against the
// DECODED text on both sides -- so a `\P` in the source must not look like an
// edit and cost the drawing its columns.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, AnImportedMTextKeepsBothItsLineBreaksAndItsColumns) {
    const std::string source = uniqueTmpDxf("mtext-breaks-and-columns");
    boost::filesystem::remove(source);
    {
        std::ofstream dxf(source);
        dxf << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1015\n0\nENDSEC\n"
            << "0\nSECTION\n2\nENTITIES\n"
            << "0\nMTEXT\n8\n0\n"
            << "10\n1.0\n20\n2.0\n30\n0.0\n"
            << "40\n2.5\n41\n100.0\n"
            << "1\none\\Ptwo\\Pthree\n"
            << "71\n1\n72\n5\n73\n1\n44\n1.0\n"
            << kColumnXData
            << "0\nENDSEC\n0\nEOF\n";
    }

    auto doc = newDocument();
    ASSERT_NO_THROW(lc::persistence::File::open(
        doc, source, lc::persistence::File::Library::LIBDXFRW));

    auto mtext = onlyMText(doc);
    ASSERT_NE(mtext, nullptr);
    EXPECT_EQ(mtext->text_value(), "one\ntwo\nthree")
        << "the escape language was not decoded";

    const std::string saved = uniqueTmpDxf("mtext-breaks-and-columns-saved");
    boost::filesystem::remove(saved);
    ASSERT_TRUE(lc::persistence::File::save(
        doc, saved, lc::persistence::File::LIBDXFRW_DXF_R2000));

    const auto groups = groupsOfRecord(saved, "MTEXT");

    // The text goes back out escaped...
    ASSERT_EQ(groups.count(1), 1u);
    EXPECT_EQ(groups.at(1).front(), "one\\Ptwo\\Pthree")
        << "the line breaks were not written back as paragraph breaks";

    // ...and the columns are still there beside it. If the staleness check
    // compared the encoded text against the decoded text, this MText would
    // look edited and the layout would have been dropped.
    ASSERT_EQ(groups.count(1001), 1u)
        << "decoding the text was mistaken for an edit and cost the columns";
    EXPECT_EQ(groups.at(1001).front(), "ACAD");
    ASSERT_EQ(groups.count(1000), 1u);
    EXPECT_EQ(groups.at(1000).front(), "ACAD_MTEXT_COLUMN_INFO_BEGIN");

    auto reopened = newDocument();
    ASSERT_NO_THROW(lc::persistence::File::open(
        reopened, saved, lc::persistence::File::Library::LIBDXFRW));
    auto readBack = onlyMText(reopened);
    ASSERT_NE(readBack, nullptr);
    EXPECT_EQ(readBack->text_value(), "one\ntwo\nthree")
        << "the lines did not survive a save and reopen";

    boost::filesystem::remove(source);
    boost::filesystem::remove(saved);
}

// The other pairing: an MTEXT read from somebody else's file carries its lines
// as `\P`, and saving that drawing to R12 has to produce one TEXT per line.
// Without the decoding it would produce a single TEXT holding a literal `\P`,
// which is the corrupted state this series set out to end.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, AnImportedMTextBecomesOneTextPerLineAtR12) {
    const std::string source = uniqueTmpDxf("mtext-imported-r12");
    boost::filesystem::remove(source);
    writeOneMText(source, "41\n100.0\n44\n1.0\n");
    {
        // writeOneMText writes a single-line body; replace it with an escaped
        // three-line one, in place, so the fixture stays inline.
        std::ifstream in(source);
        std::string contents((std::istreambuf_iterator<char>(in)),
                             std::istreambuf_iterator<char>());
        in.close();
        const std::string needle = "1\ntext\n";
        const auto at = contents.find(needle);
        ASSERT_NE(at, std::string::npos);
        contents.replace(at, needle.size(), "1\nalpha\\Pbeta\\Pgamma\n");
        std::ofstream out(source);
        out << contents;
    }

    auto doc = newDocument();
    ASSERT_NO_THROW(lc::persistence::File::open(
        doc, source, lc::persistence::File::Library::LIBDXFRW));
    auto mtext = onlyMText(doc);
    ASSERT_NE(mtext, nullptr);
    ASSERT_EQ(mtext->text_value(), "alpha\nbeta\ngamma");

    const std::string saved = uniqueTmpDxf("mtext-imported-r12-saved");
    boost::filesystem::remove(saved);
    ASSERT_TRUE(lc::persistence::File::exportFile(
        doc, saved, lc::persistence::File::Type::LIBDXFRW_DXF_R12).ok);

    const auto census = recordsInFile(saved);
    EXPECT_EQ(census.count("MTEXT"), 0u);
    ASSERT_EQ(census.count("TEXT"), 1u)
        << "the imported lines were dropped instead of converted";
    EXPECT_EQ(census.at("TEXT"), 3u) << "one TEXT per line";

    const auto texts = recordsOfType(saved, "TEXT");
    ASSERT_EQ(texts.size(), 3u);
    EXPECT_EQ(texts[0].at(1), "alpha");
    EXPECT_EQ(texts[1].at(1), "beta");
    EXPECT_EQ(texts[2].at(1), "gamma")
        << "a literal paragraph escape reached a TEXT record";

    boost::filesystem::remove(source);
    boost::filesystem::remove(saved);
}

// At R12 the spacing IS the geometry -- there is no group 44 on a TEXT -- so a
// text that asks for double spacing has to come out double spaced. The
// splitter was written before the entity carried a factor and stepped by the
// bare 5/3 pitch, which collapsed every wide-spaced MTEXT to single spacing.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, TheR12TextLinesKeepTheLineSpacingFactor) {
    for (const double factor : {1.0, 1.75, 3.0}) {
        auto doc = newDocument();
        ASSERT_NO_THROW(insertThroughBuilder(doc, {
            std::make_shared<lc::entity::MText>(
                lc::geo::Coordinate(10.0, 100.0, 0.0), "one\ntwo\nthree",
                /*height=*/3.0, /*angle=*/0.0, "STANDARD",
                lc::TextConst::DrawingDirection::None,
                lc::TextConst::HAlign::HALeft, lc::TextConst::VAlign::VATop,
                false, false, false, false,
                /*width=*/0.0, lc::TextConst::MTextDrawingDirection::ByStyle,
                factor, lc::TextConst::LineSpacingStyle::AtLeast,
                defaultLayer())})) << factor;

        const std::string path = uniqueTmpDxf("mtext-r12-factor");
        boost::filesystem::remove(path);
        ASSERT_TRUE(lc::persistence::File::exportFile(
            doc, path, lc::persistence::File::Type::LIBDXFRW_DXF_R12).ok) << factor;

        const auto texts = recordsOfType(path, "TEXT");
        ASSERT_EQ(texts.size(), 3u) << factor;

        const double pitch = lc::TextConst::mtextLinePitch(3.0, factor);
        for (std::size_t i = 0; i < texts.size(); i++) {
            EXPECT_NEAR(std::stod(texts[i].at(20)),
                        100.0 - static_cast<double>(i) * pitch, 1e-9)
                << "a factor of " << factor << " did not reach line " << i;
        }

        boost::filesystem::remove(path);
    }
}

namespace {

/// The text of every TEXT record in the BLOCKS section, in file order.
std::vector<std::string> textInBlocks(const std::string& path) {
    std::vector<std::string> texts;
    std::ifstream file(path);
    std::string code;
    std::string value;
    std::string section;
    std::string record;

    auto trim = [](std::string& text) {
        while (!text.empty() && (text.back() == '\r' || text.back() == ' ')) {
            text.pop_back();
        }
        while (!text.empty() && (text.front() == ' ' || text.front() == '\t')) {
            text.erase(text.begin());
        }
    };

    while (std::getline(file, code) && std::getline(file, value)) {
        trim(code);
        trim(value);
        if (code == "0") {
            record = value;
        } else if (code == "2" && record == "SECTION") {
            section = value;
        } else if (code == "1" && record == "TEXT" && section == "BLOCKS") {
            texts.push_back(value);
        }
    }

    return texts;
}

}  // namespace

// An ordinate dimension used to be dropped on the way in -- counted as loss,
// but gone all the same -- and there was no way to write one. Both kinds now
// make the trip out and back: the points, which axis is measured, and the text
// the user asked for, with the geometry block every DIMENSION needs.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, AnOrdinateDimensionSurvivesARoundTrip) {
    auto layer = defaultLayer();
    auto doc = newDocument();

    // The datum is off the origin, so a writer that loses group 10 shows.
    ASSERT_NO_THROW(insertThroughBuilder(doc, {
        std::make_shared<lc::entity::DimOrdinate>(
            lc::geo::Coordinate(5, 7, 0), lc::geo::Coordinate(27, 45, 0),
            lc::TextConst::AttachmentPoint::Bottom_center, 0.0, 1.0,
            lc::TextConst::LineSpacingStyle::AtLeast, "<>",
            lc::geo::Coordinate(25, 10, 0), lc::geo::Coordinate(27, 45, 0), true, layer),
        std::make_shared<lc::entity::DimOrdinate>(
            lc::geo::Coordinate(5, 7, 0), lc::geo::Coordinate(-60, 12, 0),
            lc::TextConst::AttachmentPoint::Middle_right, 0.0, 1.0,
            lc::TextConst::LineSpacingStyle::AtLeast, "Y = <>",
            lc::geo::Coordinate(25, 4, 0), lc::geo::Coordinate(-60, 12, 0), false, layer)}));

    const std::string path = uniqueTmpDxf("ordinate");
    boost::filesystem::remove(path);
    const auto written = lc::persistence::File::exportFile(
        doc, path, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(written.ok);
    EXPECT_TRUE(written.loss.empty()) << "an ordinate was left out of the file";

    // Group 70: subtype 6, bit 5 because the block is this dimension's own,
    // and bit 6 on the one that measures X.
    const auto records = recordsOfType(path, "DIMENSION");
    ASSERT_EQ(records.size(), 2u);
    std::vector<std::string> types;
    for (const auto& record : records) {
        ASSERT_EQ(record.count(70), 1u);
        types.push_back(record.at(70));
    }
    std::sort(types.begin(), types.end());
    EXPECT_EQ(types, (std::vector<std::string>{"102", "38"}));

    // Each names a geometry block, and the block has something in it.
    const auto blocks = dimensionBlocksIn(path);
    ASSERT_EQ(blocks.referenced.size(), 2u);
    for (const auto& name : blocks.referenced) {
        ASSERT_FALSE(name.empty()) << "an ordinate names no geometry block";
        ASSERT_EQ(blocks.defined.count(name), 1u) << name << " is named but not defined";
        EXPECT_GT(blocks.defined.at(name), 0u) << name << " is empty";
    }

    // The text in those blocks is the ordinate, unsigned, with "<>" standing
    // for it rather than written out.
    auto texts = textInBlocks(path);
    std::sort(texts.begin(), texts.end());
    EXPECT_EQ(texts, (std::vector<std::string>{"20", "Y = 3"}));

    auto reopened = newDocument();
    const auto read = lc::persistence::File::importFile(
        reopened, path, lc::persistence::File::Library::LIBDXFRW);
    ASSERT_TRUE(read.ok);
    EXPECT_EQ(read.loss.droppedByType.count("DIMENSION (ordinate)"), 0u)
        << "the ordinates were dropped on the way back in";
    EXPECT_TRUE(read.loss.empty());

    lc::entity::DimOrdinate_CSPtr measuringX;
    lc::entity::DimOrdinate_CSPtr measuringY;
    for (const auto& entity : reopened->entityContainer().asVector()) {
        if (auto ordinate = std::dynamic_pointer_cast<const lc::entity::DimOrdinate>(entity)) {
            (ordinate->xType() ? measuringX : measuringY) = ordinate;
        }
    }
    ASSERT_NE(measuringX, nullptr) << "the ordinate measuring X did not come back";
    ASSERT_NE(measuringY, nullptr) << "the ordinate measuring Y did not come back";

    EXPECT_DOUBLE_EQ(measuringX->definitionPoint().x(), 5.0);
    EXPECT_DOUBLE_EQ(measuringX->definitionPoint().y(), 7.0);
    EXPECT_DOUBLE_EQ(measuringX->featurePoint().x(), 25.0);
    EXPECT_DOUBLE_EQ(measuringX->featurePoint().y(), 10.0);
    EXPECT_DOUBLE_EQ(measuringX->leaderEndPoint().x(), 27.0);
    EXPECT_DOUBLE_EQ(measuringX->leaderEndPoint().y(), 45.0);
    EXPECT_DOUBLE_EQ(measuringX->middleOfText().y(), 45.0);
    EXPECT_EQ(measuringX->attachmentPoint(), lc::TextConst::AttachmentPoint::Bottom_center);
    EXPECT_EQ(measuringX->explicitValue(), "<>");
    EXPECT_DOUBLE_EQ(measuringX->value(), 20.0);

    EXPECT_DOUBLE_EQ(measuringY->featurePoint().y(), 4.0);
    EXPECT_DOUBLE_EQ(measuringY->leaderEndPoint().x(), -60.0);
    EXPECT_EQ(measuringY->attachmentPoint(), lc::TextConst::AttachmentPoint::Middle_right);
    EXPECT_EQ(measuringY->explicitValue(), "Y = <>");
    EXPECT_DOUBLE_EQ(measuringY->value(), -3.0);
    ASSERT_NE(measuringY->layer(), nullptr);
    EXPECT_EQ(measuringY->layer()->name(), "0");

    boost::filesystem::remove(path);
}

// A linear dimension's angle is radians in LibreCAD and degrees in DXF group
// 50. It used to be copied across unconverted, so a vertical dimension from
// another program (50 = 90) came in as 90 radians, and its geometry block was
// written back with the dimension line at 116.6 degrees.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DxfRoundTripTest, ALinearDimensionKeepsItsAngleInDegreesInTheFile) {
    const std::string saved = uniqueTmpDxf("dimlinear-angle");
    {
        auto doc = newDocument();
        lc::builder::DimLinearBuilder dimension;
        dimension.setLayer(doc->layerByName("0"));
        dimension.setDefinitionPoint({60.0, 50.0});
        dimension.setDefinitionPoint2({0.0, 0.0});
        dimension.setDefinitionPoint3({100.0, 50.0});
        dimension.setMiddleOfText({60.0, 25.0});
        dimension.setAngle(M_PI / 2);
        ASSERT_NO_THROW(insertThroughBuilder(doc, {dimension.build()}));
        ASSERT_TRUE(lc::persistence::File::save(
            doc, saved, lc::persistence::File::Type::LIBDXFRW_DXF_R2000));
    }

    const auto dimensionGroups = groupsOfRecord(saved, "DIMENSION");
    ASSERT_EQ(dimensionGroups.count(50), 1u) << "a vertical dimension has to say so in group 50";
    EXPECT_DOUBLE_EQ(std::stod(dimensionGroups.at(50).front()), 90.0);

    // Its geometry block draws it vertical: the dimension line runs from the
    // foot of one extension line to the other, 50 long, at x = 60.
    const auto lineGroups = groupsOfRecord(saved, "LINE");
    bool verticalDimensionLine = false;
    for (std::size_t i = 0; i < lineGroups.at(10).size(); i++) {
        const double x1 = std::stod(lineGroups.at(10)[i]);
        const double y1 = std::stod(lineGroups.at(20)[i]);
        const double x2 = std::stod(lineGroups.at(11)[i]);
        const double y2 = std::stod(lineGroups.at(21)[i]);
        if (std::abs(x1 - 60.0) < 1e-6 && std::abs(x2 - 60.0) < 1e-6
            && std::abs(std::abs(y2 - y1) - 50.0) < 1e-6) {
            verticalDimensionLine = true;
        }
    }
    EXPECT_TRUE(verticalDimensionLine) << "the geometry block does not draw the dimension vertical";

    auto reopened = newDocument();
    ASSERT_NO_THROW(lc::persistence::File::open(
        reopened, saved, lc::persistence::File::Library::LIBDXFRW));
    lc::entity::DimLinear_CSPtr readBack;
    for (const auto& entity : reopened->entityContainer().asVector()) {
        if (auto linear = std::dynamic_pointer_cast<const lc::entity::DimLinear>(entity)) {
            readBack = linear;
        }
    }
    ASSERT_NE(readBack, nullptr) << "the dimension did not survive the round trip";
    EXPECT_NEAR(readBack->angle(), M_PI / 2, 1e-9);
}

TEST(DxfRoundTripTest, RationalSplineKeepsWeightsOnSave) {
    const std::string input = uniqueTmpDxf("rational-input");
    const std::string output = uniqueTmpDxf("rational-output");
    {
        std::ofstream dxf(input);
        dxf << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1015\n0\nENDSEC\n"
            << "0\nSECTION\n2\nENTITIES\n0\nSPLINE\n8\n0\n70\n4\n71\n2\n72\n6\n73\n3\n74\n0\n"
            << "40\n0\n40\n0\n40\n0\n40\n1\n40\n1\n40\n1\n"
            << "10\n1\n20\n0\n30\n3\n41\n1\n"
            << "10\n1\n20\n1\n30\n3\n41\n0.7071067811865476\n"
            << "10\n0\n20\n1\n30\n3\n41\n1\n0\nENDSEC\n0\nEOF\n";
    }
    auto doc = newDocument();
    const auto imported = lc::persistence::File::importFile(doc, input, lc::persistence::File::Library::LIBDXFRW);
    ASSERT_TRUE(imported.ok);
    ASSERT_EQ(imported.loss.notes.size(), 1u);
    EXPECT_NE(imported.loss.notes[0].find("displayed curve"), std::string::npos);
    lc::entity::Spline_CSPtr spline;
    for (const auto& entity : doc->entityContainer().asVector())
        if (auto value = std::dynamic_pointer_cast<const lc::entity::Spline>(entity)) spline = value;
    ASSERT_NE(spline, nullptr);
    const std::vector<double> expected{1.0, 0.7071067811865476, 1.0};
    EXPECT_EQ(spline->weights(), expected);
    const auto checkWeights = [&expected](const lc::entity::CADEntity_CSPtr& entity) {
        auto result = std::dynamic_pointer_cast<const lc::entity::Spline>(entity);
        ASSERT_NE(result, nullptr);
        EXPECT_EQ(result->weights(), expected);
    };
    checkWeights(spline->move({2,3,0}));
    checkWeights(spline->copy({2,3,0}));
    checkWeights(spline->rotate({0,0,0}, 0.5));
    checkWeights(spline->scale({0,0,0}, {2,3,1}));
    checkWeights(spline->mirror({0,0,0}, {1,0,0}));
    checkWeights(spline->modify(spline->layer(), spline->metaInfo(), spline->block()));
    checkWeights(spline->setDragPoints(spline->dragPoints()));
    checkWeights(spline->setProperties({}));
    checkWeights(std::make_shared<lc::entity::Spline>(spline, true));
    lc::builder::SplineBuilder builder;
    builder.copy(spline);
    EXPECT_EQ(builder.weights(), expected);
    EXPECT_EQ(builder.build()->weights(), expected);
    for (auto type : {lc::persistence::File::Type::LIBDXFRW_DXF_R2000,
                      lc::persistence::File::Type::LIBDXFRW_DXB_R2000}) {
        ASSERT_TRUE(lc::persistence::File::save(doc, output, type));
        auto reopened = newDocument();
        ASSERT_NO_THROW(lc::persistence::File::open(reopened, output, lc::persistence::File::Library::LIBDXFRW));
        lc::entity::Spline_CSPtr readback;
        for (const auto& entity : reopened->entityContainer().asVector())
            if (auto value = std::dynamic_pointer_cast<const lc::entity::Spline>(entity)) readback = value;
        ASSERT_NE(readback, nullptr);
        EXPECT_EQ(readback->weights(), expected);
        EXPECT_NE(readback->flags() & lc::geo::Spline::RATIONAL, 0);
    }
    // Some producers include non-unit weights without setting the rational bit.
    // The weights still affect geometry and must trigger the same warning.
    std::ifstream in(input);
    std::string source((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();
    const auto flag = source.find("70\n4\n71\n2\n");
    ASSERT_NE(flag, std::string::npos);
    source.replace(flag, std::string("70\n4\n").size(), "70\n0\n");
    { std::ofstream out(input, std::ios::trunc); out << source; }
    auto unflagged = newDocument();
    const auto unflaggedImport = lc::persistence::File::importFile(unflagged, input, lc::persistence::File::Library::LIBDXFRW);
    ASSERT_TRUE(unflaggedImport.ok);
    ASSERT_EQ(unflaggedImport.loss.notes.size(), 1u);
    EXPECT_NE(unflaggedImport.loss.notes[0].find("displayed curve"), std::string::npos);
    ASSERT_TRUE(lc::persistence::File::save(unflagged, output, lc::persistence::File::Type::LIBDXFRW_DXF_R2000));
    auto unflaggedReopen = newDocument();
    lc::persistence::File::open(unflaggedReopen, output, lc::persistence::File::Library::LIBDXFRW);
    lc::entity::Spline_CSPtr unflaggedSpline;
    for (const auto& entity : unflaggedReopen->entityContainer().asVector())
        if (auto value = std::dynamic_pointer_cast<const lc::entity::Spline>(entity)) unflaggedSpline = value;
    ASSERT_NE(unflaggedSpline, nullptr);
    EXPECT_EQ(unflaggedSpline->weights(), expected);
    boost::filesystem::remove(input);
    boost::filesystem::remove(output);
}

TEST(DxfRoundTripTest, BlockLayerVisibilitySurvivesSaveReopenAndUndo) {
    const auto source = uniqueTmpDxf("block-layer-source");
    {
        std::ofstream f(source);
        f << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1009\n0\nENDSEC\n"
             "0\nSECTION\n2\nTABLES\n0\nTABLE\n2\nLAYER\n70\n1\n"
             "0\nLAYER\n2\nVISIBLE_LAYER\n70\n0\n62\n3\n6\nCONTINUOUS\n"
             "0\nENDTAB\n0\nENDSEC\n0\nSECTION\n2\nBLOCKS\n"
             "0\nBLOCK\n8\n0\n2\nBLOCK_A\n70\n0\n10\n0\n20\n0\n30\n0\n3\nBLOCK_A\n"
             "0\nLINE\n8\nVISIBLE_LAYER\n10\n1\n20\n2\n30\n0\n11\n4\n21\n6\n31\n0\n"
             "0\nENDBLK\n8\n0\n0\nENDSEC\n0\nSECTION\n2\nENTITIES\n"
             "0\nINSERT\n8\n0\n2\nBLOCK_A\n10\n10\n20\n20\n30\n0\n"
             "0\nENDSEC\n0\nEOF\n";
    }
    auto doc = newDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(doc, source, lc::persistence::File::LIBDXFRW).ok);
    const auto on = doc->layerByName("VISIBLE_LAYER"); ASSERT_TRUE(on); EXPECT_FALSE(on->isOff());
    const auto block = doc->blockByName("BLOCK_A"); ASSERT_TRUE(block);
    auto children = doc->entitiesByBlock(block).asVector(); ASSERT_EQ(children.size(), 1u);
    const auto childId = children.front()->id();
    auto off = std::make_shared<lc::meta::Layer>(on->name(), on->lineWidth(), on->color(), on->linePattern(), on->isFrozen(), true);
    auto change = std::make_shared<lc::operation::Builder>(doc, "turn off");
    change->append(std::make_shared<lc::operation::ReplaceLayer>(doc, on, off)); change->execute();
    ASSERT_TRUE(doc->entityByID(childId)); EXPECT_EQ(doc->entityByID(childId)->layer(), off);
    for (const auto type : {lc::persistence::File::LIBDXFRW_DXF_R2004, lc::persistence::File::LIBDXFRW_DXB_R2004}) {
        const auto output = uniqueTmpDxf("block-layer-off");
        const auto written = lc::persistence::File::exportFile(doc, output, type); ASSERT_TRUE(written.ok);
        auto reopened = newDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(reopened, output, lc::persistence::File::LIBDXFRW).ok);
        auto layer = reopened->layerByName("VISIBLE_LAYER"); ASSERT_TRUE(layer); EXPECT_TRUE(layer->isOff());
        auto reopenedBlock = reopened->blockByName("BLOCK_A"); ASSERT_TRUE(reopenedBlock);
        const auto reopenedChildren = reopened->entitiesByBlock(reopenedBlock).asVector(); ASSERT_EQ(reopenedChildren.size(), 1u);
        auto line = std::dynamic_pointer_cast<const lc::entity::Line>(reopenedChildren.front()); ASSERT_TRUE(line);
        EXPECT_EQ(line->layer(), layer); EXPECT_DOUBLE_EQ(line->start().x(), 1); EXPECT_DOUBLE_EQ(line->end().y(), 6);
        boost::filesystem::remove(output);
    }
    change->undo();
    ASSERT_TRUE(doc->entityByID(childId)); EXPECT_EQ(doc->entityByID(childId)->layer(), on);
    change->redo();
    ASSERT_TRUE(doc->entityByID(childId)); EXPECT_EQ(doc->entityByID(childId)->layer(), off);
    boost::filesystem::remove(source);
}

TEST(DxfRoundTripTest, FrozenAndLockedFlagsRoundTripIndependently) {
    const auto source = uniqueTmpDxf("layer-state-source");
    {
        std::ofstream f(source);
        f << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1009\n0\nENDSEC\n"
             "0\nSECTION\n2\nTABLES\n0\nTABLE\n2\nLAYER\n70\n3\n"
             "0\nLAYER\n2\nFROZEN\n70\n1\n62\n3\n6\nCONTINUOUS\n"
             "0\nLAYER\n2\nLOCKED\n70\n4\n62\n3\n6\nCONTINUOUS\n"
             "0\nLAYER\n2\nBOTH\n70\n5\n62\n-3\n6\nCONTINUOUS\n"
             "0\nENDTAB\n0\nENDSEC\n0\nEOF\n";
    }
    auto check = [](const std::shared_ptr<lc::storage::DocumentImpl>& doc) {
        auto frozen = doc->layerByName("FROZEN"), locked = doc->layerByName("LOCKED"), both = doc->layerByName("BOTH");
        ASSERT_TRUE(frozen); ASSERT_TRUE(locked); ASSERT_TRUE(both);
        EXPECT_TRUE(frozen->isFrozen()); EXPECT_FALSE(frozen->isLocked()); EXPECT_FALSE(frozen->isOff());
        EXPECT_FALSE(locked->isFrozen()); EXPECT_TRUE(locked->isLocked()); EXPECT_FALSE(locked->isOff());
        EXPECT_TRUE(both->isFrozen()); EXPECT_TRUE(both->isLocked()); EXPECT_TRUE(both->isOff());
    };
    auto doc = newDocument(); ASSERT_TRUE(lc::persistence::File::importFile(doc, source, lc::persistence::File::LIBDXFRW).ok); check(doc);
    for (const auto type : {lc::persistence::File::LIBDXFRW_DXF_R2004, lc::persistence::File::LIBDXFRW_DXB_R2004}) {
        auto path = uniqueTmpDxf("layer-state-output");
        ASSERT_TRUE(lc::persistence::File::exportFile(doc, path, type).ok);
        auto reopened = newDocument(); ASSERT_TRUE(lc::persistence::File::importFile(reopened, path, lc::persistence::File::LIBDXFRW).ok);
        check(reopened); boost::filesystem::remove(path);
    }
    boost::filesystem::remove(source);
}

TEST(DxfRoundTripTest, BlockEntityTransferUndoRedoSurvivesSaveReopen) {
    auto doc = newDocument();
    auto first = std::make_shared<lc::meta::Block>("BLOCK_FIRST");
    auto second = std::make_shared<lc::meta::Block>("BLOCK_SECOND");
    doc->addDocumentMetaType(first); doc->addDocumentMetaType(second);
    auto original = std::make_shared<lc::entity::Line>(lc::geo::Coordinate(1, 2, 3), lc::geo::Coordinate(4, 6, 9),
                                                       doc->layerByName("0"), nullptr, first);
    auto add = std::make_shared<lc::operation::EntityBuilder>(doc); add->appendEntity(original); add->execute();
    auto moved = original->move(lc::geo::Coordinate(5, 7, 11));
    auto edit = std::make_shared<lc::operation::EntityBuilder>(doc); edit->appendEntity(moved); edit->execute();
    auto relocated = moved->modify(moved->layer(), moved->metaInfo(), second);
    auto transfer = std::make_shared<lc::operation::EntityBuilder>(doc); transfer->appendEntity(relocated); transfer->execute();
    auto checkSave = [&](lc::persistence::File::Type type, const char* populated, const char* empty) {
        auto output = uniqueTmpDxf("block-transfer");
        ASSERT_TRUE(lc::persistence::File::exportFile(doc, output, type).ok);
        auto reopened = newDocument(); ASSERT_TRUE(lc::persistence::File::importFile(reopened, output, lc::persistence::File::LIBDXFRW).ok);
        auto destination = reopened->blockByName(populated), other = reopened->blockByName(empty);
        ASSERT_TRUE(destination); ASSERT_TRUE(other);
        auto children = reopened->entitiesByBlock(destination).asVector(); ASSERT_EQ(children.size(), 1u);
        EXPECT_TRUE(reopened->entitiesByBlock(other).asVector().empty());
        auto line = std::dynamic_pointer_cast<const lc::entity::Line>(children.front()); ASSERT_TRUE(line);
        EXPECT_DOUBLE_EQ(line->start().x(), 6); EXPECT_DOUBLE_EQ(line->start().y(), 9); EXPECT_DOUBLE_EQ(line->start().z(), 14);
        EXPECT_DOUBLE_EQ(line->end().x(), 9); EXPECT_DOUBLE_EQ(line->end().y(), 13); EXPECT_DOUBLE_EQ(line->end().z(), 20);
        boost::filesystem::remove(output);
    };
    checkSave(lc::persistence::File::LIBDXFRW_DXF_R2004, "BLOCK_SECOND", "BLOCK_FIRST");
    transfer->undo();
    checkSave(lc::persistence::File::LIBDXFRW_DXF_R2004, "BLOCK_FIRST", "BLOCK_SECOND");
    transfer->redo();
    checkSave(lc::persistence::File::LIBDXFRW_DXB_R2004, "BLOCK_SECOND", "BLOCK_FIRST");
}

TEST(DxfRoundTripTest, LayerStateChangesPreserveOnlyTheirOwnColorProvenance) {
    const auto source = uniqueTmpDxf("layer-state-color-source");
    {
        std::ofstream f(source);
        f << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1018\n0\nENDSEC\n"
             "0\nSECTION\n2\nTABLES\n0\nTABLE\n2\nLAYER\n5\n2\n70\n1\n"
             "0\nLAYER\n5\n60\n100\nAcDbSymbolTableRecord\n100\nAcDbLayerTableRecord\n"
             "2\nNAMED\n70\n0\n62\n-3\n420\n0\n430\nTest$Black\n6\nCONTINUOUS\n"
             "0\nENDTAB\n0\nENDSEC\n0\nEOF\n";
    }
    auto doc = newDocument(); ASSERT_TRUE(lc::persistence::File::importFile(doc, source, lc::persistence::File::LIBDXFRW).ok);
    const auto original = doc->layerByName("NAMED"); ASSERT_TRUE(original); ASSERT_TRUE(original->isOff());
    const auto changed = lc::meta::Layer::withState(original, true, false, true); ASSERT_TRUE(changed);
    auto edit = std::make_shared<lc::operation::Builder>(doc, "change state");
    edit->append(std::make_shared<lc::operation::ReplaceLayer>(doc, original, changed)); edit->execute();
    auto checkAscii = [](const std::string& path, bool hasName, int index) {
        std::ifstream f(path); std::string code, value; bool layer = false, named = false;
        std::map<int, std::string> values;
        while (std::getline(f, code) && std::getline(f, value)) {
            const int group = std::stoi(code);
            if (group == 0) {
                if (layer && named) break;
                layer = value == "LAYER"; named = false; values.clear();
            } else if (layer) {
                values[group] = value;
                if (group == 2) named = value == "NAMED";
            }
        }
        ASSERT_TRUE(named); ASSERT_EQ(values.count(62), 1u); EXPECT_EQ(std::stoi(values[62]), index);
        ASSERT_EQ(values.count(420), 1u); EXPECT_EQ(std::stoi(values[420]), 0);
        EXPECT_EQ(values.count(430) != 0, hasName);
        if (hasName) EXPECT_EQ(values[430], "Test$Black");
    };
    const auto onOutput = uniqueTmpDxf("layer-state-color-on");
    ASSERT_TRUE(lc::persistence::File::exportFile(doc, onOutput, lc::persistence::File::LIBDXFRW_DXF_R2004).ok);
    checkAscii(onOutput, true, 3); boost::filesystem::remove(onOutput);
    const auto current = lc::meta::Layer::withState(changed, false, true, false); ASSERT_TRUE(current);
    auto editAgain = std::make_shared<lc::operation::Builder>(doc, "change state again");
    editAgain->append(std::make_shared<lc::operation::ReplaceLayer>(doc, changed, current)); editAgain->execute();
    for (const auto type : {lc::persistence::File::LIBDXFRW_DXF_R2004, lc::persistence::File::LIBDXFRW_DXB_R2004}) {
        const auto output = uniqueTmpDxf("layer-state-color-output");
        const auto result = lc::persistence::File::exportFile(doc, output, type); ASSERT_TRUE(result.ok);
        EXPECT_TRUE(std::none_of(result.loss.notes.begin(), result.loss.notes.end(), [](const std::string& note) {
            return note.find("source color name") != std::string::npos;
        }));
        if (type == lc::persistence::File::LIBDXFRW_DXF_R2004) checkAscii(output, true, -3);
        else {
            auto reopened = newDocument(); ASSERT_TRUE(lc::persistence::File::importFile(reopened, output, lc::persistence::File::LIBDXFRW).ok);
            const auto ascii = uniqueTmpDxf("layer-state-color-ascii");
            ASSERT_TRUE(lc::persistence::File::exportFile(reopened, ascii, lc::persistence::File::LIBDXFRW_DXF_R2004).ok);
            checkAscii(ascii, true, -3); boost::filesystem::remove(ascii);
        }
        boost::filesystem::remove(output);
    }
    // A separately constructed replacement, even with equal RGB, is not a
    // state-only descendant and must not inherit a stale source color name.
    const auto unrelated = std::make_shared<lc::meta::Layer>("NAMED", current->lineWidth(), current->color(),
        current->linePattern(), current->isFrozen(), current->isOff(), current->isLocked());
    auto replace = std::make_shared<lc::operation::Builder>(doc, "replace color");
    replace->append(std::make_shared<lc::operation::ReplaceLayer>(doc, current, unrelated)); replace->execute();
    const auto output = uniqueTmpDxf("layer-state-color-replaced");
    const auto result = lc::persistence::File::exportFile(doc, output, lc::persistence::File::LIBDXFRW_DXF_R2004);
    ASSERT_TRUE(result.ok);
    EXPECT_TRUE(std::any_of(result.loss.notes.begin(), result.loss.notes.end(), [](const std::string& note) {
        return note.find("source color name") != std::string::npos;
    }));
    std::ifstream f(output); std::string text((std::istreambuf_iterator<char>(f)), {});
    EXPECT_EQ(text.find("Test$Black"), std::string::npos); f.close();
    boost::filesystem::remove(output); boost::filesystem::remove(source);
}
