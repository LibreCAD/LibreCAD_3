// Phase 3 PR-3.4 — dual-language ScriptDock acceptance test.
//
// Runs the SAME scenario (create a line in the child document, then
// count line entities) via both language legs of ScriptDock.  The Lua
// leg is the stated verification that Lua console behavior is
// unchanged from the pre-refactor LuaScript widget — no pre-existing
// LuaScript-widget test exists, so this new test is the guard.
//
// The Python leg is the phase-3 acceptance test: the sub-plan's
// "user opens Script dock, selects Python, runs a script that draws
// into the active document; undo works" bullet, boiled down to a
// unit-testable assertion.  Behind #ifdef LC_WITH_PYTHONSCRIPT so an
// OFF build (Windows/embed CPython) doesn't compile-fail here.

#include <QApplication>
#include <QComboBox>
#include <QTextEdit>
#include <gtest/gtest.h>

#include <string>
#include <thread>

#include <mainwindow.h>
#include <widgets/guiAPI/menu.h>
#include <widgets/scriptdock.h>
#include <cad/storage/document.h>

#ifdef LC_WITH_PYTHONSCRIPT
#include <qt_keywords_push.h>
#include <pybind11/embed.h>
#include <qt_keywords_pop.h>
#include <lcpython.h>
#include <python/pyeventhooks.h>
#include <scriptadapter/pythoncallback.h>
#include <scriptadapter/pythonobject.h>
#include <lcscripting/scriptvalue.h>
#include <luainterface.h>
#include <lua/opaquetags.h>
#endif

#include "uitests.h"

using namespace lc::ui::widgets;

namespace {

// Count how many CADEntity_CSPtrs a document holds via the
// EntityContainer's asVector() snapshot (there's no direct size() at
// the current EntityContainer API surface).  Deep scan (levels = max).
std::size_t entityCount(const std::shared_ptr<lc::storage::Document>& doc) {
    if (!doc) return 0;
    return doc->entityContainer().asVector().size();
}

} // namespace

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(ScriptDockTest, LuaCreatesLine) {
    // Baseline: verify Lua console behavior is unchanged from the
    // pre-refactor LuaScript widget.  Emit a Lua script that creates
    // a single Line entity via lc.builder.LineBuilder + the operation
    // builder, count entities, expect 1.  Lua has no lc.entity.Line
    // constructor (entities are built through builders, as
    // lineoperations.lua does): calling the class falls through to
    // lc.entity.ID's and hands appendEntity an ID, not an entity.
    ensureUiApplication();
    lc::ui::MainWindow mainWindow;
    ScriptDock dock(&mainWindow);
    ASSERT_EQ(entityCount(mainWindow.cadMdiChild()->document()), 0u);

    dock.setInputText(QStringLiteral(R"lua(
        local start_pt = lc.geo.Coordinate(0, 0, 0)
        local end_pt = lc.geo.Coordinate(10, 0, 0)
        local layer = mainWindow:cadMdiChild():activeLayer()
        local lineBuilder = lc.builder.LineBuilder()
        lineBuilder:setStartPoint(start_pt)
        lineBuilder:setEndPoint(end_pt)
        lineBuilder:setLayer(layer)
        local line = lineBuilder:build()
        local builder = lc.operation.EntityBuilder(mainWindow:cadMdiChild():document())
        builder:appendEntity(line)
        builder:execute()
    )lua"));
    dock.run();

    EXPECT_EQ(entityCount(mainWindow.cadMdiChild()->document()), 1u)
        << "Lua leg of ScriptDock must create exactly one Line entity";
}

#ifdef LC_WITH_PYTHONSCRIPT

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(ScriptDockTest, PythonCreatesLine) {
    // Same scenario in Python — proves the Python leg has a functional
    // path from ScriptDock's `runPython` through lcgui's mainWindow/
    // document injection and lc's kernel bindings.
    ensureUiApplication();
    lc::ui::MainWindow mainWindow;
    ScriptDock dock(&mainWindow);
    ASSERT_EQ(entityCount(mainWindow.cadMdiChild()->document()), 0u);

    // Switch the language combo to Python.  The combo is exposed by
    // Ui::ScriptDock's `ui->language` — we can't reach it directly from
    // outside, so drive it via a Qt search over child widgets.
    QComboBox* combo = dock.findChild<QComboBox*>(QStringLiteral("language"));
    ASSERT_NE(combo, nullptr) << "ScriptDock must expose a `language` combo";
    // Index 1 = Python (Lua is 0; Python only present when built with
    // LC_WITH_PYTHONSCRIPT).
    combo->setCurrentIndex(1);

    dock.setInputText(QStringLiteral(R"py(
start_pt = lc.geo.Coordinate(0, 0, 0)
end_pt   = lc.geo.Coordinate(10, 0, 0)
# Reach layer through the (holder-less) CadMdiChild binding — same
# call shape as the Lua sibling test but with Python attr syntax.
mdi      = mainWindow.cadMdiChild()
layer    = mdi.activeLayer()
line_builder = lc.builder.LineBuilder()
line_builder.setStartPoint(start_pt)
line_builder.setEndPoint(end_pt)
line_builder.setLayer(layer)
line     = line_builder.build()
builder  = lc.operation.EntityBuilder.new(mdi.document())
builder.appendEntity(line)
builder.execute()
)py"));
    dock.run();

    QTextEdit* cliHistory = mainWindow.cliCommand()->findChild<QTextEdit*>(
        QStringLiteral("history"));
    ASSERT_NE(cliHistory, nullptr);
    EXPECT_EQ(entityCount(mainWindow.cadMdiChild()->document()), 1u)
        << "Python leg of ScriptDock must create exactly one Line entity "
           "— proves the mainWindow/document injection + lcgui bindings "
           "reach the kernel correctly. ScriptDock output: "
        << cliHistory->toPlainText().toStdString();
}


// NOLINTNEXTLINE(readability-identifier-naming)
TEST(ScriptDockTest, PythonEventDrivenOnEventReachesMainWindow) {
    // Phase 5 PR-5.1 fixup — the coordinator's review flagged that the
    // ORIGINAL PR-5.1 tests only exercised the top-level `exec()` path
    // (which does have `mainWindow` in scope via the ScriptDock
    // injection).  Nothing verified the ACTUAL event-driven path:
    // Qt slot → EventBus → PythonCallbackImpl::invokeEvent → onEvent →
    // `_get_main_window()`.  Under the old frame-walking
    // implementation, `_get_main_window()` returned None in this
    // path — every CreateOperations subclass silently no-op'd on
    // real events.  This test exercises that exact path with a
    // stand-in listener class that records what onEvent saw.
    //
    // Setup: construct a MainWindow (which installs the event hooks),
    // register a Python listener via `lc.event.register("point",
    // self)`, trigger the event through `LuaInterface::triggerEvent`
    // (mirroring what MainWindow's `triggerCoordinateEntered` slot
    // does), and verify:
    //   1. The Python listener's onEvent fired at all.
    //   2. Inside onEvent, `_get_main_window()`'s underlying call
    //      (`lcgui.currentMainWindow()`) returned a non-None
    //      MainWindow (proves the C++-side lookup works from a
    //      C++-invoked callback path with no Python caller frame).
    //
    // A directly-constructed MainWindow is deliberately not inserted in
    // WindowManager. Bind the explicit owner scope used by application
    // entrypoints instead of relying on global insertion order.
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    lc::ui::MainWindow mainWindow;
    lc::ui::python::MainWindowExecutionScope ownerScope(&mainWindow);

    // Set up the Python-side listener + record buffer.
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::module_::import("builtins").attr("_lc_event_test_state") =
            pybind11::dict();
        pybind11::dict ns;
        ns["__builtins__"] = pybind11::module_::import("builtins");
        pybind11::exec(R"py(
import builtins
import lc
import lcgui

class Listener:
    def __init__(self):
        self.saw_event = False
        self.mw_from_onEvent = None
        self.widget = None
    def onEvent(self, event_name, args):
        # THIS is the failure mode the fixup addresses.  Under the
        # broken frame-walking implementation, lcgui.currentMainWindow()
        # would work but a manual sys._getframe walk would NOT find
        # `mainWindow`.  We call the same function CreateOperations
        # now calls internally.
        self.saw_event = True
        self.mw_from_onEvent = lcgui.currentMainWindow()
        self.widget = args["widget"]

_listener = Listener()
builtins._lc_event_test_state["listener"] = _listener
lc.event.register("point", _listener)
)py",
            ns);
    }

    // Fire the event through the same path MainWindow's trigger* slots
    // use: LuaInterface::triggerEvent(string, ScriptValue).  The
    // event bus routes to the Python listener's PythonCallbackImpl,
    // whose invokeEvent shape-checks for onEvent and fires it fresh
    // from C++ — exactly the path _get_main_window() has to work in.
    auto payload = lc::scripting::makeMap();
    (*payload)["widget"] = lc::scripting::ScriptValue(
        lc::scripting::OpaquePtr{mainWindow.cadMdiChild(),
                                 lc::ui::opaquetag::CadMdiChild});
    mainWindow.luaInterface()->triggerEvent(
        "point", lc::scripting::ScriptValue(payload));

    // Verify: onEvent fired AND saw a non-None MainWindow.
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::object listener =
            pybind11::module_::import("builtins")
                .attr("_lc_event_test_state")["listener"];
        EXPECT_TRUE(listener.attr("saw_event").cast<bool>())
            << "onEvent must fire when the event is triggered — proves "
               "lc.event.register wired the callback to the EventBus";
        pybind11::object mw = listener.attr("mw_from_onEvent");
        EXPECT_FALSE(mw.is_none())
            << "lcgui.currentMainWindow() called from onEvent must "
               "return the active MainWindow — the frame-walking "
               "implementation returned None here, breaking every "
               "CreateOperations subclass on every real event";

        pybind11::object widget = listener.attr("widget");
        pybind11::object lcgui = pybind11::module_::import("lcgui");
        EXPECT_TRUE(pybind11::isinstance(widget, lcgui.attr("CadMdiChild")))
            << "CadMdiChild OpaquePtr must materialize as the holder-less "
               "lcgui type, not silently become None";
        EXPECT_FALSE(widget.attr("document")().is_none());

        lc::ui::api::Menu contextMenu("Python opaque event menu");
        pybind11::module_ builtins = pybind11::module_::import("builtins");
        builtins.attr("_lc_opaque_menu_type") = pybind11::none();
        pybind11::object menuCallbackFn = pybind11::eval(
            "lambda event, args: setattr(__import__('builtins'), "
            "'_lc_opaque_menu_type', type(args['widget']).__name__)");
        auto menuCallback =
            lc::python::makePythonCallback(std::move(menuCallbackFn));
        auto menuPayload = lc::scripting::makeMap();
        (*menuPayload)["widget"] = lc::scripting::ScriptValue(
            lc::scripting::OpaquePtr{&contextMenu, lc::ui::opaquetag::Menu});
        std::string menuDiagnostic;
        menuCallback.invokeEvent("menuOpaque",
                                 lc::scripting::ScriptValue(menuPayload),
                                 [&menuDiagnostic](const std::string& message) {
                                     menuDiagnostic = message;
                                 });
        EXPECT_TRUE(menuDiagnostic.empty()) << menuDiagnostic;
        EXPECT_EQ(builtins.attr("_lc_opaque_menu_type").cast<std::string>(),
                  "Menu");

        pybind11::dict objectNamespace;
        objectNamespace["__builtins__"] = pybind11::module_::import("builtins");
        pybind11::exec(
            "class _OpaqueProbe:\n"
            "    def check(self, widget):\n"
            "        return type(widget).__name__\n",
            objectNamespace);
        pybind11::object probeClass = objectNamespace["_OpaqueProbe"];
        auto operationObject =
            lc::python::makePythonObject(probeClass());
        const auto objectResult = operationObject.callMethod(
            "check", {lc::scripting::ScriptValue(
                lc::scripting::OpaquePtr{&contextMenu,
                                         lc::ui::opaquetag::Menu})});
        EXPECT_EQ(objectResult.asString(), "Menu")
            << "ScriptObject must use the shared Python value converter";

        // Unknown tags must produce a callback diagnostic instead of
        // silently becoming Python None.
        pybind11::object noop = pybind11::eval("lambda event, args: None");
        auto callback = lc::python::makePythonCallback(std::move(noop));
        auto unknownPayload = lc::scripting::makeMap();
        (*unknownPayload)["widget"] = lc::scripting::ScriptValue(
            lc::scripting::OpaquePtr{&mainWindow, "test::UnknownUiType*"});
        std::string diagnostic;
        callback.invokeEvent("unknownOpaque",
                             lc::scripting::ScriptValue(unknownPayload),
                             [&diagnostic](const std::string& message) {
                                 diagnostic = message;
                             });
        EXPECT_NE(diagnostic.find("test::UnknownUiType*"), std::string::npos);

        // The known tag is also rejected from a worker thread. GIL
        // acquisition does not make Qt-owned objects thread-safe.
        std::string workerDiagnostic;
        {
            pybind11::gil_scoped_release release;
            std::thread worker([&]() {
                callback.invokeEvent(
                    "workerOpaque", lc::scripting::ScriptValue(payload),
                    [&workerDiagnostic](const std::string& message) {
                        workerDiagnostic = message;
                    });
            });
            worker.join();
        }
        EXPECT_NE(workerDiagnostic.find("Qt application thread"),
                  std::string::npos);

        // Cleanup: deregister so the listener doesn't outlive this test.
        pybind11::exec(R"py(
import builtins, lc
lc.event.deregister("point", builtins._lc_event_test_state["listener"])
builtins._lc_event_test_state = {}
)py");
    }

}

#endif  // LC_WITH_PYTHONSCRIPT
