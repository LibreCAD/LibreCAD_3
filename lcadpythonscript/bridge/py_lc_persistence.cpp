// py_lc_persistence.cpp — Python port of the persistence gateway.
// Phase 2 slice 2.6.
//
// The C++ class `lc::persistence::File` is a namespace of static functions
// (open/save + 4 introspection helpers), not an instantiable class.  We
// bind it as a py::class_ with no ctors so Python users spell it
// `lc.persistence.File.save(doc, path, T)` — 1:1 with the C++ call.

#include "py_lc_persistence.h"

#include <pybind11/stl.h>

#include <persistence/file.h>

namespace py = pybind11;

namespace lc {
namespace python {

void import_py_lc_persistence_namespace(py::module_& m_persistence) {
    py::class_<lc::persistence::File> file(m_persistence, "File");

    // Enum: lc.persistence.File.Type — every DXF revision + DXB flavor plus
    // LIBOPENCAD_DWG.  Values match the C++ enum exactly.
    py::enum_<lc::persistence::File::Type>(file, "Type")
        .value("LIBDXFRW_DXF_R12",    lc::persistence::File::LIBDXFRW_DXF_R12)
        .value("LIBDXFRW_DXF_R14",    lc::persistence::File::LIBDXFRW_DXF_R14)
        .value("LIBDXFRW_DXF_R2000",  lc::persistence::File::LIBDXFRW_DXF_R2000)
        .value("LIBDXFRW_DXF_R2004",  lc::persistence::File::LIBDXFRW_DXF_R2004)
        .value("LIBDXFRW_DXF_R2007",  lc::persistence::File::LIBDXFRW_DXF_R2007)
        .value("LIBDXFRW_DXF_R2010",  lc::persistence::File::LIBDXFRW_DXF_R2010)
        .value("LIBDXFRW_DXF_R2013",  lc::persistence::File::LIBDXFRW_DXF_R2013)
        .value("LIBDXFRW_DXB_R12",    lc::persistence::File::LIBDXFRW_DXB_R12)
        .value("LIBDXFRW_DXB_R14",    lc::persistence::File::LIBDXFRW_DXB_R14)
        .value("LIBDXFRW_DXB_R2000",  lc::persistence::File::LIBDXFRW_DXB_R2000)
        .value("LIBDXFRW_DXB_R2004",  lc::persistence::File::LIBDXFRW_DXB_R2004)
        .value("LIBDXFRW_DXB_R2007",  lc::persistence::File::LIBDXFRW_DXB_R2007)
        .value("LIBDXFRW_DXB_R2010",  lc::persistence::File::LIBDXFRW_DXB_R2010)
        .value("LIBDXFRW_DXB_R2013",  lc::persistence::File::LIBDXFRW_DXB_R2013)
        .value("LIBOPENCAD_DWG",      lc::persistence::File::LIBOPENCAD_DWG)
        .export_values();

    py::enum_<lc::persistence::File::Library>(file, "Library")
        .value("LIBDXFRW",   lc::persistence::File::LIBDXFRW)
        .value("LIBOPENCAD", lc::persistence::File::LIBOPENCAD)
        .export_values();

    // Static methods.  Preserve exact kernel names and arg orders for
    // parity with the Lua binding (added in the same slice).
    file.def_static("open",
                    &lc::persistence::File::open,
                    py::arg("document"), py::arg("path"), py::arg("library"));
    file.def_static("save",
                    &lc::persistence::File::save,
                    py::arg("document"), py::arg("path"), py::arg("type"));
    file.def_static("getAvailableFileTypes",
                    &lc::persistence::File::getAvailableFileTypes);
    file.def_static("getAvailableLibrariesForFormat",
                    &lc::persistence::File::getAvailableLibrariesForFormat,
                    py::arg("format"));
    file.def_static("getExtensionForFileType",
                    &lc::persistence::File::getExtensionForFileType,
                    py::arg("type"));
    file.def_static("getSupportedFileExtensions",
                    &lc::persistence::File::getSupportedFileExtensions);
}

} // namespace python
} // namespace lc
