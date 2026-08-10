// Phase 3 PR-3.2 — lcgui embedded module.
//
// See pyguibridge.h for the design rationale (holder-less bindings for
// Qt-owned widgets, `import lc` first to register kernel types,
// scope-limited to what ScriptDock's phase-3 Python leg needs).

#ifdef LC_WITH_PYTHONSCRIPT

#include "pyguibridge.h"

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include "mainwindow.h"
#include "cadmdichild.h"
#include "widgets/clicommand.h"
#include "lcadviewer.h"
#include "luainterface.h"
#include "pyeventhooks.h"   // Phase 5 PR-5.1 fixup: currentMainWindow()

#include <drawables/tempentities.h>

namespace py = pybind11;

PYBIND11_EMBEDDED_MODULE(lcgui, m) {
    m.doc() = "LibreCAD GUI bindings — phase 3 minimal set.  See the "
              "phase-3 sub-plan for the exact API surface (MainWindow "
              "+ CadMdiChild) and the deliberate hold-less-pointer "
              "parity with Lua.";

    // The kernel bindings (lc.entity.*, lc.operation.*, lc.geo.*) live
    // in lcpythonscript's embedded `lc` module — import here so lcgui's
    // return-type signatures below (Document, CADEntity_CSPtr, ...)
    // find their registered types.  Without this, the first call
    // returning a `Document` from Python would raise
    // "Unregistered type: lc::storage::Document".
    py::module_::import("lc");

    // -----------------------------------------------------------------
    // MainWindow (holder-less; Qt owns it, Python holds a raw pointer).
    // -----------------------------------------------------------------
    // Deliberately NO smart-pointer holder — see the header note.
    py::class_<lc::ui::MainWindow>(m, "MainWindow")
        // Convenience: `mainWindow.message("hello")` writes to the CLI.
        // Mirrors Lua's `mainWindow:cliCommand():write(m)` at
        // luainterface.cpp:160 pattern, with the tostring() applied
        // Python-side by the caller (str(x) if x isn't already a string).
        .def("message",
             [](lc::ui::MainWindow& self, const std::string& msg) {
                 self.cliCommand()->write(msg);
             },
             py::arg("message"),
             "Write a message to the CLI command output.")
        // Returning a raw `CadMdiChild*` — Qt owns the child; pybind11
        // must NOT try to delete it.  Explicit reference RVP.
        .def("cadMdiChild",
             &lc::ui::MainWindow::cadMdiChild,
             py::return_value_policy::reference,
             "Return the active CadMdiChild (Qt-owned; do not store "
             "across window close).")
        .def("undo",  &lc::ui::MainWindow::undo,
             "Undo the last kernel operation.")
        .def("redo",  &lc::ui::MainWindow::redo,
             "Redo the last undone kernel operation.")
        // Phase 5 PR-5.1 fixup — Python operations need luaInterface()
        // for triggerEvent (double-fire operationFinished→finishOperation
        // order in CreateOperations.close) and cliCommand() for the
        // CLI-state reset there.  Both were missing in the phase-3
        // minimum surface; the fixup review flagged them as blockers.
        .def("luaInterface",
             &lc::ui::MainWindow::luaInterface,
             py::return_value_policy::reference,
             "Return the MainWindow's LuaInterface (Qt-owned).")
        .def("cliCommand",
             &lc::ui::MainWindow::cliCommand,
             py::return_value_policy::reference,
             "Return the MainWindow's CLI command widget (Qt-owned).");

    // -----------------------------------------------------------------
    // LuaInterface — the minimal surface CreateOperations.close() needs
    // to fire operationFinished before its own event-teardown.  We only
    // expose triggerEvent(name) because that's all the base uses today;
    // future phases can extend.
    // -----------------------------------------------------------------
    py::class_<lc::ui::LuaInterface>(m, "LuaInterface")
        .def("triggerEvent",
             [](lc::ui::LuaInterface& self, const std::string& event) {
                 self.triggerEvent(event, lc::scripting::ScriptValue{});
             },
             py::arg("event"),
             "Fire the named event with an empty payload.  Used by "
             "CreateOperations.close() to run the "
             "`operationFinished` double-fire ordering.");

    // -----------------------------------------------------------------
    // CliCommand — surface CreateOperations.close() reaches into for the
    // CLI-state reset (`returnText(False)` + `commandActive(False)`).
    // -----------------------------------------------------------------
    py::class_<lc::ui::widgets::CliCommand>(m, "CliCommand")
        .def("write",
             &lc::ui::widgets::CliCommand::write,
             "Write a message to the CLI output.")
        .def("returnText",
             &lc::ui::widgets::CliCommand::returnText,
             "Toggle the returnText mode.")
        .def("commandActive",
             &lc::ui::widgets::CliCommand::commandActive,
             py::arg("active"),
             "Set whether a command is currently active (drives the "
             "prompt state).");

    // -----------------------------------------------------------------
    // CadMdiChild (holder-less; Qt owns it via the MainWindow tree).
    // -----------------------------------------------------------------
    py::class_<lc::ui::CadMdiChild>(m, "CadMdiChild")
        // `document()` returns `shared_ptr<Document>` — that's the
        // KERNEL's holder type (Document is bound with shared_ptr in
        // lc/lcpythonscript).  We pass the shared_ptr straight through
        // — pybind11 knows how to marshal it because lc registered the
        // holder.
        .def("document",
             &lc::ui::CadMdiChild::document,
             "Return the underlying kernel Document (shared_ptr; "
             "safe to store).")
        // Returns `vector<CADEntity_CSPtr>` — pybind11/stl.h handles
        // the vector conversion; CADEntity is bound in lc.
        .def("selection",
             &lc::ui::CadMdiChild::selection,
             "Return the currently selected entities.")
        // `tempEntities()` returns a shared_ptr<TempEntities>; kernel
        // manages the lifetime.
        .def("tempEntities",
             &lc::ui::CadMdiChild::tempEntities,
             "Return the temporary-entities container.")
        // `activeLayer()` returns a shared_ptr<const Layer>.  Same
        // shape as document() — kernel manages the lifetime.  Not
        // strictly in the sub-plan's phase-3 scope list but included
        // here because the dual-language ScriptDock test (PR-3.4) needs
        // it as parity with Lua's `mainWindow:cadMdiChild():activeLayer()`
        // pattern; adding one more accessor is trivially in-scope.
        .def("activeLayer",
             &lc::ui::CadMdiChild::activeLayer,
             "Return the currently active layer.")
        // Convenience: `cadmdi.autoScale()` reaches through
        // `viewer()->autoScale()`.  Avoids exposing the LCADViewer
        // class in phase 3 (out of scope per the sub-plan).
        .def("autoScale",
             [](lc::ui::CadMdiChild& self) {
                 self.viewer()->autoScale();
             },
             "Re-fit the viewport to the current document extents.");

    // -----------------------------------------------------------------
    // TempEntities — the drawables::TempEntities container.  Phase 5
    // PR-5.1 fixup: CreateOperations.refreshTempEntity /
    // removeTempEntity call `.addEntity()`/`.removeEntity()` unguarded;
    // without this binding those would raise AttributeError which the
    // adapter's blanket `catch (py::error_already_set&)` swallows,
    // silently breaking every temp-entity refresh.
    // -----------------------------------------------------------------
    py::class_<lc::viewer::drawable::TempEntities,
               std::shared_ptr<lc::viewer::drawable::TempEntities>>(
        m, "TempEntities")
        .def("addEntity",
             &lc::viewer::drawable::TempEntities::addEntity,
             py::arg("entity"),
             "Add a temporary preview entity (removed on operation "
             "close or overwritten by the next refresh).")
        .def("removeEntity",
             &lc::viewer::drawable::TempEntities::removeEntity,
             py::arg("entity"),
             "Remove a temporary preview entity added earlier.");

    // -----------------------------------------------------------------
    // Module-level currentMainWindow() — Phase 5 PR-5.1 fixup: what
    // Python operation base classes call INSTEAD of frame-walking to
    // reach the active MainWindow.  Delegates to the C++ helper in
    // pyeventhooks.cpp which reads WindowManager::mainWindows.back().
    // Returns None (Python) when there is no window (headless CLI).
    // -----------------------------------------------------------------
    m.def("currentMainWindow",
          []() -> py::object {
              lc::ui::MainWindow* mw =
                  lc::ui::python::currentMainWindow();
              if (mw == nullptr) {
                  return py::none();
              }
              return py::cast(mw,
                              py::return_value_policy::reference);
          },
          "Return the currently-active MainWindow (raw pointer, Qt-"
          "owned).  Returns None in headless mode.");
}

#endif  // LC_WITH_PYTHONSCRIPT
