#pragma once

// Bridge — lc.storage.* (mirrors lcadluascript/bridge/lc_storage.h).  Slice 1.8.

#include <pybind11/pybind11.h>

namespace lc {
namespace python {

/// Register on lc.storage:
///   - `QuadTreeSub<CADEntity_CSPtr>` and `QuadTree<CADEntity_CSPtr>` — the
///     template instantiation Lua uses
///   - `EntityContainer<CADEntity_CSPtr>` — same
///   - `StorageManager` interface + `StorageManagerImpl`
///   - `Document` interface + `DocumentImpl` (the concrete class the
///     LCPython facade's `setDocument()` binds into the per-context ns)
///   - `UndoManager` + `UndoManagerImpl`
///
/// `Document` and `StorageManager` are py::classh: they cross as _CSPtr /
/// _SPtr through the entire operations layer.
void import_py_lc_storage_namespace(pybind11::module_& m_storage);

} // namespace python
} // namespace lc
