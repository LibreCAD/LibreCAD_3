// py_lc_persistence.cpp — Python port of the persistence gateway.
// Phase 2 slice 2.6.
//
// The C++ class `lc::persistence::File` is a namespace of static functions
// (open/save + 4 introspection helpers), not an instantiable class.  We
// bind it as a py::class_ with no ctors so Python users spell it
// `lc.persistence.File.save(doc, path, T)` — 1:1 with the C++ call.

#include "py_lc_persistence.h"

#include <pybind11/stl.h>

#include <persistence/documentsource.h>
#include <persistence/file.h>
#include <persistence/format.h>

namespace py = pybind11;

namespace lc {
namespace python {

void import_py_lc_persistence_namespace(py::module_& m_persistence) {
    // The result types first: File's new entry points return them.
    //
    // These are the answers File::open could never give -- whether the read
    // finished, how much of it arrived, what the file said its revision was,
    // and what had to be dropped. A script that reads a directory of drawings
    // needs all four to say anything true about what it found.
    py::enum_<lc::persistence::Severity>(m_persistence, "Severity")
        .value("Info", lc::persistence::Severity::Info)
        .value("Warning", lc::persistence::Severity::Warning)
        .value("Error", lc::persistence::Severity::Error)
        .export_values();

    py::class_<lc::persistence::Diagnostic>(m_persistence, "Diagnostic")
        .def_readonly("severity", &lc::persistence::Diagnostic::severity)
        .def_readonly("code", &lc::persistence::Diagnostic::code,
                      "Short stable token, e.g. 'unsupported-version'. Match on this.")
        .def_readonly("message", &lc::persistence::Diagnostic::message)
        .def("__repr__", [](const lc::persistence::Diagnostic& d) {
            return "<Diagnostic " + d.code + ": " + d.message + ">";
        });

    py::class_<lc::persistence::LossSummary>(m_persistence, "LossSummary")
        .def_readonly("droppedByType", &lc::persistence::LossSummary::droppedByType,
                      "DXF record kind -> how many were dropped for having no "
                      "LibreCAD equivalent.")
        .def_readonly("notes", &lc::persistence::LossSummary::notes)
        .def("empty", &lc::persistence::LossSummary::empty)
        .def("total", &lc::persistence::LossSummary::total);

    py::class_<lc::persistence::ImportResult>(m_persistence, "ImportResult")
        .def_readonly("ok", &lc::persistence::ImportResult::ok)
        .def_readonly("partial", &lc::persistence::ImportResult::partial,
                      "True when the read failed but entities arrived first. "
                      "The document is real, and is not the whole file.")
        .def_readonly("variantId", &lc::persistence::ImportResult::variantId)
        .def_readonly("sourceVersionTag", &lc::persistence::ImportResult::sourceVersionTag,
                      "The file's own $ACADVER, empty when it carries none.")
        .def_readonly("entitiesDelivered", &lc::persistence::ImportResult::entitiesDelivered)
        .def_readonly("diagnostics", &lc::persistence::ImportResult::diagnostics)
        .def_readonly("loss", &lc::persistence::ImportResult::loss);

    py::class_<lc::persistence::ExportResult>(m_persistence, "ExportResult")
        .def_readonly("ok", &lc::persistence::ExportResult::ok)
        .def_readonly("variantId", &lc::persistence::ExportResult::variantId)
        .def_readonly("diagnostics", &lc::persistence::ExportResult::diagnostics)
        .def_readonly("loss", &lc::persistence::ExportResult::loss);

    py::class_<lc::persistence::FormatVariant>(m_persistence, "FormatVariant")
        .def_readonly("id", &lc::persistence::FormatVariant::id)
        .def_readonly("formatId", &lc::persistence::FormatVariant::formatId)
        .def_readonly("label", &lc::persistence::FormatVariant::label)
        .def_readonly("versionTag", &lc::persistence::FormatVariant::versionTag)
        .def_readonly("extension", &lc::persistence::FormatVariant::extension)
        .def_readonly("libraryId", &lc::persistence::FormatVariant::libraryId)
        .def_readonly("binary", &lc::persistence::FormatVariant::binary)
        .def_readonly("readable", &lc::persistence::FormatVariant::readable)
        .def_readonly("writable", &lc::persistence::FormatVariant::writable)
        .def_readonly("experimental", &lc::persistence::FormatVariant::experimental);

    m_persistence.def("formatVariants", &lc::persistence::formatVariants,
                      py::return_value_policy::reference,
                      "Every format, revision and encoding LibreCAD knows.");
    m_persistence.def("formatVariantById",
                      [](const std::string& id) -> py::object {
                          const auto* variant = lc::persistence::formatVariantById(id);
                          return variant == nullptr
                              ? py::none()
                              : py::cast(*variant, py::return_value_policy::copy);
                      },
                      py::arg("id"), "The variant with this id, or None.");

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
    file.def_static("importFile",
                    &lc::persistence::File::importFile,
                    py::arg("document"), py::arg("path"), py::arg("library"),
                    "Read a file and report what happened.");
    file.def_static("exportFile",
                    &lc::persistence::File::exportFile,
                    py::arg("document"), py::arg("path"), py::arg("type"),
                    "Write a file and report what happened.");
    file.def_static("variantIdForType",
                    &lc::persistence::File::variantIdForType,
                    py::arg("type"));
    file.def_static("typeForVariantId",
                    [](const std::string& id) -> py::object {
                        lc::persistence::File::Type type;
                        return lc::persistence::File::typeForVariantId(id, type)
                            ? py::cast(type) : py::none();
                    },
                    py::arg("id"), "The Type for a variant id, or None.");

    // open and save remain: they are what every existing script calls.
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
