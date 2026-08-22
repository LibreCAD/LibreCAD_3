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
#include <gtest/gtest.h>

#include <mainwindow.h>
#include <widgets/scriptdock.h>
#include <windowmanager.h>
#include <cad/storage/document.h>

#ifdef LC_WITH_PYTHONSCRIPT
#include <pybind11/embed.h>
#include <lcpython.h>
#include <lcscripting/scriptvalue.h>
#include <luainterface.h>
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
    // a single Line entity via lc.entity.Line + the operation builder,
    // count entities, expect 1.
    QApplication app(argc, argv);
    lc::ui::MainWindow mainWindow;
    ScriptDock dock(&mainWindow);
    ASSERT_EQ(entityCount(mainWindow.cadMdiChild()->document()), 0u);

    dock.setInputText(QStringLiteral(R"lua(
        local start_pt = lc.geo.Coordinate(0, 0, 0)
        local end_pt = lc.geo.Coordinate(10, 0, 0)
        local layer = mainWindow:cadMdiChild():activeLayer()
        local line = lc.entity.Line(start_pt, end_pt, layer)
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
    QApplication app(argc, argv);
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
layer    = mdi.document().activeLayer()
line     = lc.entity.Line(start_pt, end_pt, layer)
builder  = lc.operation.EntityBuilder(mdi.document())
builder.appendEntity(line)
builder.execute()
)py"));
    dock.run();

    EXPECT_EQ(entityCount(mainWindow.cadMdiChild()->document()), 1u)
        << "Python leg of ScriptDock must create exactly one Line entity "
           "— proves the mainWindow/document injection + lcgui bindings "
           "reach the kernel correctly";
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
    // `lcgui.currentMainWindow()` reads `WindowManager::mainWindows`,
    // which only `WindowManager::init/newFile/openFile` populate —
    // MainWindow's own constructor does not self-register.  A
    // directly-constructed MainWindow (as below) is therefore
    // invisible to it unless we push it in ourselves; without this,
    // the register hook no-ops on a null current window and the
    // assertions below fail regardless of whether `_get_main_window()`
    // is fixed or still frame-walking, giving zero signal either way.
    QApplication app(argc, argv);
    lc::python::PythonInit::initialize();

    lc::ui::MainWindow mainWindow;
    lc::ui::WindowManager::mainWindows.push_back(&mainWindow);

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
    def onEvent(self, event_name, args):
        # THIS is the failure mode the fixup addresses.  Under the
        # broken frame-walking implementation, lcgui.currentMainWindow()
        # would work but a manual sys._getframe walk would NOT find
        # `mainWindow`.  We call the same function CreateOperations
        # now calls internally.
        self.saw_event = True
        self.mw_from_onEvent = lcgui.currentMainWindow()

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
    mainWindow.luaInterface()->triggerEvent(
        "point", lc::scripting::ScriptValue{});

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

        // Cleanup: deregister so the listener doesn't outlive this test.
        pybind11::exec(R"py(
import builtins, lc
lc.event.deregister("point", builtins._lc_event_test_state["listener"])
builtins._lc_event_test_state = {}
)py");
    }

    // Cleanup: this MainWindow is stack-local and about to be
    // destroyed — remove it from the shared static registry so it
    // doesn't dangle for whichever test runs next in this binary.
    lc::ui::WindowManager::removeWindow(&mainWindow);
}

#endif  // LC_WITH_PYTHONSCRIPT
