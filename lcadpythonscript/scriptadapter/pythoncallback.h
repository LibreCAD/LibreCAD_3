#pragma once

// Python adapter for lcscripting::ScriptCallback.  Phase 4 PR-1.
//
// Wraps a py::object (callable or object-with-onEvent) in a
// ScriptCallbackImpl.  The pImpl acquires the GIL in:
//   * destructor  — because ~py::object decrements refcount, which is a
//     Python C-API op that requires the GIL
//   * copy ops    — py::object copy-construction is an incref, same
//   * invoke      — because we call into Python
//
// This is EXACTLY the pattern that phase 1's slice-1.12 gtest suite bug
// forced us to formalize: under the permanent-release-GIL pattern of
// plan decision 7, the Qt main thread runs GIL-free; any py::object
// touch outside a `gil_scoped_acquire` scope crashes on Py_DECREF.
//
// The bound-method equality trap (`self.on_point is self.on_point == False`,
// but `self.on_point == self.on_point == True`) is closed by
// `PythonCallback::equals` using `is OR equal`, not identity alone
// (master plan design decision 4).

#include <lcscripting/scriptcallback.h>
#include <lcscripting/scriptvalue.h>

#include <pybind11/pybind11.h>

#include <memory>
#include <string>
#include <vector>

namespace lc {
namespace python {

/// Wrap a py::object into a ScriptCallback.  The `runtime()` tag is
/// "python", and the underlying pImpl handles the GIL discipline.
lc::scripting::ScriptCallback makePythonCallback(pybind11::object obj);

/// Convert a ScriptValue to a py::object (returns py::none() for Nil).
/// Must be called with the GIL held.
pybind11::object toPy(const lc::scripting::ScriptValue& v);

/// Convert a py::object to a ScriptValue.  Best-effort — falls back to
/// Nil for shapes we can't map (adapters may extend for their own types).
/// Must be called with the GIL held.
lc::scripting::ScriptValue fromPy(pybind11::handle o);

} // namespace python
} // namespace lc
