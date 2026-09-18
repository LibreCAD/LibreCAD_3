// Characterisation of lc::persistence::File's four format helpers.
//
// Written before `format-variant-table` moves them onto a table, and asserting
// exactly what they answered beforehand -- every label, every extension, every
// library, character for character. The point is not that these strings are
// good. Two of them are wrong on their face ("DXB" names a format LibreCAD does
// not write) and the labels are visible in the Save dialog. The point is that a
// refactor which quietly changes one is a user-visible regression that no other
// test in this repository would catch.
//
// Like the rest of the persistence tests, this includes no libdxfrw header:
// `persistence` links the library PRIVATE so that its C++17 requirement stays
// out of lcunittest, kaguya scripting tests included.

#include <map>
#include <set>
#include <string>

#include <gtest/gtest.h>

#include "persistence/file.h"
#include "persistence/format.h"

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(FormatTest, AdvertisedFileTypesAreUnchanged) {
    const std::map<lc::persistence::File::Type, std::string> expected{
        {lc::persistence::File::LIBDXFRW_DXF_R12, "DXF R12 (libdxfrw)"},
        {lc::persistence::File::LIBDXFRW_DXF_R14, "DXF R14 (libdxfrw)"},
        {lc::persistence::File::LIBDXFRW_DXF_R2000, "DXF 2000 (libdxfrw)"},
        {lc::persistence::File::LIBDXFRW_DXF_R2004, "DXF 2004 (libdxfrw)"},
        {lc::persistence::File::LIBDXFRW_DXF_R2007, "DXF 2007 (libdxfrw)"},
        {lc::persistence::File::LIBDXFRW_DXF_R2010, "DXF 2010 (libdxfrw)"},
        {lc::persistence::File::LIBDXFRW_DXF_R2013, "DXF 2013 (libdxfrw)"},
        {lc::persistence::File::LIBDXFRW_DXB_R12, "DXF R12 binary (libdxfrw)"},
        {lc::persistence::File::LIBDXFRW_DXB_R14, "DXF R14 binary (libdxfrw)"},
        {lc::persistence::File::LIBDXFRW_DXB_R2000, "DXF 2000 binary (libdxfrw)"},
        {lc::persistence::File::LIBDXFRW_DXB_R2004, "DXF 2004 binary (libdxfrw)"},
        {lc::persistence::File::LIBDXFRW_DXB_R2007, "DXF 2007 binary (libdxfrw)"},
        {lc::persistence::File::LIBDXFRW_DXB_R2010, "DXF 2010 binary (libdxfrw)"},
        {lc::persistence::File::LIBDXFRW_DXB_R2013, "DXF 2013 binary (libdxfrw)"},
    };

    EXPECT_EQ(lc::persistence::File::getAvailableFileTypes(), expected);
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(FormatTest, ExtensionsAreUnchanged) {
    for (const auto& advertised : lc::persistence::File::getAvailableFileTypes()) {
        EXPECT_EQ(lc::persistence::File::getExtensionForFileType(advertised.first), "dxf")
            << advertised.second << " is written through libdxfrw; only the encoding differs.";
    }

    // The one type with no writer answers with no extension.
    EXPECT_EQ(lc::persistence::File::getExtensionForFileType(
                  lc::persistence::File::LIBOPENCAD_DWG), "");

    const std::map<std::string, std::string> expected{
        {"dxf", "DXF files"},
        {"dwg", "DWG files"},
    };
    EXPECT_EQ(lc::persistence::File::getSupportedFileExtensions(), expected);
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(FormatTest, LibrariesPerFormatAreUnchanged) {
    const std::map<lc::persistence::File::Library, std::string> dxf{
        {lc::persistence::File::LIBDXFRW, "libdxfrw"},
    };

    EXPECT_EQ(lc::persistence::File::getAvailableLibrariesForFormat("dxf"), dxf);
    EXPECT_EQ(lc::persistence::File::getAvailableLibrariesForFormat("DXF"), dxf)
        << "The format is matched case-insensitively.";
    EXPECT_EQ(lc::persistence::File::getAvailableLibrariesForFormat("Dxf"), dxf);

    // DWG answers with nothing unless a DWG reader is compiled in, and this
    // build has none. PatternProvider dereferences begin() on this map.
    EXPECT_TRUE(lc::persistence::File::getAvailableLibrariesForFormat("dwg").empty());
    EXPECT_TRUE(lc::persistence::File::getAvailableLibrariesForFormat("png").empty());
    EXPECT_TRUE(lc::persistence::File::getAvailableLibrariesForFormat("").empty());
}

// The type predicates are the reason the enumerator *order* stopped being
// load-bearing; pin them per enumerator so a reordering or an insertion cannot
// quietly change which types are writable or binary.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(FormatTest, TypePredicatesAreUnchanged) {
    const struct {
        lc::persistence::File::Type type;
        bool libdxfrw;
        bool binary;
    } expected[] = {
        {lc::persistence::File::LIBDXFRW_DXF_R12, true, false},
        {lc::persistence::File::LIBDXFRW_DXF_R14, true, false},
        {lc::persistence::File::LIBDXFRW_DXF_R2000, true, false},
        {lc::persistence::File::LIBDXFRW_DXF_R2004, true, false},
        {lc::persistence::File::LIBDXFRW_DXF_R2007, true, false},
        {lc::persistence::File::LIBDXFRW_DXF_R2010, true, false},
        {lc::persistence::File::LIBDXFRW_DXF_R2013, true, false},
        {lc::persistence::File::LIBDXFRW_DXB_R12, true, true},
        {lc::persistence::File::LIBDXFRW_DXB_R14, true, true},
        {lc::persistence::File::LIBDXFRW_DXB_R2000, true, true},
        {lc::persistence::File::LIBDXFRW_DXB_R2004, true, true},
        {lc::persistence::File::LIBDXFRW_DXB_R2007, true, true},
        {lc::persistence::File::LIBDXFRW_DXB_R2010, true, true},
        {lc::persistence::File::LIBDXFRW_DXB_R2013, true, true},
        {lc::persistence::File::LIBOPENCAD_DWG, false, false},
    };

    for (const auto& e : expected) {
        EXPECT_EQ(lc::persistence::File::isLibdxfrwType(e.type), e.libdxfrw)
            << "type " << static_cast<int>(e.type);
        EXPECT_EQ(lc::persistence::File::isBinaryType(e.type), e.binary)
            << "type " << static_cast<int>(e.type);
    }

    // The enumerator values themselves are persisted in settings and scripts.
    EXPECT_EQ(static_cast<int>(lc::persistence::File::LIBDXFRW_DXF_R12), 0);
    EXPECT_EQ(static_cast<int>(lc::persistence::File::LIBOPENCAD_DWG), 14);
}

// The table behind those helpers, checked for the properties the helpers rely
// on rather than for its contents: a duplicate id would make
// formatVariantById() answer with whichever came first, and a writable variant
// with no extension or no library is a Save target nothing can carry out.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(FormatTest, VariantTableIsWellFormed) {
    std::set<std::string> ids;
    std::set<std::string> knownFormats;
    for (const auto& format : lc::persistence::formats()) {
        EXPECT_FALSE(format.id.empty());
        EXPECT_FALSE(format.label.empty());
        knownFormats.insert(format.id);
    }

    for (const auto& variant : lc::persistence::formatVariants()) {
        EXPECT_FALSE(variant.id.empty());
        EXPECT_TRUE(ids.insert(variant.id).second) << variant.id << " is in the table twice.";
        EXPECT_EQ(lc::persistence::formatVariantById(variant.id), &variant) << variant.id;
        EXPECT_EQ(knownFormats.count(variant.formatId), 1u)
            << variant.id << " belongs to a format nothing lists.";
        EXPECT_FALSE(variant.label.empty()) << variant.id;

        if (variant.writable) {
            EXPECT_FALSE(variant.extension.empty()) << variant.id << " can be saved to nothing.";
            EXPECT_FALSE(variant.libraryId.empty()) << variant.id << " has no writer.";
            EXPECT_TRUE(variant.readable) << variant.id << " could be written but never reopened.";
        }
        if (!variant.versionTag.empty()) {
            EXPECT_EQ(variant.versionTag.substr(0, 2), "AC") << variant.id;
        }
    }

    EXPECT_EQ(lc::persistence::formatVariantById("dxf.nope"), nullptr);
    EXPECT_EQ(lc::persistence::formatVariantById(""), nullptr);
}

// File::Type is a wire value that scripts and saved settings hold; the variant
// id is what everything above persistence should hold instead. The two must
// agree in both directions, or the next layer up gets a different answer
// depending on which one it asked with.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(FormatTest, EveryTypeRoundTripsThroughItsVariantId) {
    const lc::persistence::File::Type all[] = {
        lc::persistence::File::LIBDXFRW_DXF_R12, lc::persistence::File::LIBDXFRW_DXF_R14,
        lc::persistence::File::LIBDXFRW_DXF_R2000, lc::persistence::File::LIBDXFRW_DXF_R2004,
        lc::persistence::File::LIBDXFRW_DXF_R2007, lc::persistence::File::LIBDXFRW_DXF_R2010,
        lc::persistence::File::LIBDXFRW_DXF_R2013, lc::persistence::File::LIBDXFRW_DXB_R12,
        lc::persistence::File::LIBDXFRW_DXB_R14, lc::persistence::File::LIBDXFRW_DXB_R2000,
        lc::persistence::File::LIBDXFRW_DXB_R2004, lc::persistence::File::LIBDXFRW_DXB_R2007,
        lc::persistence::File::LIBDXFRW_DXB_R2010, lc::persistence::File::LIBDXFRW_DXB_R2013,
        lc::persistence::File::LIBOPENCAD_DWG,
    };

    for (const auto type : all) {
        const auto id = lc::persistence::File::variantIdForType(type);
        ASSERT_FALSE(id.empty()) << "type " << static_cast<int>(type) << " has no variant.";

        const auto* variant = lc::persistence::formatVariantById(id);
        ASSERT_NE(variant, nullptr) << id;

        // The two predicates must agree with the variant they name.
        EXPECT_EQ(lc::persistence::File::isBinaryType(type), variant->binary && variant->writable)
            << id;
        EXPECT_EQ(lc::persistence::File::isLibdxfrwType(type), variant->libraryId == "libdxfrw")
            << id;

        lc::persistence::File::Type back = lc::persistence::File::LIBOPENCAD_DWG;
        EXPECT_TRUE(lc::persistence::File::typeForVariantId(id, back)) << id;
        EXPECT_EQ(back, type) << id;
    }

    lc::persistence::File::Type untouched = lc::persistence::File::LIBDXFRW_DXF_R2000;
    EXPECT_FALSE(lc::persistence::File::typeForVariantId("dxf.nope", untouched));
    EXPECT_EQ(untouched, lc::persistence::File::LIBDXFRW_DXF_R2000)
        << "A failed lookup must not have written to the output.";
}

// The libdxfrw values LibreCAD's behaviour rests on. The assertions themselves
// are static_asserts in persistence/libdxfrw/apipins.cpp, where the library's
// headers are already compiled at C++17 -- they now use inline variables, which
// lcunittest cannot include at the gnu++14 that keeps kaguya building. What
// arrives here is the fingerprint they produce, so an upstream renumbering
// fails this test as well as that build.
//
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(FormatTest, LibdxfrwApiIsUnchanged) {
    EXPECT_EQ(lc::persistence::libdxfrwApiFingerprint(),
              "version:UNKNOWNV=0,AC1009=10,AC1015=13,AC1027=17,AC1032=18;"
              "error:NONE=0,OPEN=2,SECTION=13,CODE=14;"
              "variant:STRING=0,INTEGER=1,DOUBLE=3,COORD=4;"
              "interface:bare-abstract=1,dxfimpl-concrete=1")
        << "libdxfrw's API moved. Check what LibreCAD compares against these "
           "values before updating the string: the revision order drives the "
           "R12 down-convert, and the variant tags guard a union read.";
}
