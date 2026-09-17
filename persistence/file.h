#pragma once

#include <cad/storage/document.h>

namespace lc {
namespace persistence {
class File {
public:
    //This enum should be grouped by library, format and ordered by version ascending.
    /// @todo test enum (is libdxfrw, is dxf, is dxb)
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
        LIBOPENCAD_DWG
    };

    enum Library {
        LIBDXFRW,
        LIBOPENCAD,
    };

    static Type open(lc::storage::Document_SPtr document, const std::string& path, Library library);

    /**
     * Write the document to path in the given format.
     * @return false if the format is not writable or the library refused the write;
     *         nothing is written in that case.
     */
    static bool save(lc::storage::Document_SPtr document, const std::string& path, Type type);

    static std::map<Type, std::string> getAvailableFileTypes();

    static std::map<Library, std::string> getAvailableLibrariesForFormat(std::string format);

    static std::string getExtensionForFileType(Type);

    static std::map<std::string, std::string> getSupportedFileExtensions();
};
}
}
