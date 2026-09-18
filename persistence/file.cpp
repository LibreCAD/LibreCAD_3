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

    switch(library) {
    case LIBDXFRW: {
        DXFimpl F(document, builder);
        dxfRW R(path.c_str());
        // Whatever was read before a failure is still loaded, as before.
        if (!R.read(&F, true)) {
            LOG_ERROR << "libdxfrw stopped reading " << path << " (DRW::error " << R.getError() << ")";
        }

        /// @todo create better mapping
        switch(R.getVersion()) {
        case DRW::UNKNOWNV: /// @todo handle this
            version = Type::LIBDXFRW_DXF_R12; /// @todo not supported ?
            break;
        case DRW::AC1006:
            version = Type::LIBDXFRW_DXF_R12;
            break;
        case DRW::AC1009:
            version = Type::LIBDXFRW_DXB_R12; //This one is correct
            break;
        case DRW::AC1012:
            version = Type::LIBDXFRW_DXF_R12;
            break;
        case DRW::AC1014:
            version = Type::LIBDXFRW_DXB_R14;
            break;
        case DRW::AC1015:
            version = Type::LIBDXFRW_DXF_R2000;
            break;
        case DRW::AC1018:
            version = Type::LIBDXFRW_DXF_R2004;
            break;
        case DRW::AC1021:
            version = Type::LIBDXFRW_DXF_R2007;
            break;
        case DRW::AC1024:
            version = Type::LIBDXFRW_DXF_R2010;
            break;
        case DRW::AC1027:
            version = Type::LIBDXFRW_DXF_R2013;
            break;
        default:
            // A revision libdxfrw recognises but this mapping does not.  Keep
            // the R12 fallback rather than reporting a type we never set.
            break;
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

    builder->execute();
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
