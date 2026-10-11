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

#include "../lcpython_api.h"
#include "../qt_keywords_push.h"
#include <pybind11/pybind11.h>
#include "../qt_keywords_pop.h"

#include <memory>
#include <functional>
#include <string>
#include <vector>

namespace lc {
namespace python {

/// Wrap a py::object into a ScriptCallback.  The `runtime()` tag is
/// "python", and the underlying pImpl handles the GIL discipline.
LC_PYTHON_API lc::scripting::ScriptCallback makePythonCallback(pybind11::object obj);

/// Invoke a Python-backed callback with already-materialized Python args.
/// Unlike ScriptCallback::invoke this preserves adapter-only argument and
/// return types such as Builder_SPtr, SimpleSnapConstrain, and snap vectors.
/// Exceptions propagate to the typed adapter so it can report the slot and
/// return a safe default without invoking the callback a second time.
LC_PYTHON_API pybind11::object invokePythonCallback(
    const lc::scripting::ScriptCallback& callback,
    const pybind11::tuple& args);

/// UI-side extension point for tagged non-owning values in ScriptValue.
/// lcadpythonscript stays independent of Qt and UI types; lcUI installs the
/// materializer after registering its holder-less pybind11 classes.
using PythonOpaqueMaterializer = std::function<pybind11::object(
    const lc::scripting::OpaquePtr&)>;
LC_PYTHON_API void setPythonOpaqueMaterializer(
    PythonOpaqueMaterializer materializer);

/// Convert a ScriptValue to a py::object (returns py::none() for Nil).
/// Must be called with the GIL held.
pybind11::object toPy(const lc::scripting::ScriptValue& v);

/// Convert a py::object to a ScriptValue.  Best-effort — falls back to
/// Nil for shapes we can't map (adapters may extend for their own types).
/// Must be called with the GIL held.
lc::scripting::ScriptValue fromPy(pybind11::handle o);

} // namespace python
} // namespace lc
