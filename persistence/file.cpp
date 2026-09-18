#include "file.h"

#include <algorithm>
#include <fstream>

#ifndef USE_DWG_IMPORT
#define USE_DWG_IMPORT 0
#endif
#include "format.h"
#include "libdxfrw/dxfimpl.h"

#if USE_DWG_IMPORT
#include <libdwgr.h>
#endif
#include <cad/logger/logger.h>

using namespace lc::persistence;

namespace {

/**
 * Which format variant each File::Type names.
 *
 * File::Type is a wire value -- scripts and saved settings hold the integers --
 * so it stays, and this is the only place that has to know what each one meant.
 * Everything else about a target (its label, extension, encoding and which
 * library writes it) lives once, in persistence/format.cpp.
 */
struct TypeVariant {
    File::Type type;
    const char* variantId;
};

const TypeVariant kTypeVariants[] = {
    {File::LIBDXFRW_DXF_R12, "dxf.ac1009.ascii"},
    {File::LIBDXFRW_DXF_R14, "dxf.ac1014.ascii"},
    {File::LIBDXFRW_DXF_R2000, "dxf.ac1015.ascii"},
    {File::LIBDXFRW_DXF_R2004, "dxf.ac1018.ascii"},
    {File::LIBDXFRW_DXF_R2007, "dxf.ac1021.ascii"},
    {File::LIBDXFRW_DXF_R2010, "dxf.ac1024.ascii"},
    {File::LIBDXFRW_DXF_R2013, "dxf.ac1027.ascii"},
    {File::LIBDXFRW_DXB_R12, "dxf.ac1009.binary"},
    {File::LIBDXFRW_DXB_R14, "dxf.ac1014.binary"},
    {File::LIBDXFRW_DXB_R2000, "dxf.ac1015.binary"},
    {File::LIBDXFRW_DXB_R2004, "dxf.ac1018.binary"},
    {File::LIBDXFRW_DXB_R2007, "dxf.ac1021.binary"},
    {File::LIBDXFRW_DXB_R2010, "dxf.ac1024.binary"},
    {File::LIBDXFRW_DXB_R2013, "dxf.ac1027.binary"},
    {File::LIBOPENCAD_DWG, "dwg"},
    {File::LIBDXFRW_DWG_IMPORT, "dwg.libdxfrw"},
};

const FormatVariant* variantFor(File::Type type) {
    for (const auto& pair : kTypeVariants) {
        if (pair.type == type) {
            return formatVariantById(pair.variantId);
        }
    }

    return nullptr;
}

}  // namespace

std::string File::variantIdForType(Type type) {
    for (const auto& pair : kTypeVariants) {
        if (pair.type == type) {
            return pair.variantId;
        }
    }

    return "";
}

bool File::typeForVariantId(const std::string& id, Type& type) {
    for (const auto& pair : kTypeVariants) {
        if (id == pair.variantId) {
            type = pair.type;
            return true;
        }
    }

    return false;
}

bool File::isLibdxfrwType(Type type) {
    const auto* variant = variantFor(type);
    return variant != nullptr && variant->libraryId == "libdxfrw";
}

bool File::isBinaryType(Type type) {
    const auto* variant = variantFor(type);
    return variant != nullptr && variant->binary && variant->writable;
}

std::string File::sniffFormat(const std::string& path) {
    // DWG opens with its version string: "AC1015", "AC1032", and so on. A DXF
    // opens either with a group code -- optional whitespace, then "0" -- or,
    // when binary, with the sentinel "AutoCAD Binary DXF", which also starts
    // with 'A'. So the test is the whole six-byte tag, not its first letter.
    std::ifstream file(path, std::ios::binary);
    char header[6] = {0, 0, 0, 0, 0, 0};
    if (!file.read(header, sizeof(header))) {
        return "dxf";
    }

    const std::string tag(header, sizeof(header));
    const bool looksLikeDwg = tag.compare(0, 2, "AC") == 0
                              && std::all_of(tag.begin() + 2, tag.end(),
                                             [](char c) { return c >= '0' && c <= '9'; });

    return looksLikeDwg ? "dwg" : "dxf";
}

File::Type File::typeForAcadVersion(const std::string& acadVersion, bool* recognised) {
    // $ACADVER is the drawing's own statement of its revision, and the only
    // one available: dxfRW::getVersion() reports the *text codec* bucket
    // (intern/drw_textcodec.cpp), which collapses R2010 and R2013 onto R2007
    // and, for a file carrying no header at all, returns its constructor's
    // default of AC1021 -- so a plain R12 body used to be recorded as R2007.
    static const std::map<std::string, Type> byAcadVersion = {
        // Everything through R12 shares the one variant we can write for it.
        {"MC0.0", LIBDXFRW_DXF_R12},
        {"AC1.2", LIBDXFRW_DXF_R12},
        {"AC1.40", LIBDXFRW_DXF_R12},
        {"AC1.50", LIBDXFRW_DXF_R12},
        {"AC2.10", LIBDXFRW_DXF_R12},
        {"AC2.21", LIBDXFRW_DXF_R12},
        {"AC2.22", LIBDXFRW_DXF_R12},
        {"AC1001", LIBDXFRW_DXF_R12},
        {"AC1002", LIBDXFRW_DXF_R12},
        {"AC1003", LIBDXFRW_DXF_R12},
        {"AC1004", LIBDXFRW_DXF_R12},
        {"AC1006", LIBDXFRW_DXF_R12},
        {"AC1009", LIBDXFRW_DXF_R12},  // R11/R12
        {"AC1012", LIBDXFRW_DXF_R14},  // R13: no variant of its own, round up
        {"AC1014", LIBDXFRW_DXF_R14},
        {"AC1015", LIBDXFRW_DXF_R2000},
        {"AC1018", LIBDXFRW_DXF_R2004},
        {"AC1021", LIBDXFRW_DXF_R2007},
        {"AC1024", LIBDXFRW_DXF_R2010},
        {"AC1027", LIBDXFRW_DXF_R2013},
        {"AC1032", LIBDXFRW_DXF_R2013},  // R2018: newest we can write
    };

    const auto it = byAcadVersion.find(acadVersion);
    const bool known = it != byAcadVersion.end();
    if (recognised != nullptr) {
        *recognised = known;
    }
    return known ? it->second : LIBDXFRW_DXF_R12;
}

std::string File::getExtensionForFileType(Type type) {
    // The extension of a *save target*: a type with no writer has none to
    // offer, which is what every caller of this does with the answer.
    const auto* variant = variantFor(type);
    return variant != nullptr && variant->writable ? variant->extension : "";
}

std::map<std::string, std::string> File::getSupportedFileExtensions() {
    // What the open dialog offers: a format is listed when this build has a
    // reader for at least one of its variants.
    //
    // DWG used to be listed unconditionally, so the dialog offered a format no
    // shipped build could read and the user got an error for choosing it.
    std::map<std::string, std::string> types;
    for (const auto& format : formats()) {
        const auto& variants = formatVariants();
        const bool readable = std::any_of(
            variants.begin(), variants.end(), [&format](const FormatVariant& variant) {
                return variant.formatId == format.id && variant.readable;
            });

        if (readable) {
            types.insert(std::make_pair(format.id, format.label));
        }
    }

    return types;
}

std::map<File::Type, std::string> File::getAvailableFileTypes() {
    std::map<File::Type, std::string> types;
    for (const auto& pair : kTypeVariants) {
        const auto* variant = formatVariantById(pair.variantId);
        if (variant != nullptr && variant->writable) {
            types.insert(std::make_pair(pair.type, variant->label));
        }
    }

    return types;
}

std::map<File::Library, std::string> File::getAvailableLibrariesForFormat(std::string format) {
    format = normalisedFormatId(std::move(format));

    std::map<File::Library, std::string> libraries;
    for (const auto& variant : formatVariants()) {
        if (variant.formatId != format || !variant.readable) {
            continue;
        }

        if (variant.libraryId == "libdxfrw") {
            libraries.insert(std::make_pair(LIBDXFRW, variant.libraryId));
        }
    }

    return libraries;
}

ImportResult File::importFile(lc::storage::Document_SPtr document,
                              const std::string& path, File::Library library) {
    ImportResult result;
    // Never leave the recorded variant indeterminate: R12 is the fallback the
    // absent-$ACADVER case documents, and it is what a caller that ignores
    // `ok` would otherwise read uninitialised.
    result.variantId = variantIdForType(Type::LIBDXFRW_DXF_R12);

    auto builder = std::make_shared<operation::Builder>(document, "Open file");

    // The reader follows the file's contents, not its name: a DXF saved as .dwg
    // and a DWG saved as .dxf are both ordinary occurrences, and choosing by
    // extension makes either unopenable.
    if (library == LIBDXFRW && sniffFormat(path) == "dwg") {
#if USE_DWG_IMPORT
        DXFimpl reader(document, builder);
        dwgRW R(path.c_str());

        result.ok = R.read(&reader, true);
        if (!result.ok) {
            LOG_ERROR << "libdxfrw stopped reading " << path
                      << " as DWG (DRW::error " << R.getError() << ")";
            result.diagnostics.push_back(Diagnostic{
                Severity::Error, "dwg-read-failure", "This DWG file could not be read"});
        }

        builder->execute();
        reader.buildDeferredInserts();

        result.entitiesDelivered = reader.entitiesDelivered();
        result.failures = reader.failures();
        result.loss = reader.loss();
        result.partial = !result.ok && result.entitiesDelivered > 0;
        result.variantId = variantIdForType(Type::LIBDXFRW_DWG_IMPORT);
        result.sourceVersionTag = reader.header().acadVersion;

        // Nothing preserves a DWG's unmodelled records: the raw net is a DXF
        // passthrough, and LibreCAD cannot write DWG at all, so there is
        // nowhere to put them back.
        return result;
#else
        LOG_ERROR << path << " is a DWG file; this build was not configured with DWG import";
        result.variantId = variantIdForType(Type::LIBDXFRW_DWG_IMPORT);
        result.diagnostics.push_back(Diagnostic{
            Severity::Error, "dwg-not-enabled",
            "This build cannot read DWG files"});
        return result;
#endif
    }

    switch(library) {
    case LIBDXFRW: {
        DXFimpl reader(document, builder);
        dxfRW R(path.c_str());

        // The DXF compatibility profile is left at its default, StandaloneSafe.
        //
        // libdxfrw offers LibreCadMasterLegacy for adapters migrating from
        // LibreCAD's own bundled fork, which used different binary widths for
        // the disputed group-code ranges 260-269 and 482-998. Measured across
        // the 43-file review corpus -- 19 of them binary DXF -- the two
        // profiles agree on every file: same result, same error code, same
        // entity count, same losses. There is nothing to gain by opting out of
        // the fail-closed default, and this note exists so the next person does
        // not have to re-run the comparison to find that out.

        // Whatever was read before a failure is still loaded, as before.
        result.ok = R.read(&reader, true);
        if (!result.ok) {
            const auto diagnostic = R.getLastDiagnostic();
            LOG_ERROR << "libdxfrw stopped reading " << path << " (DRW::error " << R.getError()
                      << ", " << diagnostic.code << ": " << diagnostic.message << ")";
            result.diagnostics.push_back(Diagnostic{
                Severity::Error,
                diagnostic.code.empty() ? std::string("read-failure") : diagnostic.code,
                diagnostic.message.empty()
                    ? "libdxfrw stopped reading the file" : diagnostic.message});
        }

        // Every INSERT in the file is built now: the blocks it references and
        // their contents are in the document only after the read's operations
        // have run, and an Insert measures its block when it is constructed.
        builder->execute();
        reader.buildDeferredInserts();

        // Whatever this build does not model rides on the document, so a later
        // save can put it back exactly as it arrived.
        reader.attachPreservedRecords();

        const auto& header = reader.header();
        result.sourceVersionTag = header.acadVersion;
        result.entitiesDelivered = reader.entitiesDelivered();
        result.failures = reader.failures();
        result.loss = reader.loss();

        if (!result.failures.empty()) {
            // Distinct from LossSummary: these are records LibreCAD has an
            // entity for and still could not build. Each one is already logged
            // with its handle and layer.
            result.diagnostics.push_back(Diagnostic{
                Severity::Warning, "entity-import-failed",
                std::to_string(result.failures.size())
                    + " entities could not be imported"});
        }
        result.partial = !result.ok && result.entitiesDelivered > 0;

        // The revision comes from the drawing's own $ACADVER, captured by
        // DXFimpl::addHeader.  A file that carries none -- or that carries one
        // no revision table knows -- falls back to R12, the only revision
        // every reader accepts.
        bool recognised = false;
        result.variantId = variantIdForType(typeForAcadVersion(header.acadVersion, &recognised));

        if (header.acadVersion.empty()) {
            LOG_DEBUG << path << " carries no $ACADVER; recording R12";
        } else if (!recognised) {
            LOG_WARNING << "unsupported-version: " << path << " reports $ACADVER "
                        << header.acadVersion << ", which this build does not know; "
                        << "recording R12";
            result.diagnostics.push_back(Diagnostic{
                Severity::Warning, "unsupported-version",
                "This build does not know DXF revision " + header.acadVersion});
        }

        for (const auto& dropped : result.loss.droppedByType) {
            LOG_WARNING << "Dropped " << dropped.second << " " << dropped.first
                        << " record(s): LibreCAD has no entity for them";
        }

        return result;
    }

    default:
        // LIBOPENCAD names a reader this build does not have: nothing was
        // read, and saying so beats reporting a successful empty import.
        LOG_ERROR << "No reader for " << path;
        result.diagnostics.push_back(Diagnostic{
            Severity::Error, "no-reader", "This build has no reader for that format"});
        break;
    }

    builder->execute();

    return result;
}

File::Type File::open(lc::storage::Document_SPtr document, const std::string& path, File::Library library) {
    const auto result = importFile(std::move(document), path, library);

    Type type = Type::LIBDXFRW_DXF_R12;
    typeForVariantId(result.variantId, type);

    return type;
}

ExportResult File::exportFile(lc::storage::Document_SPtr document,
                              const std::string& path, File::Type type) {
    ExportResult result;
    result.variantId = variantIdForType(type);

    if(!isLibdxfrwType(type)) {
        LOG_ERROR << "No writer for file type " << static_cast<int>(type) << "; " << path << " was not written";
        result.diagnostics.push_back(Diagnostic{
            Severity::Error, "no-writer", "This build has no writer for that format"});
        return result;
    }

    DXFimpl writer(std::move(document));
    result.ok = writer.writeDXF(path, type);
    result.loss = writer.loss();

    if (!result.ok) {
        result.diagnostics.push_back(Diagnostic{
            Severity::Error, "emit-failure", "The file could not be written at this revision"});
    } else if (!result.loss.empty()) {
        // The file exists and holds the rest of the drawing. Saying so is the
        // point: before this, one such entity meant no file at all.
        std::string kinds;
        for (const auto& dropped : result.loss.droppedByType) {
            if (!kinds.empty()) {
                kinds += ", ";
            }
            kinds += std::to_string(dropped.second) + " " + dropped.first;
        }

        LOG_WARNING << path << " was written without " << kinds
                    << ": this revision cannot carry them. Save as R2000 or newer to keep them.";
        result.diagnostics.push_back(Diagnostic{
            Severity::Warning, "record-not-in-revision",
            "Written without " + kinds + ": this revision cannot carry them"});
    }

    return result;
}

bool File::save(lc::storage::Document_SPtr document, const std::string& path, File::Type type) {
    const auto result = exportFile(std::move(document), path, type);

    // True means "the whole drawing was written". A save that had to leave
    // records behind is not that, and a caller holding only a bool must not be
    // told otherwise -- exportFile says which records, and how many.
    return result.ok && result.loss.empty();
}


