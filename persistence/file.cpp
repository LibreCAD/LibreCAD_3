#include "file.h"
#include "format.h"
#include "libdxfrw/dxfimpl.h"
#include <cad/logger/logger.h>
#ifdef LIBOPENCAD_ENABLED
#include "libopencad_interface/libopencad.h"
#endif

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
    std::map<std::string, std::string> types;
    for (const auto& format : formats()) {
        types.insert(std::make_pair(format.id, format.label));
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
#ifdef LIBOPENCAD_ENABLED
        else if (variant.libraryId == "libopencad") {
            libraries.insert(std::make_pair(LIBOPENCAD, variant.libraryId));
        }
#endif
    }

    return libraries;
}

File::Type File::open(lc::storage::Document_SPtr document, const std::string& path, File::Library library) {
    auto builder = std::make_shared<operation::Builder>(document, "Open file");
    // Never leave this indeterminate.  Neither switch below is exhaustive --
    // the inner one has no case for older revisions (AC1002/AC1003/AC1004,
    // AC12/AC14/AC150/AC210, MC00) or newer ones (AC1032), and the outer one
    // has no LIBOPENCAD case unless LIBOPENCAD_ENABLED is defined -- so an
    // unmapped file used to return an uninitialised File::Type, which the
    // caller then uses to choose the save format.  R12 is what the existing
    // UNKNOWNV case already falls back to.
    File::Type version = Type::LIBDXFRW_DXF_R12;

    // The DXF path has to run the queued operations while its reader is still
    // alive, because the INSERTs it held back can only be built against a
    // populated document.  Everything else executes once, at the end.
    bool operationsExecuted = false;

    switch(library) {
    case LIBDXFRW: {
        DXFimpl F(document, builder);
        dxfRW R(path.c_str());
        // Whatever was read before a failure is still loaded, as before.
        if (!R.read(&F, true)) {
            LOG_ERROR << "libdxfrw stopped reading " << path << " (DRW::error " << R.getError() << ")";
        }

        // Every INSERT in the file is built now: the blocks it references and
        // their contents are in the document only after the read's operations
        // have run, and an Insert measures its block when it is constructed.
        builder->execute();
        operationsExecuted = true;
        F.buildDeferredInserts();

        // The revision comes from the drawing's own $ACADVER, captured by
        // DXFimpl::addHeader.  A file that carries none -- or that carries one
        // no revision table knows -- falls back to R12, the only revision
        // every reader accepts.
        const auto& header = F.header();
        bool recognised = false;
        version = typeForAcadVersion(header.acadVersion, &recognised);

        if (header.acadVersion.empty()) {
            LOG_DEBUG << path << " carries no $ACADVER; recording R12";
        } else if (!recognised) {
            LOG_WARNING << "unsupported-version: " << path << " reports $ACADVER "
                        << header.acadVersion << ", which this build does not know; "
                        << "recording R12";
        }

        break;
    }

#ifdef LIBOPENCAD_ENABLED
    case LIBOPENCAD: {
        lc::persistence::LibOpenCad opencad(document, builder);
        opencad.open(path);
        version = Type::LIBOPENCAD_DWG;
        break;
    }
#endif
    default:
        // Includes LIBOPENCAD when built without LIBOPENCAD_ENABLED: nothing
        // was read, so report the fallback rather than an unset value.
        break;
    }

    if (!operationsExecuted) {
        builder->execute();
    }

    return version;
}

bool File::save(lc::storage::Document_SPtr document, const std::string& path, File::Type type) {
    if(!isLibdxfrwType(type)) {
        LOG_ERROR << "No writer for file type " << static_cast<int>(type) << "; " << path << " was not written";
        return false;
    }

    DXFimpl writer(std::move(document));
    return writer.writeDXF(path, type);
}


