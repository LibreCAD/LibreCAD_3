#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTextEdit>
#include <algorithm>
#include <cmath>
#include <memory>
#include <thread>

#include <gtest/gtest.h>
#include "mainwindowtest.h"

#include "uitests.h"

#ifdef LC_WITH_PYTHONSCRIPT
#include <qt_keywords_push.h>
#include <pybind11/embed.h>
#include <pybind11/eval.h>
#include <qt_keywords_pop.h>
#include <lcpython.h>
#include <cad/primitive/insert.h>
#include <cad/primitive/line.h>
#include <cad/primitive/point.h>
#if LC_USE_PERSISTENCE
#include <cad/storage/documentimpl.h>
#include <cad/storage/storagemanagerimpl.h>
#include <persistence/file.h>
#endif
#include <python/pyeventhooks.h>
#include <lcscripting/scriptvalue.h>
#include <lcscripting/scriptobject.h>
#include <lcscripting/scriptcallback.h>  // Phase 6 PR-6.1 nativeCallback
#include <managers/luacustomentitymanager.h>  // Phase 6 PR-6.1 multi-window bug test
#include <luainterface.h>  // Phase 6 PR-6.1 — construct/destruct LuaInterface directly
#endif

TEST(MWindowTest, WindowWidgetTest) {
    ensureUiApplication();
    MainWindowTest* mainWindow = new MainWindowTest();

    EXPECT_TRUE(mainWindow->testAddingOfWidgets());
    EXPECT_TRUE(mainWindow->testAddingOfSelectTools());
}

TEST(MWindowTest, PointTriggerTest) {
    ensureUiApplication();
    MainWindowTest* mainWindow = new MainWindowTest();

    // test if last point is updated
    EXPECT_TRUE(mainWindow->testTriggerPoint(lc::geo::Coordinate(2,2)));
}

TEST(MWindowTest, OperationFinishedTest) {
    ensureUiApplication();
    MainWindowTest* mainWindow = new MainWindowTest();

    EXPECT_TRUE(mainWindow->testOperationFinished());
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MWindowTest, TriggerEventArrayPayloadPreserved) {
    // Phase 4 post-review regression test — verify
    // LuaInterface::triggerEvent(LuaRef) passes array-table + function
    // payloads through to Lua listeners without the lossy
    // fromLua/toLua round-trip.
    ensureUiApplication();
    MainWindowTest* mainWindow = new MainWindowTest();

    EXPECT_TRUE(mainWindow->testTriggerEventArrayPayloadPreserved());
}

TEST(MWindowTest, FailedLuaEventListenersAreRemovedAfterSnapshot) {
    ensureUiApplication();
    MainWindowTest* mainWindow = new MainWindowTest();

    EXPECT_TRUE(mainWindow->testFailedLuaListenersAreRemovedAfterSnapshot());
}

#ifdef LC_WITH_PYTHONSCRIPT
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MWindowTest, PythonFailedEventListenerIsRemovedAndOtherListenerContinues) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();
    MainWindowTest mainWindow;

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(R"py(
import builtins
import lc
import lcgui

builtins._lc_python_event_failure_state = {
    "failed_calls": 0,
    "healthy_calls": 0,
    "owner": None,
}

def _lc_python_event_failure(event_name, args):
    state = builtins._lc_python_event_failure_state
    state["failed_calls"] += 1
    raise RuntimeError("expected window-owned Python event failure")

def _lc_python_event_survivor(event_name, args):
    state = builtins._lc_python_event_failure_state
    state["healthy_calls"] += 1
    state["owner"] = lcgui.currentMainWindow()
)py");
        lc::ui::python::MainWindowExecutionScope ownerScope(&mainWindow);
        pybind11::exec(R"py(
import builtins, lc
lc.event.register(
    "pythonWindowFailureProbe", _lc_python_event_failure)
lc.event.register(
    "pythonWindowFailureProbe", _lc_python_event_survivor)
)py");
    }

    const lc::scripting::ScriptValue noPayload;
    mainWindow.luaInterface()->triggerEvent("pythonWindowFailureProbe",
                                             noPayload);
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount(
                  "pythonWindowFailureProbe"), 1u)
        << "the failing exact registration must be removed after dispatch";

    // LuaInterface's diagnostic sink queues traceback output to the CLI.
    QCoreApplication::processEvents();
    QTextEdit* history = mainWindow.cliCommand()->findChild<QTextEdit*>(
        QStringLiteral("history"));
    ASSERT_NE(history, nullptr);
    EXPECT_TRUE(history->toPlainText().contains(
        QStringLiteral("expected window-owned Python event failure")));

    mainWindow.luaInterface()->triggerEvent("pythonWindowFailureProbe",
                                             noPayload);
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount(
                  "pythonWindowFailureProbe"), 1u);

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::module_ builtins = pybind11::module_::import("builtins");
        pybind11::dict state = builtins.attr(
            "_lc_python_event_failure_state");
        EXPECT_EQ(state["failed_calls"].cast<int>(), 1);
        EXPECT_EQ(state["healthy_calls"].cast<int>(), 2)
            << "unrelated listeners must continue on both snapshots";
        pybind11::object owner = state["owner"];
        ASSERT_FALSE(owner.is_none());
        EXPECT_EQ(owner.cast<lc::ui::MainWindow*>(), &mainWindow);

        lc::ui::python::MainWindowExecutionScope ownerScope(&mainWindow);
        pybind11::exec(R"py(
import builtins, lc
lc.event.deregister(
    "pythonWindowFailureProbe", _lc_python_event_survivor)
del _lc_python_event_failure
del _lc_python_event_survivor
del builtins._lc_python_event_failure_state
)py");
    }
}
#endif

TEST(MWindowTest, LuaOperationEventRegistrationsAreScoped) {
    ensureUiApplication();
    MainWindowTest mainWindow;

    EXPECT_TRUE(mainWindow.testLuaOperationEventRegistrationsAreScoped());
}

TEST(MWindowTest, RunOperationTest) {
    ensureUiApplication();
    MainWindowTest* mainWindow = new MainWindowTest();

    EXPECT_TRUE(mainWindow->testRunOperation());
}

TEST(MWindowTest, AddOperationOptionsTest) {
    ensureUiApplication();
    MainWindowTest* mainWindow = new MainWindowTest();

    EXPECT_TRUE(mainWindow->testAddOperationOptions());
}

TEST(MWindowTest, ConnectMenuTest) {
    ensureUiApplication();
    MainWindowTest* mainWindow = new MainWindowTest();

    EXPECT_TRUE(mainWindow->testConnectMenuItem());
}

TEST(MWindowTest, MenuAPITest) {
    ensureUiApplication();
    MainWindowTest* mainWindow = new MainWindowTest();

    lc::ui::api::Menu* menu1 = mainWindow->addMenu("Menu1");
    lc::ui::api::Menu* menu2 = new lc::ui::api::Menu("Menu2");
    mainWindow->addMenu(menu2);

    EXPECT_EQ(menu1, mainWindow->menuByName("Menu1"));
    EXPECT_EQ(menu1, mainWindow->menuByPosition(6));

    EXPECT_EQ(menu2, mainWindow->menuByName("Menu2"));
    EXPECT_EQ(menu2, mainWindow->menuByPosition(7));

    EXPECT_EQ(6, menu1->position());
    EXPECT_EQ(7, menu2->position());

    mainWindow->removeMenu(0);

    EXPECT_EQ(5, menu1->position());
    EXPECT_EQ(6, menu2->position());

    mainWindow->removeMenu("Create");

    EXPECT_EQ(4, menu1->position());
    EXPECT_EQ(5, menu2->position());
}

#ifdef LC_WITH_PYTHONSCRIPT

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MWindowTest, PythonWindowScopeAndOperationEventCleanup) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    // The operation loader invokes init() while MainWindow is still being
    // constructed and before WindowManager can append it to mainWindows.
    // The registration must still route to this window's EventBus.
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(R"py(
import lc
import lcgui

def _python_binding_scope_probe_init():
    global _python_binding_scope_probe_owner, _python_binding_scope_probe_id
    _python_binding_scope_probe_owner = lcgui.currentMainWindow()
    _python_binding_scope_probe_id = lc.event.register(
        "_python_binding_scope_probe", lambda event, args: None)

@lc.register_operation
class PyWindowScopeProbe:
    name = "PyWindowScopeProbe"
    init = staticmethod(_python_binding_scope_probe_init)
)py");
    }

    auto mainWindow = std::make_unique<MainWindowTest>();
    EXPECT_EQ(mainWindow->luaInterface()->listenerCount(
                  "_python_binding_scope_probe"), 1u);

    // Construct a real CreateOperations subclass after startup. Its four
    // subscriptions are tracked by exact ID, and lcgui.currentMainWindow()
    // must resolve to the runOperation owner despite no vector insertion.
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(R"py(
import lc
import lcgui
from lcUIPy.create_operations import CreateOperations

@lc.register_operation
class PyScopedCreateOperation(CreateOperations):
    name = "PyScopedCreateOperation"
    def _init_default(self):
        self.owner_was_available = lcgui.currentMainWindow() is not None
        # Deliberately add equal registrations twice; exact tokens must remove
        # both rather than deleting only the first matching callback.
        self.registerEvents()
)py");
    }
    mainWindow->runOperationByName("PyScopedCreateOperation");
    const auto operation = mainWindow->currentOperation();
    ASSERT_FALSE(operation.isNil());
    EXPECT_TRUE(operation.getAttr("owner_was_available").asBool());
    EXPECT_EQ(mainWindow->luaInterface()->listenerCount("point"), 2u);
    EXPECT_EQ(mainWindow->luaInterface()->listenerCount("mouseMove"), 2u);

    mainWindow->luaInterface()->finishOperation();
    EXPECT_EQ(mainWindow->luaInterface()->listenerCount("point"), 0u);
    EXPECT_EQ(mainWindow->luaInterface()->listenerCount("mouseMove"), 0u);
    EXPECT_EQ(mainWindow->luaInterface()->listenerCount(
                  "_python_binding_scope_probe"), 1u);

    // Remove test-only operation definitions and let MainWindow teardown
    // clear its independent startup probe registration.
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(R"py(
import lc
lc.operation_registry.pop("PyWindowScopeProbe", None)
lc.operation_registry.pop("PyScopedCreateOperation", None)
globals().pop("_python_binding_scope_probe_owner", None)
globals().pop("_python_binding_scope_probe_id", None)
)py");
    }
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MWindowTest, PythonFailedOperationConstructorDoesNotLeakEvents) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();
    MainWindowTest mainWindow;

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(R"py(
import lc
from lcUIPy.create_operations import CreateOperations

@lc.register_operation
class PyFailAfterEventRegistration(CreateOperations):
    name = "PyFailAfterEventRegistration"
    def __init__(self):
        super().__init__(step="PAUSE")
        raise RuntimeError("intentional operation constructor failure")
)py");
    }

    mainWindow.runOperationByName("PyFailAfterEventRegistration");
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount("point"), 0u);
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount("mouseMove"), 0u);
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount("number"), 0u);
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount("text"), 0u);
    auto* history = mainWindow.cliCommand()->findChild<QTextEdit*>(
        QStringLiteral("history"));
    ASSERT_NE(history, nullptr);
    EXPECT_TRUE(history->toPlainText().contains(
        QStringLiteral("intentional operation constructor failure")));

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(R"py(
import lc
lc.operation_registry.pop("PyFailAfterEventRegistration", None)
)py");
    }
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MWindowTest, PythonFailedOperationInitReportsAndCleansEvents) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();
    MainWindowTest mainWindow;

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(R"py(
import lc
from lcUIPy.create_operations import CreateOperations

@lc.register_operation
class PyFailDuringOperationInit(CreateOperations):
    name = "PyFailDuringOperationInit"
    def __init__(self):
        super().__init__(step="PAUSE")
    def _init_default(self):
        raise RuntimeError("intentional operation init failure")
)py");
    }

    mainWindow.runOperationByName("PyFailDuringOperationInit");
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount("point"), 0u);
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount("mouseMove"), 0u);
    auto* history = mainWindow.cliCommand()->findChild<QTextEdit*>(
        QStringLiteral("history"));
    ASSERT_NE(history, nullptr);
    EXPECT_TRUE(history->toPlainText().contains(
        QStringLiteral("intentional operation init failure")));

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(R"py(
import lc
lc.operation_registry.pop("PyFailDuringOperationInit", None)
)py");
    }
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MWindowTest, WindowTeardownClearsOnlyItsPythonEventCallbacks) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(R"py(
import builtins
import lc
import weakref

builtins._lc_window_teardown_counter = 0
builtins._lc_window_teardown_callbacks = {}
builtins._lc_window_teardown_weakrefs = {}
builtins._lc_window_teardown_hits = {}

def _lc_window_teardown_callback_alive(event):
    reference = builtins._lc_window_teardown_weakrefs.get(event)
    return reference is not None and reference() is not None

def _lc_window_teardown_hit_count(event):
    return builtins._lc_window_teardown_hits.get(event, 0)

builtins._lc_window_teardown_callback_alive = _lc_window_teardown_callback_alive
builtins._lc_window_teardown_hit_count = _lc_window_teardown_hit_count

@lc.register_operation
class PyWindowTeardownProbe:
    name = "PyWindowTeardownProbe"
    def _init_default(self):
        builtins._lc_window_teardown_counter += 1
        event = "_lc_window_teardown_" + str(builtins._lc_window_teardown_counter)
        builtins._lc_window_teardown_hits[event] = 0
        def callback(name, args):
            builtins._lc_window_teardown_hits[event] += 1
        builtins._lc_window_teardown_callbacks[event] = callback
        builtins._lc_window_teardown_weakrefs[event] = weakref.ref(callback)
        lc.event.register(event, callback)
        builtins._lc_window_teardown_callbacks.pop(event)
        self.event_name = event
)py");
    }

    auto callbackAlive = [](const std::string& event) {
        pybind11::gil_scoped_acquire gil;
        return pybind11::module_::import("builtins")
            .attr("_lc_window_teardown_callback_alive")(event).cast<bool>();
    };
    auto hitCount = [](const std::string& event) {
        pybind11::gil_scoped_acquire gil;
        return pybind11::module_::import("builtins")
            .attr("_lc_window_teardown_hit_count")(event).cast<int>();
    };

    auto first = std::make_unique<MainWindowTest>();
    first->runOperationByName("PyWindowTeardownProbe");
    const std::string firstEvent = first->luaInterface()->operation()
        .getAttr("event_name").asString();
    EXPECT_FALSE(firstEvent.empty());
    EXPECT_EQ(first->luaInterface()->listenerCount(firstEvent), 1u);
    EXPECT_TRUE(callbackAlive(firstEvent));

    auto second = std::make_unique<MainWindowTest>();
    second->runOperationByName("PyWindowTeardownProbe");
    const std::string secondEvent = second->luaInterface()->operation()
        .getAttr("event_name").asString();
    EXPECT_FALSE(secondEvent.empty());
    EXPECT_EQ(second->luaInterface()->listenerCount(secondEvent), 1u);
    EXPECT_TRUE(callbackAlive(secondEvent));

    first.reset();
    EXPECT_FALSE(callbackAlive(firstEvent));
    EXPECT_TRUE(callbackAlive(secondEvent));
    EXPECT_EQ(second->luaInterface()->listenerCount(secondEvent), 1u);
    second->luaInterface()->triggerEvent(
        secondEvent, lc::scripting::ScriptValue{});
    EXPECT_EQ(hitCount(secondEvent), 1);

    second.reset();
    EXPECT_FALSE(callbackAlive(secondEvent));
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(R"py(
import builtins, lc
lc.operation_registry.pop("PyWindowTeardownProbe", None)
for _name in (
    "_lc_window_teardown_counter",
    "_lc_window_teardown_callbacks",
    "_lc_window_teardown_weakrefs",
    "_lc_window_teardown_hits",
    "_lc_window_teardown_callback_alive",
    "_lc_window_teardown_hit_count",
):
    builtins.__dict__.pop(_name, None)
)py");
    }
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MWindowTest, PyGuiBridgeRejectsWorkerThreadAccess) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();
    MainWindowTest mainWindow;
    lc::ui::python::MainWindowExecutionScope ownerScope(&mainWindow);

    pybind11::object pythonWindow;
    pybind11::object textGuiClass;
    {
        pybind11::gil_scoped_acquire gil;
        pythonWindow = pybind11::module_::import("lcgui")
                           .attr("currentMainWindow")();
        textGuiClass = pybind11::module_::import("lcgui").attr("Text");
    }
    PyObject* const borrowedWindow = pythonWindow.ptr();
    PyObject* const borrowedTextGuiClass = textGuiClass.ptr();
    std::string methodError;
    std::string constructorError;
    std::thread worker([borrowedWindow, borrowedTextGuiClass,
                        &methodError, &constructorError]() {
        pybind11::gil_scoped_acquire gil;
        pybind11::object window =
            pybind11::reinterpret_borrow<pybind11::object>(borrowedWindow);
        pybind11::object textGui =
            pybind11::reinterpret_borrow<pybind11::object>(borrowedTextGuiClass);
        try {
            window.attr("message")("must not access Qt from this worker");
        } catch (const pybind11::error_already_set& error) {
            methodError = error.what();
        }
        try {
            textGui("worker widget");
        } catch (const pybind11::error_already_set& error) {
            constructorError = error.what();
        }
    });
    worker.join();

    EXPECT_NE(methodError.find(
                  "LibreCAD GUI bindings must be called from the Qt application thread"),
              std::string::npos) << methodError;
    EXPECT_NE(constructorError.find(
                  "LibreCAD GUI bindings must be called from the Qt application thread"),
              std::string::npos) << constructorError;
    {
        pybind11::gil_scoped_acquire gil;
        // Drop the Python references while holding the GIL. Assigning None
        // would leave two owned Py_None references whose later destructors
        // run after PythonInit has released the main-thread GIL.
        pythonWindow = pybind11::object();
        textGuiClass = pybind11::object();
    }
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MWindowTest, PythonOperationDictCommandLineWiresCliCommand) {
    // Phase 5 PR-5.2 fixup — exercise the wiring loop with a
    // dict-form command_line to guard against the key/value swap
    // bug the review found in the original PR-5.2 code.  Registers
    // an op with `command_line = { "1": "SIMPLECMD", "NAMEDCMD":
    // "second" }` (str keys because Python dicts can't have int
    // literals as keys the same way Lua tables can — the "1" str
    // matches the digit-only-key branch).  After MainWindow
    // construction runs loadPythonOperations, verify:
    //   * Typing "SIMPLECMD" resolves + runs the default init.
    //   * Typing "NAMEDCMD" resolves + runs `_init_second`.
    // This test would FAIL against the original PR-5.2 code because
    // the swap meant the "NAMEDCMD" CLI entry was never registered
    // (it registered "second" against `_init_NAMEDCMD` instead).
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::dict ns;
        ns["__builtins__"] = pybind11::module_::import("builtins");
        pybind11::exec(R"py(
import lc
@lc.register_operation
class PyDictCmdOp:
    name = "PyDictCmdOp"
    command_line = {"1": "SIMPLECMD", "NAMEDCMD": "second"}
    def _init_default(self):
        self.step = "DEFAULT_INIT_RAN"
    def _init_second(self):
        self.step = "SECOND_INIT_RAN"
)py",
            ns);
    }

    MainWindowTest* mainWindow = new MainWindowTest();

    // Both CLI-command entries should route to the SAME class, but
    // via different init methods.  Drive them via CliCommand::runCommand
    // — same path as user-typed input.
    mainWindow->cliCommand()->runCommand("SIMPLECMD");
    {
        lc::scripting::ScriptObject op = mainWindow->currentOperation();
        ASSERT_FALSE(op.isNil())
            << "SIMPLECMD must route to PyDictCmdOp via the digit-key "
               "branch (default init).  Failure indicates the wiring "
               "loop's str form isn't attaching the CLI entry.";
        lc::scripting::ScriptValue step = op.getAttr("step");
        EXPECT_EQ(step.asString(), "DEFAULT_INIT_RAN");
    }

    mainWindow->cliCommand()->runCommand("NAMEDCMD");
    {
        lc::scripting::ScriptObject op = mainWindow->currentOperation();
        ASSERT_FALSE(op.isNil())
            << "NAMEDCMD must route to PyDictCmdOp via the non-digit "
               "key branch (CLI text = key, init suffix = value).  "
               "Failure indicates the key/value swap bug: the CLI "
               "entry `NAMEDCMD` was never registered because the "
               "old code used `value` as CLI text and `key` as init.";
        lc::scripting::ScriptValue step = op.getAttr("step");
        EXPECT_EQ(step.asString(), "SECOND_INIT_RAN")
            << "The non-digit branch's init routing was inverted in "
               "the original PR-5.2 code — `_init_NAMEDCMD` would "
               "have been the (nonexistent) target instead of "
               "`_init_second`.";
    }

    // Cleanup.
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(R"py(
import lc
if "PyDictCmdOp" in lc.operation_registry:
    del lc.operation_registry["PyDictCmdOp"]
)py");
    }
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MWindowTest, PythonOperationResolverWiresIntoRunOperationByName) {
    // Phase 5 PR-5.2 — verify OperationLoader::loadPythonOperations
    // (a) pushed the Python-registry resolver onto MainWindow's ordered
    // resolver list, so `runOperationByName("PyDemoOp")` reaches a
    // Python-registered class instead of just Lua globals, and (b)
    // the resolver invokes ScriptObject::instantiate() (PR-7's
    // primitive) to produce a real instance whose init method fires
    // and mutates instance state readable back via ScriptObject::getAttr.
    //
    // This is the exit-bar test for PR-5.2: a Python operation
    // registered via @lc.register_operation reaches the runOperation
    // pipeline end-to-end.
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    // Register a Python operation BEFORE MainWindow construction so
    // both the ordered-resolver push AND the CliCommand/toolbar
    // wiring happen in the ctor's loadPythonOperations() step.
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::dict ns;
        ns["__builtins__"] = pybind11::module_::import("builtins");
        pybind11::exec(R"py(
import lc
@lc.register_operation
class PyDemoOp:
    name = "PyDemoOp"
    command_line = "PYDEMO"
    def _init_default(self):
        # Signal that runOperation reached us.
        self.step = "PY_DEMO_INIT_RAN"
)py",
            ns);
    }

    MainWindowTest* mainWindow = new MainWindowTest();

    // Resolve + run the Python operation via the ordered resolver
    // list.  Should route: runOperationByName → PythonRegistryResolver
    // → makePythonObject(PyDemoOp) → runOperation → instantiate() →
    // _init_default() → sets step attribute.
    mainWindow->runOperationByName("PyDemoOp");

    lc::scripting::ScriptObject curOp = mainWindow->currentOperation();
    ASSERT_FALSE(curOp.isNil())
        << "runOperationByName must produce a non-nil current operation "
           "when the Python registry contains a matching class";

    lc::scripting::ScriptValue step = curOp.getAttr("step");
    EXPECT_EQ(step.kind(), lc::scripting::ScriptValue::Kind::String);
    EXPECT_EQ(step.asString(), "PY_DEMO_INIT_RAN")
        << "the Python operation's _init_default() must have set "
           "`self.step` — proving instantiate() produced a real "
           "instance and the init call routed through it";

    // Cleanup: remove from the registry so subsequent tests aren't
    // affected.
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(R"py(
import lc
if "PyDemoOp" in lc.operation_registry:
    del lc.operation_registry["PyDemoOp"]
)py");
    }
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MWindowTest, PythonOperationOptionsWireDefaultAndNamedActions) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    constexpr const char* operationName = "PyToolbarOptionsOp";
    QTemporaryDir fixtureDirectory;
    ASSERT_TRUE(fixtureDirectory.isValid());
    ASSERT_TRUE(QDir().mkpath(fixtureDirectory.filePath("icons")));
    QFile fixtureIcon(fixtureDirectory.filePath("icons/option.svg"));
    ASSERT_TRUE(fixtureIcon.open(QIODevice::WriteOnly));
    static constexpr char svgIcon[] =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 16 16\">"
        "<rect width=\"16\" height=\"16\" fill=\"#336699\"/></svg>";
    ASSERT_EQ(fixtureIcon.write(svgIcon, sizeof(svgIcon) - 1),
              static_cast<qint64>(sizeof(svgIcon) - 1));
    fixtureIcon.close();

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::dict ns;
        ns["__builtins__"] = pybind11::module_::import("builtins");
        ns["fixture_file"] = fixtureDirectory.filePath("operation.py").toStdString();
        pybind11::exec(R"py(
import lc
import sys
import types

fixture_module = types.ModuleType("_lc_pytoolbar_options_fixture")
fixture_module.__file__ = fixture_file
sys.modules[fixture_module.__name__] = fixture_module
fixture_module.__dict__["lc"] = lc
exec(r'''
@lc.register_operation
class PyToolbarOptionsOp:
    name = "PyToolbarOptionsOp"
    command_line = "PYTOOLBAROPTIONS"
    operation_options = {
        "Line": {"icon": "option.svg", "action": "choose_line"},
        "Fail": {"icon": "option.svg", "action": "choose_error"},
        "Broken": {"icon": 12, "action": "choose_broken"},
        "_init_alternate": {
            "Arc": {"icon": "option.svg", "action": "choose_arc"},
        },
    }

    def _init_default(self):
        self.selected = "default"

    def _init_alternate(self):
        self.selected = "alternate-init"

    def choose_line(self):
        self.selected = "line"

    def choose_error(self):
        raise RuntimeError("expected toolbar option failure")

    def choose_arc(self):
        self.selected = "arc"
''', fixture_module.__dict__)
)py",
            ns);
    }

    MainWindowTest mainWindow;
    auto* quickAccess = mainWindow.toolbar()->tabByName("Quick Access");
    auto findOption = [quickAccess](const char* tooltip) {
        if (quickAccess == nullptr) return static_cast<lc::ui::api::ToolbarButton*>(nullptr);
        auto* group = quickAccess->groupByName("Current operation");
        if (group == nullptr) return static_cast<lc::ui::api::ToolbarButton*>(nullptr);
        for (auto* button : group->buttons()) {
            if (button->toolTip().toStdString() == tooltip) return button;
        }
        return static_cast<lc::ui::api::ToolbarButton*>(nullptr);
    };

    mainWindow.runOperationByName(operationName);
    lc::scripting::ScriptObject current = mainWindow.currentOperation();
    EXPECT_FALSE(current.isNil());
    if (!current.isNil()) {
        EXPECT_EQ(current.getAttr("selected").asString(), "default");
    }
    auto* lineOption = findOption("Line");
    EXPECT_NE(lineOption, nullptr) << "default option metadata was not wired";
    if (lineOption != nullptr) {
        EXPECT_FALSE(lineOption->icon().isNull())
            << "plugin-relative option icon was not loaded";
        lineOption->click();
        EXPECT_EQ(mainWindow.currentOperation().getAttr("selected").asString(), "line");
    }
    EXPECT_EQ(findOption("Broken"), nullptr)
        << "malformed option metadata must be skipped without suppressing valid entries";
    auto* failingOption = findOption("Fail");
    EXPECT_NE(failingOption, nullptr);
    auto* cliHistory = mainWindow.cliCommand()->findChild<QTextEdit*>();
    EXPECT_NE(cliHistory, nullptr);
    if (failingOption != nullptr && cliHistory != nullptr) {
        failingOption->click();
        EXPECT_TRUE(cliHistory->toPlainText().contains(
            "Python operation 'PyToolbarOptionsOp' option 'choose_error' failed"))
            << "Python option exceptions must be surfaced through the CLI";
    }

    mainWindow.operationFinished();
    mainWindow.runOperationByName(operationName, "_init_alternate");
    current = mainWindow.currentOperation();
    EXPECT_FALSE(current.isNil());
    if (!current.isNil()) {
        EXPECT_EQ(current.getAttr("selected").asString(), "alternate-init");
    }
    auto* arcOption = findOption("Arc");
    EXPECT_NE(arcOption, nullptr) << "init-specific option metadata was not selected";
    EXPECT_EQ(findOption("Line"), nullptr)
        << "an init-specific list must take precedence over default options";
    if (arcOption != nullptr) {
        EXPECT_FALSE(arcOption->icon().isNull())
            << "plugin-relative init option icon was not loaded";
        arcOption->click();
        EXPECT_EQ(mainWindow.currentOperation().getAttr("selected").asString(), "arc");
    }

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(R"py(
import lc
import sys
lc.operation_registry.pop("PyToolbarOptionsOp", None)
sys.modules.pop("_lc_pytoolbar_options_fixture", None)
)py");
    }
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MWindowTest, PyProofPortsImportAndRegister) {
    // Phase 5 PR-5.7 — verify the proof ports (point_operations.py +
    // remove_operation.py) import cleanly, run their
    // @lc.register_operation decorator, and produce classes with the
    // expected metadata attributes.  This is the discipline the
    // coordinator called out repeatedly: don't rely on py_compile
    // alone; import the ACTUAL modules and touch what they expose.
    //
    // The build mirrors lcUIPy beside the test executable; add its parent to
    // sys.path because this test doesn't initialize LuaInterface.
    ensureUiApplication();
    lc::python::PythonInit::initialize();
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::module_::import("sys").attr("path").attr("insert")(
            0, QCoreApplication::applicationDirPath().toStdString());
    }

    // Import the proof-port modules directly.  If they raise (missing
    // dep, syntax error, decorator failure), py::exec propagates the
    // exception and the test fails.
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(R"py(
import lc

# The test requires the package copied beside the test executable.
import importlib.util
if importlib.util.find_spec('lcUIPy') is None:
    raise RuntimeError('lcUIPy is missing beside the test executable')

# Ensure clean slate — the tests above may have injected transient
# PyDemoOp / PyDictCmdOp classes.
if 'PyPointOperations' in lc.operation_registry:
    del lc.operation_registry['PyPointOperations']
if 'PyRemoveOperation' in lc.operation_registry:
    del lc.operation_registry['PyRemoveOperation']

# Import — triggers @lc.register_operation at module top-level.
# LuaInterface imports operation modules during MainWindow construction, so
# these modules may already be cached by earlier tests. Reload after clearing
# the registry to exercise the decorators rather than relying on import cache.
import importlib
for _module_name in (
    'lcUIPy.create_actions.point_operations',
    'lcUIPy.actions.remove_operation',
):
    _module = importlib.import_module(_module_name)
    importlib.reload(_module)

# Registry entries must exist under their declared `name`.
assert 'PyPointOperations' in lc.operation_registry, \
    'point_operations.py did not register PyPointOperations'
assert 'PyRemoveOperation' in lc.operation_registry, \
    'remove_operation.py did not register PyRemoveOperation'

# Class metadata: `command_line` must be PYPOINT/PYREMOVE (avoids
# CLI collision with Lua's POINT/REMOVE).
p_cls = lc.operation_registry['PyPointOperations']
r_cls = lc.operation_registry['PyRemoveOperation']

assert p_cls.command_line == 'PYPOINT', \
    'PyPointOperations.command_line must be PYPOINT for Lua parity'
assert r_cls.command_line == 'PYREMOVE', \
    'PyRemoveOperation.command_line must be PYREMOVE for Lua parity'

# icon / description / menu_actions attributes present.
assert p_cls.icon == 'point.svg'
assert p_cls.description == 'Python Point'
assert p_cls.menu_actions == {'default': 'actionPoint'}

assert r_cls.icon == 'delete.svg'
assert r_cls.description == 'Python Remove'
)py");
    }

    // Cleanup — remove the classes from the registry so subsequent
    // tests aren't affected.  We can't reliably undo the module
    // import (CPython caches modules in sys.modules) but we CAN
    // clear the registry entries.
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(R"py(
import lc
for _name in ('PyPointOperations', 'PyRemoveOperation'):
    if _name in lc.operation_registry:
        del lc.operation_registry[_name]
)py");
    }
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MWindowTest, PythonPointProofPortCreatesOnePointAndSupportsUndoRedo) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    // The registration-only proof-port test intentionally clears these
    // entries. Reload the shipped module so this test is independent of test
    // order and MainWindow can wire the real PYPOINT CLI operation.
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::module_::import("sys").attr("path").attr("insert")(
            0, QCoreApplication::applicationDirPath().toStdString());
        pybind11::object importlib = pybind11::module_::import("importlib");
        pybind11::object pointModule = importlib.attr("import_module")(
            "lcUIPy.create_actions.point_operations");
        importlib.attr("reload")(pointModule);
    }

    MainWindowTest mainWindow;
    auto document = mainWindow.cadMdiChild()->document();
    const auto initialEntityCount =
        document->entityContainer().asVector().size();
    const auto initialPointListenerCount =
        mainWindow.luaInterface()->listenerCount("point");

    mainWindow.cliCommand()->runCommand("PYPOINT");
    const auto firstOperation = mainWindow.currentOperation();
    ASSERT_FALSE(firstOperation.isNil())
        << "PYPOINT must resolve through the shipped Python operation registry";
    EXPECT_EQ(firstOperation.getAttr("step").asString(), "enterPoint");
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount("point"),
              initialPointListenerCount + 1);

    const lc::geo::Coordinate firstCoordinate(12.5, -3.25);
    mainWindow.triggerCoordinateEntered(firstCoordinate);

    auto entities = document->entityContainer().asVector();
    ASSERT_EQ(entities.size(), initialEntityCount + 1)
        << "one coordinate event must commit exactly one point";
    auto firstPoint = std::dynamic_pointer_cast<const lc::entity::Point>(
        entities.back());
    ASSERT_NE(firstPoint, nullptr);
    EXPECT_DOUBLE_EQ(firstPoint->x(), firstCoordinate.x());
    EXPECT_DOUBLE_EQ(firstPoint->y(), firstCoordinate.y());
    EXPECT_TRUE(firstOperation.getAttr("finished").asBool())
        << "the one-shot point operation must mark itself finished after committing";
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount("point"),
              initialPointListenerCount)
        << "finishing PYPOINT must unregister its point listener";

    mainWindow.undo();
    EXPECT_EQ(document->entityContainer().asVector().size(),
              initialEntityCount);
    mainWindow.redo();
    EXPECT_EQ(document->entityContainer().asVector().size(),
              initialEntityCount + 1);

    // A second invocation catches stale duplicate registrations that can make
    // the next click create multiple entities or leave a listener behind.
    mainWindow.cliCommand()->runCommand("PYPOINT");
    ASSERT_FALSE(mainWindow.currentOperation().isNil());
    mainWindow.triggerCoordinateEntered(lc::geo::Coordinate(-4.0, 7.75));
    EXPECT_EQ(document->entityContainer().asVector().size(),
              initialEntityCount + 2);
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount("point"),
              initialPointListenerCount);
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MWindowTest, PythonRemoveProofPortRemovesSelectedPointAndSupportsUndoRedo) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    // The registration-only proof-port test may have cleared these entries.
    // Reload the real modules before MainWindow wires their CLI operations.
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::module_::import("sys").attr("path").attr("insert")(
            0, QCoreApplication::applicationDirPath().toStdString());
        pybind11::object importlib = pybind11::module_::import("importlib");
        for (const char* moduleName : {
                 "lcUIPy.create_actions.point_operations",
                 "lcUIPy.actions.remove_operation"}) {
            pybind11::object module =
                importlib.attr("import_module")(moduleName);
            importlib.attr("reload")(module);
        }
    }

    MainWindowTest mainWindow;
    auto document = mainWindow.cadMdiChild()->document();
    const auto initialEntityCount =
        document->entityContainer().asVector().size();

    mainWindow.cliCommand()->runCommand("PYPOINT");
    ASSERT_FALSE(mainWindow.currentOperation().isNil());
    mainWindow.triggerCoordinateEntered(lc::geo::Coordinate(6.5, 9.25));
    ASSERT_EQ(document->entityContainer().asVector().size(),
              initialEntityCount + 1);

    auto* documentCanvas = mainWindow.cadMdiChild()->viewer()->documentCanvas().get();
    ASSERT_NE(documentCanvas, nullptr);
    documentCanvas->selectAll();
    ASSERT_EQ(mainWindow.cadMdiChild()->selection().size(), 1)
        << "the point must be selected through the real document canvas";

    mainWindow.cliCommand()->runCommand("PYREMOVE");
    const auto removeOperation = mainWindow.currentOperation();
    ASSERT_FALSE(removeOperation.isNil())
        << "PYREMOVE must resolve through the shipped Python operation registry";
    EXPECT_TRUE(removeOperation.getAttr("finished").asBool());
    EXPECT_EQ(document->entityContainer().asVector().size(), initialEntityCount)
        << "PYREMOVE must remove the selected point from the document";

    mainWindow.undo();
    EXPECT_EQ(document->entityContainer().asVector().size(),
              initialEntityCount + 1)
        << "undo must restore the removed point";
    mainWindow.redo();
    EXPECT_EQ(document->entityContainer().asVector().size(), initialEntityCount)
        << "redo must remove the point again";
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MWindowTest, PythonGearProofPortCreatesClosedOutlineAndSupportsUndoRedo) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    // Evaluate the shipped plugin using the same namespace setup as the
    // production loader. This makes the test independent of test order and
    // verifies the actual plugin file rather than a parallel test fixture.
    const QString packageRoot = QDir(QCoreApplication::applicationDirPath())
                                    .filePath("lcUIPy");
    const QString pluginPath = QDir(packageRoot).filePath(
        "plugins/gear/plugin.py");
    ASSERT_TRUE(QFile::exists(pluginPath))
        << "the test binary must have the shipped gear plugin beside it: "
        << pluginPath.toStdString();

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::module_::import("sys").attr("path").attr("insert")(
            0, QCoreApplication::applicationDirPath().toStdString());
        pybind11::dict pluginNamespace;
        pluginNamespace["__builtins__"] =
            pybind11::module_::import("builtins");
        pybind11::exec("import lc\nimport lcgui as gui\n", pluginNamespace);
        pluginNamespace["LC_interface"] = "gui";
        pybind11::eval_file(pluginPath.toStdString(), pluginNamespace);
    }

    MainWindowTest mainWindow;
    auto document = mainWindow.cadMdiChild()->document();
    const auto initialEntityCount =
        document->entityContainer().asVector().size();
    const auto initialBlockCount = document->blocks().size();
    const auto initialPointListeners =
        mainWindow.luaInterface()->listenerCount("point");
    const auto initialMouseMoveListeners =
        mainWindow.luaInterface()->listenerCount("mouseMove");
    const auto initialNumberListeners =
        mainWindow.luaInterface()->listenerCount("number");
    const auto initialTextListeners =
        mainWindow.luaInterface()->listenerCount("text");

    mainWindow.cliCommand()->runCommand("PYGEAR");
    const auto gearOperation = mainWindow.currentOperation();
    ASSERT_FALSE(gearOperation.isNil())
        << "PYGEAR must resolve through the shipped Python plugin registry";
    EXPECT_EQ(gearOperation.getAttr("step").asString(), "AWAIT_TEETH");
    mainWindow.triggerMouseMoved();
    EXPECT_GT(gearOperation.getAttr("_preview_count").asInt(), 0)
        << "a mouse move must create the temporary gear outline";

    // This input combination exercises the rr < rb branch: eight outline
    // vertices per tooth, committed as one closed 64-Line outline.
    mainWindow.triggerNumberEntered(8.0);
    EXPECT_EQ(gearOperation.getAttr("step").asString(), "AWAIT_PHI");
    mainWindow.triggerNumberEntered(20.0);
    EXPECT_EQ(gearOperation.getAttr("step").asString(), "AWAIT_PC");
    mainWindow.triggerNumberEntered(15.0);
    EXPECT_EQ(gearOperation.getAttr("step").asString(), "AWAIT_ORIGIN");
    mainWindow.triggerCoordinateEntered(lc::geo::Coordinate(2.0, 3.0));
    EXPECT_EQ(gearOperation.getAttr("step").asString(), "AWAIT_SCALE");
    mainWindow.triggerCoordinateEntered(lc::geo::Coordinate(4.0, 3.0));

    const auto entities = document->entityContainer().asVector();
    ASSERT_EQ(entities.size(), initialEntityCount + 1)
        << "PYGEAR must commit one block Insert after five input events";
    auto insert = std::dynamic_pointer_cast<const lc::entity::Insert>(
        entities[initialEntityCount]);
    ASSERT_NE(insert, nullptr) << "the new model-space entity must be an Insert";
    const auto block = insert->displayBlock();
    ASSERT_NE(block, nullptr);
    EXPECT_DOUBLE_EQ(insert->position().x(), 2.0);
    EXPECT_DOUBLE_EQ(insert->position().y(), 3.0);
    EXPECT_EQ(document->blocks().size(), initialBlockCount + 1);

    auto blockEntities = document->entitiesByBlock(block).asVector();
    ASSERT_EQ(blockEntities.size(), 64u)
        << "the inserted block must contain the complete 64-Line outline";
    std::vector<std::shared_ptr<const lc::entity::Line>> lines;
    lines.reserve(64);
    for (const auto& entity : blockEntities) {
        auto line = std::dynamic_pointer_cast<const lc::entity::Line>(entity);
        ASSERT_NE(line, nullptr) << "every gear-outline entity must be a Line";
        EXPECT_EQ(line->block(), block);
        lines.push_back(std::move(line));
    }
    ASSERT_EQ(lines.size(), 64u);
    std::sort(lines.begin(), lines.end(), [](const auto& left, const auto& right) {
        return left->id() < right->id();
    });
    for (std::size_t i = 0; i < lines.size(); ++i) {
        const auto& current = lines[i];
        const auto& next = lines[(i + 1) % lines.size()];
        EXPECT_DOUBLE_EQ(current->end().x(), next->start().x());
        EXPECT_DOUBLE_EQ(current->end().y(), next->start().y());
    }
    EXPECT_TRUE(gearOperation.getAttr("finished").asBool());
    EXPECT_EQ(gearOperation.getAttr("_preview_count").asInt(), 0)
        << "finishing the operation must remove all preview Lines";
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount("point"),
              initialPointListeners);
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount("mouseMove"),
              initialMouseMoveListeners);
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount("number"),
              initialNumberListeners);
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount("text"),
              initialTextListeners);

    mainWindow.undo();
    EXPECT_EQ(document->entityContainer().asVector().size(), initialEntityCount)
        << "one undo must remove the Insert";
    EXPECT_EQ(document->blocks().size(), initialBlockCount)
        << "the same undo must remove the block definition";
    EXPECT_TRUE(document->entitiesByBlock(block).asVector().empty())
        << "the same undo must remove the block-local outline";
    mainWindow.redo();
    EXPECT_EQ(document->entityContainer().asVector().size(),
              initialEntityCount + 1)
        << "one redo must restore the Insert";
    EXPECT_EQ(document->blocks().size(), initialBlockCount + 1);
    EXPECT_EQ(document->entitiesByBlock(block).asVector().size(), 64u)
        << "the same redo must restore all block-local Lines";

#if LC_USE_PERSISTENCE
    QTemporaryDir temporaryDirectory;
    ASSERT_TRUE(temporaryDirectory.isValid());
    const auto dxfPath = QDir(temporaryDirectory.path())
                             .filePath("python-gear-roundtrip.dxf")
                             .toStdString();
    ASSERT_TRUE(lc::persistence::File::save(
        document, dxfPath,
        lc::persistence::File::Type::LIBDXFRW_DXF_R2000));

    auto reopenedStorage = std::make_shared<lc::storage::StorageManagerImpl>();
    auto reopenedDocument =
        std::make_shared<lc::storage::DocumentImpl>(reopenedStorage);
    EXPECT_EQ(lc::persistence::File::open(
                  reopenedDocument, dxfPath,
                  lc::persistence::File::Library::LIBDXFRW),
              lc::persistence::File::Type::LIBDXFRW_DXF_R2000);

    const auto reopenedEntities =
        reopenedDocument->entityContainer().asVector();
    ASSERT_EQ(reopenedEntities.size(), 1u)
        << "the DXF must restore only the model-space gear Insert";
    auto reopenedInsert =
        std::dynamic_pointer_cast<const lc::entity::Insert>(
            reopenedEntities.front());
    ASSERT_NE(reopenedInsert, nullptr);
    EXPECT_NEAR(reopenedInsert->position().x(), 2.0, 1e-8);
    EXPECT_NEAR(reopenedInsert->position().y(), 3.0, 1e-8);

    const auto reopenedBlock = reopenedInsert->displayBlock();
    ASSERT_NE(reopenedBlock, nullptr);
    const auto reopenedBlockEntities =
        reopenedDocument->entitiesByBlock(reopenedBlock).asVector();
    ASSERT_EQ(reopenedBlockEntities.size(), 64u)
        << "the DXF must preserve all block-local gear outline Lines";
    std::vector<std::shared_ptr<const lc::entity::Line>> reopenedLines;
    reopenedLines.reserve(reopenedBlockEntities.size());
    for (const auto& entity : reopenedBlockEntities) {
        auto line = std::dynamic_pointer_cast<const lc::entity::Line>(entity);
        ASSERT_NE(line, nullptr)
            << "the reopened gear block must contain only Lines";
        EXPECT_EQ(line->block(), reopenedBlock);
        reopenedLines.push_back(std::move(line));
    }

    const auto samePoint = [](const lc::geo::Coordinate& left,
                              const lc::geo::Coordinate& right) {
        return std::abs(left.x() - right.x()) <= 1e-8
               && std::abs(left.y() - right.y()) <= 1e-8;
    };
    std::vector<bool> used(reopenedLines.size(), false);
    used.front() = true;
    const auto outlineStart = reopenedLines.front()->start();
    auto currentPoint = reopenedLines.front()->end();
    for (std::size_t i = 1; i < reopenedLines.size(); ++i) {
        std::size_t nextIndex = reopenedLines.size();
        lc::geo::Coordinate nextPoint;
        for (std::size_t candidate = 0; candidate < reopenedLines.size();
             ++candidate) {
            if (used[candidate]) {
                continue;
            }
            if (samePoint(reopenedLines[candidate]->start(), currentPoint)) {
                nextIndex = candidate;
                nextPoint = reopenedLines[candidate]->end();
                break;
            }
            if (samePoint(reopenedLines[candidate]->end(), currentPoint)) {
                nextIndex = candidate;
                nextPoint = reopenedLines[candidate]->start();
                break;
            }
        }
        ASSERT_LT(nextIndex, reopenedLines.size())
            << "the reopened block outline must remain connected";
        used[nextIndex] = true;
        currentPoint = nextPoint;
    }
    EXPECT_TRUE(samePoint(currentPoint, outlineStart))
        << "the reopened 64-Line outline must remain closed";
#endif
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MWindowTest, PluginRegistrationSurvivesLuaInterfaceDestructor) {
    // Phase 6 PR-6.1 — multi-window bug regression test.  The bug:
    // `~LuaInterface` used to call
    // `LuaCustomEntityManager::getInstance().removePlugins()`, wiping
    // out custom-entity plugins for the entire process (the manager is
    // a singleton).  Closing one MainWindow silently broke every other
    // window's custom-entity reconstruction on next DXF open.
    //
    // The fix (see luainterface.cpp:31-44): don't call removePlugins()
    // from ~LuaInterface.  Plugin lifecycle is process-scoped.
    //
    // This test proves the fix behaviorally:
    //   1. Install a hook (via `lc.register_plugin`) that registers a
    //      Python callback with the manager.
    //   2. Construct a LuaInterface, then destruct it (simulating one
    //      window close).
    //   3. Verify the plugin is STILL registered in the manager.
    //
    // Post-fix: assertion passes.  Pre-fix: it fails because the
    // ~LuaInterface call cleared the manager's map.
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    // Ensure a clean manager state so previous tests don't skew.
    lc::lua::LuaCustomEntityManager::getInstance().removePlugins();

    // Register a plugin via the neutralized manager directly (ScriptCallback
    // overload).  We use a nativeCallback so the test doesn't depend on
    // Python or Lua state being alive — pure C++ registration.
    int fire_count = 0;
    auto native_cb = lc::scripting::nativeCallback(
        [&fire_count]() {
            fire_count++;
            return lc::scripting::ScriptValue{};
        });
    lc::lua::LuaCustomEntityManager::getInstance().registerPlugin(
        "MultiWindowTestPlugin", native_cb);

    ASSERT_TRUE(lc::lua::LuaCustomEntityManager::getInstance().hasPlugin(
        "MultiWindowTestPlugin"))
        << "manager must have the plugin registered before we simulate "
           "a window close";

    // Now create + destroy a LuaInterface, simulating one MainWindow
    // close.  If the pre-fix behavior returned (removePlugins() in the
    // destructor), the plugin map would be cleared.
    {
        lc::ui::LuaInterface luaInterface;
        // Destructor fires at end of scope.
    }

    EXPECT_TRUE(lc::lua::LuaCustomEntityManager::getInstance().hasPlugin(
        "MultiWindowTestPlugin"))
        << "MULTI-WINDOW BUG: ~LuaInterface must NOT clear the "
           "manager's plugin map.  Plugin registration is process-"
           "scoped; only the singleton's own destructor at process exit "
           "should clear it.  If this assertion fails, someone re-added "
           "removePlugins() to ~LuaInterface — do NOT re-add it.  See "
           "luainterface.cpp:31-44 for the explanation.";

    // Cleanup — clear the manager for the next test.
    lc::lua::LuaCustomEntityManager::getInstance().removePlugins();
}

#endif  // LC_WITH_PYTHONSCRIPT
