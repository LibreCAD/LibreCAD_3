// DWG import, behind WITH_DWG_IMPORT.
//
// LibreCAD has offered DWG in its open dialog for years with nothing behind it:
// libopencad was optional, unmaintained and off in every shipped build. The
// reader that works is the one libdxfrw already brings -- dwgRW takes the same
// DRW_Interface that DXFimpl implements, so there is no second adapter.
//
// It is off by default, because a format LibreCAD can read and cannot write
// puts the user in a position worth introducing deliberately: the drawing has
// to be saved somewhere other than where it came from.
//
// Fixtures are libdxfrw's own, under third_party/libdxfrw/tests/fixtures/dwg,
// so they come with the submodule and need no corpus of their own.

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include <gtest/gtest.h>

#include <cad/primitive/arc.h>
#include <cad/primitive/ellipse.h>
#include <cad/primitive/lwpolyline.h>
#include <cad/primitive/spline.h>
#include <cad/primitive/dimension.h>
#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>

#include <cad/primitive/dimradial.h>
#include <cad/primitive/hatch.h>
#include <cad/primitive/line.h>
#include <cad/primitive/text.h>

#include "persistence/file.h"
#include "persistence/documentsource.h"
#include "persistence/format.h"

#ifndef USE_DWG_IMPORT
#define USE_DWG_IMPORT 0
#endif

#ifndef DWG_FIXTURES_DIR
#error "DWG_FIXTURES_DIR must name libdxfrw's DWG fixture directory (see unittest/CMakeLists.txt)"
#endif

namespace {

std::shared_ptr<lc::storage::DocumentImpl> newDwgDocument() {
    return std::make_shared<lc::storage::DocumentImpl>(
        std::make_shared<lc::storage::StorageManagerImpl>());
}

std::string dwgFixture(const char* name) {
    return std::string(DWG_FIXTURES_DIR) + "/" + name;
}

std::string tmpPath(const char* tag) {
    return (boost::filesystem::temp_directory_path()
            / boost::filesystem::unique_path(
                std::string("lc3-dwg-") + tag + "-%%%%%%%%-%%%%%%%%.dxf"))
        .string();
}

// Every fixture that ships with the submodule.
const char* const kFixtures[] = {
    "large_radial.dwg",
    "mpolygon_solid.dwg",
    "ordinary_enc_AC1015.dwg",
    "ordinary_enc_AC1018.dwg",
    "ordinary_enc_AC1021.dwg",
    "ordinary_enc_AC1027.dwg",
    "ordinary_enc_ac1027_ansi932.dwg",
    "rtext_arctext.dwg",
};

}  // namespace

// The reader follows the file's contents, not its name. A DXF saved as .dwg and
// a DWG saved as .dxf are both ordinary, and choosing by extension makes either
// unopenable.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DwgImportTest, FormatIsSniffedFromContentNotName) {
    for (const char* name : kFixtures) {
        EXPECT_EQ(lc::persistence::File::sniffFormat(dwgFixture(name)), "dwg") << name;
    }

    // Copied to a .dxf name, it is still a DWG.
    const std::string misnamed = tmpPath("misnamed");
    boost::filesystem::remove(misnamed);
    boost::filesystem::copy_file(dwgFixture("ordinary_enc_AC1015.dwg"), misnamed);
    EXPECT_EQ(lc::persistence::File::sniffFormat(misnamed), "dwg")
        << "A DWG named .dxf is still a DWG.";
    boost::filesystem::remove(misnamed);

    // And a DXF is a DXF whatever it is called. The binary sentinel also starts
    // with 'A', which is why the check is the whole six-byte tag.
    const std::string ascii = tmpPath("ascii");
    const std::string binary = tmpPath("binary");
    for (const auto& pair : std::map<std::string, lc::persistence::File::Type>{
             {ascii, lc::persistence::File::LIBDXFRW_DXF_R2000},
             {binary, lc::persistence::File::LIBDXFRW_DXB_R2000}}) {
        boost::filesystem::remove(pair.first);
        auto doc = newDwgDocument();
        ASSERT_TRUE(lc::persistence::File::save(doc, pair.first, pair.second));
        EXPECT_EQ(lc::persistence::File::sniffFormat(pair.first), "dxf") << pair.first;
        boost::filesystem::remove(pair.first);
    }

    EXPECT_EQ(lc::persistence::File::sniffFormat(tmpPath("does-not-exist")), "dxf")
        << "A file that cannot be read is left to the DXF reader to report on.";
}

#if USE_DWG_IMPORT

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DwgImportTest, FixturesPopulateADocument) {
    for (const char* name : kFixtures) {
        auto doc = newDwgDocument();
        lc::persistence::ImportResult result;
        ASSERT_NO_THROW(result = lc::persistence::File::importFile(
            doc, dwgFixture(name), lc::persistence::File::LIBDXFRW)) << name;

        EXPECT_TRUE(result.ok) << name;
        EXPECT_FALSE(result.partial) << name;
        EXPECT_TRUE(result.failures.empty()) << name;
        EXPECT_GT(result.entitiesDelivered, 0u) << name << " delivered nothing";
        EXPECT_EQ(result.variantId, "dwg.libdxfrw") << name;

        // DWG is read, never written: a document from one must be sent to Save
        // As rather than back over the file it came from.
        const auto source = lc::persistence::sourceFromImport(dwgFixture(name), result);
        EXPECT_FALSE(source.writable) << name;
        EXPECT_EQ(lc::persistence::decideSave(source).action,
                  lc::persistence::SaveAction::AskForTarget) << name;
    }
}

// The parity the plan asks for: what a DWG puts in a document must survive
// LibreCAD's own DXF export of it.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DwgImportTest, DwgAndItsDxfExportAgree) {
    for (const char* name : kFixtures) {
        auto fromDwg = newDwgDocument();
        const auto read = lc::persistence::File::importFile(
            fromDwg, dwgFixture(name), lc::persistence::File::LIBDXFRW);
        ASSERT_TRUE(read.ok) << name;

        const std::string exported = tmpPath(name);
        boost::filesystem::remove(exported);
        const auto written = lc::persistence::File::exportFile(
            fromDwg, exported, lc::persistence::File::LIBDXFRW_DXF_R2000);
        ASSERT_TRUE(written.ok) << name;

        auto fromDxf = newDwgDocument();
        const auto reread = lc::persistence::File::importFile(
            fromDxf, exported, lc::persistence::File::LIBDXFRW);
        ASSERT_TRUE(reread.ok) << name;

        // Blocks the user made must survive exactly. Anonymous dimension
        // blocks (*D<n>) are excluded on purpose: they hold the geometry a
        // dimension is drawn with, LibreCAD mints its own on every write, and
        // a source file can carry more or fewer of them than it has dimensions
        // -- large_radial.dwg carries two for one dimension. Comparing the raw
        // counts asserted that LibreCAD reproduces another writer's bookkeeping,
        // which it neither does nor should.
        const auto namedBlocks = [](const std::shared_ptr<lc::storage::Document>& doc) {
            std::vector<std::string> names;
            for (const auto& block : doc->blocks()) {
                const std::string& blockName = block->name();
                const bool anonymousDimension =
                    blockName.size() >= 3 && blockName[0] == '*'
                    && (blockName[1] == 'D' || blockName[1] == 'd')
                    && blockName.find_first_not_of("0123456789", 2) == std::string::npos;
                if (!anonymousDimension) {
                    names.push_back(blockName);
                }
            }
            std::sort(names.begin(), names.end());
            return names;
        };
        EXPECT_EQ(namedBlocks(fromDxf), namedBlocks(fromDwg)) << name;

        // And the export must give every dimension exactly one block to draw
        // with -- no strays carried over, none missing.
        std::size_t dimensions = 0;
        for (const auto& entity : fromDwg->entityContainer().asVector()) {
            if (std::dynamic_pointer_cast<const lc::entity::Dimension>(entity)) {
                dimensions++;
            }
        }
        std::size_t anonymousInExport = 0;
        for (const auto& block : fromDxf->blocks()) {
            const std::string& blockName = block->name();
            if (blockName.size() >= 3 && blockName[0] == '*'
                && (blockName[1] == 'D' || blockName[1] == 'd')
                && blockName.find_first_not_of("0123456789", 2) == std::string::npos) {
                anonymousInExport++;
            }
        }
        EXPECT_EQ(anonymousInExport, dimensions)
            << name << ": one anonymous block per dimension, no strays";
        EXPECT_EQ(fromDxf->allLayers().size(), fromDwg->allLayers().size()) << name;

        // Entities are allowed to differ only by what the export said it could
        // not carry -- never silently.
        const std::size_t before = fromDwg->entityContainer().asVector().size();
        const std::size_t after = fromDxf->entityContainer().asVector().size();
        EXPECT_EQ(after + written.loss.total(), before + reread.loss.total())
            << name << ": " << before << " entities became " << after
            << " and the export reported " << written.loss.total() << " left behind";

        boost::filesystem::remove(exported);
    }
}

// The five ordinary_enc_* fixtures are one drawing saved at five revisions with
// different string encodings. Same three lines every time -- which is the point
// of them, and which counting entities would never check.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DwgImportTest, EveryEncodingCarriesTheSameGeometry) {
    const struct {
        double startX;
        double startY;
        double endX;
        double endY;
    } expected[] = {{1.0, 2.0, 3.0, 4.0}, {5.0, 6.0, 7.0, 8.0}, {9.0, 10.0, 11.0, 12.0}};

    for (const char* name : {"ordinary_enc_AC1015.dwg", "ordinary_enc_AC1018.dwg",
                             "ordinary_enc_AC1021.dwg", "ordinary_enc_AC1027.dwg",
                             "ordinary_enc_ac1027_ansi932.dwg"}) {
        auto doc = newDwgDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(
            doc, dwgFixture(name), lc::persistence::File::LIBDXFRW).ok) << name;

        std::vector<lc::entity::Line_CSPtr> lines;
        for (const auto& entity : doc->entityContainer().asVector()) {
            if (auto line = std::dynamic_pointer_cast<const lc::entity::Line>(entity)) {
                lines.push_back(line);
            }
        }

        ASSERT_EQ(lines.size(), 3u) << name;
        std::sort(lines.begin(), lines.end(),
                  [](const lc::entity::Line_CSPtr& a, const lc::entity::Line_CSPtr& b) {
                      return a->start().x() < b->start().x();
                  });

        for (std::size_t i = 0; i < 3; i++) {
            EXPECT_DOUBLE_EQ(lines[i]->start().x(), expected[i].startX) << name << " line " << i;
            EXPECT_DOUBLE_EQ(lines[i]->start().y(), expected[i].startY) << name << " line " << i;
            EXPECT_DOUBLE_EQ(lines[i]->end().x(), expected[i].endX) << name << " line " << i;
            EXPECT_DOUBLE_EQ(lines[i]->end().y(), expected[i].endY) << name << " line " << i;
            ASSERT_NE(lines[i]->layer(), nullptr) << name;
            EXPECT_EQ(lines[i]->layer()->name(), "0") << name;
        }
    }
}

// Fields, not just counts: a reader that produced the right number of entities
// with the wrong contents would pass every count-based assertion here.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DwgImportTest, TextAndDimensionFieldsSurvive) {
    auto texts = newDwgDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(
        texts, dwgFixture("rtext_arctext.dwg"), lc::persistence::File::LIBDXFRW).ok);

    std::map<std::string, lc::entity::Text_CSPtr> byValue;
    for (const auto& entity : texts->entityContainer().asVector()) {
        if (auto text = std::dynamic_pointer_cast<const lc::entity::Text>(entity)) {
            byValue[text->text_value()] = text;
        }
    }

    ASSERT_EQ(byValue.count("RTEXT-DIESEL-TEST"), 1u);
    EXPECT_DOUBLE_EQ(byValue["RTEXT-DIESEL-TEST"]->height(), 2.5);
    EXPECT_DOUBLE_EQ(byValue["RTEXT-DIESEL-TEST"]->insertion_point().x(), 100.0);
    EXPECT_DOUBLE_EQ(byValue["RTEXT-DIESEL-TEST"]->insertion_point().y(), 200.0);

    ASSERT_EQ(byValue.count("ARC-TEXT-TEST"), 1u);
    EXPECT_DOUBLE_EQ(byValue["ARC-TEXT-TEST"]->height(), 2.5);

    auto radial = newDwgDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(
        radial, dwgFixture("large_radial.dwg"), lc::persistence::File::LIBDXFRW).ok);

    lc::entity::DimRadial_CSPtr dimension;
    for (const auto& entity : radial->entityContainer().asVector()) {
        if (auto found = std::dynamic_pointer_cast<const lc::entity::DimRadial>(entity)) {
            dimension = found;
        }
    }
    ASSERT_NE(dimension, nullptr) << "the radial dimension did not arrive as one";
    EXPECT_DOUBLE_EQ(dimension->definitionPoint().x(), 5.0);
    EXPECT_DOUBLE_EQ(dimension->definitionPoint().y(), 6.0);
    EXPECT_DOUBLE_EQ(dimension->definitionPoint2().x(), 10.0);
    EXPECT_DOUBLE_EQ(dimension->definitionPoint2().y(), 0.0);
}

// A known gap, pinned so that closing it is noticed.
//
// An MPOLYGON arrives as a Hatch with no boundary loops at all: the reader has
// no MPOLYGON boundary path, so nothing reaches the entity. That is why the
// hatch does not survive a DXF export -- there is nothing to write, not
// something the writer mishandles -- and why the export counts it as a loss
// rather than dropping it in silence.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DwgImportTest, MpolygonArrivesWithoutItsBoundary) {
    auto doc = newDwgDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(
        doc, dwgFixture("mpolygon_solid.dwg"), lc::persistence::File::LIBDXFRW).ok);

    lc::entity::Hatch_CSPtr hatch;
    for (const auto& entity : doc->entityContainer().asVector()) {
        if (auto found = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity)) {
            hatch = found;
        }
    }
    ASSERT_NE(hatch, nullptr);
    EXPECT_TRUE(hatch->isSolid());
    EXPECT_EQ(hatch->getRegion().loopList().size(), 0u)
        << "If the boundary now arrives, this gap has closed: assert the loops "
           "and update DwgAndItsDxfExportAgree, which allows for its loss.";

    // The export says so rather than dropping it quietly.
    const std::string exported = tmpPath("mpolygon");
    boost::filesystem::remove(exported);
    const auto written = lc::persistence::File::exportFile(
        doc, exported, lc::persistence::File::LIBDXFRW_DXF_R2000);
    EXPECT_TRUE(written.ok);
    EXPECT_EQ(written.loss.droppedByType.count("HATCH"), 1u);
    boost::filesystem::remove(exported);
}

// A DWG is a container of self-describing objects, and a corrupt or hostile one
// can claim to hold far more than it does. The budget is what turns that from
// an allocation failure deep inside a parser into a refusal with a reason the
// user can act on -- raise the budget, or distrust the file.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DwgImportTest, ABudgetExceedingFileIsRefusedWithAReason) {
    const std::string fixture = dwgFixture("rtext_arctext.dwg");

    // Whatever the default is, this file reads within it.
    auto within = newDwgDocument();
    const auto normal = lc::persistence::File::importFile(
        within, fixture, lc::persistence::File::LIBDXFRW);
    ASSERT_TRUE(normal.ok);
    ASSERT_GT(normal.entitiesDelivered, 3u);

    // A budget it cannot fit in is a refusal, not a partial read and not a
    // crash: nothing is delivered, and the code says which limit was hit.
    lc::persistence::File::setDwgReadObjectBudget(3);

    auto beyond = newDwgDocument();
    lc::persistence::ImportResult result;
    ASSERT_NO_THROW(result = lc::persistence::File::importFile(
        beyond, fixture, lc::persistence::File::LIBDXFRW));

    EXPECT_FALSE(result.ok);
    EXPECT_FALSE(result.partial);
    EXPECT_EQ(result.entitiesDelivered, 0u);
    EXPECT_EQ(beyond->entityContainer().asVector().size(), 0u);
    ASSERT_FALSE(result.diagnostics.empty());
    EXPECT_EQ(result.diagnostics.front().code, "dwg-object-budget")
        << "A resource limit must be distinguishable from a malformed file.";

    // Back to the library's own default, so the order tests run in cannot
    // matter.
    lc::persistence::File::setDwgReadObjectBudget(0);
    EXPECT_EQ(lc::persistence::File::dwgReadObjectBudget(), 0u);

    auto again = newDwgDocument();
    EXPECT_TRUE(lc::persistence::File::importFile(
        again, fixture, lc::persistence::File::LIBDXFRW).ok)
        << "The budget must be restorable, or one test poisons the rest.";
}

// A ceiling, not a benchmark. These fixtures are small; a read that suddenly
// takes seconds means something has gone quadratic, which is how DWG reading
// fails in practice rather than by crashing.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DwgImportTest, FixturesReadWellWithinTheirBudget) {
    for (const char* name : kFixtures) {
        const auto started = std::chrono::steady_clock::now();

        auto doc = newDwgDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(
            doc, dwgFixture(name), lc::persistence::File::LIBDXFRW).ok) << name;

        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - started).count();
        EXPECT_LT(elapsed, 2000) << name << " took " << elapsed << " ms";
    }
}

#else

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DwgImportTest, DwgIsRefusedWhenTheBuildHasNoReader) {
    auto doc = newDwgDocument();
    const auto result = lc::persistence::File::importFile(
        doc, dwgFixture("ordinary_enc_AC1015.dwg"), lc::persistence::File::LIBDXFRW);

    EXPECT_FALSE(result.ok);
    EXPECT_FALSE(result.partial);
    EXPECT_EQ(result.entitiesDelivered, 0u);
    EXPECT_EQ(doc->entityContainer().asVector().size(), 0u)
        << "A refused open must leave nothing behind for Save to write back.";
    ASSERT_FALSE(result.diagnostics.empty());
    EXPECT_EQ(result.diagnostics.front().code, "dwg-not-enabled")
        << "The refusal needs a code the UI can act on.";

    // And the format is not offered in the first place.
    EXPECT_TRUE(lc::persistence::File::getSupportedFileExtensions().count("dwg") == 0);
    EXPECT_TRUE(lc::persistence::File::getAvailableLibrariesForFormat("dwg").empty());
}

#endif

TEST(DwgImportTest, ElevatedLineEdgeHatchRoundTrip) {
    const std::string input = tmpPath("elevated-line-hatch");
    {
        std::ofstream f(input);
        ASSERT_TRUE(f.good());
        f << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1018\n0\nENDSEC\n"
             "0\nSECTION\n2\nENTITIES\n0\nHATCH\n5\n55\n100\nAcDbEntity\n8\n0\n"
             "100\nAcDbHatch\n10\n0\n20\n0\n30\n3\n210\n0\n220\n0\n230\n1\n"
             "2\nSOLID\n70\n1\n71\n0\n91\n1\n92\n0\n93\n3\n"
             "72\n1\n10\n1\n20\n2\n11\n4\n21\n2\n"
             "72\n1\n10\n4\n20\n2\n11\n3\n21\n5\n"
             "72\n1\n10\n3\n20\n5\n11\n1\n21\n2\n"
             "97\n0\n75\n0\n76\n1\n78\n0\n98\n0\n0\nENDSEC\n0\nEOF\n";
        ASSERT_TRUE(f.good());
    }
    const auto check = [](const std::shared_ptr<lc::storage::DocumentImpl>& doc) {
        lc::entity::Hatch_CSPtr hatch;
        for (const auto& entity : doc->entityContainer().asVector())
            if (auto value = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity)) hatch = value;
        ASSERT_NE(hatch, nullptr);
        const auto& loops = hatch->getRegion().loopList();
        ASSERT_EQ(loops.size(), 1u);
        ASSERT_EQ(loops.front().entities().size(), 3u);
        const double expected[3][4] = {{1,2,4,2}, {4,2,3,5}, {3,5,1,2}};
        std::size_t index = 0;
        for (const auto& edge : loops.front().entities()) {
            const auto line = std::dynamic_pointer_cast<const lc::entity::Line>(edge);
            ASSERT_NE(line, nullptr);
            EXPECT_DOUBLE_EQ(line->start().x(), expected[index][0]);
            EXPECT_DOUBLE_EQ(line->start().y(), expected[index][1]);
            EXPECT_DOUBLE_EQ(line->end().x(), expected[index][2]);
            EXPECT_DOUBLE_EQ(line->end().y(), expected[index][3]);
            EXPECT_DOUBLE_EQ(line->start().z(), 3.0);
            EXPECT_DOUBLE_EQ(line->end().z(), 3.0);
            ++index;
        }
    };
    auto doc = newDwgDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(doc, input, lc::persistence::File::LIBDXFRW).ok);
    check(doc);
    const char* capture = std::getenv("LC3_EDGE_CAPTURE");
    for (const auto type : {lc::persistence::File::LIBDXFRW_DXF_R2000,
                            lc::persistence::File::LIBDXFRW_DXB_R2000}) {
        const auto output = capture ? std::string(capture) + (type == lc::persistence::File::LIBDXFRW_DXF_R2000 ? "/line-ascii.dxf" : "/line-binary.dxf") : tmpPath("elevated-line-hatch-output");
        ASSERT_TRUE(lc::persistence::File::exportFile(doc, output, type).ok);
        auto reopened = newDwgDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(reopened, output, lc::persistence::File::LIBDXFRW).ok);
        check(reopened);
        if (!capture) boost::filesystem::remove(output);
    }
    lc::entity::Hatch_CSPtr original;
    for (const auto& entity : doc->entityContainer().asVector())
        if (auto value = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity)) original = value;
    ASSERT_NE(original, nullptr);
    const auto& edges = original->getRegion().loopList().front().entities();
    std::vector<lc::entity::CADEntity_CSPtr> modified;
    for (std::size_t i = 0; i < edges.size(); ++i) {
        const auto line = std::dynamic_pointer_cast<const lc::entity::Line>(edges[i]);
        ASSERT_NE(line, nullptr);
        const double z = i == 1 ? 4.0 : 3.0;
        modified.push_back(std::make_shared<lc::entity::Line>(
            lc::geo::Coordinate(line->start().x(), line->start().y(), z),
            lc::geo::Coordinate(line->end().x(), line->end().y(), z), line->layer()));
    }
    lc::geo::Region mixedRegion;
    mixedRegion.addLoop(lc::geo::Loop(modified));
    auto mixed = std::make_shared<lc::entity::Hatch>(original->layer(), original->metaInfo(), original->block());
    mixed->setRegion(mixedRegion);
    mixed->setPatternName(original->getPatternName());
    mixed->setSolid(original->isSolid());
    doc->removeEntity(original);
    doc->insertEntity(mixed);
    const auto mixedOutput = capture ? std::string(capture) + "/mixed.dxf" : tmpPath("mixed-line-hatch");
    const auto mixedResult = lc::persistence::File::exportFile(doc, mixedOutput, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(mixedResult.ok);
    EXPECT_TRUE(std::any_of(mixedResult.loss.notes.begin(), mixedResult.loss.notes.end(),
        [](const std::string& note) { return note.find("HATCH boundary elevation varies") != std::string::npos; }));
    doc->removeEntity(mixed);
    doc->insertEntity(original);
    const auto retry = capture ? std::string(capture) + "/retry.dxf" : tmpPath("retry-line-hatch");
    ASSERT_TRUE(lc::persistence::File::exportFile(doc, retry, lc::persistence::File::LIBDXFRW_DXF_R2000).ok);
    auto reopened = newDwgDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(reopened, retry, lc::persistence::File::LIBDXFRW).ok);
    check(reopened);
    if (!capture) { boost::filesystem::remove(mixedOutput); boost::filesystem::remove(retry); }
    boost::filesystem::remove(input);
}

TEST(DwgImportTest, ElevatedMixedLinePolylineHatchRoundTrip) {
    const auto input = tmpPath("mixed-edge-paths");
    {
        std::ofstream f(input);
        ASSERT_TRUE(f.good());
        f << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1018\n0\nENDSEC\n"
             "0\nSECTION\n2\nENTITIES\n0\nHATCH\n5\n55\n100\nAcDbEntity\n8\n0\n"
             "100\nAcDbHatch\n10\n0\n20\n0\n30\n3\n210\n0\n220\n0\n230\n1\n"
             "2\nSOLID\n70\n1\n71\n0\n91\n2\n92\n0\n93\n3\n"
             "72\n1\n10\n1\n20\n2\n11\n4\n21\n2\n"
             "72\n1\n10\n4\n20\n2\n11\n3\n21\n5\n"
             "72\n1\n10\n3\n20\n5\n11\n1\n21\n2\n97\n0\n"
             "92\n2\n73\n1\n72\n0\n93\n3\n"
             "10\n7\n20\n2\n10\n9\n20\n2\n10\n8\n20\n4\n97\n0\n"
             "75\n0\n76\n1\n78\n0\n98\n0\n0\nENDSEC\n0\nEOF\n";
        ASSERT_TRUE(f.good());
    }
    const auto check = [](const std::shared_ptr<lc::storage::DocumentImpl>& doc) {
        lc::entity::Hatch_CSPtr hatch;
        for (const auto& entity : doc->entityContainer().asVector())
            if (auto value = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity)) hatch = value;
        ASSERT_NE(hatch, nullptr);
        const auto& loops = hatch->getRegion().loopList();
        ASSERT_EQ(loops.size(), 2u);
        ASSERT_EQ(loops[0].entities().size(), 3u);
        for (const auto& edge : loops[0].entities()) {
            const auto line = std::dynamic_pointer_cast<const lc::entity::Line>(edge);
            ASSERT_NE(line, nullptr);
            EXPECT_DOUBLE_EQ(line->start().z(), 3.0);
            EXPECT_DOUBLE_EQ(line->end().z(), 3.0);
        }
        ASSERT_EQ(loops[1].entities().size(), 1u);
        const auto polyline = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(loops[1].entities()[0]);
        ASSERT_NE(polyline, nullptr);
        ASSERT_EQ(polyline->vertex().size(), 3u);
        for (const auto& vertex : polyline->vertex()) EXPECT_DOUBLE_EQ(vertex.location().z(), 3.0);
    };
    auto doc = newDwgDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(doc, input, lc::persistence::File::LIBDXFRW).ok);
    check(doc);
    for (const auto type : {lc::persistence::File::LIBDXFRW_DXF_R2000,
                            lc::persistence::File::LIBDXFRW_DXB_R2000}) {
        const auto output = tmpPath("mixed-edge-paths-output");
        ASSERT_TRUE(lc::persistence::File::exportFile(doc, output, type).ok);
        auto reopened = newDwgDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(reopened, output, lc::persistence::File::LIBDXFRW).ok);
        check(reopened);
        boost::filesystem::remove(output);
    }
    boost::filesystem::remove(input);
}

TEST(DwgImportTest, ElevatedCurvedEdgeHatchRoundTrip) {
    for (const bool ccw : {true, false}) {
    SCOPED_TRACE(ccw ? "ccw" : "cw");
    const auto input = tmpPath("elevated-curved-hatch");
    {
        std::ofstream f(input);
        ASSERT_TRUE(f.good());
        f << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1018\n0\nENDSEC\n"
             "0\nSECTION\n2\nENTITIES\n0\nHATCH\n5\n55\n100\nAcDbEntity\n8\n0\n"
             "100\nAcDbHatch\n10\n0\n20\n0\n30\n3\n210\n0\n220\n0\n230\n1\n"
             "2\nSOLID\n70\n1\n71\n0\n91\n1\n92\n0\n93\n2\n"
             "72\n2\n10\n1\n20\n2\n40\n2\n50\n0\n51\n180\n73\n1\n"
             "72\n3\n10\n1\n20\n2\n11\n2\n21\n0\n40\n0.5\n50\n35\n51\n210\n73\n" << (ccw ? 1 : 0) << "\n"
             "97\n0\n75\n0\n76\n1\n78\n0\n98\n0\n0\nENDSEC\n0\nEOF\n";
        ASSERT_TRUE(f.good());
    }
    const auto check = [ccw](const std::shared_ptr<lc::storage::DocumentImpl>& doc) {
        lc::entity::Hatch_CSPtr hatch;
        for (const auto& entity : doc->entityContainer().asVector())
            if (auto value = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity)) hatch = value;
        ASSERT_NE(hatch, nullptr);
        const auto& loops = hatch->getRegion().loopList();
        ASSERT_EQ(loops.size(), 1u);
        ASSERT_EQ(loops.front().entities().size(), 2u);
        const auto arc = std::dynamic_pointer_cast<const lc::entity::Arc>(loops[0].entities()[0]);
        const auto ellipse = std::dynamic_pointer_cast<const lc::entity::Ellipse>(loops[0].entities()[1]);
        ASSERT_NE(arc, nullptr);
        ASSERT_NE(ellipse, nullptr);
        EXPECT_DOUBLE_EQ(arc->center().x(), 1.0);
        EXPECT_DOUBLE_EQ(arc->center().y(), 2.0);
        EXPECT_DOUBLE_EQ(arc->center().z(), 3.0);
        EXPECT_DOUBLE_EQ(arc->radius(), 2.0);
        EXPECT_DOUBLE_EQ(ellipse->center().x(), 1.0);
        EXPECT_DOUBLE_EQ(ellipse->center().y(), 2.0);
        EXPECT_DOUBLE_EQ(ellipse->center().z(), 3.0);
        EXPECT_DOUBLE_EQ(ellipse->majorP().x(), 2.0);
        EXPECT_DOUBLE_EQ(ellipse->majorP().y(), 0.0);
        EXPECT_DOUBLE_EQ(ellipse->majorP().z(), 0.0);
        EXPECT_EQ(ellipse->isReversed(), !ccw);
    };
    auto doc = newDwgDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(doc, input, lc::persistence::File::LIBDXFRW).ok);
    check(doc);
    const char* capture = std::getenv("LC3_CURVE_CAPTURE");
    if (capture) {
        std::ifstream from(input, std::ios::binary);
        std::ofstream to(std::string(capture) + (ccw ? "/input-ccw.dxf" : "/input-cw.dxf"), std::ios::binary);
        ASSERT_TRUE(from.good() && to.good());
        to << from.rdbuf();
        ASSERT_TRUE(to.good());
    }
    for (const auto type : {lc::persistence::File::LIBDXFRW_DXF_R2000,
                            lc::persistence::File::LIBDXFRW_DXB_R2000}) {
        const auto output = capture ? std::string(capture) + (ccw ? "/ccw-" : "/cw-") + (type == lc::persistence::File::LIBDXFRW_DXF_R2000 ? "ascii.dxf" : "binary.dxf") : tmpPath("elevated-curved-hatch-output");
        ASSERT_TRUE(lc::persistence::File::exportFile(doc, output, type).ok);
        auto reopened = newDwgDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(reopened, output, lc::persistence::File::LIBDXFRW).ok);
        check(reopened);
        if (!capture) boost::filesystem::remove(output);
    }
    lc::entity::Hatch_CSPtr original;
    for (const auto& entity : doc->entityContainer().asVector())
        if (auto value = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity)) original = value;
    ASSERT_NE(original, nullptr);
    const auto& edges = original->getRegion().loopList().front().entities();
    ASSERT_EQ(edges.size(), 2u);
    const auto ellipse = std::dynamic_pointer_cast<const lc::entity::Ellipse>(edges[1]);
    ASSERT_NE(ellipse, nullptr);
    lc::geo::Region mixedRegion;
    mixedRegion.addLoop(lc::geo::Loop({edges[0], ellipse->move(lc::geo::Coordinate(0, 0, 1))}));
    auto mixed = std::make_shared<lc::entity::Hatch>(original->layer(), original->metaInfo(), original->block());
    mixed->setRegion(mixedRegion);
    mixed->setPatternName(original->getPatternName());
    mixed->setSolid(original->isSolid());
    doc->removeEntity(original);
    doc->insertEntity(mixed);
    const auto mixedOutput = capture ? std::string(capture) + (ccw ? "/ccw-mixed.dxf" : "/cw-mixed.dxf") : tmpPath("mixed-curved-hatch");
    const auto mixedResult = lc::persistence::File::exportFile(doc, mixedOutput, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(mixedResult.ok);
    EXPECT_TRUE(std::any_of(mixedResult.loss.notes.begin(), mixedResult.loss.notes.end(),
        [](const std::string& note) { return note.find("HATCH boundary elevation varies") != std::string::npos; }));
    doc->removeEntity(mixed);
    doc->insertEntity(original);
    const auto retry = capture ? std::string(capture) + (ccw ? "/ccw-retry.dxf" : "/cw-retry.dxf") : tmpPath("retry-curved-hatch");
    ASSERT_TRUE(lc::persistence::File::exportFile(doc, retry, lc::persistence::File::LIBDXFRW_DXF_R2000).ok);
    auto reopened = newDwgDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(reopened, retry, lc::persistence::File::LIBDXFRW).ok);
    check(reopened);
    if (!capture) { boost::filesystem::remove(mixedOutput); boost::filesystem::remove(retry); }
    boost::filesystem::remove(input);
    }
}

TEST(DwgImportTest, ElevatedSplineEdgeHatchRoundTrip) {
    const auto input = tmpPath("elevated-spline-hatch");
    {
        std::ofstream f(input);
        ASSERT_TRUE(f.good());
        f << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1018\n0\nENDSEC\n"
             "0\nSECTION\n2\nENTITIES\n0\nHATCH\n5\n55\n100\nAcDbEntity\n8\n0\n"
             "100\nAcDbHatch\n10\n0\n20\n0\n30\n3\n210\n0\n220\n0\n230\n1\n"
             "2\nSOLID\n70\n1\n71\n0\n91\n1\n92\n0\n93\n1\n"
             "72\n4\n94\n2\n73\n0\n74\n0\n95\n6\n96\n3\n"
             "40\n0\n40\n0\n40\n0\n40\n1\n40\n1\n40\n1\n"
             "10\n1\n20\n2\n10\n2\n20\n4\n10\n3\n20\n2\n"
             "97\n2\n11\n1\n21\n2\n11\n3\n21\n2\n"
             "12\n1\n22\n2\n13\n1\n23\n-2\n"
             "97\n0\n75\n0\n76\n1\n78\n0\n98\n0\n0\nENDSEC\n0\nEOF\n";
        ASSERT_TRUE(f.good());
    }
    const auto check = [](const std::shared_ptr<lc::storage::DocumentImpl>& doc,
                          const lc::geo::Coordinate& offset = lc::geo::Coordinate(0, 0, 0)) {
        lc::entity::Hatch_CSPtr hatch;
        for (const auto& entity : doc->entityContainer().asVector())
            if (auto value = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity)) hatch = value;
        ASSERT_NE(hatch, nullptr);
        const auto& loops = hatch->getRegion().loopList();
        ASSERT_EQ(loops.size(), 1u);
        ASSERT_EQ(loops[0].entities().size(), 1u);
        const auto spline = std::dynamic_pointer_cast<const lc::entity::Spline>(loops[0].entities()[0]);
        ASSERT_NE(spline, nullptr);
        ASSERT_EQ(spline->controlPoints().size(), 3u);
        EXPECT_EQ(spline->degree(), 2);
        ASSERT_EQ(spline->fitPoints().size(), 2u);
        const double fitXY[2][2] = {{1,2},{3,2}};
        for (std::size_t i = 0; i < 2; ++i) {
            EXPECT_DOUBLE_EQ(spline->fitPoints()[i].x(), fitXY[i][0] + offset.x());
            EXPECT_DOUBLE_EQ(spline->fitPoints()[i].y(), fitXY[i][1] + offset.y());
            EXPECT_DOUBLE_EQ(spline->fitPoints()[i].z(), 3.0 + offset.z());
        }
        EXPECT_DOUBLE_EQ(spline->startTanX(), 1.0);
        EXPECT_DOUBLE_EQ(spline->startTanY(), 2.0);
        EXPECT_DOUBLE_EQ(spline->startTanZ(), 0.0);
        EXPECT_DOUBLE_EQ(spline->endTanX(), 1.0);
        EXPECT_DOUBLE_EQ(spline->endTanY(), -2.0);
        EXPECT_DOUBLE_EQ(spline->endTanZ(), 0.0);
        const double xy[3][2] = {{1,2},{2,4},{3,2}};
        for (std::size_t i = 0; i < 3; ++i) {
            EXPECT_DOUBLE_EQ(spline->controlPoints()[i].x(), xy[i][0] + offset.x());
            EXPECT_DOUBLE_EQ(spline->controlPoints()[i].y(), xy[i][1] + offset.y());
            EXPECT_DOUBLE_EQ(spline->controlPoints()[i].z(), 3.0 + offset.z());
        }
    };
    auto doc = newDwgDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(doc, input, lc::persistence::File::LIBDXFRW).ok);
    check(doc);
    const char* capture = std::getenv("LC3_SPLINE_CAPTURE");
    if (capture) {
        std::ifstream from(input, std::ios::binary);
        std::ofstream to(std::string(capture) + "/input.dxf", std::ios::binary);
        ASSERT_TRUE(from.good() && to.good());
        to << from.rdbuf();
        ASSERT_TRUE(to.good());
    }
    for (const auto type : {lc::persistence::File::LIBDXFRW_DXF_R2000,
                            lc::persistence::File::LIBDXFRW_DXB_R2000}) {
        const auto output = capture ? std::string(capture) + (type == lc::persistence::File::LIBDXFRW_DXF_R2000 ? "/spline-ascii.dxf" : "/spline-binary.dxf") : tmpPath("elevated-spline-hatch-output");
        ASSERT_TRUE(lc::persistence::File::exportFile(doc, output, type).ok);
        auto reopened = newDwgDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(reopened, output, lc::persistence::File::LIBDXFRW).ok);
        check(reopened);
        if (!capture) boost::filesystem::remove(output);
    }
    lc::entity::Hatch_CSPtr original;
    for (const auto& entity : doc->entityContainer().asVector())
        if (auto value = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity)) original = value;
    ASSERT_NE(original, nullptr);
    const lc::geo::Coordinate offset(2, 5, 7);
    for (const bool copy : {false, true}) {
        SCOPED_TRACE(copy ? "copy" : "move");
        const auto transformed = copy ? original->copy(offset) : original->move(offset);
        ASSERT_NE(transformed, nullptr);
        doc->removeEntity(original);
        doc->insertEntity(transformed);
        check(doc, offset);
        for (const auto type : {lc::persistence::File::LIBDXFRW_DXF_R2000,
                                lc::persistence::File::LIBDXFRW_DXB_R2000}) {
            const auto output = capture ? std::string(capture) + (copy ? "/copied-" : "/moved-")
                + (type == lc::persistence::File::LIBDXFRW_DXF_R2000 ? "ascii.dxf" : "binary.dxf")
                : tmpPath("translated-spline-hatch");
            ASSERT_TRUE(lc::persistence::File::exportFile(doc, output, type).ok);
            auto reopened = newDwgDocument();
            ASSERT_TRUE(lc::persistence::File::importFile(reopened, output, lc::persistence::File::LIBDXFRW).ok);
            check(reopened, offset);
            if (!capture) boost::filesystem::remove(output);
        }
        doc->removeEntity(transformed);
        doc->insertEntity(original);
        check(doc);
    }
    const auto& edges = original->getRegion().loopList().front().entities();
    ASSERT_EQ(edges.size(), 1u);
    const auto spline = std::dynamic_pointer_cast<const lc::entity::Spline>(edges[0]);
    ASSERT_NE(spline, nullptr);
    const auto fitOnly = std::make_shared<lc::entity::Spline>(
        std::vector<lc::geo::Coordinate>{}, std::vector<double>{}, spline->fitPoints(),
        2, false, spline->fitTolerance(), spline->startTanX(), spline->startTanY(), spline->startTanZ(),
        spline->endTanX(), spline->endTanY(), spline->endTanZ(),
        spline->nX(), spline->nY(), spline->nZ(), spline->flags(), spline->layer());
    for (const bool copy : {false, true}) {
        const auto transformed = std::dynamic_pointer_cast<const lc::entity::Spline>(
            copy ? fitOnly->copy(offset) : fitOnly->move(offset));
        ASSERT_NE(transformed, nullptr);
        ASSERT_TRUE(transformed->controlPoints().empty());
        ASSERT_EQ(transformed->fitPoints().size(), 2u);
        EXPECT_DOUBLE_EQ(transformed->fitPoints()[0].x(), 3.0);
        EXPECT_DOUBLE_EQ(transformed->fitPoints()[0].y(), 7.0);
        EXPECT_DOUBLE_EQ(transformed->fitPoints()[0].z(), 10.0);
        EXPECT_DOUBLE_EQ(transformed->boundingBox().minP().x(), 3.0);
        EXPECT_DOUBLE_EQ(transformed->boundingBox().maxP().x(), 5.0);
    }
    auto points = spline->controlPoints();
    ASSERT_EQ(points.size(), 3u);
    points[1] = lc::geo::Coordinate(points[1].x(), points[1].y(), 4.0);
    auto changed = std::make_shared<lc::entity::Spline>(points, spline->knotPoints(), spline->fitPoints(),
        spline->degree(), spline->closed(), spline->fitTolerance(),
        spline->startTanX(), spline->startTanY(), spline->startTanZ(),
        spline->endTanX(), spline->endTanY(), spline->endTanZ(),
        spline->nX(), spline->nY(), spline->nZ(), spline->flags(), spline->layer());
    lc::geo::Region mixedRegion;
    mixedRegion.addLoop(lc::geo::Loop({changed}));
    auto mixed = std::make_shared<lc::entity::Hatch>(original->layer(), original->metaInfo(), original->block());
    mixed->setRegion(mixedRegion);
    mixed->setPatternName(original->getPatternName());
    mixed->setSolid(original->isSolid());
    doc->removeEntity(original);
    doc->insertEntity(mixed);
    const auto mixedOutput = capture ? std::string(capture) + "/mixed.dxf" : tmpPath("mixed-spline-hatch");
    { std::ofstream sentinel(mixedOutput); sentinel << "previous destination"; }
    const auto mixedResult = lc::persistence::File::exportFile(doc, mixedOutput, lc::persistence::File::LIBDXFRW_DXF_R2000);
    if (mixedResult.ok) {
        EXPECT_TRUE(std::any_of(mixedResult.loss.notes.begin(), mixedResult.loss.notes.end(),
            [](const std::string& note) { return note.find("HATCH boundary elevation varies") != std::string::npos; }));
    } else {
        std::ifstream sentinel(mixedOutput);
        std::string value; std::getline(sentinel, value);
        EXPECT_EQ(value, "previous destination");
    }
    doc->removeEntity(mixed);
    doc->insertEntity(original);
    const auto retry = capture ? std::string(capture) + "/retry.dxf" : tmpPath("retry-spline-hatch");
    ASSERT_TRUE(lc::persistence::File::exportFile(doc, retry, lc::persistence::File::LIBDXFRW_DXF_R2000).ok);
    auto reopened = newDwgDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(reopened, retry, lc::persistence::File::LIBDXFRW).ok);
    check(reopened);
    if (!capture) { boost::filesystem::remove(mixedOutput); boost::filesystem::remove(retry); }
    boost::filesystem::remove(input);
}
