#include "file.h"
#include "libdxfrw/dxfimpl.h"
#include <algorithm>
#include <fstream>
#ifdef LIBOPENCAD_ENABLED
#include "libopencad_interface/libopencad.h"
#endif

using namespace lc::persistence;

namespace {

bool isBinaryDxfFile(const std::string& path) {
    static constexpr char binaryDxfSentinel[] = "AutoCAD Binary DXF\r\n\x1a";
    std::ifstream file(path, std::ios::binary);
    char header[sizeof(binaryDxfSentinel)] = {};
    return file.read(header, sizeof(header))
           && std::equal(header, header + sizeof(header), binaryDxfSentinel);
}

File::Type binaryVariantFor(File::Type type) {
    switch (type) {
    case File::LIBDXFRW_DXF_R12: return File::LIBDXFRW_DXB_R12;
    case File::LIBDXFRW_DXF_R14: return File::LIBDXFRW_DXB_R14;
    case File::LIBDXFRW_DXF_R2000: return File::LIBDXFRW_DXB_R2000;
    case File::LIBDXFRW_DXF_R2004: return File::LIBDXFRW_DXB_R2004;
    case File::LIBDXFRW_DXF_R2007: return File::LIBDXFRW_DXB_R2007;
    case File::LIBDXFRW_DXF_R2010: return File::LIBDXFRW_DXB_R2010;
    case File::LIBDXFRW_DXF_R2013: return File::LIBDXFRW_DXB_R2013;
    default: return type;
    }
}

File::Type typeForAcadVersion(const std::string& acadVersion, bool* recognised) {
    static const std::map<std::string, File::Type> byAcadVersion = {
        {"MC0.0", File::LIBDXFRW_DXF_R12},
        {"AC1.2", File::LIBDXFRW_DXF_R12},
        {"AC1.40", File::LIBDXFRW_DXF_R12},
        {"AC1.50", File::LIBDXFRW_DXF_R12},
        {"AC2.10", File::LIBDXFRW_DXF_R12},
        {"AC2.21", File::LIBDXFRW_DXF_R12},
        {"AC2.22", File::LIBDXFRW_DXF_R12},
        {"AC1001", File::LIBDXFRW_DXF_R12},
        {"AC1002", File::LIBDXFRW_DXF_R12},
        {"AC1003", File::LIBDXFRW_DXF_R12},
        {"AC1004", File::LIBDXFRW_DXF_R12},
        {"AC1006", File::LIBDXFRW_DXF_R12},
        {"AC1009", File::LIBDXFRW_DXF_R12},
        {"AC1012", File::LIBDXFRW_DXF_R14},
        {"AC1014", File::LIBDXFRW_DXF_R14},
        {"AC1015", File::LIBDXFRW_DXF_R2000},
        {"AC1018", File::LIBDXFRW_DXF_R2004},
        {"AC1021", File::LIBDXFRW_DXF_R2007},
        {"AC1024", File::LIBDXFRW_DXF_R2010},
        {"AC1027", File::LIBDXFRW_DXF_R2013},
        {"AC1032", File::LIBDXFRW_DXF_R2013},
    };

    const auto it = byAcadVersion.find(acadVersion);
    const bool known = it != byAcadVersion.end();
    if (recognised != nullptr) {
        *recognised = known;
    }
    return known ? it->second : File::LIBDXFRW_DXF_R12;
}

} // namespace

std::string File::getExtensionForFileType(Type type) {
    std::string x;
    if(type >= LIBDXFRW_DXF_R12 && type <= LIBDXFRW_DXB_R2013) {
        x = "dxf";
    }
    return x;
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
        R.read(&F, true);

        bool mappedVersion = false;
        if (!F.acadVersion().empty()) {
            version = typeForAcadVersion(F.acadVersion(), &mappedVersion);
        }
        else switch(R.getVersion()) {
        case DRW::UNKNOWNV: /// @todo handle this
            version = Type::LIBDXFRW_DXF_R12; /// @todo not supported ?
            mappedVersion = false;
            break;
        case DRW::AC1006:
            version = Type::LIBDXFRW_DXF_R12;
            break;
        case DRW::AC1009:
            version = Type::LIBDXFRW_DXF_R12;
            break;
        case DRW::AC1012:
            version = Type::LIBDXFRW_DXF_R12;
            break;
        case DRW::AC1014:
            version = Type::LIBDXFRW_DXF_R14;
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
            mappedVersion = false;
            break;
        }
        if (mappedVersion && isBinaryDxfFile(path)) {
            version = binaryVariantFor(version);
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
    if(type >= LIBDXFRW_DXF_R12 && type <= LIBDXFRW_DXB_R2013) {
        DXFimpl writer(std::move(document));
        return writer.writeDXF(path, type);
    }
    return false;
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
    types.insert(std::pair<File::Type, std::string>(LIBDXFRW_DXB_R2013, "DXB 2013 (libdxfrw)"));
    types.insert(std::pair<File::Type, std::string>(LIBDXFRW_DXB_R2010, "DXB 2010 (libdxfrw)"));
    types.insert(std::pair<File::Type, std::string>(LIBDXFRW_DXB_R2007, "DXB 2007 (libdxfrw)"));
    types.insert(std::pair<File::Type, std::string>(LIBDXFRW_DXB_R2004, "DXB 2004 (libdxfrw)"));
    types.insert(std::pair<File::Type, std::string>(LIBDXFRW_DXB_R2000, "DXB 2000 (libdxfrw)"));
    types.insert(std::pair<File::Type, std::string>(LIBDXFRW_DXB_R14, "DXB R14 (libdxfrw)"));
    types.insert(std::pair<File::Type, std::string>(LIBDXFRW_DXB_R12, "DXB R12 (libdxfrw)"));

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
