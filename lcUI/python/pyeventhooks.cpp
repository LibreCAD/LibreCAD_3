// Phase 5 PR-5.1 — install lc.event.register/deregister hooks that
// route to the active MainWindow's LuaInterface EventBus.

#ifdef LC_WITH_PYTHONSCRIPT

#include "pyeventhooks.h"

#include <pybind11/pybind11.h>

#include <bridge/py_lc_event.h>
#include <scriptadapter/pythoncallback.h>

#include "mainwindow.h"
#include "windowmanager.h"
#include "luainterface.h"

namespace py = pybind11;

namespace lc {
namespace ui {
namespace python {

lc::ui::MainWindow* currentMainWindow() {
    // Phase 5 PR-5.1 fixup — promoted from anonymous-namespace static
    // to public API so Python (via a binding in pyguibridge.cpp) can
    // reach the same "active MainWindow" the hooks use.  This is what
    // CreateOperations's `_get_main_window()` calls INSTEAD of the
    // broken frame-walk.
    auto& wins = lc::ui::WindowManager::mainWindows;
    return wins.empty() ? nullptr : wins.back();
}

void installEventHooks() {
    // Register hook: `lc.event.register("point", self)` → wrap self as
    // a PythonCallback, forward to the LuaInterface's registerEvent.
    // The LuaInterface's native overload `registerEvent(std::string,
    // ScriptCallback)` (phase 4 PR-9a) does the right thing — the
    // ScriptCallback's runtime tag is "python", so Lua-side listeners
    // and Python-side listeners coexist on the same EventBus.
    lc::python::setEventRegisterHook(
        [](const std::string& name, py::object obj) {
            lc::ui::MainWindow* mw = currentMainWindow();
            if (mw == nullptr) return;
            mw->luaInterface()->registerEvent(
                name, lc::python::makePythonCallback(std::move(obj)));
        });

    // Deregister hook: same routing, delete instead of register.
    // Phase-4 note on equality: PythonCallbackImpl::equals uses
    // is-or-equal so a fresh `self.on_point` bound-method access still
    // matches the registered one — this is what makes
    // `lc.event.register("point", self.on_point)` /
    // `lc.event.deregister("point", self.on_point)` work even though
    // Python creates a fresh bound-method object per attribute access.
    lc::python::setEventDeregisterHook(
        [](const std::string& name, py::object obj) {
            lc::ui::MainWindow* mw = currentMainWindow();
            if (mw == nullptr) return;
            mw->luaInterface()->deleteEvent(
                name, lc::python::makePythonCallback(std::move(obj)));
        });
}

} // namespace python
} // namespace ui
} // namespace lc

#endif  // LC_WITH_PYTHONSCRIPT
