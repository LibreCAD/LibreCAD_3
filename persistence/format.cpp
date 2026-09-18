#include "format.h"

#include <algorithm>

namespace lc {
namespace persistence {

namespace {

// The labels are the ones LibreCAD has always shown, pinned by FormatTest.
// "DXF <ver> binary" replaced "DXB" because DXB is a different format that
// LibreCAD does not write; the revision and the encoding are separate axes,
// which is the whole point of the id beside each label.
const std::vector<FormatVariant>& table() {
    static const std::vector<FormatVariant> variants = {
        {"dxf.ac1009.ascii",  "dxf", "DXF R12 (libdxfrw)",          "AC1009", "dxf", "libdxfrw", false, true, true, false},
        {"dxf.ac1014.ascii",  "dxf", "DXF R14 (libdxfrw)",          "AC1014", "dxf", "libdxfrw", false, true, true, false},
        {"dxf.ac1015.ascii",  "dxf", "DXF 2000 (libdxfrw)",         "AC1015", "dxf", "libdxfrw", false, true, true, false},
        {"dxf.ac1018.ascii",  "dxf", "DXF 2004 (libdxfrw)",         "AC1018", "dxf", "libdxfrw", false, true, true, false},
        {"dxf.ac1021.ascii",  "dxf", "DXF 2007 (libdxfrw)",         "AC1021", "dxf", "libdxfrw", false, true, true, false},
        {"dxf.ac1024.ascii",  "dxf", "DXF 2010 (libdxfrw)",         "AC1024", "dxf", "libdxfrw", false, true, true, false},
        {"dxf.ac1027.ascii",  "dxf", "DXF 2013 (libdxfrw)",         "AC1027", "dxf", "libdxfrw", false, true, true, false},
        {"dxf.ac1009.binary", "dxf", "DXF R12 binary (libdxfrw)",   "AC1009", "dxf", "libdxfrw", true,  true, true, false},
        {"dxf.ac1014.binary", "dxf", "DXF R14 binary (libdxfrw)",   "AC1014", "dxf", "libdxfrw", true,  true, true, false},
        {"dxf.ac1015.binary", "dxf", "DXF 2000 binary (libdxfrw)",  "AC1015", "dxf", "libdxfrw", true,  true, true, false},
        {"dxf.ac1018.binary", "dxf", "DXF 2004 binary (libdxfrw)",  "AC1018", "dxf", "libdxfrw", true,  true, true, false},
        {"dxf.ac1021.binary", "dxf", "DXF 2007 binary (libdxfrw)",  "AC1021", "dxf", "libdxfrw", true,  true, true, false},
        {"dxf.ac1024.binary", "dxf", "DXF 2010 binary (libdxfrw)",  "AC1024", "dxf", "libdxfrw", true,  true, true, false},
        {"dxf.ac1027.binary", "dxf", "DXF 2013 binary (libdxfrw)",  "AC1027", "dxf", "libdxfrw", true,  true, true, false},
        // DWG is offered in the open dialog and has never had a reader in a
        // default build: libopencad is optional, unmaintained and off. Phase 3
        // turns this one readable; nothing else in the table has to move.
        {"dwg",               "dwg", "DWG (not supported)",         "",       "dwg", "",         true,  false, false, false},
    };

    return variants;
}

}  // namespace

const std::vector<FormatInfo>& formats() {
    // Both are listed whether or not a reader exists, because the open dialog
    // offers both and has always done so; a DWG picked there is refused with a
    // message rather than hidden.
    static const std::vector<FormatInfo> known = {
        {"dxf", "DXF files"},
        {"dwg", "DWG files"},
    };

    return known;
}

const std::vector<FormatVariant>& formatVariants() {
    return table();
}

const FormatVariant* formatVariantById(const std::string& id) {
    const auto& variants = table();
    const auto found = std::find_if(
        variants.begin(), variants.end(),
        [&id](const FormatVariant& variant) { return variant.id == id; });

    return found == variants.end() ? nullptr : &*found;
}

std::string normalisedFormatId(std::string id) {
    std::transform(id.begin(), id.end(), id.begin(), ::tolower);
    return id;
}

}  // namespace persistence
}  // namespace lc
