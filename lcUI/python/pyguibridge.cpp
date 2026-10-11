// Phase 3 PR-3.2 — lcgui embedded module.
// Phase 5 PR-5.6 — full lc-side band + gui.* widget family.
//
// See pyguibridge.h for the Qt lifetime/ownership policy and importing `lc`
// first to register kernel types.
//
// Cross-referenced every `.def(...)` against the real header signature
// per coordinator discipline (code review alone hasn't been sufficient
// on GL-blocked files this session — a real .h read is authoritative).

#ifdef LC_WITH_PYTHONSCRIPT

#include "pyguibridge.h"

#include <qt_keywords_push.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <qt_keywords_pop.h>

#include <QCoreApplication>
#include <QThread>
#include <stdexcept>

#include "mainwindow.h"
#include "cadmdichild.h"
#include "widgets/clicommand.h"
#include "widgets/toolbar.h"
#include "widgets/layers.h"
#include "widgets/guiAPI/toolbartab.h"
#include "widgets/guiAPI/toolbarbutton.h"
#include "widgets/guiAPI/toolbargroup.h"
#include "widgets/guiAPI/menu.h"
#include "widgets/guiAPI/menuitem.h"
#include "widgets/guiAPI/inputgui.h"
#include "widgets/guiAPI/inputguicontainer.h"
#include "widgets/guiAPI/dialogwidget.h"
#include "widgets/guiAPI/textgui.h"
#include "widgets/guiAPI/buttongui.h"
#include "widgets/guiAPI/checkboxgui.h"
#include "widgets/guiAPI/radiobuttongui.h"
#include "widgets/guiAPI/horizontalgroupgui.h"
#include "widgets/guiAPI/radiogroupgui.h"
#include "widgets/guiAPI/coordinategui.h"
#include "widgets/guiAPI/anglegui.h"
#include "widgets/guiAPI/slidergui.h"
#include "widgets/guiAPI/comboboxgui.h"
#include "widgets/guiAPI/numbergui.h"
#include "widgets/guiAPI/colorgui.h"
#include "widgets/guiAPI/entitygui.h"
#include "widgets/guiAPI/listgui.h"
#include "propertyeditor.h"
#include "lcadviewer.h"
#include "documentcanvas.h"
#include "drawables/lccursor.h"
#include "luainterface.h"
#include "managers/metainfomanager.h"
#include "pyeventhooks.h"   // Phase 5 PR-5.1 fixup: currentMainWindow()
#include "qtpythonlifetime.h"

#include <drawables/tempentities.h>
#include <managers/snapmanagerimpl.h>

#include <scriptadapter/pythoncallback.h>

namespace py = pybind11;

using lc::ui::python::QtUiThreadCallGuard;
using lc::ui::python::QtClass;

QObject* lc::ui::python::qtLifetimeOwner(lc::ui::LuaInterface* value) {
    return value->ownerObject();
}

PYBIND11_EMBEDDED_MODULE(lcgui, m) {
    m.doc() = "LibreCAD GUI bindings — phase 3 minimum + phase-5 PR-5.6 "
              "full widget API surface (mirrors the Lua guibridge.cpp "
              "exposed API, minus LineSelectGUI/LWVertexGroup which are "
              "PropertyEditor-internal and never script-created).";

    // The kernel bindings (lc.entity.*, lc.operation.*, lc.geo.*) live
    // in lcpythonscript's embedded `lc` module — import here so lcgui's
    // return-type signatures below (Document, CADEntity_CSPtr, ...)
    // find their registered types.  Without this, the first call
    // returning a `Document` from Python would raise
    // "Unregistered type: lc::storage::Document".
    py::module_::import("lc");

    // -----------------------------------------------------------------
    // MainWindow (native-owned; Python holds a guarded reference).
    // -----------------------------------------------------------------
    QtClass<lc::ui::MainWindow>(m, "MainWindow")
        // Convenience: `mainWindow.message("hello")` writes to the CLI.
        // Mirrors Lua's `mainWindow:cliCommand():write(m)` at
        // luainterface.cpp:160 pattern, with the tostring() applied
        // Python-side by the caller (str(x) if x isn't already a string).
        .def("message",
             [](lc::ui::MainWindow& self, const std::string& msg) {
                 self.cliCommand()->write(msg);
             },
             py::arg("message"),
             "Write a message to the CLI command output.", py::call_guard<QtUiThreadCallGuard>())
        // Returning a raw `CadMdiChild*` — Qt owns the child; pybind11
        // must NOT try to delete it.  Explicit reference RVP.
        .def("cadMdiChild",
             &lc::ui::MainWindow::cadMdiChild,
             py::return_value_policy::reference,
             "Return the active CadMdiChild (Qt-owned; do not store "
             "across window close).", py::call_guard<QtUiThreadCallGuard>())
        .def("undo",  &lc::ui::MainWindow::undo,
             "Undo the last kernel operation.", py::call_guard<QtUiThreadCallGuard>())
        .def("redo",  &lc::ui::MainWindow::redo,
             "Redo the last undone kernel operation.", py::call_guard<QtUiThreadCallGuard>())
        // Phase 5 PR-5.1 fixup — Python operations need luaInterface()
        // for triggerEvent (double-fire operationFinished→finishOperation
        // order in CreateOperations.close) and cliCommand() for the
        // CLI-state reset there.
        .def("luaInterface",
             &lc::ui::MainWindow::luaInterface,
             py::return_value_policy::reference,
             "Return the MainWindow's LuaInterface (Qt-owned).", py::call_guard<QtUiThreadCallGuard>())
        .def("cliCommand",
             &lc::ui::MainWindow::cliCommand,
             py::return_value_policy::reference,
             "Return the MainWindow's CLI command widget (Qt-owned).", py::call_guard<QtUiThreadCallGuard>())
        // Phase 5 PR-5.6 — remainder of Lua's MainWindow surface.
        .def("toolbar",
             &lc::ui::MainWindow::toolbar,
             py::return_value_policy::reference,
             "Return the MainWindow's Toolbar widget (Qt-owned).", py::call_guard<QtUiThreadCallGuard>())
        .def("layers",
             &lc::ui::MainWindow::layers,
             py::return_value_policy::reference,
             "Return the MainWindow's Layers widget (Qt-owned).", py::call_guard<QtUiThreadCallGuard>())
        .def("operationFinished",
             &lc::ui::MainWindow::operationFinished,
             "Called on operation finish — signals cleanup to the "
             "current operation and clears the LuaInterface's "
             "operation slot.", py::call_guard<QtUiThreadCallGuard>())
        .def("findMenuItem",
             &lc::ui::MainWindow::findMenuItem,
             py::arg("label"),
             py::return_value_policy::reference,
             "Find and return a menu item by label.", py::call_guard<QtUiThreadCallGuard>())
        .def("findMenuItemByObjectName",
             &lc::ui::MainWindow::findMenuItemByObjectName,
             py::arg("objectName"),
             py::return_value_policy::reference,
             "Find and return a menu item by Qt object name.", py::call_guard<QtUiThreadCallGuard>())
        .def("menuByName",
             &lc::ui::MainWindow::menuByName,
             py::arg("name"),
             py::return_value_policy::reference,
             "Return a menu by label.", py::call_guard<QtUiThreadCallGuard>())
        .def("menuByPosition",
             &lc::ui::MainWindow::menuByPosition,
             py::arg("position"),
             py::return_value_policy::reference,
             "Return a menu by position.", py::call_guard<QtUiThreadCallGuard>())
        .def("runCustomizeToolbar",
             &lc::ui::MainWindow::runCustomizeToolbar,
             "Open the customize-toolbar dialog.", py::call_guard<QtUiThreadCallGuard>())
        .def("changeDockLayout",
             &lc::ui::MainWindow::changeDockLayout,
             py::arg("layout"),
             "Change the dock layout.", py::call_guard<QtUiThreadCallGuard>())
        .def("saveDockLayout",
             &lc::ui::MainWindow::saveDockLayout,
             "Persist the current dock layout.", py::call_guard<QtUiThreadCallGuard>())
        .def("loadDockLayout",
             &lc::ui::MainWindow::loadDockLayout,
             "Load a previously-persisted dock layout.", py::call_guard<QtUiThreadCallGuard>())
        .def("selectAll",
             &lc::ui::MainWindow::selectAll,
             "Select all entities in the active document.", py::call_guard<QtUiThreadCallGuard>())
        .def("selectNone",
             &lc::ui::MainWindow::selectNone,
             "Clear the current selection.", py::call_guard<QtUiThreadCallGuard>())
        .def("invertSelection",
             &lc::ui::MainWindow::invertSelection,
             "Invert the current selection.", py::call_guard<QtUiThreadCallGuard>())
        .def("runLastOperation",
             &lc::ui::MainWindow::runLastOperation,
             "Re-invoke the last operation with its previous init "
             "method.", py::call_guard<QtUiThreadCallGuard>())
        // Phase 5 PR-5.7 fixup — `currentOperation()` returns
        // `lc::scripting::ScriptObject`, which is NOT registered with
        // pybind11 anywhere in the tree.  Calling it from Python would
        // raise `TypeError: Unregistered type: lc::scripting::ScriptObject`
        // immediately.  The Python surface doesn't need to introspect
        // the current operation from Python (operations are managed
        // through `@lc.register_operation` + `runOperationByName`, not
        // by inspecting an internal state slot).  Removed pending a
        // proper ScriptObject binding — track as a follow-up if a
        // Python caller ever actually needs to inspect an in-flight op.
        .def("copySelectedEntities",
             &lc::ui::MainWindow::copySelectedEntities,
             py::arg("entities"),
             "Copy the given entities to the clipboard.", py::call_guard<QtUiThreadCallGuard>())
        .def("cutSelectedEntities",
             &lc::ui::MainWindow::cutSelectedEntities,
             py::arg("entities"),
             "Copy the given entities to the clipboard and remove them "
             "from the drawing, as one undo step.", py::call_guard<QtUiThreadCallGuard>())
        .def("pasteEvent",
             &lc::ui::MainWindow::pasteEvent,
             "Paste clipboard entities into the active document: start "
             "PasteOperation, which asks for the insertion point.", py::call_guard<QtUiThreadCallGuard>())
        .def("clipboardEntities",
             &lc::ui::MainWindow::clipboardEntities,
             "The clipboard entities a paste into this window adds "
             "copies of.", py::call_guard<QtUiThreadCallGuard>())
        .def("clipboardBasePoint",
             &lc::ui::MainWindow::clipboardBasePoint,
             "The point of the clipboard entities that a paste puts "
             "at the insertion point.", py::call_guard<QtUiThreadCallGuard>())
        .def("pasteClipboard",
             &lc::ui::MainWindow::pasteClipboard,
             py::arg("offset"),
             "Add copies of the clipboard entities, moved by offset, as "
             "one undo step.", py::call_guard<QtUiThreadCallGuard>())
        // addMenu has 2 overloads — bind both via lambdas so pybind11
        // resolves by arg type without ambiguity.
        .def("addMenu",
             [](lc::ui::MainWindow& self, const std::string& name)
                 -> lc::ui::api::Menu* {
                 return self.addMenu(name);
             },
             py::arg("name"),
             py::return_value_policy::reference,
             "Add a new menu to the menu bar; returns the new Menu*.", py::call_guard<QtUiThreadCallGuard>())
        .def("addMenu",
             [](lc::ui::MainWindow& self, lc::ui::api::Menu* menu) {
                 self.addMenu(menu);
             },
             py::arg("menu"),
             "Add an existing Menu* to the menu bar.", py::call_guard<QtUiThreadCallGuard>())
        // removeMenu has 2 overloads: (const char*) and (int).
        .def("removeMenu",
             [](lc::ui::MainWindow& self, const std::string& label) {
                 self.removeMenu(label.c_str());
             },
             py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        .def("removeMenu",
             [](lc::ui::MainWindow& self, int pos) {
                 self.removeMenu(pos);
             },
             py::arg("position"), py::call_guard<QtUiThreadCallGuard>())
        // connectMenuItem takes a ScriptCallback; wrap py::function.
        .def("connectMenuItem",
             [](lc::ui::MainWindow& self, const std::string& itemName,
                py::object cb) {
                 self.connectMenuItem(
                     itemName,
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("item_name"), py::arg("callback"),
             "Connect a Python callable to an existing menu item.", py::call_guard<QtUiThreadCallGuard>())
        // runOperationByName — the native entrypoint the killed Lua
        // dostring codegen replaced with.  Python operations resolve
        // through the ordered resolver list installed at phase-5 init.
        .def("runOperationByName",
             &lc::ui::MainWindow::runOperationByName,
             py::arg("name"),
             py::arg("init_method") = std::string{},
             "Run an operation resolved by name; init_method is the "
             "FULL suffix (e.g. `_init_default`, `_init_arc`).", py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // LuaInterface — surface CreateOperations.close() reaches into for
    // the double-fire operationFinished ordering.  Phase 5 PR-5.6
    // completes the plugin-list + operation slots + finishOperation
    // remainder (Lua exposes them under the same names).
    // -----------------------------------------------------------------
    QtClass<lc::ui::LuaInterface>(m, "LuaInterface")
        .def("triggerEvent",
             [](lc::ui::LuaInterface& self, const std::string& event) {
                 self.triggerEvent(event, lc::scripting::ScriptValue{});
             },
             py::arg("event"),
             "Fire the named event with an empty payload.  Used by "
             "CreateOperations.close() to run the "
             "`operationFinished` double-fire ordering.", py::call_guard<QtUiThreadCallGuard>())
        .def("pluginList",
             &lc::ui::LuaInterface::pluginList,
             py::arg("path"),
             "Enumerate plugin subdirectories under `path` (relative "
             "to the plugin root).", py::call_guard<QtUiThreadCallGuard>())
        .def("finishOperation",
             &lc::ui::LuaInterface::finishOperation,
             "Clear the current operation slot and reset the CLI "
             "prompt to command-entry mode.", py::call_guard<QtUiThreadCallGuard>());
        // Phase 5 PR-5.7 fixup — `operation()` returns
        // `lc::scripting::ScriptObject` and `setOperation()` takes one.
        // Same unregistered-type issue as MainWindow.currentOperation()
        // above: `ScriptObject` isn't bound with pybind11, so calling
        // either from Python raises `TypeError: Unregistered type` at
        // dispatch.  Removed pending a proper ScriptObject binding.
        // Python callers don't need to touch the internal
        // operation-slot state directly — same rationale as the
        // MainWindow removal above.

    // -----------------------------------------------------------------
    // CliCommand — surface CreateOperations.close() reaches into for the
    // CLI-state reset (`returnText(False)` + `commandActive(False)`).
    // Phase 5 PR-5.6 adds the full Lua-exposed remainder.
    // -----------------------------------------------------------------
    QtClass<lc::ui::widgets::CliCommand>(m, "CliCommand")
        .def("write",
             &lc::ui::widgets::CliCommand::write,
             py::arg("message"),
             "Write a message to the CLI output.", py::call_guard<QtUiThreadCallGuard>())
        .def("returnText",
             &lc::ui::widgets::CliCommand::returnText,
             py::arg("returnText"),
             "Toggle the returnText mode.", py::call_guard<QtUiThreadCallGuard>())
        .def("commandActive",
             &lc::ui::widgets::CliCommand::commandActive,
             py::arg("active"),
             "Set whether a command is currently active (drives the "
             "prompt state).", py::call_guard<QtUiThreadCallGuard>())
        // addCommand — Python callback wrapped into ScriptCallback.
        .def("addCommand",
             [](lc::ui::widgets::CliCommand& self,
                const std::string& name, py::object cb) -> bool {
                 return self.addCommand(
                     name.c_str(),
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("name"), py::arg("callback"),
             "Register a Python callable under a command name.  "
             "Returns true when the name is new (mirrors "
             "CliCommand::addCommand semantics), false on collision.", py::call_guard<QtUiThreadCallGuard>())
        .def("runCommand",
             &lc::ui::widgets::CliCommand::runCommand,
             py::arg("command"),
             "Execute a registered command by name.", py::call_guard<QtUiThreadCallGuard>())
        // enableCommand takes 2 args in the header; Lua exposes two
        // convenience shapes (`enableCommand`, `disableCommand`) with a
        // fixed second arg.  Mirror them for parity.
        .def("enableCommand",
             [](lc::ui::widgets::CliCommand& self,
                const std::string& name) {
                 self.enableCommand(name.c_str(), true);
             },
             py::arg("command"),
             "Enable a registered command.", py::call_guard<QtUiThreadCallGuard>())
        .def("disableCommand",
             [](lc::ui::widgets::CliCommand& self,
                const std::string& name) {
                 self.enableCommand(name.c_str(), false);
             },
             py::arg("command"),
             "Disable a registered command.", py::call_guard<QtUiThreadCallGuard>())
        .def("isCommandEnabled",
             &lc::ui::widgets::CliCommand::isCommandEnabled,
             py::arg("command"),
             "Return whether the named command is currently enabled.", py::call_guard<QtUiThreadCallGuard>())
        .def("availableCommands",
             &lc::ui::widgets::CliCommand::availableCommands,
             "Return the list of registered command names.", py::call_guard<QtUiThreadCallGuard>())
        .def("commandsHistory",
             &lc::ui::widgets::CliCommand::commandsHistory,
             "Return the history of executed commands.", py::call_guard<QtUiThreadCallGuard>())
        .def("clear",
             &lc::ui::widgets::CliCommand::clear,
             "Clear the CLI output.", py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // CadMdiChild (holder-less; Qt owns it via the MainWindow tree).
    // -----------------------------------------------------------------
    QtClass<lc::ui::CadMdiChild>(m, "CadMdiChild")
        .def("document",
             &lc::ui::CadMdiChild::document,
             "Return the underlying kernel Document (shared_ptr; "
             "safe to store).", py::call_guard<QtUiThreadCallGuard>())
        .def("selection",
             &lc::ui::CadMdiChild::selection,
             "Return the currently selected entities.", py::call_guard<QtUiThreadCallGuard>())
        .def("tempEntities",
             &lc::ui::CadMdiChild::tempEntities,
             "Return the temporary-entities container.", py::call_guard<QtUiThreadCallGuard>())
        .def("activeLayer",
             &lc::ui::CadMdiChild::activeLayer,
             "Return the currently active layer.", py::call_guard<QtUiThreadCallGuard>())
        .def("autoScale",
             [](lc::ui::CadMdiChild& self) {
                 self.viewer()->autoScale();
             },
             "Re-fit the viewport to the current document extents.", py::call_guard<QtUiThreadCallGuard>())
        // Phase 5 PR-5.6 — remainder of Lua's CadMdiChild surface.
        .def("newDocument",
             &lc::ui::CadMdiChild::newDocument,
             "Start a fresh document in this child.", py::call_guard<QtUiThreadCallGuard>())
        .def("openFile",
             &lc::ui::CadMdiChild::openFile,
             "Load a file into this child; returns true on success.", py::call_guard<QtUiThreadCallGuard>())
        .def("saveFile",
             &lc::ui::CadMdiChild::saveFile,
             "Save the current document.", py::call_guard<QtUiThreadCallGuard>())
        .def("saveAsFile",
             &lc::ui::CadMdiChild::saveAsFile,
             "Save the current document to a new file.", py::call_guard<QtUiThreadCallGuard>())
        .def("undoManager",
             &lc::ui::CadMdiChild::undoManager,
             "Return the document's UndoManager (shared_ptr).", py::call_guard<QtUiThreadCallGuard>())
        .def("viewer",
             &lc::ui::CadMdiChild::viewer,
             py::return_value_policy::reference,
             "Return the LCADViewer widget (Qt-owned).", py::call_guard<QtUiThreadCallGuard>())
        .def("cursor",
             &lc::ui::CadMdiChild::cursor,
             "Return the drawable cursor (shared_ptr).", py::call_guard<QtUiThreadCallGuard>())
        .def("metaInfoManager",
             &lc::ui::CadMdiChild::metaInfoManager,
             "Return the child's MetaInfoManager (shared_ptr).", py::call_guard<QtUiThreadCallGuard>())
        .def("getSnapManager",
             &lc::ui::CadMdiChild::getSnapManager,
             "Return the child's SnapManagerImpl (shared_ptr).", py::call_guard<QtUiThreadCallGuard>())
        .def("getFilename",
             &lc::ui::CadMdiChild::getFilename,
             "Return the persisted filename of this child (empty if "
             "unsaved).", py::call_guard<QtUiThreadCallGuard>())
        .def("activeViewport",
             &lc::ui::CadMdiChild::activeViewport,
             "Return the active viewport (block CSPtr).", py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // Phase 5 PR-5.6 — LCADViewer.  Lua exposes autoScale +
    // setOperationActive + docCanvas; mirror them here.  Qt-owned; raw
    // pointer, no holder.
    // -----------------------------------------------------------------
    QtClass<lc::ui::LCADViewer>(m, "LCADViewer")
        .def("autoScale",
             &lc::ui::LCADViewer::autoScale,
             "Re-fit the viewport to the document's extents.", py::call_guard<QtUiThreadCallGuard>())
        .def("setOperationActive",
             &lc::ui::LCADViewer::setOperationActive,
             py::arg("active"), py::call_guard<QtUiThreadCallGuard>())
        .def("docCanvas",
             &lc::ui::LCADViewer::docCanvas,
             py::return_value_policy::reference,
             "Return the underlying DocumentCanvas (shared_ptr).", py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // Phase 5 PR-5.6 — DocumentCanvas.  Lua exposes autoScale (needs a
    // painter arg it doesn't pass — this is a Lua-side bug we mirror
    // by NOT re-exposing that overload here; the CadMdiChild-level
    // `autoScale` reaches through viewer()->autoScale() which is the
    // painterless path).  We do bind the selection ops that Lua uses
    // for parity.
    // -----------------------------------------------------------------
    py::class_<lc::viewer::DocumentCanvas,
               std::shared_ptr<lc::viewer::DocumentCanvas>>(
        m, "DocumentCanvas")
        .def("selectPoint",
             &lc::viewer::DocumentCanvas::selectPoint,
             py::arg("x"), py::arg("y"), py::call_guard<QtUiThreadCallGuard>())
        .def("removeSelection",
             &lc::viewer::DocumentCanvas::removeSelection, py::call_guard<QtUiThreadCallGuard>())
        .def("inverseSelection",
             &lc::viewer::DocumentCanvas::inverseSelection, py::call_guard<QtUiThreadCallGuard>())
        .def("selectAll",
             &lc::viewer::DocumentCanvas::selectAll, py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // Phase 5 PR-5.6 — Cursor.  Lua exposes just position().  Bound
    // with shared_ptr holder since CadMdiChild returns
    // shared_ptr<drawable::Cursor>.
    // -----------------------------------------------------------------
    py::class_<lc::viewer::drawable::Cursor,
               std::shared_ptr<lc::viewer::drawable::Cursor>>(
        m, "Cursor")
        .def("position",
             &lc::viewer::drawable::Cursor::position,
             "Return the current cursor position (real coordinates).", py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // TempEntities — the drawables::TempEntities container.  Phase 5
    // PR-5.1 fixup — CreateOperations.refreshTempEntity /
    // removeTempEntity call `.addEntity()`/`.removeEntity()`.
    // -----------------------------------------------------------------
    py::class_<lc::viewer::drawable::TempEntities,
               std::shared_ptr<lc::viewer::drawable::TempEntities>>(
        m, "TempEntities")
        .def("addEntity",
             &lc::viewer::drawable::TempEntities::addEntity,
             py::arg("entity"),
             "Add a temporary preview entity.", py::call_guard<QtUiThreadCallGuard>())
        .def("removeEntity",
             &lc::viewer::drawable::TempEntities::removeEntity,
             py::arg("entity"),
             "Remove a temporary preview entity added earlier.", py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // Phase 5 PR-5.6 — SnapManager setters.  Lua exposes just the four
    // setters; Python does the same for parity.  Bound via the
    // SnapManagerImpl_SPtr holder (SnapManagerImpl is stored as
    // shared_ptr throughout the ui tree).
    // -----------------------------------------------------------------
    py::class_<lc::viewer::manager::SnapManagerImpl,
               std::shared_ptr<lc::viewer::manager::SnapManagerImpl>>(
        m, "SnapManager")
        .def("setGridSnappable",
             &lc::viewer::manager::SnapManagerImpl::setGridSnappable,
             py::arg("enabled"), py::call_guard<QtUiThreadCallGuard>())
        .def("setIntersectionSnappable",
             &lc::viewer::manager::SnapManagerImpl
                 ::setIntersectionsSnappable,
             py::arg("enabled"), py::call_guard<QtUiThreadCallGuard>())
        .def("setMiddleSnappable",
             &lc::viewer::manager::SnapManagerImpl::setMiddleSnappable,
             py::arg("enabled"), py::call_guard<QtUiThreadCallGuard>())
        .def("setEntitySnappable",
             &lc::viewer::manager::SnapManagerImpl::setEntitySnappable,
             py::arg("enabled"), py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // Phase 5 PR-5.6 — MetaInfoManager (thin: metaInfo() only, matching
    // Lua's exposed surface at guibridge.cpp:241-243).  Bound with its
    // SPtr holder since CadMdiChild returns
    // `MetaInfoManager_SPtr metaInfoManager() const`.
    // -----------------------------------------------------------------
    py::class_<lc::ui::MetaInfoManager,
               std::shared_ptr<lc::ui::MetaInfoManager>>(
        m, "MetaInfoManager")
        .def("metaInfo",
             &lc::ui::MetaInfoManager::metaInfo,
             "Return the current MetaInfo aggregate "
             "(linePattern/color/lineWidth).", py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // Phase 5 PR-5.6 — Toolbar.  Layout mirrors Lua's binding.
    // addButton takes a ScriptCallback; wrap py::object.
    // -----------------------------------------------------------------
    QtClass<lc::ui::widgets::Toolbar>(m, "Toolbar")
        .def("tabByName",
             &lc::ui::widgets::Toolbar::tabByName,
             py::arg("name"),
             py::return_value_policy::reference, py::call_guard<QtUiThreadCallGuard>())
        .def("removeGroupByName",
             &lc::ui::widgets::Toolbar::removeGroupByName,
             py::arg("group_name"),
             py::arg("tab_name") = "Quick Access", py::call_guard<QtUiThreadCallGuard>())
        .def("updateSnapButtons",
             &lc::ui::widgets::Toolbar::updateSnapButtons,
             py::arg("snap_manager"), py::call_guard<QtUiThreadCallGuard>())
        // addTab has 2 overloads.
        .def("addTab",
             [](lc::ui::widgets::Toolbar& self, const std::string& name)
                 -> lc::ui::api::ToolbarTab* {
                 return self.addTab(name.c_str());
             },
             py::arg("name"),
             py::return_value_policy::reference, py::call_guard<QtUiThreadCallGuard>())
        .def("addTab",
             [](lc::ui::widgets::Toolbar& self,
                lc::ui::api::ToolbarTab* tab) {
                 self.addTab(tab);
             },
             py::arg("tab"), py::call_guard<QtUiThreadCallGuard>())
        // removeTab has 2 overloads (const char*, pointer).
        .def("removeTab",
             [](lc::ui::widgets::Toolbar& self,
                const std::string& name) {
                 self.removeTab(name.c_str());
             },
             py::arg("name"), py::call_guard<QtUiThreadCallGuard>())
        .def("removeTab",
             [](lc::ui::widgets::Toolbar& self,
                lc::ui::api::ToolbarTab* tab) {
                 self.removeTab(tab);
             },
             py::arg("tab"), py::call_guard<QtUiThreadCallGuard>())
        // addButton with Python callback.  Header signature matches
        // Lua's binding shape: (name, icon, group, cb, tooltip).
        .def("addButton",
             [](lc::ui::widgets::Toolbar& self, const std::string& name,
                const std::string& icon, const std::string& group,
                py::object cb, const std::string& tooltip) {
                 self.addButton(
                     name.c_str(), icon.c_str(), group.c_str(),
                     lc::python::makePythonCallback(std::move(cb)),
                     tooltip.c_str());
             },
             py::arg("name"), py::arg("icon"), py::arg("group"),
             py::arg("callback"), py::arg("tooltip") = std::string{}, py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // Phase 5 PR-5.6 — Layers.  Multi-overload addLayer/removeLayer/
    // renameLayer/replaceLayer mirroring Lua's binding.  The overload
    // arities that take a Color are exposed as-is; pybind11 will
    // dispatch by argument types.
    // -----------------------------------------------------------------
    QtClass<lc::ui::widgets::Layers>(m, "Layers")
        .def("setMdiChild",
             &lc::ui::widgets::Layers::setMdiChild,
             py::arg("mdi_child"), py::call_guard<QtUiThreadCallGuard>())
        .def("layerByName",
             &lc::ui::widgets::Layers::layerByName,
             py::arg("name"), py::call_guard<QtUiThreadCallGuard>())
        // addLayer overloads: 7 in the header.  Bind explicit lambdas
        // per arg-shape so pybind11 resolves without ambiguity.
        .def("addLayer",
             [](lc::ui::widgets::Layers& self,
                lc::meta::Layer_CSPtr layer) {
                 self.addLayer(layer);
             },
             py::arg("layer"), py::call_guard<QtUiThreadCallGuard>())
        .def("addLayer",
             [](lc::ui::widgets::Layers& self, const std::string& name)
                 -> lc::meta::Layer_CSPtr {
                 return self.addLayer(name.c_str());
             },
             py::arg("name"), py::call_guard<QtUiThreadCallGuard>())
        .def("addLayer",
             [](lc::ui::widgets::Layers& self, const std::string& name,
                double lineWidth) -> lc::meta::Layer_CSPtr {
                 return self.addLayer(name.c_str(), lineWidth);
             },
             py::arg("name"), py::arg("line_width"), py::call_guard<QtUiThreadCallGuard>())
        .def("addLayer",
             [](lc::ui::widgets::Layers& self, const std::string& name,
                int r, int g, int b) -> lc::meta::Layer_CSPtr {
                 return self.addLayer(name.c_str(), r, g, b);
             },
             py::arg("name"), py::arg("r"), py::arg("g"), py::arg("b"), py::call_guard<QtUiThreadCallGuard>())
        .def("addLayer",
             [](lc::ui::widgets::Layers& self, const std::string& name,
                double lineWidth, int r, int g, int b)
                 -> lc::meta::Layer_CSPtr {
                 return self.addLayer(name.c_str(), lineWidth, r, g, b);
             },
             py::arg("name"), py::arg("line_width"),
             py::arg("r"), py::arg("g"), py::arg("b"), py::call_guard<QtUiThreadCallGuard>())
        .def("addLayer",
             [](lc::ui::widgets::Layers& self, const std::string& name,
                lc::Color color) -> lc::meta::Layer_CSPtr {
                 return self.addLayer(name.c_str(), color);
             },
             py::arg("name"), py::arg("color"), py::call_guard<QtUiThreadCallGuard>())
        .def("addLayer",
             [](lc::ui::widgets::Layers& self, const std::string& name,
                double lineWidth, lc::Color color)
                 -> lc::meta::Layer_CSPtr {
                 return self.addLayer(name.c_str(), lineWidth, color);
             },
             py::arg("name"), py::arg("line_width"), py::arg("color"), py::call_guard<QtUiThreadCallGuard>())
        // removeLayer: 2 overloads.
        .def("removeLayer",
             [](lc::ui::widgets::Layers& self,
                lc::meta::Layer_CSPtr layer) {
                 self.removeLayer(layer);
             },
             py::arg("layer"), py::call_guard<QtUiThreadCallGuard>())
        .def("removeLayer",
             [](lc::ui::widgets::Layers& self, const std::string& name) {
                 self.removeLayer(name.c_str());
             },
             py::arg("name"), py::call_guard<QtUiThreadCallGuard>())
        // renameLayer: 2 overloads.
        .def("renameLayer",
             [](lc::ui::widgets::Layers& self,
                lc::meta::Layer_CSPtr layer,
                const std::string& newName) {
                 self.renameLayer(layer, newName.c_str());
             },
             py::arg("layer"), py::arg("new_name"), py::call_guard<QtUiThreadCallGuard>())
        .def("renameLayer",
             [](lc::ui::widgets::Layers& self,
                const std::string& oldName,
                const std::string& newName) {
                 self.renameLayer(oldName.c_str(), newName.c_str());
             },
             py::arg("old_name"), py::arg("new_name"), py::call_guard<QtUiThreadCallGuard>())
        // replaceLayer: 2 overloads, header name is replaceLayerAPI.
        .def("replaceLayer",
             [](lc::ui::widgets::Layers& self,
                lc::meta::Layer_CSPtr oldLayer,
                lc::meta::Layer_CSPtr newLayer) {
                 self.replaceLayerAPI(oldLayer, newLayer);
             },
             py::arg("old_layer"), py::arg("new_layer"), py::call_guard<QtUiThreadCallGuard>())
        .def("replaceLayer",
             [](lc::ui::widgets::Layers& self,
                const std::string& oldName,
                lc::meta::Layer_CSPtr newLayer) {
                 self.replaceLayerAPI(oldName.c_str(), newLayer);
             },
             py::arg("old_name"), py::arg("new_layer"), py::call_guard<QtUiThreadCallGuard>())
        .def("layers",
             &lc::ui::widgets::Layers::layers,
             "Return the full list of layers.", py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // Phase 5 PR-5.6 — PropertyEditor: the Lua binding only exposes
    // GetPropertyEditor + propertyChanged.  Same surface here.
    // -----------------------------------------------------------------
    QtClass<lc::ui::PropertyEditor>(m, "PropertyEditor")
        // Header allows nullptr but the singleton needs a MainWindow at
        // first call; make the arg mandatory Python-side so callers pass
        // `mainWindow` explicitly.  Lua's binding takes an arg too.
        .def_static("GetPropertyEditor",
             &lc::ui::PropertyEditor::GetPropertyEditor,
             py::arg("main_window"),
             py::return_value_policy::reference,
             "Return the PropertyEditor singleton for the given "
             "MainWindow (creating it if needed).", py::call_guard<QtUiThreadCallGuard>())
        .def("propertyChanged",
             &lc::ui::PropertyEditor::propertyChanged,
             py::arg("key"),
             "Notify the editor that the property under `key` "
             "changed.", py::call_guard<QtUiThreadCallGuard>());

    // =================================================================
    // gui.* widget API — Menu + Toolbar widget family.
    //
    // Phase 5 PR-5.6 sub-piece 2 — mirrors the Lua guibridge.cpp
    // `addLuaGUIAPIBindings` shape.  Under a single embedded module,
    // Lua puts these under `gui.*`; we put them under `lcgui.*` (the
    // module name Python operations import).  All callback-taking
    // methods take a py::object and wrap via makePythonCallback so
    // the neutral ScriptCallback layer picks them up (same wrap-at-
    // adapter contract Lua uses via makeLuaCallback).
    // =================================================================

    // -----------------------------------------------------------------
    // Menu.  Header confirms 3 ctors; we expose the (name) ctor (the
    // only one Lua exposes) — the other two are QMenuBar/QMenu-taking
    // internal ctors, not script-callable.
    // -----------------------------------------------------------------
    QtClass<lc::ui::api::Menu>(m, "Menu")
        .def(py::init([](const std::string& name) {
                 return new lc::ui::api::Menu(name.c_str());
             }),
             py::arg("name"), py::call_guard<QtUiThreadCallGuard>())
        .def("label",       &lc::ui::api::Menu::label, py::call_guard<QtUiThreadCallGuard>())
        .def("setLabel",
             [](lc::ui::api::Menu& self, const std::string& label) {
                 self.setLabel(label.c_str());
             },
             py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        .def("position",    &lc::ui::api::Menu::position, py::call_guard<QtUiThreadCallGuard>())
        .def("setPosition", &lc::ui::api::Menu::setPosition,
             py::arg("position"), py::call_guard<QtUiThreadCallGuard>())
        .def("remove",      &lc::ui::api::Menu::remove, py::call_guard<QtUiThreadCallGuard>())
        .def("itemByName",
             [](lc::ui::api::Menu& self, const std::string& name) {
                 return self.itemByName(name.c_str());
             },
             py::arg("name"),
             py::return_value_policy::reference, py::call_guard<QtUiThreadCallGuard>())
        .def("itemByPosition",
             &lc::ui::api::Menu::itemByPosition,
             py::arg("position"),
             py::return_value_policy::reference, py::call_guard<QtUiThreadCallGuard>())
        .def("menuByName",
             [](lc::ui::api::Menu& self, const std::string& name) {
                 return self.menuByName(name.c_str());
             },
             py::arg("name"),
             py::return_value_policy::reference, py::call_guard<QtUiThreadCallGuard>())
        .def("menuByPosition",
             &lc::ui::api::Menu::menuByPosition,
             py::arg("position"),
             py::return_value_policy::reference, py::call_guard<QtUiThreadCallGuard>())
        // QMenu-inherited hide/show/setEnabled/isEnabled — same pattern
        // Lua uses (lambdas around the Qt-base methods).
        .def("hide", [](lc::ui::api::Menu& self) { self.hide(); }, py::call_guard<QtUiThreadCallGuard>())
        .def("show", [](lc::ui::api::Menu& self) { self.show(); }, py::call_guard<QtUiThreadCallGuard>())
        .def("isEnabled",
             [](lc::ui::api::Menu& self) { return self.isEnabled(); }, py::call_guard<QtUiThreadCallGuard>())
        .def("setEnabled",
             [](lc::ui::api::Menu& self, bool enable) {
                 self.setEnabled(enable);
             },
             py::arg("enabled"), py::call_guard<QtUiThreadCallGuard>())
        // addItem — 3 overloads.  (MenuItem*), (const char*), and
        // (const char*, ScriptCallback).
        .def("addItem",
             [](lc::ui::api::Menu& self, lc::ui::api::MenuItem* item) {
                 self.addItem(item);
             },
             py::arg("item"), py::call_guard<QtUiThreadCallGuard>())
        .def("addItem",
             [](lc::ui::api::Menu& self, const std::string& label) {
                 return self.addItem(label.c_str());
             },
             py::arg("label"),
             py::return_value_policy::reference, py::call_guard<QtUiThreadCallGuard>())
        .def("addItem",
             [](lc::ui::api::Menu& self, const std::string& label,
                py::object cb) {
                 return self.addItem(
                     label.c_str(),
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("label"), py::arg("callback"),
             py::return_value_policy::reference, py::call_guard<QtUiThreadCallGuard>())
        // removeItem: 2 overloads.
        .def("removeItem",
             [](lc::ui::api::Menu& self, lc::ui::api::MenuItem* item) {
                 self.removeItem(item);
             },
             py::arg("item"), py::call_guard<QtUiThreadCallGuard>())
        .def("removeItem",
             [](lc::ui::api::Menu& self, const std::string& label) {
                 self.removeItem(label.c_str());
             },
             py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        // removeMenu: 2 overloads (const char*, Menu*).
        .def("removeMenu",
             [](lc::ui::api::Menu& self, const std::string& label) {
                 self.removeMenu(label.c_str());
             },
             py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        .def("removeMenu",
             [](lc::ui::api::Menu& self, lc::ui::api::Menu* menu) {
                 self.removeMenu(menu);
             },
             py::arg("menu"), py::call_guard<QtUiThreadCallGuard>())
        // addMenu: 2 overloads (const char*, Menu*).
        .def("addMenu",
             [](lc::ui::api::Menu& self, const std::string& label) {
                 return self.addMenu(label.c_str());
             },
             py::arg("label"),
             py::return_value_policy::reference, py::call_guard<QtUiThreadCallGuard>())
        .def("addMenu",
             [](lc::ui::api::Menu& self, lc::ui::api::Menu* menu) {
                 self.addMenu(menu);
             },
             py::arg("menu"), py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // MenuItem.  2 ctors in the header — (name) and (name, callback).
    // Lua exposes the callback-taking one as `MenuItem.new(label, cb)`
    // static factory; mirror that so a Python-side `MenuItem("label")`
    // is the plain ctor and `MenuItem.new("label", callback)` is the
    // callback-attached form.
    // -----------------------------------------------------------------
    QtClass<lc::ui::api::MenuItem>(m, "MenuItem")
        .def(py::init([](const std::string& label) {
                 return new lc::ui::api::MenuItem(label.c_str());
             }),
             py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        .def_static("new",
             [](const std::string& label, py::object cb) {
                 return new lc::ui::api::MenuItem(
                     label.c_str(),
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("label"), py::arg("callback"),
             py::return_value_policy::take_ownership, py::call_guard<QtUiThreadCallGuard>())
        .def("label",       &lc::ui::api::MenuItem::label, py::call_guard<QtUiThreadCallGuard>())
        .def("setLabel",
             [](lc::ui::api::MenuItem& self, const std::string& label) {
                 self.setLabel(label.c_str());
             },
             py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        .def("position",    &lc::ui::api::MenuItem::position, py::call_guard<QtUiThreadCallGuard>())
        .def("setPosition", &lc::ui::api::MenuItem::setPosition,
             py::arg("position"), py::call_guard<QtUiThreadCallGuard>())
        .def("remove",      &lc::ui::api::MenuItem::remove, py::call_guard<QtUiThreadCallGuard>())
        .def("removeCallback",
             [](lc::ui::api::MenuItem& self,
                const std::string& name) {
                 self.removeCallback(name.c_str());
             },
             py::arg("name"), py::call_guard<QtUiThreadCallGuard>())
        .def("setCheckable",
             &lc::ui::api::MenuItem::setCheckable,
             py::arg("checkable"), py::call_guard<QtUiThreadCallGuard>())
        .def("setChecked",
             &lc::ui::api::MenuItem::setChecked,
             py::arg("checked"), py::call_guard<QtUiThreadCallGuard>())
        // QAction-inherited hide/show/isEnabled/setEnabled — Lua
        // mirrors these with lambdas around the Qt-base methods.
        .def("hide", [](lc::ui::api::MenuItem& self) { self.hide(); }, py::call_guard<QtUiThreadCallGuard>())
        .def("show", [](lc::ui::api::MenuItem& self) { self.show(); }, py::call_guard<QtUiThreadCallGuard>())
        .def("isEnabled",
             [](lc::ui::api::MenuItem& self) {
                 return self.isEnabled();
             }, py::call_guard<QtUiThreadCallGuard>())
        .def("setEnabled",
             [](lc::ui::api::MenuItem& self, bool enable) {
                 self.setEnabled(enable);
             },
             py::arg("enabled"), py::call_guard<QtUiThreadCallGuard>())
        // addCheckedCallback — takes ScriptCallback; wrap py::object.
        .def("addCheckedCallback",
             [](lc::ui::api::MenuItem& self, py::object cb) {
                 self.addCheckedCallback(
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("callback"), py::call_guard<QtUiThreadCallGuard>())
        // addCallback: 2 overloads (callback) and (name, callback).
        .def("addCallback",
             [](lc::ui::api::MenuItem& self, py::object cb) {
                 self.addCallback(
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("callback"), py::call_guard<QtUiThreadCallGuard>())
        .def("addCallback",
             [](lc::ui::api::MenuItem& self, const std::string& name,
                py::object cb) {
                 self.addCallback(
                     name.c_str(),
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("name"), py::arg("callback"), py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // ToolbarTab.  Single ctor (name, parent) — Lua exposes just
    // (name).  Mirror that.  addGroup + removeGroup have 2 overloads;
    // bind both.
    // -----------------------------------------------------------------
    QtClass<lc::ui::api::ToolbarTab>(m, "ToolbarTab")
        .def(py::init([](const std::string& name) {
                 return new lc::ui::api::ToolbarTab(name.c_str());
             }),
             py::arg("name"), py::call_guard<QtUiThreadCallGuard>())
        // Header: `void addButton(ToolbarButton*, const char* groupName)`
        // — different from ToolbarGroup::addButton.  This is a
        // convenience that appends into a named group.
        .def("addButton",
             [](lc::ui::api::ToolbarTab& self,
                lc::ui::api::ToolbarButton* button,
                const std::string& groupName) {
                 self.addButton(button, groupName.c_str());
             },
             py::arg("button"), py::arg("group_name"), py::call_guard<QtUiThreadCallGuard>())
        .def("buttonByText",
             [](lc::ui::api::ToolbarTab& self,
                lc::ui::api::ToolbarGroup* group,
                const std::string& text) {
                 return self.buttonByText(group, text.c_str());
             },
             py::arg("group"), py::arg("text"),
             py::return_value_policy::reference, py::call_guard<QtUiThreadCallGuard>())
        .def("groupByName",
             [](lc::ui::api::ToolbarTab& self,
                const std::string& name) {
                 return self.groupByName(name.c_str());
             },
             py::arg("name"),
             py::return_value_policy::reference, py::call_guard<QtUiThreadCallGuard>())
        .def("label",    &lc::ui::api::ToolbarTab::label, py::call_guard<QtUiThreadCallGuard>())
        .def("setLabel",
             [](lc::ui::api::ToolbarTab& self,
                const std::string& label) {
                 self.setLabel(label.c_str());
             },
             py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        .def("groups",   &lc::ui::api::ToolbarTab::groups, py::call_guard<QtUiThreadCallGuard>())
        .def("remove",   &lc::ui::api::ToolbarTab::remove, py::call_guard<QtUiThreadCallGuard>())
        // QWidget-inherited setEnabled — Lua exposes as
        // enable/disable lambdas.
        .def("enable",
             [](lc::ui::api::ToolbarTab& self) {
                 self.setEnabled(true);
             }, py::call_guard<QtUiThreadCallGuard>())
        .def("disable",
             [](lc::ui::api::ToolbarTab& self) {
                 self.setEnabled(false);
             }, py::call_guard<QtUiThreadCallGuard>())
        // removeGroup: 2 overloads (ToolbarGroup*, const char*).
        .def("removeGroup",
             [](lc::ui::api::ToolbarTab& self,
                lc::ui::api::ToolbarGroup* group) {
                 self.removeGroup(group);
             },
             py::arg("group"), py::call_guard<QtUiThreadCallGuard>())
        .def("removeGroup",
             [](lc::ui::api::ToolbarTab& self,
                const std::string& name) {
                 self.removeGroup(name.c_str());
             },
             py::arg("name"), py::call_guard<QtUiThreadCallGuard>())
        // addGroup: 3 overloads (ToolbarGroup*, name+width, name-only).
        .def("addGroup",
             [](lc::ui::api::ToolbarTab& self,
                lc::ui::api::ToolbarGroup* group) {
                 self.addGroup(group);
             },
             py::arg("group"), py::call_guard<QtUiThreadCallGuard>())
        .def("addGroup",
             [](lc::ui::api::ToolbarTab& self,
                const std::string& name, int width) {
                 return self.addGroup(name.c_str(), width);
             },
             py::arg("name"), py::arg("width"),
             py::return_value_policy::reference, py::call_guard<QtUiThreadCallGuard>())
        .def("addGroup",
             [](lc::ui::api::ToolbarTab& self,
                const std::string& name) {
                 return self.addGroup(name.c_str());
             },
             py::arg("name"),
             py::return_value_policy::reference, py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // ToolbarButton.  4 ctors in the header (2 with callback, 2 without)
    // — Lua exposes callback-taking as `.new(label, icon, cb)`,
    // `.newWithTooltip(...)`, `.newCheckable(...)` static factories,
    // and the plain ctors as py::init variants.  Mirror that shape.
    // -----------------------------------------------------------------
    QtClass<lc::ui::api::ToolbarButton>(m, "ToolbarButton")
        // Plain (no-callback) ctors: (label, icon) + (label, icon,
        // tooltip) + (label, icon, tooltip, checkable).  The header
        // packs these as a single ctor with default args — pybind11's
        // py::init<...> with py::arg().default() would work, but for
        // clarity use lambdas per arity.
        .def(py::init([](const std::string& label,
                         const std::string& icon) {
                 return new lc::ui::api::ToolbarButton(
                     label.c_str(), icon.c_str());
             }),
             py::arg("label"), py::arg("icon"), py::call_guard<QtUiThreadCallGuard>())
        .def(py::init([](const std::string& label,
                         const std::string& icon,
                         const std::string& tooltip) {
                 return new lc::ui::api::ToolbarButton(
                     label.c_str(), icon.c_str(), tooltip.c_str());
             }),
             py::arg("label"), py::arg("icon"), py::arg("tooltip"), py::call_guard<QtUiThreadCallGuard>())
        .def(py::init([](const std::string& label,
                         const std::string& icon,
                         const std::string& tooltip,
                         bool checkable) {
                 return new lc::ui::api::ToolbarButton(
                     label.c_str(), icon.c_str(), tooltip.c_str(),
                     checkable);
             }),
             py::arg("label"), py::arg("icon"),
             py::arg("tooltip"), py::arg("checkable"), py::call_guard<QtUiThreadCallGuard>())
        // Callback-taking static factories.
        .def_static("new",
             [](const std::string& label, const std::string& icon,
                py::object cb) {
                 return new lc::ui::api::ToolbarButton(
                     label.c_str(), icon.c_str(),
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("label"), py::arg("icon"), py::arg("callback"),
             py::return_value_policy::take_ownership, py::call_guard<QtUiThreadCallGuard>())
        .def_static("newWithTooltip",
             [](const std::string& label, const std::string& icon,
                py::object cb, const std::string& tooltip) {
                 return new lc::ui::api::ToolbarButton(
                     label.c_str(), icon.c_str(),
                     lc::python::makePythonCallback(std::move(cb)),
                     tooltip.c_str());
             },
             py::arg("label"), py::arg("icon"), py::arg("callback"),
             py::arg("tooltip"),
             py::return_value_policy::take_ownership, py::call_guard<QtUiThreadCallGuard>())
        .def_static("newCheckable",
             [](const std::string& label, const std::string& icon,
                py::object cb, const std::string& tooltip,
                bool checkable) {
                 return new lc::ui::api::ToolbarButton(
                     label.c_str(), icon.c_str(),
                     lc::python::makePythonCallback(std::move(cb)),
                     tooltip.c_str(), checkable);
             },
             py::arg("label"), py::arg("icon"), py::arg("callback"),
             py::arg("tooltip"), py::arg("checkable"),
             py::return_value_policy::take_ownership, py::call_guard<QtUiThreadCallGuard>())
        .def("label",    &lc::ui::api::ToolbarButton::label, py::call_guard<QtUiThreadCallGuard>())
        .def("setLabel",
             [](lc::ui::api::ToolbarButton& self,
                const std::string& label) {
                 self.setLabel(label.c_str());
             },
             py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        .def("setTooltip",
             [](lc::ui::api::ToolbarButton& self,
                const std::string& tooltip) {
                 self.setTooltip(tooltip.c_str());
             },
             py::arg("tooltip"), py::call_guard<QtUiThreadCallGuard>())
        .def("removeCallback",
             [](lc::ui::api::ToolbarButton& self,
                const std::string& name) {
                 self.removeCallback(name.c_str());
             },
             py::arg("name"), py::call_guard<QtUiThreadCallGuard>())
        .def("remove",   &lc::ui::api::ToolbarButton::remove, py::call_guard<QtUiThreadCallGuard>())
        // QPushButton-inherited hide/show/setEnabled/isEnabled.
        .def("hide",
             [](lc::ui::api::ToolbarButton& self) { self.hide(); }, py::call_guard<QtUiThreadCallGuard>())
        .def("show",
             [](lc::ui::api::ToolbarButton& self) { self.show(); }, py::call_guard<QtUiThreadCallGuard>())
        .def("isEnabled",
             [](lc::ui::api::ToolbarButton& self) {
                 return self.isEnabled();
             }, py::call_guard<QtUiThreadCallGuard>())
        .def("enable",
             [](lc::ui::api::ToolbarButton& self) {
                 self.setEnabled(true);
             }, py::call_guard<QtUiThreadCallGuard>())
        .def("disable",
             [](lc::ui::api::ToolbarButton& self) {
                 self.setEnabled(false);
             }, py::call_guard<QtUiThreadCallGuard>())
        // addCallback: 2 overloads (callback) and (name, callback).
        .def("addCallback",
             [](lc::ui::api::ToolbarButton& self, py::object cb) {
                 self.addCallback(
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("callback"), py::call_guard<QtUiThreadCallGuard>())
        .def("addCallback",
             [](lc::ui::api::ToolbarButton& self,
                const std::string& name, py::object cb) {
                 self.addCallback(
                     name.c_str(),
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("name"), py::arg("callback"), py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // ToolbarGroup.  Ctor (name, width=3).  addButton has 3 overloads;
    // Lua's binding mirrors all 3.
    // -----------------------------------------------------------------
    QtClass<lc::ui::api::ToolbarGroup>(m, "ToolbarGroup")
        .def(py::init([](const std::string& name) {
                 return new lc::ui::api::ToolbarGroup(name.c_str());
             }),
             py::arg("name"), py::call_guard<QtUiThreadCallGuard>())
        .def(py::init([](const std::string& name, int width) {
                 return new lc::ui::api::ToolbarGroup(
                     name.c_str(), width);
             }),
             py::arg("name"), py::arg("width"), py::call_guard<QtUiThreadCallGuard>())
        .def("label",    &lc::ui::api::ToolbarGroup::label, py::call_guard<QtUiThreadCallGuard>())
        .def("setLabel",
             [](lc::ui::api::ToolbarGroup& self,
                const std::string& label) {
                 self.setLabel(label.c_str());
             },
             py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        .def("buttonByName",
             [](lc::ui::api::ToolbarGroup& self,
                const std::string& name) {
                 return self.buttonByName(name.c_str());
             },
             py::arg("name"),
             py::return_value_policy::reference, py::call_guard<QtUiThreadCallGuard>())
        .def("buttons",  &lc::ui::api::ToolbarGroup::buttons, py::call_guard<QtUiThreadCallGuard>())
        .def("remove",   &lc::ui::api::ToolbarGroup::remove, py::call_guard<QtUiThreadCallGuard>())
        .def("setWidth", &lc::ui::api::ToolbarGroup::setWidth,
             py::arg("width"), py::call_guard<QtUiThreadCallGuard>())
        .def("width",    &lc::ui::api::ToolbarGroup::width, py::call_guard<QtUiThreadCallGuard>())
        // QGroupBox-inherited hide/show/setEnabled/enabled — enable/
        // disable lambdas per Lua's mirror.
        .def("hide",
             [](lc::ui::api::ToolbarGroup& self) { self.hide(); }, py::call_guard<QtUiThreadCallGuard>())
        .def("show",
             [](lc::ui::api::ToolbarGroup& self) { self.show(); }, py::call_guard<QtUiThreadCallGuard>())
        .def("enable",
             [](lc::ui::api::ToolbarGroup& self) {
                 self.setEnabled(true);
             }, py::call_guard<QtUiThreadCallGuard>())
        .def("disable",
             [](lc::ui::api::ToolbarGroup& self) {
                 self.setEnabled(false);
             }, py::call_guard<QtUiThreadCallGuard>())
        // addButton: 3 overloads (button, name+icon, name+icon+callback).
        .def("addButton",
             [](lc::ui::api::ToolbarGroup& self,
                lc::ui::api::ToolbarButton* button) {
                 self.addButton(button);
             },
             py::arg("button"), py::call_guard<QtUiThreadCallGuard>())
        .def("addButton",
             [](lc::ui::api::ToolbarGroup& self,
                const std::string& name, const std::string& icon) {
                 return self.addButton(name.c_str(), icon.c_str());
             },
             py::arg("name"), py::arg("icon"),
             py::return_value_policy::reference, py::call_guard<QtUiThreadCallGuard>())
        .def("addButton",
             [](lc::ui::api::ToolbarGroup& self,
                const std::string& name, const std::string& icon,
                py::object cb) {
                 return self.addButton(
                     name.c_str(), icon.c_str(),
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("name"), py::arg("icon"), py::arg("callback"),
             py::return_value_policy::reference, py::call_guard<QtUiThreadCallGuard>())
        // removeButton: 2 overloads (ToolbarButton*, const char*).
        .def("removeButton",
             [](lc::ui::api::ToolbarGroup& self,
                lc::ui::api::ToolbarButton* button) {
                 self.removeButton(button);
             },
             py::arg("button"), py::call_guard<QtUiThreadCallGuard>())
        .def("removeButton",
             [](lc::ui::api::ToolbarGroup& self,
                const std::string& name) {
                 self.removeButton(name.c_str());
             },
             py::arg("name"), py::call_guard<QtUiThreadCallGuard>());

    // =================================================================
    // gui.* widget API — Input widget family.
    //
    // Phase 5 PR-5.6 sub-piece 3.  InputGUIContainer (base for
    // DialogWidget + PropertyEditor) + InputGUI (base for the 13 input
    // widget subclasses) + the widget subclasses under their
    // Lua-exposed names (Text/Button/CheckBox/RadioButton/
    // HorizontalGroup/RadioGroup/Coordinate/Angle/Slider/ComboBox/
    // Number/ColorPicker=ColorGUI/EntityPicker=EntityGUI/List).
    //
    // gui.List has NO addCallbackToAll binding — Lua's exposed surface
    // deliberately excludes it (see phase 3 sub-plan) and Python mirrors
    // that decision.  LineSelectGUI and LWVertexGroup are
    // PropertyEditor-internal and NEVER script-created; not bound
    // (same as Lua).
    // =================================================================

    // -----------------------------------------------------------------
    // InputGUIContainer — the base for DialogWidget + PropertyEditor.
    // Lua exposes inputWidgets/addFinishCallback/keys/addWidget.  Mirror
    // that surface.
    // -----------------------------------------------------------------
    QtClass<lc::ui::api::InputGUIContainer>(m, "InputGUIContainer")
        .def("inputWidgets",
             &lc::ui::api::InputGUIContainer::inputWidgets,
             "Return the list of all child InputGUI widgets.", py::call_guard<QtUiThreadCallGuard>())
        .def("addFinishCallback",
             [](lc::ui::api::InputGUIContainer& self, py::object cb) {
                 self.addFinishCallback(
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("callback"),
             "Register a Python callable fired when the container "
             "finishes (e.g. DialogWidget accept).", py::call_guard<QtUiThreadCallGuard>())
        .def("keys",
             &lc::ui::api::InputGUIContainer::keys,
             "Return the list of registered widget keys.", py::call_guard<QtUiThreadCallGuard>())
        .def("addWidget",
             &lc::ui::api::InputGUIContainer::addWidget,
             py::arg("key"), py::arg("widget"),
             "Add an input widget under `key`; returns whether the "
             "add succeeded (key must be unique).", py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // DialogWidget — subclasses QDialog + InputGUIContainer.  Ctor
    // takes (title, MainWindow*).  Lua exposes setFinishButton/
    // enable/disable + addWidget override.
    // -----------------------------------------------------------------
    QtClass<lc::ui::api::DialogWidget,
               lc::ui::api::InputGUIContainer>(m, "DialogWidget")
        .def(py::init([](const std::string& title, lc::ui::MainWindow* owner) {
                 return new lc::ui::api::DialogWidget(title, owner, owner);
             }),
             py::arg("title"), py::arg("main_window"), py::call_guard<QtUiThreadCallGuard>())
        .def("setFinishButton",
             &lc::ui::api::DialogWidget::setFinishButton,
             py::arg("button"),
             "Wire a ButtonGUI as the widget that fires "
             "finishCallbacks on click.", py::call_guard<QtUiThreadCallGuard>())
        .def("enable",
             [](lc::ui::api::DialogWidget& self) {
                 self.setEnabled(true);
             }, py::call_guard<QtUiThreadCallGuard>())
        .def("disable",
             [](lc::ui::api::DialogWidget& self) {
                 self.setEnabled(false);
             }, py::call_guard<QtUiThreadCallGuard>())
        // Bind the shared container surface against DialogWidget directly.
        // This avoids relying on inherited-member dispatch across the
        // QDialog + InputGUIContainer multiple-inheritance layout.
        .def("inputWidgets",
             [](lc::ui::api::DialogWidget& self) {
                 return self.inputWidgets();
             },
             "Return the list of all child InputGUI widgets.", py::call_guard<QtUiThreadCallGuard>())
        .def("keys",
             [](lc::ui::api::DialogWidget& self) {
                 return self.keys();
             },
             "Return the list of registered widget keys.", py::call_guard<QtUiThreadCallGuard>())
        .def("addFinishCallback",
             [](lc::ui::api::DialogWidget& self, py::object cb) {
                 self.addFinishCallback(
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("callback"), py::call_guard<QtUiThreadCallGuard>())
        .def("addWidget",
             &lc::ui::api::DialogWidget::addWidget,
             py::arg("key"), py::arg("widget"),
             "Add an input widget under `key`.", py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // InputGUI — the base for the widget subclasses below.  Bound as
    // a class so subclass upcasts work (needed by
    // HorizontalGroupGUI::addWidget which takes QWidget* — pybind11
    // passes InputGUI* implicitly).
    // -----------------------------------------------------------------
    QtClass<lc::ui::api::InputGUI>(m, "InputGUI")
        .def("label", &lc::ui::api::InputGUI::label, py::call_guard<QtUiThreadCallGuard>())
        .def("setLabel",
             &lc::ui::api::InputGUI::setLabel,
             py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        .def("key",   &lc::ui::api::InputGUI::key, py::call_guard<QtUiThreadCallGuard>())
        .def("enable",
             [](lc::ui::api::InputGUI& self) {
                 self.setEnabled(true);
             }, py::call_guard<QtUiThreadCallGuard>())
        .def("disable",
             [](lc::ui::api::InputGUI& self) {
                 self.setEnabled(false);
             }, py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // TextGUI (bound as `Text` per Lua's exposure).  Ctor: (label).
    // -----------------------------------------------------------------
    QtClass<lc::ui::api::TextGUI, lc::ui::api::InputGUI>(m, "Text")
        .def(py::init<std::string>(), py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        .def("value",    &lc::ui::api::TextGUI::value, py::call_guard<QtUiThreadCallGuard>())
        .def("setValue",
             &lc::ui::api::TextGUI::setValue,
             py::arg("value"), py::call_guard<QtUiThreadCallGuard>())
        .def("addFinishCallback",
             [](lc::ui::api::TextGUI& self, py::object cb) {
                 self.addFinishCallback(
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("callback"), py::call_guard<QtUiThreadCallGuard>())
        .def("addOnChangeCallback",
             [](lc::ui::api::TextGUI& self, py::object cb) {
                 self.addOnChangeCallback(
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("callback"), py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // ButtonGUI (bound as `Button`).  Ctor: (label).
    // -----------------------------------------------------------------
    QtClass<lc::ui::api::ButtonGUI, lc::ui::api::InputGUI>(m, "Button")
        .def(py::init<std::string>(), py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        .def("addCallback",
             [](lc::ui::api::ButtonGUI& self, py::object cb) {
                 self.addCallback(
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("callback"), py::call_guard<QtUiThreadCallGuard>())
        .def("click",   &lc::ui::api::ButtonGUI::click, py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // CheckBoxGUI (bound as `CheckBox`).  Header ctor:
    // `CheckBoxGUI(std::string label, bool checked = false)` — 1 ctor
    // with 1 default arg.  Bind with py::arg default so Python callers
    // can use CheckBox("Label") or CheckBox("Label", True).
    // -----------------------------------------------------------------
    QtClass<lc::ui::api::CheckBoxGUI,
               lc::ui::api::InputGUI>(m, "CheckBox")
        .def(py::init<std::string, bool>(),
             py::arg("label"), py::arg("checked") = false, py::call_guard<QtUiThreadCallGuard>())
        .def("addCallback",
             [](lc::ui::api::CheckBoxGUI& self, py::object cb) {
                 self.addCallback(
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("callback"), py::call_guard<QtUiThreadCallGuard>())
        .def("value",    &lc::ui::api::CheckBoxGUI::value, py::call_guard<QtUiThreadCallGuard>())
        .def("setValue",
             &lc::ui::api::CheckBoxGUI::setValue,
             py::arg("checked"), py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // RadioButtonGUI (bound as `RadioButton`).  Ctor: (label).  Does
    // NOT inherit InputGUI (per header comment) — bound standalone.
    // Lua binds with method names `checked`/`setChecked` but the header
    // has `value()`/`setValue()`.  Expose the header names for
    // correctness; Python callers see the actual .h signature.
    // -----------------------------------------------------------------
    QtClass<lc::ui::api::RadioButtonGUI>(m, "RadioButton")
        .def(py::init<std::string>(), py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        .def("label",    &lc::ui::api::RadioButtonGUI::label, py::call_guard<QtUiThreadCallGuard>())
        .def("setLabel",
             &lc::ui::api::RadioButtonGUI::setLabel,
             py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        .def("addCallback",
             [](lc::ui::api::RadioButtonGUI& self, py::object cb) {
                 self.addCallback(
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("callback"), py::call_guard<QtUiThreadCallGuard>())
        .def("value",    &lc::ui::api::RadioButtonGUI::value, py::call_guard<QtUiThreadCallGuard>())
        .def("setValue",
             &lc::ui::api::RadioButtonGUI::setValue,
             py::arg("toggle"), py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // HorizontalGroupGUI (bound as `HorizontalGroup`).  Header:
    // `HorizontalGroupGUI(std::string, bool vertical=false)`.  addWidget
    // takes (key, QWidget*).  We expose an InputGUI* overload only —
    // ButtonGUI/CheckBoxGUI upcast automatically via pybind11's
    // class hierarchy.
    // -----------------------------------------------------------------
    QtClass<lc::ui::api::HorizontalGroupGUI,
               lc::ui::api::InputGUI>(m, "HorizontalGroup")
        .def(py::init<std::string, bool>(),
             py::arg("label"), py::arg("vertical") = false, py::call_guard<QtUiThreadCallGuard>())
        .def("addWidget",
             [](lc::ui::api::HorizontalGroupGUI& self,
                const std::string& key,
                lc::ui::api::InputGUI* widget) {
                 self.addWidget(key, widget);
             },
             py::arg("key"), py::arg("widget"), py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // RadioGroupGUI (bound as `RadioGroup`).  Ctor: (label).
    // addButton(key, RadioButtonGUI*).
    // -----------------------------------------------------------------
    QtClass<lc::ui::api::RadioGroupGUI,
               lc::ui::api::InputGUI>(m, "RadioGroup")
        .def(py::init<std::string>(), py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        .def("addButton",
             &lc::ui::api::RadioGroupGUI::addButton,
             py::arg("key"), py::arg("radio_button"), py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // CoordinateGUI (bound as `Coordinate`).  Ctor: (label).  value()
    // returns lc::geo::Coordinate — kernel type registered by lc.
    // -----------------------------------------------------------------
    QtClass<lc::ui::api::CoordinateGUI,
               lc::ui::api::InputGUI>(m, "Coordinate")
        .def(py::init<std::string>(), py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        .def("addFinishCallback",
             [](lc::ui::api::CoordinateGUI& self, py::object cb) {
                 self.addFinishCallback(
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("callback"), py::call_guard<QtUiThreadCallGuard>())
        .def("addOnChangeCallback",
             [](lc::ui::api::CoordinateGUI& self, py::object cb) {
                 self.addOnChangeCallback(
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("callback"), py::call_guard<QtUiThreadCallGuard>())
        .def("value",    &lc::ui::api::CoordinateGUI::value, py::call_guard<QtUiThreadCallGuard>())
        .def("setValue",
             &lc::ui::api::CoordinateGUI::setValue,
             py::arg("coordinate"), py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // AngleGUI (bound as `Angle`).  Ctor: (label).
    // -----------------------------------------------------------------
    QtClass<lc::ui::api::AngleGUI,
               lc::ui::api::InputGUI>(m, "Angle")
        .def(py::init<std::string>(), py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        .def("toDegrees", &lc::ui::api::AngleGUI::toDegrees, py::call_guard<QtUiThreadCallGuard>())
        .def("toRadians", &lc::ui::api::AngleGUI::toRadians, py::call_guard<QtUiThreadCallGuard>())
        .def("addFinishCallback",
             [](lc::ui::api::AngleGUI& self, py::object cb) {
                 self.addFinishCallback(
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("callback"), py::call_guard<QtUiThreadCallGuard>())
        .def("addOnChangeCallback",
             [](lc::ui::api::AngleGUI& self, py::object cb) {
                 self.addOnChangeCallback(
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("callback"), py::call_guard<QtUiThreadCallGuard>())
        .def("value",    &lc::ui::api::AngleGUI::value, py::call_guard<QtUiThreadCallGuard>())
        .def("setValue",
             &lc::ui::api::AngleGUI::setValue,
             py::arg("value"), py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // SliderGUI (bound as `Slider`).  Header ctor:
    // `SliderGUI(std::string, int setMin=0, int setMax=100)`.  Expose
    // both arities.
    // -----------------------------------------------------------------
    QtClass<lc::ui::api::SliderGUI,
               lc::ui::api::InputGUI>(m, "Slider")
        .def(py::init<std::string>(), py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        .def(py::init<std::string, int, int>(),
             py::arg("label"), py::arg("min"), py::arg("max"), py::call_guard<QtUiThreadCallGuard>())
        .def("addCallback",
             [](lc::ui::api::SliderGUI& self, py::object cb) {
                 self.addCallback(
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("callback"), py::call_guard<QtUiThreadCallGuard>())
        .def("value",    &lc::ui::api::SliderGUI::value, py::call_guard<QtUiThreadCallGuard>())
        .def("setValue",
             &lc::ui::api::SliderGUI::setValue,
             py::arg("value"), py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // ComboBoxGUI (bound as `ComboBox`).  Ctor: (label).  addItem has
    // an index default; setValue has 2 overloads (string / int).
    // -----------------------------------------------------------------
    QtClass<lc::ui::api::ComboBoxGUI,
               lc::ui::api::InputGUI>(m, "ComboBox")
        .def(py::init<std::string>(), py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        .def("addCallback",
             [](lc::ui::api::ComboBoxGUI& self, py::object cb) {
                 self.addCallback(
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("callback"), py::call_guard<QtUiThreadCallGuard>())
        .def("addItem",
             &lc::ui::api::ComboBoxGUI::addItem,
             py::arg("item"), py::arg("index") = -1, py::call_guard<QtUiThreadCallGuard>())
        .def("value",    &lc::ui::api::ComboBoxGUI::value, py::call_guard<QtUiThreadCallGuard>())
        // setValue: 2 overloads (string / int).
        .def("setValue",
             [](lc::ui::api::ComboBoxGUI& self,
                const std::string& value) {
                 self.setValue(value);
             },
             py::arg("value"), py::call_guard<QtUiThreadCallGuard>())
        .def("setValue",
             [](lc::ui::api::ComboBoxGUI& self, int index) {
                 self.setValue(index);
             },
             py::arg("index"), py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // NumberGUI (bound as `Number`).  Header ctor:
    // `NumberGUI(std::string, double minVal=-10000, double maxVal=10000)`.
    // Lua only exposes the (label) form; we mirror that.
    // -----------------------------------------------------------------
    QtClass<lc::ui::api::NumberGUI,
               lc::ui::api::InputGUI>(m, "Number")
        .def(py::init<std::string>(), py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        .def(py::init<std::string, double, double>(),
             py::arg("label"), py::arg("min"), py::arg("max"), py::call_guard<QtUiThreadCallGuard>())
        .def("addCallback",
             [](lc::ui::api::NumberGUI& self, py::object cb) {
                 self.addCallback(
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("callback"), py::call_guard<QtUiThreadCallGuard>())
        .def("value",    &lc::ui::api::NumberGUI::value, py::call_guard<QtUiThreadCallGuard>())
        .def("setValue",
             &lc::ui::api::NumberGUI::setValue,
             py::arg("value"), py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // ColorGUI (bound as `ColorPicker`).  Ctor: (label).
    // value()/setValue() take lc::Color.
    // -----------------------------------------------------------------
    QtClass<lc::ui::api::ColorGUI,
               lc::ui::api::InputGUI>(m, "ColorPicker")
        .def(py::init<std::string>(), py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        .def("value",    &lc::ui::api::ColorGUI::value, py::call_guard<QtUiThreadCallGuard>())
        .def("setValue",
             &lc::ui::api::ColorGUI::setValue,
             py::arg("color"), py::call_guard<QtUiThreadCallGuard>())
        .def("addCallback",
             [](lc::ui::api::ColorGUI& self, py::object cb) {
                 self.addCallback(
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("callback"), py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // EntityGUI (bound as `EntityPicker`).  Ctor: (label).  value()
    // returns vector<CADEntity_CSPtr>.
    // -----------------------------------------------------------------
    QtClass<lc::ui::api::EntityGUI,
               lc::ui::api::InputGUI>(m, "EntityPicker")
        .def(py::init<std::string>(), py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        .def("value",    &lc::ui::api::EntityGUI::value, py::call_guard<QtUiThreadCallGuard>())
        .def("setValue",
             &lc::ui::api::EntityGUI::setValue,
             py::arg("entities"), py::call_guard<QtUiThreadCallGuard>())
        .def("addEntity",
             &lc::ui::api::EntityGUI::addEntity,
             py::arg("entity"), py::call_guard<QtUiThreadCallGuard>())
        .def("addCallback",
             [](lc::ui::api::EntityGUI& self, py::object cb) {
                 self.addCallback(
                     lc::python::makePythonCallback(std::move(cb)));
             },
             py::arg("callback"), py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // ListGUI (bound as `List`).  Header ctor:
    // `ListGUI(std::string, ListType=NONE)`.  Lua exposes JUST (label);
    // mirror that + the (label, string-list-type) overload for feature
    // parity with Lua's `setListType(string)`.  addItem takes an
    // InputGUI*.  DELIBERATELY NO addCallbackToAll (Lua omits it too).
    // -----------------------------------------------------------------
    QtClass<lc::ui::api::ListGUI,
               lc::ui::api::InputGUI>(m, "List")
        .def(py::init<std::string>(), py::arg("label"), py::call_guard<QtUiThreadCallGuard>())
        .def("addItem",
             &lc::ui::api::ListGUI::addItem,
             py::arg("key"), py::arg("widget"), py::call_guard<QtUiThreadCallGuard>())
        .def("setListType",
             [](lc::ui::api::ListGUI& self,
                const std::string& listTypeStr) {
                 self.setListType(listTypeStr);
             },
             py::arg("list_type"),
             "Set the list type by name: `NONE`, `COORDINATE`, or "
             "`LW_VERTEX`.", py::call_guard<QtUiThreadCallGuard>());

    // -----------------------------------------------------------------
    // Module-level currentMainWindow() — Phase 5 PR-5.1 fixup: what
    // Python operation base classes call INSTEAD of frame-walking to
    // reach their owner. Delegates to the C++ execution scope in
    // pyeventhooks.cpp, which is valid during construction and nested
    // callbacks as well as ordinary operation dispatch.
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
          "Return the MainWindow that owns the current UI execution scope "
          "(raw pointer, Qt-owned). Returns None when no UI owner is active.",
          py::call_guard<QtUiThreadCallGuard>());
}

#endif  // LC_WITH_PYTHONSCRIPT
