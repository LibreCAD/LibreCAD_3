#include "file.h"
#include "libdxfrw/dxfimpl.h"
#include <cad/logger/logger.h>
#ifdef LIBOPENCAD_ENABLED
#include "libopencad_interface/libopencad.h"
#endif

using namespace lc::persistence;

bool File::isLibdxfrwType(Type type) {
    switch(type) {
    case LIBDXFRW_DXF_R12:
    case LIBDXFRW_DXF_R14:
    case LIBDXFRW_DXF_R2000:
    case LIBDXFRW_DXF_R2004:
    case LIBDXFRW_DXF_R2007:
    case LIBDXFRW_DXF_R2010:
    case LIBDXFRW_DXF_R2013:
    case LIBDXFRW_DXB_R12:
    case LIBDXFRW_DXB_R14:
    case LIBDXFRW_DXB_R2000:
    case LIBDXFRW_DXB_R2004:
    case LIBDXFRW_DXB_R2007:
    case LIBDXFRW_DXB_R2010:
    case LIBDXFRW_DXB_R2013:
        return true;
    case LIBOPENCAD_DWG:
        return false;
    }

    return false;
}

bool File::isBinaryType(Type type) {
    switch(type) {
    case LIBDXFRW_DXB_R12:
    case LIBDXFRW_DXB_R14:
    case LIBDXFRW_DXB_R2000:
    case LIBDXFRW_DXB_R2004:
    case LIBDXFRW_DXB_R2007:
    case LIBDXFRW_DXB_R2010:
    case LIBDXFRW_DXB_R2013:
        return true;
    default:
        return false;
    }
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
    // Every writable type is DXF; only the encoding differs.
    return isLibdxfrwType(type) ? "dxf" : "";
}

std::map<std::string, std::string> File::getSupportedFileExtensions() {
    std::map<std::string, std::string> types;
    types.insert(std::pair<std::string, std::string>("dxf","DXF files"));
    types.insert(std::pair<std::string, std::string>("dwg","DWG files"));
    return types;
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

std::map<File::Type, std::string> File::getAvailableFileTypes() {
    std::map<File::Type, std::string> types;

    types.insert(std::pair<File::Type, std::string>(LIBDXFRW_DXF_R2013, "DXF 2013 (libdxfrw)"));
    types.insert(std::pair<File::Type, std::string>(LIBDXFRW_DXF_R2010, "DXF 2010 (libdxfrw)"));
    types.insert(std::pair<File::Type, std::string>(LIBDXFRW_DXF_R2007, "DXF 2007 (libdxfrw)"));
    types.insert(std::pair<File::Type, std::string>(LIBDXFRW_DXF_R2004, "DXF 2004 (libdxfrw)"));
    types.insert(std::pair<File::Type, std::string>(LIBDXFRW_DXF_R2000, "DXF 2000 (libdxfrw)"));
    types.insert(std::pair<File::Type, std::string>(LIBDXFRW_DXF_R14, "DXF R14 (libdxfrw)"));
    types.insert(std::pair<File::Type, std::string>(LIBDXFRW_DXF_R12, "DXF R12 (libdxfrw)"));
    types.insert(std::pair<File::Type, std::string>(LIBDXFRW_DXB_R2013, "DXF 2013 binary (libdxfrw)"));
    types.insert(std::pair<File::Type, std::string>(LIBDXFRW_DXB_R2010, "DXF 2010 binary (libdxfrw)"));
    types.insert(std::pair<File::Type, std::string>(LIBDXFRW_DXB_R2007, "DXF 2007 binary (libdxfrw)"));
    types.insert(std::pair<File::Type, std::string>(LIBDXFRW_DXB_R2004, "DXF 2004 binary (libdxfrw)"));
    types.insert(std::pair<File::Type, std::string>(LIBDXFRW_DXB_R2000, "DXF 2000 binary (libdxfrw)"));
    types.insert(std::pair<File::Type, std::string>(LIBDXFRW_DXB_R14, "DXF R14 binary (libdxfrw)"));
    types.insert(std::pair<File::Type, std::string>(LIBDXFRW_DXB_R12, "DXF R12 binary (libdxfrw)"));

    return types;
}

std::map<File::Library, std::string> File::getAvailableLibrariesForFormat(std::string format) {
    std::transform(format.begin(), format.end(), format.begin(), ::tolower);

    std::map<File::Library, std::string> libraries;

    if(format == "dxf") {
        libraries.insert(std::pair<File::Library, std::string>(LIBDXFRW, "libdxfrw"));
    }
    if(format == "dwg") {
#ifdef LIBOPENCAD_ENABLED
        libraries.insert(std::pair<File::Library, std::string>(LIBOPENCAD, "libopencad"));
#endif
    }

    return libraries;
}
