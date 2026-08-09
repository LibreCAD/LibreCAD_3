#pragma once

// Bridge — lc.persistence.* (phase 2 slice 2.6).  Binds the
// lc::persistence::File open/save API into Python so headless scripts can
// load and save DXF/DWG without going through the GUI's file dialog.
//
// Promoted from optional to REQUIRED in the phase-2 sub-plan because
// phase 6's custom-entity DXF round-trip regression test hard-depends on
// script-callable persistence — there is no working GUI-harness fallback,
// and today no script (Lua or Python) can do headless DXF I/O.

#include <pybind11/pybind11.h>

namespace lc {
namespace python {

/// Register `lc.persistence.File` with its two enums (Type, Library) and
/// its 5 static methods (open, save, getAvailableFileTypes,
/// getAvailableLibrariesForFormat, getExtensionForFileType,
/// getSupportedFileExtensions).  py::class_ — File is a static-methods-only
/// gateway class; no instances ever cross the boundary.
void import_py_lc_persistence_namespace(pybind11::module_& m_persistence);

} // namespace python
} // namespace lc
