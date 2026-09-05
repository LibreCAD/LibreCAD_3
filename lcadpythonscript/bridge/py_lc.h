#pragma once

// Bridge — top-level lc.* names (mirrors lcadluascript/bridge/lc.h).
// Populated in slice 1.3.

#include "../qt_keywords_push.h"
#include <pybind11/pybind11.h>
#include "../qt_keywords_pop.h"

#include <functional>
#include <string>

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

// -----------------------------------------------------------------------------
// Phase 6 PR-6.1 — custom-entity plugin registration hook.
// -----------------------------------------------------------------------------
// `lc.register_plugin(name, fn)` is the Python analog of Lua's global
// `registerPlugin(name, fn)`.  The plugin function runs when a custom-
// entity Insert whose displayBlock is a `CustomEntityStorage` with matching
// pluginName is added to any tracked Document (via
// `LuaCustomEntityManager::onNewWaitingEntity`, listening on
// `DocumentList::newWaitingCustomEntityEvent`).
//
// lcpythonscript can't name lcadluascript's `LuaCustomEntityManager`
// (dependency direction: lcadluascript and lcadpythonscript are siblings,
// both linking lcscripting + lckernel).  The binding routes through a
// hook slot; lcUI installs the hook at initLua time (paired with the
// event register/deregister hooks).  Unset hook = silent no-op
// (headless CLI mode — matches `lc.event.register` behavior).
//
// The plugin's callable receives ONE argument: an `lc.entity.Insert`
// (upcast from Insert_CSPtr → CADEntity_CSPtr → ScriptValue::Entity →
// py::cast to the concrete class via pybind11's polymorphic downcast).

/// Function shape: `(plugin_name, py::object) -> void`.  py::object is
/// the plugin callable the Python script passed to `lc.register_plugin`.
using RegisterPluginHook = std::function<void(const std::string&, pybind11::object)>;

/// Install a hook fired by `lc.register_plugin(name, fn)`.  Later
/// installations overwrite; one hook per process (same policy as the
/// event hooks).
void setRegisterPluginHook(RegisterPluginHook hook);

/// Query for the current hook.  Used by tests that need to verify a
/// hook was installed.  Non-const so the hook can be invoked directly
/// in test scaffolding.
const RegisterPluginHook& registerPluginHook();

} // namespace python
} // namespace lc
