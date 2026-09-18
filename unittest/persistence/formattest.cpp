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
#include <string>

#include <gtest/gtest.h>

#include "persistence/file.h"

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
