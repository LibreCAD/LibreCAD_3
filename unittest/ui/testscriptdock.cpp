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
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTextEdit>
#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <thread>
#include <vector>

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
#include <python/pypluginloader.h>
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
    // a single Line entity via lc.entity.Line + the operation builder,
    // count entities, expect 1.
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

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(ScriptDockTest, PythonEventRoutingUsesEachOwningWindow) {
    // S4 contract: event hooks are process-installed, but registrations and
    // callback execution are scoped to the MainWindow whose script is active.
    // Exercise two simultaneously-live windows so an accidental process-global
    // or insertion-order lookup cannot pass by seeing only one owner.
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    lc::ui::MainWindow firstWindow;
    lc::ui::MainWindow secondWindow;

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::dict ns;
        ns["__builtins__"] = pybind11::module_::import("builtins");
        pybind11::exec(R"py(
import builtins
import lc
import lcgui

class _OwnerProbe:
    def __init__(self):
        self.observed_window = None
    def onEvent(self, event_name, args):
        self.observed_window = lcgui.currentMainWindow()

builtins._lc_python_owner_probe_a = _OwnerProbe()
builtins._lc_python_owner_probe_b = _OwnerProbe()
)py", ns);
    }

    // Registration must attach to the window in the active scope, even
    // though the process-wide Python hook itself was installed only once.
    {
        lc::ui::python::MainWindowExecutionScope ownerScope(&firstWindow);
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(
            "import builtins, lc\n"
            "lc.event.register('pythonOwnerProbe', "
            "builtins._lc_python_owner_probe_a)");
    }
    {
        lc::ui::python::MainWindowExecutionScope ownerScope(&secondWindow);
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(
            "import builtins, lc\n"
            "lc.event.register('pythonOwnerProbe', "
            "builtins._lc_python_owner_probe_b)");
    }
    EXPECT_EQ(lc::ui::python::currentMainWindow(), nullptr)
        << "nested execution scopes must restore the prior no-owner state";

    // LuaInterface enters its owning scope while dispatching from C++, just
    // as the UI slots do. Each listener must see the corresponding instance.
    const lc::scripting::ScriptValue noPayload;
    firstWindow.luaInterface()->triggerEvent("pythonOwnerProbe", noPayload);
    secondWindow.luaInterface()->triggerEvent("pythonOwnerProbe", noPayload);

    bool firstListenerFired = false;
    bool secondListenerFired = false;
    lc::ui::MainWindow* firstObservedWindow = nullptr;
    lc::ui::MainWindow* secondObservedWindow = nullptr;
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::module_ builtins = pybind11::module_::import("builtins");
        pybind11::object observedA =
            builtins.attr("_lc_python_owner_probe_a").attr("observed_window");
        pybind11::object observedB =
            builtins.attr("_lc_python_owner_probe_b").attr("observed_window");
        firstListenerFired = !observedA.is_none();
        secondListenerFired = !observedB.is_none();
        if (firstListenerFired) {
            firstObservedWindow = observedA.cast<lc::ui::MainWindow*>();
        }
        if (secondListenerFired) {
            secondObservedWindow = observedB.cast<lc::ui::MainWindow*>();
        }

        // Remove each exact tracked registration under the same owner scope,
        // then release the Python wrappers before either Qt window is closed.
        {
            lc::ui::python::MainWindowExecutionScope ownerScope(&firstWindow);
            pybind11::exec(
                "lc.event.deregister('pythonOwnerProbe', "
                "builtins._lc_python_owner_probe_a)");
        }
        {
            lc::ui::python::MainWindowExecutionScope ownerScope(&secondWindow);
            pybind11::exec(
                "lc.event.deregister('pythonOwnerProbe', "
                "builtins._lc_python_owner_probe_b)");
        }
        builtins.attr("_lc_python_owner_probe_a") = pybind11::none();
        builtins.attr("_lc_python_owner_probe_b") = pybind11::none();
    }
    EXPECT_TRUE(firstListenerFired) << "window A listener did not fire";
    EXPECT_TRUE(secondListenerFired) << "window B listener did not fire";
    EXPECT_EQ(firstObservedWindow, &firstWindow);
    EXPECT_EQ(secondObservedWindow, &secondWindow);
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(ScriptDockTest, NestedPythonEventCallbacksRetainTheirOwningWindow) {
    // A Python event callback may synchronously cause another native event
    // dispatch. The inner callback must observe the same window, and the
    // outer callback must resume with its owner scope intact afterward.
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    lc::ui::MainWindow mainWindow;
    const lc::scripting::ScriptValue noPayload;
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::dict ns;
        ns["__builtins__"] = pybind11::module_::import("builtins");
        pybind11::exec(R"py(
import builtins
import lc
import lcgui

class _NestedOwnerProbe:
    def __init__(self):
        self.events = []
        self.inner_window = None
    def on_outer(self, event_name, args):
        self.events.append("outer-start")
        builtins._lc_trigger_nested_python_event()
        self.events.append("outer-end")
    def on_inner(self, event_name, args):
        self.events.append("inner")
        self.inner_window = lcgui.currentMainWindow()

builtins._lc_nested_owner_probe = _NestedOwnerProbe()
)py",
                       ns);
        pybind11::module_::import("builtins").attr(
            "_lc_trigger_nested_python_event") = pybind11::cpp_function(
            [&mainWindow, noPayload]() {
                mainWindow.luaInterface()->triggerEvent(
                    "pythonNestedOwnerProbe", noPayload);
            });

        lc::ui::python::MainWindowExecutionScope ownerScope(&mainWindow);
        pybind11::exec(R"py(
import builtins, lc
probe = builtins._lc_nested_owner_probe
lc.event.register("pythonOuterOwnerProbe", probe.on_outer)
lc.event.register("pythonNestedOwnerProbe", probe.on_inner)
)py");
    }

    mainWindow.luaInterface()->triggerEvent("pythonOuterOwnerProbe", noPayload);

    bool nestedOwnerMatched = false;
    std::vector<std::string> observedEvents;
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::module_ builtins = pybind11::module_::import("builtins");
        pybind11::object probe = builtins.attr("_lc_nested_owner_probe");
        const pybind11::list events = probe.attr("events");
        observedEvents.reserve(events.size());
        for (const pybind11::handle event : events) {
            observedEvents.push_back(pybind11::cast<std::string>(event));
        }
        pybind11::object innerWindow = probe.attr("inner_window");
        nestedOwnerMatched = !innerWindow.is_none() &&
            innerWindow.cast<lc::ui::MainWindow*>() == &mainWindow;

        lc::ui::python::MainWindowExecutionScope ownerScope(&mainWindow);
        pybind11::exec(R"py(
import builtins, lc
probe = builtins._lc_nested_owner_probe
lc.event.deregister("pythonOuterOwnerProbe", probe.on_outer)
lc.event.deregister("pythonNestedOwnerProbe", probe.on_inner)
builtins._lc_trigger_nested_python_event = None
builtins._lc_nested_owner_probe = None
)py");
    }

    EXPECT_EQ(observedEvents,
              (std::vector<std::string>{"outer-start", "inner", "outer-end"}));
    EXPECT_TRUE(nestedOwnerMatched)
        << "nested Python callback must observe its MainWindow owner";
    EXPECT_EQ(lc::ui::python::currentMainWindow(), nullptr)
        << "nested callback return must unwind all owner scopes";
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(ScriptDockTest, PythonPluginDefinitionsLoadOncePerProcess) {
    // S4 contract: MainWindow startup may discover operations repeatedly, but
    // executing plugin.py must not repeat process-global registrations.
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    const QString pluginDirectory =
        QDir(tempDir.path()).filePath(QStringLiteral("plugins/probe"));
    ASSERT_TRUE(QDir().mkpath(pluginDirectory));
    QFile pluginFile(QDir(pluginDirectory).filePath(QStringLiteral("plugin.py")));
    ASSERT_TRUE(pluginFile.open(QIODevice::WriteOnly | QIODevice::Text));
    const QByteArray pluginSource =
        "import builtins\n"
        "builtins._lc_plugin_definition_run_count = "
        "getattr(builtins, '_lc_plugin_definition_run_count', 0) + 1\n";
    ASSERT_EQ(pluginFile.write(pluginSource), pluginSource.size());
    pluginFile.close();

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::module_::import("builtins")
            .attr("_lc_plugin_definition_run_count") = 0;
    }

    const auto firstLoad = lc::ui::python::loadPythonPlugins(
        QDir(tempDir.path()).filePath(QStringLiteral("plugins")).toStdString(),
        "gui");
    const auto secondLoad = lc::ui::python::loadPythonPlugins(
        QDir(tempDir.path()).filePath(QStringLiteral("plugins")).toStdString(),
        "gui");

    EXPECT_TRUE(firstLoad.directoryFound);
    EXPECT_TRUE(secondLoad.directoryFound);
    EXPECT_EQ(firstLoad.loadedPlugins, (std::vector<std::string>{"probe"}));
    EXPECT_EQ(secondLoad.loadedPlugins, (std::vector<std::string>{"probe"}));
    EXPECT_TRUE(firstLoad.failures.empty());
    EXPECT_TRUE(secondLoad.failures.empty());
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::module_ builtins = pybind11::module_::import("builtins");
        EXPECT_EQ(builtins.attr("_lc_plugin_definition_run_count").cast<int>(),
                  1);
        builtins.attr("_lc_plugin_definition_run_count") = pybind11::none();
    }
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(ScriptDockTest, PythonBoundMethodDeregistrationRemovesExactDuplicate) {
    // Public compatibility removes the first equal callback using a fresh
    // bound-method object; the returned registration ID removes the remaining
    // duplicate without disturbing another equal registration.
    ensureUiApplication();
    lc::python::PythonInit::initialize();
    lc::ui::MainWindow mainWindow;

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(R"py(
import builtins
import lc

class _BoundMethodProbe:
    def __init__(self):
        self.calls = 0
    def callback(self, event_name, args):
        self.calls += 1

builtins._lc_bound_method_probe = _BoundMethodProbe()
)py");
    }

    unsigned long long exactRegistrationId = 0;
    {
        lc::ui::python::MainWindowExecutionScope ownerScope(&mainWindow);
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(
            "import builtins, lc\n"
            "builtins._lc_bound_method_id_a = lc.event.register(\n"
            "    'pythonBoundMethodProbe', builtins._lc_bound_method_probe.callback)\n"
            "builtins._lc_bound_method_id_b = lc.event.register(\n"
            "    'pythonBoundMethodProbe', builtins._lc_bound_method_probe.callback)\n");
        exactRegistrationId = pybind11::module_::import("builtins")
            .attr("_lc_bound_method_id_b").cast<unsigned long long>();
    }
    EXPECT_NE(exactRegistrationId, 0u);
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount("pythonBoundMethodProbe"),
              2u);

    {
        lc::ui::python::MainWindowExecutionScope ownerScope(&mainWindow);
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(
            "import builtins, lc\n"
            "lc.event.deregister('pythonBoundMethodProbe', "
            "builtins._lc_bound_method_probe.callback)");
    }
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount("pythonBoundMethodProbe"),
              1u);

    mainWindow.luaInterface()->triggerEvent(
        "pythonBoundMethodProbe", lc::scripting::ScriptValue{});
    int callbackCount = -1;
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::module_ builtins = pybind11::module_::import("builtins");
        callbackCount =
            builtins.attr("_lc_bound_method_probe").attr("calls").cast<int>();
    }
    EXPECT_EQ(callbackCount, 1);

    {
        lc::ui::python::MainWindowExecutionScope ownerScope(&mainWindow);
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(
            "import builtins, lc\n"
            "lc.event.deregister('pythonBoundMethodProbe', "
            "builtins._lc_bound_method_id_b)");
        pybind11::module_::import("builtins")
            .attr("_lc_bound_method_probe") = pybind11::none();
    }
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount("pythonBoundMethodProbe"),
              0u);
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(ScriptDockTest, CurrentMainWindowScopeInvalidatesAfterWindowDestruction) {
    // QPointer-backed owner lookup must never leave a dangling MainWindow
    // visible to callbacks after the owning Qt window has been destroyed.
    ensureUiApplication();
    auto mainWindow = std::make_unique<lc::ui::MainWindow>();
    auto ownerScope =
        std::make_unique<lc::ui::python::MainWindowExecutionScope>(
            mainWindow.get());

    EXPECT_EQ(lc::ui::python::currentMainWindow(), mainWindow.get());
    mainWindow.reset();
    EXPECT_EQ(lc::ui::python::currentMainWindow(), nullptr);

    ownerScope.reset();
    EXPECT_EQ(lc::ui::python::currentMainWindow(), nullptr);
}

#endif  // LC_WITH_PYTHONSCRIPT
