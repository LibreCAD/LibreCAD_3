#pragma once

#include <cad/storage/document.h>

namespace lc {
namespace persistence {
class File {
public:
    // Values are explicit and must not be reused: scripts and saved settings
    // may hold them. Append only. The DXB_* names are historical -- those
    // targets write *binary DXF*, not AutoCAD DXB; see the labels in
    // getAvailableFileTypes().
    enum Type {
        LIBDXFRW_DXF_R12 = 0,
        LIBDXFRW_DXF_R14 = 1,
        LIBDXFRW_DXF_R2000 = 2,
        LIBDXFRW_DXF_R2004 = 3,
        LIBDXFRW_DXF_R2007 = 4,
        LIBDXFRW_DXF_R2010 = 5,
        LIBDXFRW_DXF_R2013 = 6,
        LIBDXFRW_DXB_R12 = 7,
        LIBDXFRW_DXB_R14 = 8,
        LIBDXFRW_DXB_R2000 = 9,
        LIBDXFRW_DXB_R2004 = 10,
        LIBDXFRW_DXB_R2007 = 11,
        LIBDXFRW_DXB_R2010 = 12,
        LIBDXFRW_DXB_R2013 = 13,
        LIBOPENCAD_DWG = 14
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

    /**
     * True when the type is written through libdxfrw.
     * Replaces the range tests that made enumerator ORDER load-bearing:
     * inserting a value used to change which types were writable.
     */
    static bool isLibdxfrwType(Type type);

    /**
     * True when the type writes binary DXF rather than ASCII.
     */
    static bool isBinaryType(Type type);

    static std::map<Type, std::string> getAvailableFileTypes();

    static std::map<Library, std::string> getAvailableLibrariesForFormat(std::string format);

    static std::string getExtensionForFileType(Type);

    static std::map<std::string, std::string> getSupportedFileExtensions();
};
}
}
