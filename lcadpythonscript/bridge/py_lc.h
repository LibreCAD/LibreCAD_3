#pragma once

// Bridge — top-level lc.* names (mirrors lcadluascript/bridge/lc.h).
// Populated in slice 1.3.

#include <pybind11/pybind11.h>

namespace lc {
namespace python {

/// Register Visitable/Color/EntityCoordinate/SimpleSnapConstrain/EntityDistance
/// on the given `lc` module.  Called from PYBIND11_EMBEDDED_MODULE(lc, m) in
/// lcpython.cpp.
///
/// EntityDispatch is deferred to slice 1.6 (py_lc_entity.cpp) because its
/// binding requires every entity Python type to be registered first —
/// pybind11's `def("visit", static_cast<...>(&EntityDispatch::visit))` needs
/// per-overload type-caster lookup at call time.
void import_py_lc_namespace(pybind11::module_& m);

} // namespace python
} // namespace lc
