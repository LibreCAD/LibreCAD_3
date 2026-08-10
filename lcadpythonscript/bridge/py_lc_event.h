#pragma once

// Bridge — lc.event.* (mirrors lcadluascript/bridge/lc_event.h).  Slice 1.11.
//
// Phase 5 PR-5.1 additions:
//   * `lc.event.register(name, obj)` / `lc.event.deregister(name, obj)` —
//     the Python analog of Lua's `luaInterface:registerEvent(...)`.  The
//     bindings themselves live in lc's embedded module (this file) but
//     the underlying EventBus is per-MainWindow (owned by
//     LuaInterface).  lcpythonscript can't name MainWindow (dependency
//     direction), so the routing goes through a HOOK:
//         lc::python::setEventRegisterHook(fn)
//         lc::python::setEventDeregisterHook(fn)
//     lcUI installs the hooks at MainWindow construction; the hook
//     wraps the py::object as a ScriptCallback (via makePythonCallback)
//     and delegates to the appropriate EventBus.
//   * If no hook is set (headless CLI use), the register/deregister
//     calls silently no-op.  Matches the pattern established by
//     lcadluascript's OpaquePtr encoder registry (phase 4 PR-8) where
//     unset entries fall back gracefully.

#include <pybind11/pybind11.h>

#include <functional>
#include <string>

namespace lc {
namespace python {

/// Called from PYBIND11_EMBEDDED_MODULE(lc, m) in lcpython.cpp to fill
/// `lc.event.*` with the kernel event classes (phase 1 payload types)
/// plus the phase-5 register/deregister free-functions.
void import_py_lc_event_namespace(pybind11::module_& m_event);

/// Function shape: `(event_name, py::object) -> void`.  py::object is
/// the callback the Python script passed to `lc.event.register`.
using EventHook = std::function<void(const std::string&, pybind11::object)>;

/// Install a hook fired by `lc.event.register(name, obj)`.  Later
/// registrations OVERWRITE the previous hook (matches the OpaquePtr
/// registry semantics — one hook per process; multi-window support
/// resolves via the hook's own logic against WindowManager).
void setEventRegisterHook(EventHook hook);

/// Install a hook fired by `lc.event.deregister(name, obj)`.
void setEventDeregisterHook(EventHook hook);

} // namespace python
} // namespace lc
