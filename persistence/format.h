#pragma once

#include <cstddef>
#include <map>
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

/** How much a diagnostic matters. */
enum class Severity {
    Info,
    Warning,
    Error,
};

/**
 * Something worth telling the user about a read or a write.
 *
 * `code` is a short stable token meant to be matched on -- "unsupported-version",
 * "emit-failure" -- and is copied from libdxfrw's own diagnostic where one
 * exists, so the string is stable across the library's own refactors even
 * though its type never crosses this header.
 */
struct Diagnostic {
    Severity severity;
    std::string code;
    std::string message;
};

/**
 * What a read or a write could not carry.
 *
 * LibreCAD has no entity for several things DXF can hold -- RAY, XLINE, SOLID,
 * 3DFACE, TRACE, LEADER -- and its reader has always dropped them in silence.
 * Counting them is what turns "the drawing looks wrong" into a list.
 */
struct LossSummary {
    std::map<std::string, std::size_t> droppedByType;
    std::vector<std::string> notes;

    bool empty() const {
        return droppedByType.empty() && notes.empty();
    }

    std::size_t total() const {
        std::size_t sum = 0;
        for (const auto& dropped : droppedByType) {
            sum += dropped.second;
        }
        return sum;
    }
};

/**
 * The outcome of reading a file.
 *
 * `ok` and `partial` are separate because a failed read is not an empty one:
 * libdxfrw stops where it fails and LibreCAD keeps what arrived before that
 * point, which is a drawing the user can see and must not be allowed to
 * silently save back over the original. That case is ok=false, partial=true,
 * entitiesDelivered > 0.
 */
struct ImportResult {
    bool ok{false};
    bool partial{false};
    std::string variantId;         //!< the variant recorded for the file
    std::string sourceVersionTag;  //!< $ACADVER as the file states it, may be empty
    std::size_t entitiesDelivered{0};
    std::vector<Diagnostic> diagnostics;
    LossSummary loss;
};

/** The outcome of writing a file. Nothing is written when ok is false. */
struct ExportResult {
    bool ok{false};
    std::string variantId;
    std::vector<Diagnostic> diagnostics;
    LossSummary loss;
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
