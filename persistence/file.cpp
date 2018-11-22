#include "file.h"
#include "libdxfrw/dxfimpl.h"
#include "libopencad_interface/libopencad.h"

using namespace lc::persistence;

std::string File::getExtensionForFileType(Type type){
    std::string x;
    if(type >= LIBDXFRW_DXF_R12 && type <= LIBDXFRW_DXB_R2013) {
        x = "dxf"; 
    }
    return x;
}

std::map<std::string, std::string> File::getSupportedFileExtensions(){
    std::map<std::string, std::string> types;
    types.insert(std::pair<std::string, std::string>("dxf" ,"DXF files"));
    types.insert(std::pair<std::string, std::string>("dwg" ,"DWG files"));
    return types;
}

File::Type File::open(lc::storage::Document_SPtr document, const std::string& path, File::Library library) {
    auto builder = std::make_shared<operation::Builder>(document, "Open file");
    File::Type version;

    switch(library) {
        case LIBDXFRW: {
            DXFimpl F(document, builder);
            dxfRW R(path.c_str());
            R.read(&F, true);

            //TODO: create better mapping
            switch(R.getVersion()) {
                case DRW::UNKNOWNV: //TODO: handle this
                    version = Type::LIBDXFRW_DXF_R12; //TODO: not supported ?
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
            }
            break;
        }

        case LIBOPENCAD: {
            lc::persistence::LibOpenCad opencad(document, builder);
            opencad.open(path);
            version = Type::LIBOPENCAD_DWG;
            break;
        }
    }

    builder->execute();
    return version;
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
