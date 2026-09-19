#include "format.h"

#include <algorithm>

#ifndef USE_DWG_IMPORT
#define USE_DWG_IMPORT 0
#endif

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
        // Two rows for DWG, because two different things are meant by it.
        //
        // "dwg" is the historical one behind File::LIBOPENCAD_DWG: a reader
        // that was optional, unmaintained, off in every shipped build, and is
        // now gone. The row stays because the enumerator is a wire value.
        //
        // "dwg.libdxfrw" is the reader libdxfrw brings. It is readable only
        // when the build asked for it -- WITH_DWG_IMPORT, off by default --
        // and writable never: LibreCAD can read DWG and cannot write it, so a
        // drawing opened from one has to be saved somewhere else, which is
        // what DocumentSource uses `writable` to decide.
        {"dwg",               "dwg", "DWG (not supported)",         "",       "dwg", "",         true,  false, false, false},
        {"dwg.libdxfrw",      "dwg", "DWG (read only)",             "",       "dwg", "libdxfrw", true,  USE_DWG_IMPORT != 0, false, true},
    };

    return variants;
}

}  // namespace

const std::vector<FormatInfo>& formats() {
    // The catalogue of formats LibreCAD knows about, whether or not this build
    // can read them -- every variant belongs to one of these. What the open
    // dialog offers is a different question, answered by
    // File::getSupportedFileExtensions() from the variants that are readable.
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

bool variantCarriesRecord(const std::string& variantId, const std::string& recordKind) {
    const auto* variant = formatVariantById(variantId);
    if (variant == nullptr) {
        return false;
    }

    // Everything from R13 on carries every record LibreCAD can write.
    // AC1006 and AC1009 are the two pre-R13 tags in the table.
    const bool preR13 = variant->versionTag == "AC1006" || variant->versionTag == "AC1009";
    if (!preR13) {
        return true;
    }

    // LWPOLYLINE is absent too, but it is not on this list: DXFimpl writes R12's
    // own POLYLINE for it, which carries the same geometry. A record is only
    // listed here when there is nothing to convert it into.
    return !(recordKind == "SPLINE" || recordKind == "MTEXT"
             || recordKind == "HATCH" || recordKind == "IMAGE");
}

std::string normalisedFormatId(std::string id) {
    std::transform(id.begin(), id.end(), id.begin(), ::tolower);
    return id;
}

}  // namespace persistence
}  // namespace lc
