#pragma once

// Bridge — lc.operation.* (mirrors lcadluascript/bridge/lc_operation.h).
// Slice 1.9.

#include <pybind11/pybind11.h>

namespace lc {
namespace python {

/// Register on lc.operation: Undoable base, DocumentOperation base, Builder,
/// Base (all-operation base), Loop/Begin/Move/Copy/Rotate/Scale/Push/
/// SelectByLayer/Remove, and Add/Remove/Replace for Layer/Block/LinePattern
/// (9 document-mutation operations), plus EntityBuilder.  All py::classh —
/// every op flows as _SPtr through the operation queue.  Every op preserves
/// the Lua-facing `new()` static factory returning `make_shared<>()` even
/// though pybind11 could handle direct init — parity is a plan constraint.
void import_py_lc_operation_namespace(pybind11::module_& m_operation);

} // namespace python
} // namespace lc
