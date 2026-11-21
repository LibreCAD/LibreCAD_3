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

// Both libdxfrw and LibreDWG decode zero boundary paths from this particular
// fixture. The empty HATCH is dropped on export and must be reported. This
// fixture alone cannot establish whether a nonempty boundary was lost.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(DwgImportTest, EmptyMpolygonProjectionReportsExportLoss) {
    auto doc = newDwgDocument();
    const auto imported = lc::persistence::File::importFile(
        doc, dwgFixture("mpolygon_solid.dwg"), lc::persistence::File::LIBDXFRW);
    ASSERT_TRUE(imported.ok);
    EXPECT_EQ(std::count_if(imported.loss.notes.begin(), imported.loss.notes.end(),
                            [](const std::string& note) {
                                return note.find("MPOLYGON entities were imported as HATCH for editing")
                                       != std::string::npos;
                            }), 1);

    lc::entity::Hatch_CSPtr hatch;
    for (const auto& entity : doc->entityContainer().asVector()) {
        if (auto found = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity)) {
            hatch = found;
        }
    }
    ASSERT_NE(hatch, nullptr);
    EXPECT_TRUE(hatch->isSolid());
    EXPECT_EQ(hatch->getRegion().loopList().size(), 0u)
        << "If the source now decodes a boundary, assert its contents and "
           "update DwgAndItsDxfExportAgree, which allows for this loss.";

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
    const auto checkRotated = [](const std::shared_ptr<lc::storage::DocumentImpl>& target) {
        lc::entity::Hatch_CSPtr hatch;
        for (const auto& entity : target->entityContainer().asVector())
            if (auto value = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity)) hatch = value;
        ASSERT_NE(hatch, nullptr);
        const auto& loops = hatch->getRegion().loopList();
        ASSERT_EQ(loops.size(), 1u);
        ASSERT_EQ(loops[0].entities().size(), 1u);
        const auto spline = std::dynamic_pointer_cast<const lc::entity::Spline>(loops[0].entities()[0]);
        ASSERT_NE(spline, nullptr);
        ASSERT_EQ(spline->controlPoints().size(), 3u);
        ASSERT_EQ(spline->fitPoints().size(), 2u);
        const double xy[3][2] = {{28,11},{26,12},{28,13}};
        for (std::size_t i = 0; i < 3; ++i) {
            EXPECT_NEAR(spline->controlPoints()[i].x(), xy[i][0], 1e-12);
            EXPECT_NEAR(spline->controlPoints()[i].y(), xy[i][1], 1e-12);
            EXPECT_DOUBLE_EQ(spline->controlPoints()[i].z(), 3.0);
        }
        EXPECT_NEAR(spline->fitPoints()[0].x(), 28.0, 1e-12);
        EXPECT_NEAR(spline->fitPoints()[0].y(), 11.0, 1e-12);
        EXPECT_DOUBLE_EQ(spline->fitPoints()[0].z(), 3.0);
        EXPECT_NEAR(spline->fitPoints()[1].x(), 28.0, 1e-12);
        EXPECT_NEAR(spline->fitPoints()[1].y(), 13.0, 1e-12);
        EXPECT_DOUBLE_EQ(spline->fitPoints()[1].z(), 3.0);
        EXPECT_NEAR(spline->startTanX(), -2.0, 1e-12);
        EXPECT_NEAR(spline->startTanY(), 1.0, 1e-12);
        EXPECT_NEAR(spline->endTanX(), 2.0, 1e-12);
        EXPECT_NEAR(spline->endTanY(), 1.0, 1e-12);
        EXPECT_DOUBLE_EQ(spline->startTanZ(), 0.0);
        EXPECT_DOUBLE_EQ(spline->endTanZ(), 0.0);
    };
    const auto rotated = original->rotate(lc::geo::Coordinate(10, 20, 100), std::acos(-1.0) / 2.0);
    ASSERT_NE(rotated, nullptr);
    doc->removeEntity(original);
    doc->insertEntity(rotated);
    checkRotated(doc);
    for (const auto type : {lc::persistence::File::LIBDXFRW_DXF_R2000,
                            lc::persistence::File::LIBDXFRW_DXB_R2000}) {
        const auto output = capture ? std::string(capture) + (type == lc::persistence::File::LIBDXFRW_DXF_R2000 ? "/rotated-ascii.dxf" : "/rotated-binary.dxf") : tmpPath("rotated-spline-hatch");
        ASSERT_TRUE(lc::persistence::File::exportFile(doc, output, type).ok);
        auto reopened = newDwgDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(reopened, output, lc::persistence::File::LIBDXFRW).ok);
        checkRotated(reopened);
        if (!capture) boost::filesystem::remove(output);
    }
    doc->removeEntity(rotated);
    doc->insertEntity(original);
    check(doc);
    for (const bool scale : {true, false}) {
        SCOPED_TRACE(scale ? "scale" : "mirror");
        const auto transformed = scale
            ? original->scale(lc::geo::Coordinate(10, 20, 1), lc::geo::Coordinate(2, 3, 2))
            : original->mirror(lc::geo::Coordinate(0, 0, 0), lc::geo::Coordinate(0, 1, 0));
        ASSERT_NE(transformed, nullptr);
        doc->removeEntity(original);
        doc->insertEntity(transformed);
        const auto checkTransform = [scale](const std::shared_ptr<lc::storage::DocumentImpl>& target) {
            lc::entity::Hatch_CSPtr hatch;
            for (const auto& entity : target->entityContainer().asVector())
                if (auto value = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity)) hatch = value;
            ASSERT_NE(hatch, nullptr);
            const auto& loops = hatch->getRegion().loopList();
            ASSERT_EQ(loops.size(), 1u);
            ASSERT_EQ(loops[0].entities().size(), 1u);
            const auto spline = std::dynamic_pointer_cast<const lc::entity::Spline>(loops[0].entities()[0]);
            ASSERT_NE(spline, nullptr);
            ASSERT_EQ(spline->controlPoints().size(), 3u);
            ASSERT_EQ(spline->fitPoints().size(), 2u);
            const double source[3][2] = {{1,2},{2,4},{3,2}};
            for (std::size_t i = 0; i < 3; ++i) {
                EXPECT_DOUBLE_EQ(spline->controlPoints()[i].x(), scale ? 2 * source[i][0] - 10 : -source[i][0]);
                EXPECT_DOUBLE_EQ(spline->controlPoints()[i].y(), scale ? 3 * source[i][1] - 40 : source[i][1]);
                EXPECT_DOUBLE_EQ(spline->controlPoints()[i].z(), scale ? 5.0 : 3.0);
            }
            for (std::size_t i = 0; i < 2; ++i) {
                const std::size_t sourceIndex = i == 0 ? 0 : 2;
                EXPECT_DOUBLE_EQ(spline->fitPoints()[i].x(), scale ? 2 * source[sourceIndex][0] - 10 : -source[sourceIndex][0]);
                EXPECT_DOUBLE_EQ(spline->fitPoints()[i].y(), scale ? 3 * source[sourceIndex][1] - 40 : source[sourceIndex][1]);
                EXPECT_DOUBLE_EQ(spline->fitPoints()[i].z(), scale ? 5.0 : 3.0);
            }
            EXPECT_DOUBLE_EQ(spline->startTanX(), scale ? 2.0 : -1.0);
            EXPECT_DOUBLE_EQ(spline->startTanY(), scale ? 6.0 : 2.0);
            EXPECT_DOUBLE_EQ(spline->endTanX(), scale ? 2.0 : -1.0);
            EXPECT_DOUBLE_EQ(spline->endTanY(), scale ? -6.0 : -2.0);
        };
        checkTransform(doc);
        for (const auto type : {lc::persistence::File::LIBDXFRW_DXF_R2000,
                                lc::persistence::File::LIBDXFRW_DXB_R2000}) {
            const auto output = capture ? std::string(capture) + (scale ? "/scaled-" : "/mirrored-")
                + (type == lc::persistence::File::LIBDXFRW_DXF_R2000 ? "ascii.dxf" : "binary.dxf")
                : tmpPath("transformed-spline-hatch");
            ASSERT_TRUE(lc::persistence::File::exportFile(doc, output, type).ok);
            auto reopened = newDwgDocument();
            ASSERT_TRUE(lc::persistence::File::importFile(reopened, output, lc::persistence::File::LIBDXFRW).ok);
            checkTransform(reopened);
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
    const auto fitRotated = std::dynamic_pointer_cast<const lc::entity::Spline>(
        fitOnly->rotate(lc::geo::Coordinate(10, 20, 100), std::acos(-1.0) / 2.0));
    ASSERT_NE(fitRotated, nullptr);
    ASSERT_TRUE(fitRotated->controlPoints().empty());
    ASSERT_EQ(fitRotated->fitPoints().size(), 2u);
    EXPECT_NEAR(fitRotated->fitPoints()[0].x(), 28.0, 1e-12);
    EXPECT_NEAR(fitRotated->fitPoints()[0].y(), 11.0, 1e-12);
    EXPECT_DOUBLE_EQ(fitRotated->fitPoints()[0].z(), 3.0);
    const auto vectors = std::make_shared<lc::entity::Spline>(
        spline->controlPoints(), spline->knotPoints(), spline->fitPoints(),
        spline->degree(), spline->closed(), spline->fitTolerance(), 4, 5, 6, 7, 8, 9,
        1, 2, 3, spline->flags(), spline->layer());
    const auto rotatedVectors = std::dynamic_pointer_cast<const lc::entity::Spline>(
        vectors->rotate(lc::geo::Coordinate(10, 20, 100), std::acos(-1.0) / 2.0));
    ASSERT_NE(rotatedVectors, nullptr);
    EXPECT_NEAR(rotatedVectors->startTanX(), -5.0, 1e-12);
    EXPECT_NEAR(rotatedVectors->startTanY(), 4.0, 1e-12);
    EXPECT_DOUBLE_EQ(rotatedVectors->startTanZ(), 6.0);
    EXPECT_NEAR(rotatedVectors->endTanX(), -8.0, 1e-12);
    EXPECT_NEAR(rotatedVectors->endTanY(), 7.0, 1e-12);
    EXPECT_DOUBLE_EQ(rotatedVectors->endTanZ(), 9.0);
    EXPECT_NEAR(rotatedVectors->nX(), -2.0, 1e-12);
    EXPECT_NEAR(rotatedVectors->nY(), 1.0, 1e-12);
    EXPECT_DOUBLE_EQ(rotatedVectors->nZ(), 3.0);
    const auto scaledFitOnly = std::dynamic_pointer_cast<const lc::entity::Spline>(
        fitOnly->scale(lc::geo::Coordinate(10, 20, 1), lc::geo::Coordinate(2, 3, 2)));
    ASSERT_NE(scaledFitOnly, nullptr);
    ASSERT_TRUE(scaledFitOnly->controlPoints().empty());
    ASSERT_EQ(scaledFitOnly->fitPoints().size(), 2u);
    EXPECT_DOUBLE_EQ(scaledFitOnly->fitPoints()[0].x(), -8.0);
    EXPECT_DOUBLE_EQ(scaledFitOnly->fitPoints()[0].y(), -34.0);
    EXPECT_DOUBLE_EQ(scaledFitOnly->fitPoints()[0].z(), 5.0);
    const auto mirroredFitOnly = std::dynamic_pointer_cast<const lc::entity::Spline>(
        fitOnly->mirror(lc::geo::Coordinate(0, 0, 0), lc::geo::Coordinate(0, 1, 0)));
    ASSERT_NE(mirroredFitOnly, nullptr);
    ASSERT_TRUE(mirroredFitOnly->controlPoints().empty());
    ASSERT_EQ(mirroredFitOnly->fitPoints().size(), 2u);
    EXPECT_DOUBLE_EQ(mirroredFitOnly->fitPoints()[0].x(), -1.0);
    EXPECT_DOUBLE_EQ(mirroredFitOnly->fitPoints()[0].y(), 2.0);
    EXPECT_DOUBLE_EQ(mirroredFitOnly->fitPoints()[0].z(), 3.0);
    const auto scaledVectors = std::dynamic_pointer_cast<const lc::entity::Spline>(
        vectors->scale(lc::geo::Coordinate(10, 20, 1), lc::geo::Coordinate(2, 3, 2)));
    ASSERT_NE(scaledVectors, nullptr);
    EXPECT_DOUBLE_EQ(scaledVectors->startTanX(), 8.0);
    EXPECT_DOUBLE_EQ(scaledVectors->startTanY(), 15.0);
    EXPECT_DOUBLE_EQ(scaledVectors->startTanZ(), 12.0);
    EXPECT_NEAR(scaledVectors->nX() / scaledVectors->nZ(), 1.0 / 3.0, 1e-12);
    EXPECT_NEAR(scaledVectors->nY() / scaledVectors->nZ(), 4.0 / 9.0, 1e-12);
    const auto mirroredVectors = std::dynamic_pointer_cast<const lc::entity::Spline>(
        vectors->mirror(lc::geo::Coordinate(0, 0, 0), lc::geo::Coordinate(0, 1, 0)));
    ASSERT_NE(mirroredVectors, nullptr);
    EXPECT_DOUBLE_EQ(mirroredVectors->startTanX(), -4.0);
    EXPECT_DOUBLE_EQ(mirroredVectors->startTanY(), 5.0);
    EXPECT_DOUBLE_EQ(mirroredVectors->startTanZ(), 6.0);
    EXPECT_DOUBLE_EQ(mirroredVectors->nX(), -1.0);
    EXPECT_DOUBLE_EQ(mirroredVectors->nY(), 2.0);
    EXPECT_DOUBLE_EQ(mirroredVectors->nZ(), 3.0);
    const auto tiltedAxis = lc::geo::Coordinate(1, 1, 1);
    const auto tiltedMirror = std::dynamic_pointer_cast<const lc::entity::Spline>(
        vectors->mirror(lc::geo::Coordinate(0, 0, 0), tiltedAxis));
    ASSERT_NE(tiltedMirror, nullptr);
    const auto expectedControl = vectors->controlPoints()[0].mirror(lc::geo::Coordinate(0, 0, 0), tiltedAxis);
    const auto expectedFit = vectors->fitPoints()[0].mirror(lc::geo::Coordinate(0, 0, 0), tiltedAxis);
    const auto expectedTangent = lc::geo::Coordinate(4, 5, 6).mirror(lc::geo::Coordinate(0, 0, 0), tiltedAxis);
    EXPECT_DOUBLE_EQ(tiltedMirror->controlPoints()[0].x(), expectedControl.x());
    EXPECT_DOUBLE_EQ(tiltedMirror->controlPoints()[0].y(), expectedControl.y());
    EXPECT_DOUBLE_EQ(tiltedMirror->controlPoints()[0].z(), expectedControl.z());
    EXPECT_DOUBLE_EQ(tiltedMirror->fitPoints()[0].x(), expectedFit.x());
    EXPECT_DOUBLE_EQ(tiltedMirror->fitPoints()[0].y(), expectedFit.y());
    EXPECT_DOUBLE_EQ(tiltedMirror->fitPoints()[0].z(), expectedFit.z());
    EXPECT_DOUBLE_EQ(tiltedMirror->startTanX(), expectedTangent.x());
    EXPECT_DOUBLE_EQ(tiltedMirror->startTanY(), expectedTangent.y());
    EXPECT_DOUBLE_EQ(tiltedMirror->startTanZ(), expectedTangent.z());
    const auto singularScale = std::dynamic_pointer_cast<const lc::entity::Spline>(
        vectors->scale(lc::geo::Coordinate(0, 0, 0), lc::geo::Coordinate(2, 0, 2)));
    ASSERT_NE(singularScale, nullptr);
    EXPECT_DOUBLE_EQ(singularScale->nX(), 1.0);
    EXPECT_DOUBLE_EQ(singularScale->nY(), 2.0);
    EXPECT_DOUBLE_EQ(singularScale->nZ(), 3.0);
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

TEST(DwgImportTest, MpolygonSimpleBoundaryAndEditedFallback) {
    const std::string input = tmpPath("simple-mpolygon-source");
    {
        std::ofstream f(input);
        ASSERT_TRUE(f.good());
        f << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1018\n0\nENDSEC\n"
             "0\nSECTION\n2\nENTITIES\n0\nMPOLYGON\n5\n55\n"
             "100\nAcDbEntity\n8\n0\n100\nAcDbMPolygon\n70\n1\n"
             "10\n0\n20\n0\n30\n0\n210\n0\n220\n0\n230\n1\n"
             "2\nSOLID\n71\n1\n91\n1\n92\n2\n73\n1\n72\n0\n93\n3\n"
             "10\n1\n20\n2\n10\n4\n20\n2\n10\n3\n20\n5\n"
             "76\n1\n78\n0\n63\n3\n98\n0\n11\n2\n21\n5\n99\n0\n"
             "0\nENDSEC\n0\nEOF\n";
        ASSERT_TRUE(f.good());
    }
    auto doc = newDwgDocument();
    const auto imported = lc::persistence::File::importFile(
        doc, input, lc::persistence::File::LIBDXFRW);
    ASSERT_TRUE(imported.ok);
    ASSERT_FALSE(imported.loss.notes.empty());
    lc::entity::Hatch_CSPtr hatch;
    for (const auto& entity : doc->entityContainer().asVector())
        if (auto candidate = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity))
            hatch = candidate;
    ASSERT_NE(hatch, nullptr);

    const std::string ascii = tmpPath("simple-mpolygon-ascii");
    const std::string binary = tmpPath("simple-mpolygon-binary");
    bool asciiPreserved = false;
    for (const auto& test : std::map<std::string, lc::persistence::File::Type>{
             {ascii, lc::persistence::File::LIBDXFRW_DXF_R2000},
             {binary, lc::persistence::File::LIBDXFRW_DXB_R2000}}) {
        const auto written = lc::persistence::File::exportFile(doc, test.first, test.second);
        ASSERT_TRUE(written.ok) << test.first;
        const bool preserved = written.loss.empty();
        if (test.first == ascii) asciiPreserved = preserved;
        if (!preserved) {
            EXPECT_TRUE(std::any_of(written.loss.notes.begin(), written.loss.notes.end(),
                                    [](const std::string& note) {
                                        return note.find("written as HATCH") != std::string::npos;
                                    }));
        }
        auto reopened = newDwgDocument();
        const auto read = lc::persistence::File::importFile(
            reopened, test.first, lc::persistence::File::LIBDXFRW);
        ASSERT_TRUE(read.ok);
        EXPECT_EQ(std::any_of(read.loss.notes.begin(), read.loss.notes.end(),
                               [](const std::string& note) {
                                   return note.find("MPOLYGON entities") != std::string::npos;
                               }), preserved);
    }
    // The ASCII tags independently expose the preserved subtype and fields.
    std::ifstream f(ascii);
    ASSERT_TRUE(f.good());
    bool inPolygon = false, seenPolygon = false, fill = false, xdir = false, ydir = false;
    std::string code, value;
    while (std::getline(f, code) && std::getline(f, value)) {
        if (!code.empty() && code.back() == '\r') code.pop_back();
        if (!value.empty() && value.back() == '\r') value.pop_back();
        const int group = std::stoi(code);
        if (group == 0) {
            if (inPolygon) break;
            inPolygon = value == "MPOLYGON";
            seenPolygon = seenPolygon || inPolygon;
        }
        if (inPolygon && group == 63) fill = std::stoi(value) == 3;
        if (inPolygon && group == 11) xdir = std::stod(value) == 2.0;
        if (inPolygon && group == 21) ydir = std::stod(value) == 5.0;
    }
    if (asciiPreserved) EXPECT_TRUE(seenPolygon && fill && xdir && ydir);
    else EXPECT_FALSE(seenPolygon);

    // A planar translation changes the offset while retaining the subtype.
    const auto moved = hatch->move(lc::geo::Coordinate(10, 0, 0));
    ASSERT_NE(moved, nullptr);
    doc->removeEntity(hatch);
    doc->insertEntity(moved);
    const std::string edited = tmpPath("edited-mpolygon");
    const auto written = lc::persistence::File::exportFile(
        doc, edited, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(written.ok);
    EXPECT_EQ(written.loss.empty(), asciiPreserved);
    if (!asciiPreserved) {
        EXPECT_TRUE(std::any_of(written.loss.notes.begin(), written.loss.notes.end(),
                                [](const std::string& note) {
                                    return note.find("written as HATCH") != std::string::npos;
                                }));
    }
    auto reopened = newDwgDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(
        reopened, edited, lc::persistence::File::LIBDXFRW).ok);
    lc::entity::Hatch_CSPtr readHatch;
    for (const auto& entity : reopened->entityContainer().asVector())
        if (auto candidate = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity))
            readHatch = candidate;
    ASSERT_NE(readHatch, nullptr);
    ASSERT_EQ(readHatch->getRegion().loopList().size(), 1u);
    const auto& edges = readHatch->getRegion().loopList()[0].entities();
    ASSERT_EQ(edges.size(), 1u);
    const auto poly = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(edges[0]);
    ASSERT_NE(poly, nullptr);
    ASSERT_EQ(poly->vertex().size(), 3u);
    EXPECT_DOUBLE_EQ(poly->vertex()[0].location().x(), 13.0);
    EXPECT_DOUBLE_EQ(poly->vertex()[0].location().y(), 7.0);

    // Rotation retains the subtype only when all encoded points reconstruct
    // exactly and the library has the corrected MPOLYGON group semantics.
    const auto rotated = hatch->rotate(lc::geo::Coordinate(0, 0, 0), 0.5);
    ASSERT_NE(rotated, nullptr);
    doc->removeEntity(moved);
    doc->insertEntity(rotated);
    const std::string rotatedOutput = tmpPath("rotated-mpolygon-output");
    const auto rotatedWrite = lc::persistence::File::exportFile(
        doc, rotatedOutput, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(rotatedWrite.ok);
    EXPECT_EQ(rotatedWrite.loss.empty(), asciiPreserved);
    if (!asciiPreserved) {
        EXPECT_TRUE(std::any_of(rotatedWrite.loss.notes.begin(), rotatedWrite.loss.notes.end(),
                                [](const std::string& note) {
                                    return note.find("written as HATCH") != std::string::npos;
                                }));
    }
    auto rotatedDoc = newDwgDocument();
    const auto rotatedRead = lc::persistence::File::importFile(
        rotatedDoc, rotatedOutput, lc::persistence::File::LIBDXFRW);
    ASSERT_TRUE(rotatedRead.ok);
    EXPECT_EQ(std::any_of(rotatedRead.loss.notes.begin(), rotatedRead.loss.notes.end(),
                           [](const std::string& note) {
                               return note.find("MPOLYGON entities") != std::string::npos;
                           }), asciiPreserved);
    const auto expectedHatch = std::dynamic_pointer_cast<const lc::entity::Hatch>(rotated);
    ASSERT_NE(expectedHatch, nullptr);
    const auto expectedPoly = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(
        expectedHatch->getRegion().loopList()[0].entities()[0]);
    lc::entity::Hatch_CSPtr rotatedHatch;
    for (const auto& entity : rotatedDoc->entityContainer().asVector())
        if (auto candidate = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity))
            rotatedHatch = candidate;
    ASSERT_NE(rotatedHatch, nullptr);
    ASSERT_EQ(rotatedHatch->getRegion().loopList().size(), 1u);
    ASSERT_EQ(rotatedHatch->getRegion().loopList()[0].entities().size(), 1u);
    const auto rotatedPoly = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(
        rotatedHatch->getRegion().loopList()[0].entities()[0]);
    ASSERT_NE(expectedPoly, nullptr);
    ASSERT_NE(rotatedPoly, nullptr);
    ASSERT_EQ(rotatedPoly->vertex().size(), expectedPoly->vertex().size());
    for (std::size_t i = 0; i < expectedPoly->vertex().size(); ++i) {
        EXPECT_DOUBLE_EQ(rotatedPoly->vertex()[i].location().x(), expectedPoly->vertex()[i].location().x());
        EXPECT_DOUBLE_EQ(rotatedPoly->vertex()[i].location().y(), expectedPoly->vertex()[i].location().y());
        EXPECT_DOUBLE_EQ(rotatedPoly->vertex()[i].bulge(), expectedPoly->vertex()[i].bulge());
    }

    // A non-rigid edit takes the reported projection path.
    const auto scaled = hatch->scale(lc::geo::Coordinate(0, 0, 0), lc::geo::Coordinate(2, 1, 1));
    ASSERT_NE(scaled, nullptr);
    doc->removeEntity(rotated);
    doc->insertEntity(scaled);
    const std::string scaledOutput = tmpPath("scaled-mpolygon-output");
    const auto scaledWrite = lc::persistence::File::exportFile(
        doc, scaledOutput, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(scaledWrite.ok);
    EXPECT_TRUE(std::any_of(scaledWrite.loss.notes.begin(), scaledWrite.loss.notes.end(),
                            [](const std::string& note) {
                                return note.find("written as HATCH") != std::string::npos;
                            }));

    // Reflection is not an orientation-preserving rotation.
    const auto reflected = hatch->scale(lc::geo::Coordinate(0, 0, 0),
                                        lc::geo::Coordinate(-1, 1, 1));
    ASSERT_NE(reflected, nullptr);
    doc->removeEntity(scaled);
    doc->insertEntity(reflected);
    const std::string reflectedOutput = tmpPath("reflected-mpolygon-output");
    const auto reflectedWrite = lc::persistence::File::exportFile(
        doc, reflectedOutput, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(reflectedWrite.ok);
    EXPECT_TRUE(std::any_of(reflectedWrite.loss.notes.begin(), reflectedWrite.loss.notes.end(),
                            [](const std::string& note) {
                                return note.find("written as HATCH") != std::string::npos;
                            }));

    // Restore the original entity and retry the same destination. The stale
    // projection warning must not leak into a fresh export.
    doc->removeEntity(reflected);
    doc->insertEntity(hatch);
    const auto retried = lc::persistence::File::exportFile(
        doc, edited, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(retried.ok);
    EXPECT_EQ(retried.loss.empty(), asciiPreserved);
    auto retriedDoc = newDwgDocument();
    const auto retriedRead = lc::persistence::File::importFile(
        retriedDoc, edited, lc::persistence::File::LIBDXFRW);
    ASSERT_TRUE(retriedRead.ok);
    EXPECT_EQ(std::any_of(retriedRead.loss.notes.begin(), retriedRead.loss.notes.end(),
                           [](const std::string& note) {
                               return note.find("MPOLYGON entities") != std::string::npos;
                           }), asciiPreserved);
    // A Z-only edit cannot be mistaken for unchanged XY coordinates.
    const auto lifted = hatch->move(lc::geo::Coordinate(0, 0, 3));
    ASSERT_NE(lifted, nullptr);
    doc->removeEntity(hatch);
    doc->insertEntity(lifted);
    const std::string liftedOutput = tmpPath("lifted-mpolygon-output");
    const auto liftedWrite = lc::persistence::File::exportFile(
        doc, liftedOutput, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(liftedWrite.ok);
    EXPECT_TRUE(std::any_of(liftedWrite.loss.notes.begin(), liftedWrite.loss.notes.end(),
                            [](const std::string& note) {
                                return note.find("boundary geometry") != std::string::npos;
                            }));
    auto liftedDoc = newDwgDocument();
    const auto liftedRead = lc::persistence::File::importFile(
        liftedDoc, liftedOutput, lc::persistence::File::LIBDXFRW);
    ASSERT_TRUE(liftedRead.ok);
    EXPECT_TRUE(std::none_of(liftedRead.loss.notes.begin(), liftedRead.loss.notes.end(),
                             [](const std::string& note) {
                                 return note.find("MPOLYGON entities") != std::string::npos;
                             }));

    // A source outside the replay profile must take the reported path too.
    std::ifstream original(input);
    std::string complexSource((std::istreambuf_iterator<char>(original)),
                              std::istreambuf_iterator<char>());
    const auto countAt = complexSource.find("91\n1\n92\n2\n");
    ASSERT_NE(countAt, std::string::npos);
    complexSource.replace(countAt, std::string("91\n1\n92\n2\n").size(),
                          "91\n2\n92\n2\n");
    const auto tailAt = complexSource.find("76\n1\n78\n0\n");
    ASSERT_NE(tailAt, std::string::npos);
    complexSource.insert(tailAt, "92\n2\n73\n1\n72\n0\n93\n3\n"
                                "10\n6\n20\n2\n10\n9\n20\n2\n10\n8\n20\n5\n");
    const std::string complexInput = tmpPath("complex-mpolygon-source");
    { std::ofstream complexFile(complexInput); complexFile << complexSource; }
    auto complexDoc = newDwgDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(
        complexDoc, complexInput, lc::persistence::File::LIBDXFRW).ok);
    const std::string complexOutput = tmpPath("complex-mpolygon-output");
    const auto complexWrite = lc::persistence::File::exportFile(
        complexDoc, complexOutput, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(complexWrite.ok);
    EXPECT_TRUE(std::any_of(complexWrite.loss.notes.begin(), complexWrite.loss.notes.end(),
                            [](const std::string& note) {
                                return note.find("written as HATCH") != std::string::npos;
                            }));
    auto complexRead = newDwgDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(
        complexRead, complexOutput, lc::persistence::File::LIBDXFRW).ok);
    lc::entity::Hatch_CSPtr complexHatch;
    for (const auto& entity : complexRead->entityContainer().asVector())
        if (auto candidate = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity))
            complexHatch = candidate;
    ASSERT_NE(complexHatch, nullptr);
    EXPECT_EQ(complexHatch->getRegion().loopList().size(), 2u);
    // Offset applies to edge points and arc centers, but not radii.
    std::ifstream edgeSourceFile(input);
    std::string edgeSource((std::istreambuf_iterator<char>(edgeSourceFile)),
                           std::istreambuf_iterator<char>());
    const std::string polyTags = "92\n2\n73\n1\n72\n0\n93\n3\n"
                                 "10\n1\n20\n2\n10\n4\n20\n2\n10\n3\n20\n5\n";
    const auto edgeAt = edgeSource.find(polyTags);
    ASSERT_NE(edgeAt, std::string::npos);
    edgeSource.replace(edgeAt, polyTags.size(),
        "92\n0\n93\n4\n72\n1\n10\n1\n20\n2\n11\n4\n21\n2\n"
        "72\n2\n10\n4\n20\n3\n40\n1\n50\n270\n51\n90\n73\n1\n"
        "72\n3\n10\n6\n20\n6\n11\n2\n21\n0\n40\n0.5\n"
        "50\n0\n51\n180\n73\n1\n"
        "72\n4\n94\n2\n73\n0\n74\n0\n95\n6\n96\n3\n"
        "40\n0\n40\n0\n40\n0\n40\n1\n40\n1\n40\n1\n"
        "10\n1\n20\n0\n10\n1\n20\n1\n10\n0\n20\n1\n97\n0\n");
    const std::string edgeInput = tmpPath("edge-mpolygon-source");
    { std::ofstream edgeFile(edgeInput); edgeFile << edgeSource; }
    auto edgeDoc = newDwgDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(
        edgeDoc, edgeInput, lc::persistence::File::LIBDXFRW).ok);
    lc::entity::Hatch_CSPtr edgeHatch;
    for (const auto& entity : edgeDoc->entityContainer().asVector())
        if (auto candidate = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity))
            edgeHatch = candidate;
    ASSERT_NE(edgeHatch, nullptr);
    ASSERT_EQ(edgeHatch->getRegion().loopList().size(), 1u);
    const auto& edgeEntities = edgeHatch->getRegion().loopList()[0].entities();
    ASSERT_EQ(edgeEntities.size(), 4u);
    const auto line = std::dynamic_pointer_cast<const lc::entity::Line>(edgeEntities[0]);
    const auto arc = std::dynamic_pointer_cast<const lc::entity::Arc>(edgeEntities[1]);
    const auto ellipse = std::dynamic_pointer_cast<const lc::entity::Ellipse>(edgeEntities[2]);
    const auto spline = std::dynamic_pointer_cast<const lc::entity::Spline>(edgeEntities[3]);
    ASSERT_NE(line, nullptr);
    ASSERT_NE(arc, nullptr);
    ASSERT_NE(ellipse, nullptr);
    ASSERT_NE(spline, nullptr);
    EXPECT_DOUBLE_EQ(line->start().x(), 3.0);
    EXPECT_DOUBLE_EQ(line->start().y(), 7.0);
    EXPECT_DOUBLE_EQ(line->end().x(), 6.0);
    EXPECT_DOUBLE_EQ(line->end().y(), 7.0);
    EXPECT_DOUBLE_EQ(arc->center().x(), 6.0);
    EXPECT_DOUBLE_EQ(arc->center().y(), 8.0);
    EXPECT_DOUBLE_EQ(arc->radius(), 1.0);
    EXPECT_DOUBLE_EQ(ellipse->center().x(), 8.0);
    EXPECT_DOUBLE_EQ(ellipse->center().y(), 11.0);
    EXPECT_DOUBLE_EQ(ellipse->majorP().x(), 2.0);
    EXPECT_DOUBLE_EQ(ellipse->majorP().y(), 0.0);
    ASSERT_EQ(spline->controlPoints().size(), 3u);
    EXPECT_DOUBLE_EQ(spline->controlPoints()[0].x(), 3.0);
    EXPECT_DOUBLE_EQ(spline->controlPoints()[0].y(), 5.0);
    EXPECT_DOUBLE_EQ(spline->controlPoints()[2].x(), 2.0);
    EXPECT_DOUBLE_EQ(spline->controlPoints()[2].y(), 6.0);
    // Ordinary HATCH coordinates must not acquire an MPOLYGON offset.
    std::ifstream simpleFile(input);
    const std::string simpleSource((std::istreambuf_iterator<char>(simpleFile)),
                                   std::istreambuf_iterator<char>());
    std::string hatchSource = simpleSource;
    auto replaceOnce = [](std::string& text, const std::string& from, const std::string& to) {
        const auto pos = text.find(from);
        if (pos == std::string::npos) return false;
        text.replace(pos, from.size(), to);
        return true;
    };
    ASSERT_TRUE(replaceOnce(hatchSource, "0\nMPOLYGON\n", "0\nHATCH\n"));
    ASSERT_TRUE(replaceOnce(hatchSource, "AcDbMPolygon", "AcDbHatch"));
    ASSERT_TRUE(replaceOnce(hatchSource, "71\n1\n91\n", "71\n0\n91\n"));
    ASSERT_TRUE(replaceOnce(hatchSource, "63\n3\n98\n0\n11\n2\n21\n5\n99\n0\n", "98\n0\n"));
    const std::string hatchInput = tmpPath("ordinary-hatch-source");
    { std::ofstream hatchFile(hatchInput); hatchFile << hatchSource; }
    auto hatchDoc = newDwgDocument();
    const auto hatchRead = lc::persistence::File::importFile(
        hatchDoc, hatchInput, lc::persistence::File::LIBDXFRW);
    ASSERT_TRUE(hatchRead.ok);
    EXPECT_TRUE(hatchRead.loss.notes.empty());
    lc::entity::Hatch_CSPtr ordinaryHatch;
    for (const auto& entity : hatchDoc->entityContainer().asVector())
        if (auto candidate = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity))
            ordinaryHatch = candidate;
    ASSERT_NE(ordinaryHatch, nullptr);
    ASSERT_EQ(ordinaryHatch->getRegion().loopList().size(), 1u);
    const auto& hatchEdges = ordinaryHatch->getRegion().loopList()[0].entities();
    ASSERT_EQ(hatchEdges.size(), 1u);
    const auto hatchPoly = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(hatchEdges[0]);
    ASSERT_NE(hatchPoly, nullptr);
    ASSERT_EQ(hatchPoly->vertex().size(), 3u);
    EXPECT_DOUBLE_EQ(hatchPoly->vertex()[0].location().x(), 1.0);
    EXPECT_DOUBLE_EQ(hatchPoly->vertex()[0].location().y(), 2.0);

    std::string tiltedSource = simpleSource;
    ASSERT_TRUE(replaceOnce(tiltedSource, "220\n0\n230\n1\n", "220\n0.6\n230\n0.8\n"));
    const std::string tiltedInput = tmpPath("tilted-mpolygon-source");
    { std::ofstream tiltedFile(tiltedInput); tiltedFile << tiltedSource; }
    auto tiltedDoc = newDwgDocument();
    const auto tiltedRead = lc::persistence::File::importFile(
        tiltedDoc, tiltedInput, lc::persistence::File::LIBDXFRW);
    ASSERT_TRUE(tiltedRead.ok);
    EXPECT_TRUE(std::any_of(tiltedRead.loss.notes.begin(), tiltedRead.loss.notes.end(),
                            [](const std::string& note) {
                                return note.find("elevation or extrusion") != std::string::npos;
                            }));

    std::string overflowSource = simpleSource;
    ASSERT_TRUE(replaceOnce(overflowSource, "10\n1\n20\n2\n10\n4\n", "10\n1e308\n20\n2\n10\n4\n"));
    ASSERT_TRUE(replaceOnce(overflowSource, "11\n2\n21\n5\n99\n", "11\n1e308\n21\n5\n99\n"));
    const std::string overflowInput = tmpPath("overflow-mpolygon-source");
    { std::ofstream overflowFile(overflowInput); overflowFile << overflowSource; }
    auto overflowDoc = newDwgDocument();
    const auto overflowRead = lc::persistence::File::importFile(
        overflowDoc, overflowInput, lc::persistence::File::LIBDXFRW);
    ASSERT_EQ(overflowRead.failures.size(), 1u);
    EXPECT_NE(overflowRead.failures.front().reason.find("offset is not finite"), std::string::npos);
    EXPECT_TRUE(overflowDoc->entityContainer().asVector().empty());
    auto retryDoc = newDwgDocument();
    const auto retryRead = lc::persistence::File::importFile(
        retryDoc, input, lc::persistence::File::LIBDXFRW);
    EXPECT_TRUE(retryRead.ok);
    EXPECT_TRUE(retryRead.failures.empty());

    // MPOLYGON's group 71 is the solid-fill flag even when the older bundled
    // codec forwards it in the HATCH associativity field.
    std::string patternSource = simpleSource;
    ASSERT_TRUE(replaceOnce(patternSource, "71\n1\n91\n", "71\n0\n91\n"));
    const std::string patternInput = tmpPath("pattern-mpolygon-source");
    { std::ofstream patternFile(patternInput); patternFile << patternSource; }
    auto patternDoc = newDwgDocument();
    const auto patternRead = lc::persistence::File::importFile(
        patternDoc, patternInput, lc::persistence::File::LIBDXFRW);
    ASSERT_TRUE(patternRead.ok);
    lc::entity::Hatch_CSPtr patternHatch;
    for (const auto& entity : patternDoc->entityContainer().asVector())
        if (auto candidate = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity))
            patternHatch = candidate;
    ASSERT_NE(patternHatch, nullptr);
    EXPECT_FALSE(patternHatch->isSolid());

    // R2000 cannot carry retained R2004 gradient companions. Use the
    // reported projection instead of poisoning the whole write attempt.
    std::string gradientSource = simpleSource;
    ASSERT_TRUE(replaceOnce(gradientSource, "99\n0\n0\nENDSEC\n",
        "99\n0\n450\n0\n451\n0\n460\n0\n461\n0\n452\n1\n"
        "462\n0.5\n453\n0\n470\nLINEAR\n0\nENDSEC\n"));
    const std::string gradientInput = tmpPath("gradient-mpolygon-source");
    { std::ofstream gradientFile(gradientInput); gradientFile << gradientSource; }
    auto gradientDoc = newDwgDocument();
    const auto gradientRead = lc::persistence::File::importFile(
        gradientDoc, gradientInput, lc::persistence::File::LIBDXFRW);
    ASSERT_TRUE(gradientRead.ok);
    ASSERT_GT(gradientRead.entitiesDelivered, 0u);
    const std::string gradientOutput = tmpPath("gradient-mpolygon-r2000");
    const auto gradientWrite = lc::persistence::File::exportFile(
        gradientDoc, gradientOutput, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(gradientWrite.ok);
    EXPECT_TRUE(std::any_of(gradientWrite.loss.notes.begin(), gradientWrite.loss.notes.end(),
                            [](const std::string& note) {
                                return note.find("written as HATCH") != std::string::npos;
                            }));

    // A newer MPOLYGON schema must not reach a version-1-only serializer.
    std::string newerSource = simpleSource;
    ASSERT_TRUE(replaceOnce(newerSource, "70\n1\n10\n", "70\n2\n10\n"));
    const std::string newerInput = tmpPath("newer-mpolygon-source");
    { std::ofstream newerFile(newerInput); newerFile << newerSource; }
    auto newerDoc = newDwgDocument();
    const auto newerRead = lc::persistence::File::importFile(
        newerDoc, newerInput, lc::persistence::File::LIBDXFRW);
    const std::string newerOutput = tmpPath("newer-mpolygon-output");
    if (newerRead.ok && newerRead.entitiesDelivered != 0u) {
        const auto newerWrite = lc::persistence::File::exportFile(
            newerDoc, newerOutput, lc::persistence::File::LIBDXFRW_DXF_R2000);
        ASSERT_TRUE(newerWrite.ok);
        EXPECT_TRUE(std::any_of(newerWrite.loss.notes.begin(), newerWrite.loss.notes.end(),
                                [](const std::string& note) {
                                    return note.find("written as HATCH") != std::string::npos;
                                }));
    }

    for (const auto& path : {input, ascii, binary, edited, rotatedOutput, scaledOutput, reflectedOutput, liftedOutput,
                             complexInput, complexOutput, edgeInput, hatchInput,
                             tiltedInput, overflowInput, gradientInput, gradientOutput,
                             newerInput, newerOutput, patternInput})
        boost::filesystem::remove(path);
}

TEST(DwgImportTest, MpolygonRotationPreservesBulges) {
    const double bulges[] = {0.0, 0.5, -0.5, 2.0};
    const double angles[] = {0.25, -0.5, 1.5707963267948966, 3.141592653589793};
    for (double bulge : bulges) {
        const std::string input = tmpPath("mpolygon-rotation-source");
        {
            std::ofstream f(input);
            ASSERT_TRUE(f.good());
            f << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1018\n0\nENDSEC\n"
                 "0\nSECTION\n2\nENTITIES\n0\nMPOLYGON\n5\n55\n"
                 "100\nAcDbEntity\n8\n0\n100\nAcDbMPolygon\n70\n1\n"
                 "10\n0\n20\n0\n30\n0\n210\n0\n220\n0\n230\n1\n"
                 "2\nSOLID\n71\n1\n91\n1\n92\n2\n73\n1\n72\n1\n93\n3\n"
                 "10\n1\n20\n2\n42\n" << bulge << "\n"
                 "10\n4\n20\n2\n42\n0\n10\n3\n20\n5\n42\n0\n"
                 "76\n1\n78\n0\n63\n3\n98\n0\n11\n2\n21\n5\n99\n0\n"
                 "0\nENDSEC\n0\nEOF\n";
            ASSERT_TRUE(f.good());
        }
        auto doc = newDwgDocument();
        const auto imported = lc::persistence::File::importFile(
            doc, input, lc::persistence::File::LIBDXFRW);
        ASSERT_TRUE(imported.ok);
        lc::entity::Hatch_CSPtr hatch;
        for (const auto& e : doc->entityContainer().asVector())
            if (auto h = std::dynamic_pointer_cast<const lc::entity::Hatch>(e)) hatch = h;
        ASSERT_NE(hatch, nullptr);
        const std::string baseline = tmpPath("mpolygon-rotation-baseline");
        const auto first = lc::persistence::File::exportFile(
            doc, baseline, lc::persistence::File::LIBDXFRW_DXF_R2000);
        ASSERT_TRUE(first.ok);
        const bool supports = first.loss.empty();
        for (double angle : angles) {
            const auto rotated = hatch->rotate(lc::geo::Coordinate(2, -1, 0), angle);
            ASSERT_NE(rotated, nullptr);
            doc->removeEntity(hatch);
            doc->insertEntity(rotated);
            const auto expectedHatch = std::dynamic_pointer_cast<const lc::entity::Hatch>(rotated);
            ASSERT_NE(expectedHatch, nullptr);
            const auto expected = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(
                expectedHatch->getRegion().loopList()[0].entities()[0]);
            ASSERT_NE(expected, nullptr);
            for (auto type : {lc::persistence::File::LIBDXFRW_DXF_R2000,
                              lc::persistence::File::LIBDXFRW_DXB_R2000}) {
                const std::string out = tmpPath("mpolygon-rotation-case");
                const auto written = lc::persistence::File::exportFile(doc, out, type);
                ASSERT_TRUE(written.ok) << bulge << "," << angle;
                EXPECT_EQ(written.loss.empty(), supports) << bulge << "," << angle;
                auto reopened = newDwgDocument();
                const auto read = lc::persistence::File::importFile(
                    reopened, out, lc::persistence::File::LIBDXFRW);
                ASSERT_TRUE(read.ok);
                const bool polygon = std::any_of(read.loss.notes.begin(), read.loss.notes.end(),
                    [](const std::string& note) { return note.find("MPOLYGON entities") != std::string::npos; });
                EXPECT_EQ(polygon, supports) << bulge << "," << angle;
                lc::entity::Hatch_CSPtr result;
                for (const auto& e : reopened->entityContainer().asVector())
                    if (auto h = std::dynamic_pointer_cast<const lc::entity::Hatch>(e)) result = h;
                ASSERT_NE(result, nullptr);
                ASSERT_EQ(result->getRegion().loopList().size(), 1u);
                ASSERT_EQ(result->getRegion().loopList()[0].entities().size(), 1u);
                const auto poly = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(
                    result->getRegion().loopList()[0].entities()[0]);
                ASSERT_NE(poly, nullptr);
                ASSERT_EQ(poly->vertex().size(), expected->vertex().size());
                for (std::size_t i = 0; i < poly->vertex().size(); ++i) {
                    EXPECT_DOUBLE_EQ(poly->vertex()[i].location().x(), expected->vertex()[i].location().x());
                    EXPECT_DOUBLE_EQ(poly->vertex()[i].location().y(), expected->vertex()[i].location().y());
                    EXPECT_DOUBLE_EQ(poly->vertex()[i].bulge(), expected->vertex()[i].bulge());
                }
                boost::filesystem::remove(out);
            }
            doc->removeEntity(rotated);
            doc->insertEntity(hatch);
        }
        boost::filesystem::remove(baseline);
        boost::filesystem::remove(input);
    }
}

// Both inputs are generated from a local LINE + MLINE source; no fixture is
// required in the repository. The DWG is converted by ODA File Converter.
TEST(DwgImportTest, MpolygonLiftFallbackRetainsPlanarElevation) {
    const std::string input = tmpPath("elevation-source");
    {
        std::ofstream f(input);
        ASSERT_TRUE(f.good());
        f << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1018\n0\nENDSEC\n"
             "0\nSECTION\n2\nENTITIES\n0\nMPOLYGON\n5\n55\n"
             "100\nAcDbEntity\n8\n0\n100\nAcDbMPolygon\n70\n1\n"
             "10\n0\n20\n0\n30\n0\n210\n0\n220\n0\n230\n1\n"
             "2\nSOLID\n71\n1\n91\n1\n92\n2\n73\n1\n72\n0\n93\n3\n"
             "10\n1\n20\n2\n10\n4\n20\n2\n10\n3\n20\n5\n"
             "76\n1\n78\n0\n63\n3\n98\n0\n11\n2\n21\n5\n99\n0\n"
             "0\nENDSEC\n0\nEOF\n";
        ASSERT_TRUE(f.good());
    }
    auto doc = newDwgDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(doc, input, lc::persistence::File::LIBDXFRW).ok);
    lc::entity::Hatch_CSPtr hatch;
    for (const auto& e : doc->entityContainer().asVector())
        if (auto h = std::dynamic_pointer_cast<const lc::entity::Hatch>(e)) hatch = h;
    ASSERT_NE(hatch, nullptr);
    auto lifted = hatch->move(lc::geo::Coordinate(0, 0, 3));
    ASSERT_NE(lifted, nullptr);
    doc->removeEntity(hatch);
    doc->insertEntity(lifted);
    const char* capture = std::getenv("LC3_ELEVATION_CAPTURE");
    for (const auto& encoding : std::vector<std::pair<const char*, lc::persistence::File::Type>>{
            {"ascii", lc::persistence::File::LIBDXFRW_DXF_R2000},
            {"binary", lc::persistence::File::LIBDXFRW_DXB_R2000}}) {
        SCOPED_TRACE(encoding.first);
        const std::string output = capture ? std::string(capture) + "/" + encoding.first + ".dxf"
                                           : tmpPath("elevation-output");
        auto written = lc::persistence::File::exportFile(doc, output, encoding.second);
        ASSERT_TRUE(written.ok);
        EXPECT_TRUE(std::any_of(written.loss.notes.begin(), written.loss.notes.end(),
            [](const std::string& n) { return n.find("written as HATCH") != std::string::npos; }));
        auto reopened = newDwgDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(reopened, output, lc::persistence::File::LIBDXFRW).ok);
        lc::entity::Hatch_CSPtr result;
        for (const auto& e : reopened->entityContainer().asVector())
            if (auto h = std::dynamic_pointer_cast<const lc::entity::Hatch>(e)) result = h;
        ASSERT_NE(result, nullptr);
        ASSERT_EQ(result->getRegion().loopList().size(), 1u);
        ASSERT_EQ(result->getRegion().loopList()[0].entities().size(), 1u);
        const auto poly = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(result->getRegion().loopList()[0].entities()[0]);
        ASSERT_NE(poly, nullptr);
        ASSERT_EQ(poly->vertex().size(), 3u);
        const double expected[][2] = {{3,7},{6,7},{5,10}};
        for (std::size_t i = 0; i < 3; ++i) {
            EXPECT_DOUBLE_EQ(poly->vertex()[i].location().x(), expected[i][0]);
            EXPECT_DOUBLE_EQ(poly->vertex()[i].location().y(), expected[i][1]);
            EXPECT_DOUBLE_EQ(poly->vertex()[i].location().z(), 3.0);
        }
        const std::string resaved = capture ? std::string(capture) + "/resave-" + encoding.first + ".dxf"
                                            : tmpPath("elevation-resave");
        ASSERT_TRUE(lc::persistence::File::exportFile(reopened, resaved, encoding.second).ok);
        auto reread = newDwgDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(reread, resaved, lc::persistence::File::LIBDXFRW).ok);
        lc::entity::Hatch_CSPtr resavedHatch;
        for (const auto& e : reread->entityContainer().asVector())
            if (auto h = std::dynamic_pointer_cast<const lc::entity::Hatch>(e)) resavedHatch = h;
        ASSERT_NE(resavedHatch, nullptr);
        ASSERT_EQ(resavedHatch->getRegion().loopList().size(), 1u);
        ASSERT_EQ(resavedHatch->getRegion().loopList()[0].entities().size(), 1u);
        const auto resavedPoly = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(
            resavedHatch->getRegion().loopList()[0].entities()[0]);
        ASSERT_NE(resavedPoly, nullptr);
        ASSERT_EQ(resavedPoly->vertex().size(), 3u);
        for (const auto& v : resavedPoly->vertex()) EXPECT_DOUBLE_EQ(v.location().z(), 3.0);
        if (!capture) { boost::filesystem::remove(output); boost::filesystem::remove(resaved); }
    }
    // Mixed Z values cannot be represented by a single HATCH elevation.
    // The existing lossy fallback must not invent one from the first vertex.
    const auto liftedHatch = std::dynamic_pointer_cast<const lc::entity::Hatch>(lifted);
    ASSERT_NE(liftedHatch, nullptr);
    const auto sourcePoly = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(
        liftedHatch->getRegion().loopList()[0].entities()[0]);
    ASSERT_NE(sourcePoly, nullptr);
    auto vertices = sourcePoly->vertex();
    ASSERT_EQ(vertices.size(), 3u);
    vertices[2]._location = lc::geo::Coordinate(vertices[2].location().x(), vertices[2].location().y(), 4.0);
    auto mixedPoly = std::make_shared<lc::entity::LWPolyline>(vertices, sourcePoly->width(),
        sourcePoly->elevation(), sourcePoly->tickness(), sourcePoly->closed(),
        sourcePoly->extrusionDirection(), sourcePoly->layer());
    lc::geo::Region mixedRegion;
    mixedRegion.addLoop(lc::geo::Loop({mixedPoly}));
    auto mixed = std::make_shared<lc::entity::Hatch>(hatch->layer(), hatch->metaInfo(), hatch->block());
    mixed->setRegion(mixedRegion);
    mixed->setPatternName(hatch->getPatternName());
    mixed->setSolid(hatch->isSolid());
    mixed->setAngle(hatch->getAngle());
    mixed->setScale(hatch->getScale());
    doc->removeEntity(lifted);
    doc->insertEntity(mixed);
    const std::string mixedOutput = capture ? std::string(capture) + "/mixed-ascii.dxf" : tmpPath("mixed-elevation-output");
    const auto mixedWrite = lc::persistence::File::exportFile(doc, mixedOutput, lc::persistence::File::LIBDXFRW_DXF_R2000);
    ASSERT_TRUE(mixedWrite.ok);
    EXPECT_TRUE(std::any_of(mixedWrite.loss.notes.begin(), mixedWrite.loss.notes.end(),
        [](const std::string& n) { return n.find("written as HATCH") != std::string::npos; }));
    std::ifstream mixedFile(mixedOutput);
    ASSERT_TRUE(mixedFile.good());
    bool inHatch = false, checkedElevation = false;
    std::string code, value;
    while (std::getline(mixedFile, code) && std::getline(mixedFile, value)) {
        const int group = std::stoi(code);
        if (group == 0) { if (inHatch) break; inHatch = value == "HATCH"; }
        if (inHatch && group == 30) {
            EXPECT_DOUBLE_EQ(std::stod(value), 0.0);
            checkedElevation = true;
            break;
        }
    }
    EXPECT_TRUE(checkedElevation);
    if (!capture) boost::filesystem::remove(mixedOutput);
    // Restore and retry using the same document after the refused plane.
    doc->removeEntity(mixed);
    doc->insertEntity(lifted);
    const std::string retryOutput = capture ? std::string(capture) + "/retry-ascii.dxf" : tmpPath("retry-elevation-output");
    ASSERT_TRUE(lc::persistence::File::exportFile(doc, retryOutput, lc::persistence::File::LIBDXFRW_DXF_R2000).ok);
    auto retryDoc = newDwgDocument();
    ASSERT_TRUE(lc::persistence::File::importFile(retryDoc, retryOutput, lc::persistence::File::LIBDXFRW).ok);
    lc::entity::Hatch_CSPtr retryHatch;
    for (const auto& e : retryDoc->entityContainer().asVector())
        if (auto h = std::dynamic_pointer_cast<const lc::entity::Hatch>(e)) retryHatch = h;
    ASSERT_NE(retryHatch, nullptr);
    ASSERT_EQ(retryHatch->getRegion().loopList().size(), 1u);
    ASSERT_EQ(retryHatch->getRegion().loopList()[0].entities().size(), 1u);
    auto retryPoly = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(retryHatch->getRegion().loopList()[0].entities()[0]);
    ASSERT_NE(retryPoly, nullptr);
    ASSERT_EQ(retryPoly->vertex().size(), 3u);
    for (const auto& v : retryPoly->vertex()) EXPECT_DOUBLE_EQ(v.location().z(), 3.0);
    if (!capture) boost::filesystem::remove(retryOutput);
    boost::filesystem::remove(input);
}

TEST(DwgImportTest, ElevatedMultiLoopHatchAndMpolygonRoundTrip) {
    const char* capture = std::getenv("LC3_MULTILOOP_CAPTURE");
    for (bool polygon : {false, true}) {
        SCOPED_TRACE(polygon ? "MPOLYGON" : "HATCH");
        const std::string input = tmpPath("elevated-multiloop-input");
        {
            std::ofstream f(input);
            ASSERT_TRUE(f.good());
            f << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1018\n0\nENDSEC\n"
                 "0\nSECTION\n2\nENTITIES\n0\n" << (polygon ? "MPOLYGON" : "HATCH")
              << "\n5\n55\n100\nAcDbEntity\n8\n0\n100\n"
              << (polygon ? "AcDbMPolygon" : "AcDbHatch")
              << "\n70\n1\n10\n0\n20\n0\n30\n3\n210\n0\n220\n0\n230\n1\n"
                 "2\nSOLID\n71\n" << (polygon ? 1 : 0)
              << "\n91\n2\n92\n2\n73\n1\n72\n0\n93\n3\n"
                 "10\n1\n20\n2\n10\n4\n20\n2\n10\n3\n20\n5\n97\n0\n"
                 "92\n2\n73\n1\n72\n0\n93\n3\n"
                 "10\n7\n20\n2\n10\n9\n20\n2\n10\n8\n20\n4\n97\n0\n"
                 "76\n1\n78\n0\n";
            if (polygon) f << "63\n3\n98\n0\n11\n2\n21\n5\n99\n0\n";
            else f << "75\n0\n98\n0\n";
            f << "0\nENDSEC\n0\nEOF\n";
            ASSERT_TRUE(f.good());
        }
        const double offsetX = polygon ? 2.0 : 0.0;
        const double offsetY = polygon ? 5.0 : 0.0;
        const double expected[2][3][2] = {{{1,2},{4,2},{3,5}},{{7,2},{9,2},{8,4}}};
        const auto check = [&](const std::shared_ptr<lc::storage::DocumentImpl>& doc) {
            lc::entity::Hatch_CSPtr hatch;
            for (const auto& e : doc->entityContainer().asVector())
                if (auto h = std::dynamic_pointer_cast<const lc::entity::Hatch>(e)) hatch = h;
            ASSERT_NE(hatch, nullptr);
            const auto& loops = hatch->getRegion().loopList();
            ASSERT_EQ(loops.size(), 2u);
            for (std::size_t loop = 0; loop < 2; ++loop) {
                ASSERT_EQ(loops[loop].entities().size(), 1u);
                const auto poly = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(loops[loop].entities()[0]);
                ASSERT_NE(poly, nullptr);
                ASSERT_EQ(poly->vertex().size(), 3u);
                for (std::size_t i = 0; i < 3; ++i) {
                    EXPECT_DOUBLE_EQ(poly->vertex()[i].location().x(), expected[loop][i][0] + offsetX);
                    EXPECT_DOUBLE_EQ(poly->vertex()[i].location().y(), expected[loop][i][1] + offsetY);
                    EXPECT_DOUBLE_EQ(poly->vertex()[i].location().z(), 3.0);
                }
            }
        };
        auto doc = newDwgDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(doc, input, lc::persistence::File::LIBDXFRW).ok);
        check(doc);
        for (const auto& enc : std::vector<std::pair<const char*, lc::persistence::File::Type>>{
                 {"ascii", lc::persistence::File::LIBDXFRW_DXF_R2000},
                 {"binary", lc::persistence::File::LIBDXFRW_DXB_R2000}}) {
            SCOPED_TRACE(enc.first);
            const std::string name = std::string(polygon ? "mpolygon-" : "hatch-") + enc.first + ".dxf";
            const std::string output = capture ? std::string(capture) + "/" + name : tmpPath("multiloop-output");
            const auto result = lc::persistence::File::exportFile(doc, output, enc.second);
            ASSERT_TRUE(result.ok);
            if (polygon) EXPECT_TRUE(std::any_of(result.loss.notes.begin(), result.loss.notes.end(),
                [](const std::string& n) { return n.find("written as HATCH") != std::string::npos; }));
            auto reopened = newDwgDocument();
            ASSERT_TRUE(lc::persistence::File::importFile(reopened, output, lc::persistence::File::LIBDXFRW).ok);
            check(reopened);
            const std::string second = capture ? std::string(capture) + "/resave-" + name : tmpPath("multiloop-resave");
            ASSERT_TRUE(lc::persistence::File::exportFile(reopened, second, enc.second).ok);
            auto reread = newDwgDocument();
            ASSERT_TRUE(lc::persistence::File::importFile(reread, second, lc::persistence::File::LIBDXFRW).ok);
            check(reread);
            if (!capture) { boost::filesystem::remove(output); boost::filesystem::remove(second); }
        }
        // Different loop planes have no single HATCH elevation. Do not
        // silently choose the first loop's Z; then restore and retry.
        lc::entity::Hatch_CSPtr original;
        for (const auto& e : doc->entityContainer().asVector())
            if (auto h = std::dynamic_pointer_cast<const lc::entity::Hatch>(e)) original = h;
        ASSERT_NE(original, nullptr);
        lc::geo::Region mixedRegion;
        ASSERT_EQ(original->getRegion().loopList().size(), 2u);
        for (std::size_t n = 0; n < 2; ++n) {
            const auto& loop = original->getRegion().loopList()[n];
            ASSERT_EQ(loop.entities().size(), 1u);
            const auto poly = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(loop.entities()[0]);
            ASSERT_NE(poly, nullptr);
            auto vertices = poly->vertex();
            for (auto& v : vertices)
                v._location = lc::geo::Coordinate(v.location().x(), v.location().y(), n == 0 ? 3.0 : 4.0);
            auto changed = std::make_shared<lc::entity::LWPolyline>(vertices, poly->width(),
                poly->elevation(), poly->tickness(), poly->closed(), poly->extrusionDirection(), poly->layer());
            mixedRegion.addLoop(lc::geo::Loop({changed}));
        }
        auto mixed = std::make_shared<lc::entity::Hatch>(original->layer(), original->metaInfo(), original->block());
        mixed->setRegion(mixedRegion);
        mixed->setPatternName(original->getPatternName());
        mixed->setSolid(original->isSolid());
        mixed->setAngle(original->getAngle());
        mixed->setScale(original->getScale());
        doc->removeEntity(original);
        doc->insertEntity(mixed);
        const std::string mixedName = polygon ? "mixed-mpolygon.dxf" : "mixed-hatch.dxf";
        const std::string mixedOutput = capture ? std::string(capture) + "/" + mixedName : tmpPath("mixed-loops");
        const auto mixedWrite = lc::persistence::File::exportFile(doc, mixedOutput, lc::persistence::File::LIBDXFRW_DXF_R2000);
        ASSERT_TRUE(mixedWrite.ok);
        if (polygon) EXPECT_TRUE(std::any_of(mixedWrite.loss.notes.begin(), mixedWrite.loss.notes.end(),
            [](const std::string& n) { return n.find("written as HATCH") != std::string::npos; }));
        else EXPECT_TRUE(std::any_of(mixedWrite.loss.notes.begin(), mixedWrite.loss.notes.end(),
            [](const std::string& n) { return n.find("HATCH boundary elevation varies") != std::string::npos; }));
        std::ifstream raw(mixedOutput);
        ASSERT_TRUE(raw.good());
        bool inHatch = false, checkedElevation = false;
        std::string code, value;
        while (std::getline(raw, code) && std::getline(raw, value)) {
            const int group = std::stoi(code);
            if (group == 0) { if (inHatch) break; inHatch = value == "HATCH"; }
            if (inHatch && group == 30) {
                EXPECT_DOUBLE_EQ(std::stod(value), 0.0);
                checkedElevation = true;
                break;
            }
        }
        EXPECT_TRUE(checkedElevation);
        if (!capture) boost::filesystem::remove(mixedOutput);
        doc->removeEntity(mixed);
        doc->insertEntity(original);
        const std::string retryName = polygon ? "retry-mpolygon.dxf" : "retry-hatch.dxf";
        const std::string retryOutput = capture ? std::string(capture) + "/" + retryName : tmpPath("retry-loops");
        ASSERT_TRUE(lc::persistence::File::exportFile(doc, retryOutput, lc::persistence::File::LIBDXFRW_DXF_R2000).ok);
        auto retried = newDwgDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(retried, retryOutput, lc::persistence::File::LIBDXFRW).ok);
        check(retried);
        if (!capture) boost::filesystem::remove(retryOutput);
        boost::filesystem::remove(input);
    }
}

TEST(DwgImportTest, RationalHatchSplineWeightsSurviveSave) {
    const auto input = tmpPath("rational-hatch-spline");
    {
        std::ofstream f(input);
        f << "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1018\n0\nENDSEC\n"
             "0\nSECTION\n2\nENTITIES\n0\nHATCH\n5\n55\n100\nAcDbEntity\n8\n0\n"
             "100\nAcDbHatch\n10\n0\n20\n0\n30\n3\n210\n0\n220\n0\n230\n1\n"
             "2\nSOLID\n70\n1\n71\n0\n91\n1\n92\n0\n93\n2\n"
             "72\n4\n94\n2\n73\n1\n74\n0\n95\n6\n96\n3\n"
             "40\n0\n40\n0\n40\n0\n40\n1\n40\n1\n40\n1\n"
             "10\n1\n20\n0\n42\n1\n10\n1\n20\n1\n42\n0.7071067811865476\n10\n0\n20\n1\n42\n1\n97\n0\n"
             "72\n1\n10\n0\n20\n1\n11\n1\n21\n0\n"
             "97\n0\n75\n0\n76\n1\n78\n0\n98\n0\n0\nENDSEC\n0\nEOF\n";
        ASSERT_TRUE(f.good());
    }
    const std::vector<double> weights{1,0.7071067811865476,1};
    const auto check = [&weights](const std::shared_ptr<lc::storage::DocumentImpl>& doc, double elevation) {
        lc::entity::Hatch_CSPtr hatch;
        for (const auto& entity : doc->entityContainer().asVector())
            if (auto value = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity)) hatch = value;
        ASSERT_NE(hatch, nullptr);
        const auto& loops = hatch->getRegion().loopList();
        ASSERT_EQ(loops.size(), 1u);
        ASSERT_EQ(loops[0].entities().size(), 2u);
        auto spline = std::dynamic_pointer_cast<const lc::entity::Spline>(loops[0].entities()[0]);
        ASSERT_NE(spline, nullptr);
        EXPECT_EQ(spline->weights(), weights);
        EXPECT_NE(spline->flags() & lc::geo::Spline::RATIONAL, 0);
        ASSERT_EQ(spline->controlPoints().size(), 3u);
        EXPECT_DOUBLE_EQ(spline->controlPoints()[1].z(), elevation);
    };
    auto doc = newDwgDocument();
    const auto imported = lc::persistence::File::importFile(doc, input, lc::persistence::File::LIBDXFRW);
    ASSERT_TRUE(imported.ok);
    ASSERT_EQ(imported.loss.notes.size(), 1u);
    EXPECT_NE(imported.loss.notes[0].find("displayed curve"), std::string::npos);
    check(doc, 3.0);
    lc::entity::Hatch_CSPtr original;
    for (const auto& entity : doc->entityContainer().asVector())
        if (auto value = std::dynamic_pointer_cast<const lc::entity::Hatch>(entity)) original = value;
    ASSERT_NE(original, nullptr);
    const char* capture = std::getenv("LC3_RATIONAL_HATCH_CAPTURE");
    for (bool moved : {false, true}) {
        if (moved) {
            doc->removeEntity(original);
            doc->insertEntity(original->move({2,5,4}));
            check(doc, 7.0);
        }
        for (auto type : {lc::persistence::File::LIBDXFRW_DXF_R2000, lc::persistence::File::LIBDXFRW_DXB_R2000}) {
            const auto output = capture ? std::string(capture) + (moved ? "/moved-" : "/initial-")
                + (type == lc::persistence::File::LIBDXFRW_DXF_R2000 ? "ascii.dxf" : "binary.dxf")
                : tmpPath("rational-hatch-output");
            const auto exported = lc::persistence::File::exportFile(doc, output, type);
            ASSERT_TRUE(exported.ok);
            EXPECT_EQ(exported.loss.total(), 0u);
            auto reopened = newDwgDocument();
            ASSERT_TRUE(lc::persistence::File::importFile(reopened, output, lc::persistence::File::LIBDXFRW).ok);
            check(reopened, moved ? 7.0 : 3.0);
            if (!capture) boost::filesystem::remove(output);
        }
    }
    // An edited spline with an incomplete weight vector must be reported as
    // an omitted HATCH, rather than being silently padded by the older codec.
    const auto& originalEdges = original->getRegion().loopList()[0].entities();
    const auto sourceSpline = std::dynamic_pointer_cast<const lc::entity::Spline>(originalEdges[0]);
    ASSERT_NE(sourceSpline, nullptr);
    lc::builder::SplineBuilder builder;
    builder.copy(sourceSpline);
    builder.setWeights({1.0});
    lc::geo::Region invalidRegion;
    invalidRegion.addLoop(lc::geo::Loop({builder.build(), originalEdges[1]}));
    auto invalidHatch = std::make_shared<lc::entity::Hatch>(original, false);
    invalidHatch->setRegion(invalidRegion);
    auto invalidDoc = newDwgDocument();
    invalidDoc->insertEntity(invalidHatch);
    const auto retryPath = tmpPath("rational-hatch-invalid");
    for (auto type : {lc::persistence::File::LIBDXFRW_DXF_R2000, lc::persistence::File::LIBDXFRW_DXB_R2000}) {
        const auto failed = lc::persistence::File::exportFile(invalidDoc, retryPath, type);
        EXPECT_TRUE(!failed.ok || failed.loss.total() > 0u);
        auto corrected = newDwgDocument();
        corrected->insertEntity(original);
        const auto retried = lc::persistence::File::exportFile(corrected, retryPath, type);
        ASSERT_TRUE(retried.ok);
        EXPECT_EQ(retried.loss.total(), 0u);
        auto reopened = newDwgDocument();
        ASSERT_TRUE(lc::persistence::File::importFile(reopened, retryPath, lc::persistence::File::LIBDXFRW).ok);
        check(reopened, 3.0);
    }
    boost::filesystem::remove(retryPath);
    boost::filesystem::remove(input);
}
