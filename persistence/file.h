#pragma once

#include <cad/document/document.h>

namespace lc {
    namespace persistence {
        class File {
            public:
                //This enum should be grouped by library, format and ordered by version ascending.
                //TODO: test enum (is libdxfrw, is dxf, is dxb)
                enum Type {
                    LIBDXFRW_DXF_R12,
                    LIBDXFRW_DXF_R14,
                    LIBDXFRW_DXF_R2000,
                    LIBDXFRW_DXF_R2004,
                    LIBDXFRW_DXF_R2007,
                    LIBDXFRW_DXF_R2010,
                    LIBDXFRW_DXF_R2013,
                    LIBDXFRW_DXB_R12,
                    LIBDXFRW_DXB_R14,
                    LIBDXFRW_DXB_R2000,
                    LIBDXFRW_DXB_R2004,
                    LIBDXFRW_DXB_R2007,
                    LIBDXFRW_DXB_R2010,
                    LIBDXFRW_DXB_R2013,
                };

                enum Library {
                    LIBDXFRW,
                    LIBOPENCAD,
                };

                static void open(lc::Document_SPtr document, const std::string& path, Library library);

                static void save(lc::Document_SPtr document, const std::string& path, Type type);

                static std::map<Type, std::string> getAvailableFileTypes();

                static std::map<Library, std::string> getAvailableLibrariesForFormat(std::string format);
        };
    }
}