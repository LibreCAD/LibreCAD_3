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

#include <map>
#include <memory>
#include <string>

#include <boost/filesystem.hpp>

#include <gtest/gtest.h>

#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>

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
            / (std::string("lc3-dwg-") + tag + "-" + std::to_string(::getpid()) + ".dxf")).string();
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

        EXPECT_EQ(fromDxf->blocks().size(), fromDwg->blocks().size()) << name;
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
