#pragma once

// Python adapter for lcscripting::ScriptObject.  Phase 4 PR-1 follow-on.
//
// Wraps a py::object (a Python instance) in a ScriptObjectImpl.  Same
// GIL discipline as PythonCallbackImpl (see pythoncallback.h):
//   * destructor  — acquires GIL (~py::object needs Py_DECREF)
//   * every op    — acquires GIL
//
// Equality uses `is OR equal`, same reasoning as ScriptCallback.

#include <lcscripting/scriptobject.h>
#include <lcscripting/scriptvalue.h>

#include "../qt_keywords_push.h"
#include <pybind11/pybind11.h>
#include "../qt_keywords_pop.h"

#include <memory>
#include <string>
#include <vector>

namespace lc {
namespace python {

/// Wrap a py::object (Python operation instance) into a ScriptObject.
lc::scripting::ScriptObject makePythonObject(pybind11::object obj);

} // namespace python
} // namespace lc
