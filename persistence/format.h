#pragma once

#include <string>
#include <vector>

namespace lc {
namespace persistence {

/**
 * One concrete thing LibreCAD can read or write: a format, a revision of that
 * format, and an encoding.
 *
 * File::Type conflates those three -- LIBDXFRW_DXB_R2000 is "DXF", "R2000" and
 * "binary" in one integer whose *order* used to decide which types were
 * writable -- and its names are wire values that scripts and saved settings
 * hold, so they cannot be corrected. `id` is the replacement: a stable string
 * naming each axis separately ("dxf.ac1015.ascii"), which the UI, the bindings
 * and the document's own source can hold without anyone having to know what
 * "DXB" was supposed to mean.
 *
 * This header deliberately names no libdxfrw type and includes nothing from
 * it: `persistence` links the library PRIVATE so that its C++17 requirement
 * cannot reach lcluascript and lcpythonscript, which are built as gnu++14 and
 * break on kaguya. It must stay compilable on its own at that standard.
 */
struct FormatVariant {
    std::string id;          //!< stable wire name, e.g. "dxf.ac1015.ascii"
    std::string formatId;    //!< "dxf", "dwg"
    std::string label;       //!< what the Save dialog shows
    std::string versionTag;  //!< "AC1015"; empty when the format has no revisions
    std::string extension;   //!< file extension, without the dot
    std::string libraryId;   //!< which reader/writer handles it, empty when none does
    bool binary;
    bool readable;
    bool writable;
    bool experimental;
};

/** One readable or writable file format. */
struct FormatInfo {
    std::string id;     //!< "dxf"
    std::string label;  //!< "DXF files"
};

/** Every format LibreCAD offers, whether or not a reader is compiled in. */
const std::vector<FormatInfo>& formats();

/** Every variant, ordered oldest revision first, ASCII before binary. */
const std::vector<FormatVariant>& formatVariants();

/** The variant with this id, or nullptr when there is none. */
const FormatVariant* formatVariantById(const std::string& id);

/** Lower-case a format or variant id, for the case-insensitive lookups. */
std::string normalisedFormatId(std::string id);

}  // namespace persistence
}  // namespace lc
