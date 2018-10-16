#include "file.h"
#include "libdxfrw/dxfimpl.h"
#include "libopencad_interface/libopencad.h"

using namespace lc::persistence;

void File::open(lc::storage::Document_SPtr document, const std::string& path, File::Library library) {
    auto builder = std::make_shared<operation::Builder>(document, "Open file");

    switch(library) {
        case LIBDXFRW: {
            DXFimpl F(document, builder);
            dxfRW R(path.c_str());
            R.read(&F, true);
            break;
        }

        case LIBOPENCAD: {
            lc::persistence::LibOpenCad opencad(document, builder);
            opencad.open(path);
            break;
        }
    }

    builder->execute();
}

void File::save(lc::storage::Document_SPtr document, const std::string& path, File::Type type) {
    if(type >= LIBDXFRW_DXF_R12 && type <= LIBDXFRW_DXB_R2013) {
        DXFimpl* F = new DXFimpl(std::move(document));
        F->writeDXF(path, type);
    }
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
        libraries.insert(std::pair<File::Library, std::string>(LIBOPENCAD, "libopencad"));
    }

    return libraries;
}