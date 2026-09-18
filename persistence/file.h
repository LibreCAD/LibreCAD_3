#pragma once

#include <cad/storage/document.h>

#include "format.h"

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

    // Wire values, like Type: kept although libopencad is gone. It was an
    // optional, unmaintained, off-by-default DWG reader; the enumerator stays
    // so a stored or scripted value keeps meaning what it meant, and asking for
    // it now reports that this build has no reader rather than silently
    // producing an empty document.
    enum Library {
        LIBDXFRW,
        LIBOPENCAD,
    };

    /**
     * Read a file into a document and report what happened.
     *
     * Whatever arrived before a failure is loaded, as it always has been, and
     * the result says so: ok=false with partial=true and a non-zero
     * entitiesDelivered is a drawing the user can see but must not silently
     * save back over its original.
     */
    static ImportResult importFile(lc::storage::Document_SPtr document,
                                   const std::string& path, Library library);

    /**
     * Write the document and report what happened, including what the target
     * revision could not carry.
     */
    static ExportResult exportFile(lc::storage::Document_SPtr document,
                                   const std::string& path, Type type);

    /**
     * The revision recorded for the file. A shim over importFile, kept because
     * every existing caller -- lcUI, the bindings, PatternProvider -- uses it.
     */
    static Type open(lc::storage::Document_SPtr document, const std::string& path, Library library);

    /**
     * Write the document to path in the given format.
     *
     * @return true only when the *whole* drawing was written. False means
     *         either that nothing was written -- no writer for the format, or
     *         the library refused -- or that the file exists without records
     *         this revision cannot carry. exportFile distinguishes the two and
     *         names what was left out.
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

    /**
     * Map a drawing's $ACADVER string ("AC1015") to the variant LibreCAD
     * records for it, which is also the format Save will reuse.
     *
     * Not every revision has a variant of its own: R13 (AC1012) and anything
     * newer than R2013 are clamped to the nearest one libdxfrw can write.
     * Clamping up is safe -- it only widens what a later save may emit -- so
     * an unwritable revision is rounded up, and a revision newer than
     * everything we can write is rounded down to the newest we have.
     *
     * An empty or unrecognised string yields the R12 fallback, which is the
     * only revision every reader accepts, and sets *recognised to false.
     */
    static Type typeForAcadVersion(const std::string& acadVersion,
                                   bool* recognised = nullptr);

    /**
     * The stable variant id for a Type ("dxf.ac1015.ascii"), or "" for a Type
     * with no variant. Type is a wire value whose name cannot be corrected;
     * this is what everything above persistence should be holding instead.
     */
    static std::string variantIdForType(Type type);

    /**
     * The Type for a variant id. Returns false and leaves `type` alone when
     * the id is not one this build knows.
     */
    static bool typeForVariantId(const std::string& id, Type& type);

    static std::map<Type, std::string> getAvailableFileTypes();

    static std::map<Library, std::string> getAvailableLibrariesForFormat(std::string format);

    static std::string getExtensionForFileType(Type);

    static std::map<std::string, std::string> getSupportedFileExtensions();
};
}
}
