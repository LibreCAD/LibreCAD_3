#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTextEdit>
#include <algorithm>
#include <memory>
#include <thread>

#include <gtest/gtest.h>
#include "mainwindowtest.h"

#include "uitests.h"

#ifdef LC_WITH_PYTHONSCRIPT
#include <qt_keywords_push.h>
#include <pybind11/embed.h>
#include <qt_keywords_pop.h>
#include <lcpython.h>
#include <python/pyeventhooks.h>
#include <python/pypluginloader.h>
#include <lcscripting/scriptvalue.h>
#include <lcscripting/scriptobject.h>
#include <lcscripting/scriptcallback.h>  // Phase 6 PR-6.1 nativeCallback
#include <managers/luacustomentitymanager.h>  // Phase 6 PR-6.1 multi-window bug test
#include <luainterface.h>  // Phase 6 PR-6.1 — construct/destruct LuaInterface directly
#include <widgets/guiAPI/toolbargroup.h>
#include <widgets/guiAPI/toolbarbutton.h>
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

TEST(MWindowTest, PythonPluginBootstrapPrecedesWindowsAndIsIdempotent) {
    ensureUiApplication();
    const auto& report = lc::ui::python::bootstrapPythonPlugins();
    EXPECT_TRUE(report.directoryFound);
    EXPECT_TRUE(report.failures.empty());
    EXPECT_EQ(std::count(report.loadedPlugins.begin(), report.loadedPlugins.end(), "gear"), 1);
    EXPECT_EQ(std::count(report.loadedPlugins.begin(), report.loadedPlugins.end(), "rectangle"), 1);
    EXPECT_TRUE(lc::lua::LuaCustomEntityManager::getInstance().hasPlugin("LC Plugin"));
    EXPECT_EQ(lc::ui::python::currentMainWindow(), nullptr);
    MainWindowTest window;
    lc::ui::python::MainWindowExecutionScope scope(&window);
    EXPECT_EQ(&lc::ui::python::bootstrapPythonPlugins(), &report);
    EXPECT_EQ(lc::ui::python::currentMainWindow(), &window);
    std::string failure;
    std::thread worker([&] {
        try {
            (void)lc::ui::python::bootstrapPythonPlugins();
        } catch (const std::exception& error) {
            failure = error.what();
        }
    });
    worker.join();
    EXPECT_NE(failure.find("Qt application thread"), std::string::npos);
}

TEST(MWindowTest, PythonPluginDefinitionsAreSharedAcrossWindows) {
    ensureUiApplication();
    lc::python::LCPython python;
    auto ns = python.makeNamespace();
    auto first = std::make_unique<MainWindowTest>();
    struct RestoreRegistrationHook {
        lc::python::LCPython& python;
        lc::python::PyNamespace& ns;
        ~RestoreRegistrationHook() {
            (void)python.runString(
                "if '_register_plugin' in globals(): lc.register_plugin = _register_plugin", ns);
        }
    } restore{python, ns};
    ASSERT_EQ(python.runString(R"py(
_definitions = {name: lc.operation_registry[name]
                for name in ('PyGearOp', 'PyRectangleOp')}
_rectangle_globals = _definitions['PyRectangleOp']._init_default.__globals__
_register_plugin = lc.register_plugin
_registrations = []
def _record_registration(name, callback):
    _registrations.append((name, callback))
    _register_plugin(name, callback)
lc.register_plugin = _record_registration
)py", ns), "");

    auto second = std::make_unique<MainWindowTest>();
    EXPECT_EQ(python.runString(R"py(
assert all(lc.operation_registry[name] is cls for name, cls in _definitions.items()), \
    'new window replaced process-global plugin operation definitions'
assert lc.operation_registry['PyRectangleOp']._init_default.__globals__ is _rectangle_globals
assert not _registrations, 'new window replayed global custom-entity registrations'
)py", ns), "");
    for (auto* window : {first.get(), second.get()}) {
        const auto commands = window->cliCommand()->availableCommands();
        EXPECT_EQ(std::count(commands.begin(), commands.end(), "PYGEAR"), 1);
        EXPECT_EQ(std::count(commands.begin(), commands.end(), "PYRECTANGLE"), 1);
        window->cliCommand()->runCommand("PYRECTANGLE");
        ASSERT_FALSE(window->currentOperation().isNil());
        EXPECT_EQ(window->currentOperation().getAttr("step").asString(), "AWAIT_P1");
        EXPECT_EQ(window->luaInterface()->listenerCount("point"), 1u);
    }
    first->triggerCoordinateEntered(lc::geo::Coordinate(1, 2));
    EXPECT_EQ(first->currentOperation().getAttr("step").asString(), "AWAIT_P2");
    EXPECT_EQ(second->currentOperation().getAttr("step").asString(), "AWAIT_P1");
    first.reset();
    EXPECT_EQ(second->luaInterface()->listenerCount("point"), 1u);
    second->triggerCoordinateEntered(lc::geo::Coordinate(3, 4));
    second->triggerCoordinateEntered(lc::geo::Coordinate(13, 12));
    EXPECT_EQ(second->cadMdiChild()->document()->entityContainer().asVector(32767).size(), 1u);
    EXPECT_EQ(second->luaInterface()->listenerCount("point"), 0u);
    EXPECT_EQ(python.runString(R"py(
assert all(lc.operation_registry[name] is cls for name, cls in _definitions.items())
)py", ns), "");
}

TEST(MWindowTest, PythonPluginDefinitionsHaveNoWindowAndFailuresRemainIsolated) {
    ensureUiApplication();
    MainWindowTest window;
    QTemporaryDir directory;
    ASSERT_TRUE(directory.isValid());
    auto writePlugin = [&](const QString& name, const QByteArray& source) {
        const QString folder = QDir(directory.path()).filePath(name);
        if (!QDir().mkpath(folder)) return false;
        QFile file(QDir(folder).filePath("plugin.py"));
        return file.open(QIODevice::WriteOnly) && file.write(source) == source.size();
    };
    ASSERT_TRUE(writePlugin("a_broken", "raise RuntimeError('expected plugin load failure')\n"));
    ASSERT_TRUE(writePlugin("z_probe", R"py(
import builtins
builtins._lc_definition_without_owner = gui.currentMainWindow() is None
assert LC_interface == 'gui'
)py"));
    lc::ui::python::MainWindowExecutionScope scope(&window);
    const auto report = lc::ui::python::loadPythonPlugins(
        directory.path().toStdString(), "gui");
    EXPECT_TRUE(report.directoryFound);
    EXPECT_EQ(report.loadedPlugins, std::vector<std::string>({"z_probe"}));
    ASSERT_EQ(report.failures.size(), 1u);
    EXPECT_NE(report.failures.front().find("expected plugin load failure"), std::string::npos);
    EXPECT_EQ(lc::ui::python::currentMainWindow(), &window);
    lc::python::LCPython python;
    auto ns = python.makeNamespace();
    EXPECT_EQ(python.runString(R"py(
import builtins
owner_free = builtins._lc_definition_without_owner
del builtins._lc_definition_without_owner
assert owner_free, 'global plugin definition captured the loading window'
)py", ns), "");
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MWindowTest, PythonOperationOptionsWireDefaultAndInitSpecificActions) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    struct RegistryEntryCleanup {
        ~RegistryEntryCleanup() {
            pybind11::gil_scoped_acquire gil;
            pybind11::module_::import("lc").attr("operation_registry")
                .attr("pop")("PyToolbarOptionsProbe", pybind11::none());
        }
    } cleanup;

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(R"py(
import lc

class PyToolbarOptionsProbe:
    name = "PyToolbarOptionsProbe"
    command_line = "PYOPTIONS"
    operation_options = {
        "Line": {"icon": "linesnormal.png", "action": "choose_line"},
        "_init_special": {
            "Arc": {"icon": "arc.svg", "action": "choose_arc"},
        },
    }

    def _init_default(self):
        self.selection = "default"

    def _init_special(self):
        self.selection = "special"

    def choose_line(self):
        self.selection = "line"

    def choose_arc(self):
        self.selection = "arc"

lc.register_operation(PyToolbarOptionsProbe)
)py");
    }

    MainWindowTest mainWindow;
    auto findOptionButton = [](lc::ui::api::ToolbarGroup* group,
                               const QString& tooltip) {
        if (group == nullptr) return static_cast<lc::ui::api::ToolbarButton*>(nullptr);
        for (auto* button : group->buttons()) {
            if (button->toolTip() == tooltip) return button;
        }
        return static_cast<lc::ui::api::ToolbarButton*>(nullptr);
    };

    mainWindow.runOperationByName("PyToolbarOptionsProbe");
    auto* currentGroup = mainWindow.toolbar()->tabByName("Quick Access")
        ->groupByName("Current operation");
    ASSERT_NE(currentGroup, nullptr);
    auto* lineOption = findOptionButton(currentGroup, QStringLiteral("Line"));
    ASSERT_NE(lineOption, nullptr);
    lineOption->click();
    EXPECT_EQ(mainWindow.currentOperation().getAttr("selection").asString(),
              "line");

    mainWindow.runOperationByName("PyToolbarOptionsProbe", "_init_special");
    currentGroup = mainWindow.toolbar()->tabByName("Quick Access")
        ->groupByName("Current operation");
    ASSERT_NE(currentGroup, nullptr);
    auto* arcOption = findOptionButton(currentGroup, QStringLiteral("Arc"));
    ASSERT_NE(arcOption, nullptr);
    arcOption->click();
    EXPECT_EQ(mainWindow.currentOperation().getAttr("selection").asString(),
              "arc");

    mainWindow.luaInterface()->finishOperation();
}

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
TEST(MWindowTest, PythonMainWindowExecutionScopeRestoresNestedOwner) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();
    MainWindowTest firstWindow;
    MainWindowTest secondWindow;

    auto currentWindowFromPython = []() -> MainWindow* {
        pybind11::gil_scoped_acquire gil;
        pybind11::object current =
            pybind11::module_::import("lcgui").attr("currentMainWindow")();
        return current.is_none() ? nullptr : current.cast<MainWindow*>();
    };

    EXPECT_EQ(currentWindowFromPython(), nullptr);
    {
        lc::ui::python::MainWindowExecutionScope firstScope(&firstWindow);
        EXPECT_EQ(currentWindowFromPython(), &firstWindow);
        {
            lc::ui::python::MainWindowExecutionScope secondScope(&secondWindow);
            EXPECT_EQ(currentWindowFromPython(), &secondWindow);
        }
        EXPECT_EQ(currentWindowFromPython(), &firstWindow);
    }
    EXPECT_EQ(currentWindowFromPython(), nullptr);

    auto closingWindow = std::make_unique<MainWindowTest>();
    {
        lc::ui::python::MainWindowExecutionScope closingScope(
            closingWindow.get());
        EXPECT_EQ(currentWindowFromPython(), closingWindow.get());
        closingWindow.reset();
        EXPECT_EQ(currentWindowFromPython(), nullptr);
    }
    EXPECT_EQ(currentWindowFromPython(), nullptr);
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
TEST(MWindowTest, PythonOperationCloseFailureReportsOnceAndCleansEvents) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();
    MainWindowTest mainWindow;

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(R"py(
import builtins
import lc

builtins._lc_close_failure_calls = 0
def _lc_close_failure_listener(event, args):
    pass

@lc.register_operation
class PyCloseFailureProbe:
    name = "PyCloseFailureProbe"
    def _init_default(self):
        lc.event.register("_lc_close_failure_cleanup", _lc_close_failure_listener)
    def close(self):
        builtins._lc_close_failure_calls += 1
        lc.event.register(
            "_lc_close_failure_cleanup_from_close",
            _lc_close_failure_listener)
        raise RuntimeError("intentional operation close failure")
)py");
    }

    mainWindow.runOperationByName("PyCloseFailureProbe");
    ASSERT_FALSE(mainWindow.currentOperation().isNil());
    ASSERT_EQ(mainWindow.luaInterface()->listenerCount(
                  "_lc_close_failure_cleanup"), 1u);
    ASSERT_EQ(mainWindow.luaInterface()->listenerCount(
                  "_lc_close_failure_cleanup_from_close"), 0u);

    mainWindow.luaInterface()->finishOperation();
    QCoreApplication::processEvents();

    EXPECT_EQ(mainWindow.luaInterface()->listenerCount(
                  "_lc_close_failure_cleanup"), 0u);
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount(
                  "_lc_close_failure_cleanup_from_close"), 0u);
    auto* history = mainWindow.cliCommand()->findChild<QTextEdit*>(
        QStringLiteral("history"));
    ASSERT_NE(history, nullptr);
    EXPECT_EQ(history->toPlainText().count(
                  QStringLiteral("intentional operation close failure")), 1);
    {
        pybind11::gil_scoped_acquire gil;
        EXPECT_EQ(pybind11::module_::import("builtins")
                      .attr("_lc_close_failure_calls").cast<int>(), 1);
        pybind11::exec(R"py(
import builtins
import lc
lc.operation_registry.pop("PyCloseFailureProbe", None)
del builtins._lc_close_failure_calls
)py");
    }
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MWindowTest, PythonOperationCallbacksTrackAndCancelTheirEventRegistrations) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();
    MainWindowTest mainWindow;

    struct PythonStateCleanup {
        ~PythonStateCleanup() {
            pybind11::gil_scoped_acquire gil;
            pybind11::exec(R"py(
import builtins
import lc
lc.operation_registry.pop("PyOperationEventTrackingProbe", None)
for _name in ("_lc_operation_late_listener",):
    if hasattr(builtins, _name):
        delattr(builtins, _name)
if hasattr(builtins, "_lc_operation_close_calls"):
    del builtins._lc_operation_close_calls
)py");
        }
    } cleanup;

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(R"py(
import builtins
import lc

def _lc_operation_late_listener(event, args):
    pass
builtins._lc_operation_late_listener = _lc_operation_late_listener
builtins._lc_operation_close_calls = 0

@lc.register_operation
class PyOperationEventTrackingProbe:
    name = "PyOperationEventTrackingProbe"
    def _init_default(self):
        lc.event.register("_lc_operation_dynamic_trigger", self)
        lc.event.register("_lc_operation_failure_trigger", self)
    @property
    def step(self):
        lc.event.register(
            "_lc_operation_read_attribute", builtins._lc_operation_late_listener)
        return "pick"
    @step.setter
    def step(self, value):
        lc.event.register(
            "_lc_operation_write_attribute", builtins._lc_operation_late_listener)
    def register_from_action(self):
        return lc.event.register(
            "_lc_operation_action_registered",
            builtins._lc_operation_late_listener)
    def onEvent(self, event, args):
        if event == "_lc_operation_dynamic_trigger":
            lc.event.register(
                "_lc_operation_late_registered",
                builtins._lc_operation_late_listener)
        elif event == "_lc_operation_failure_trigger":
            raise RuntimeError("intentional operation event failure")
    def close(self):
        builtins._lc_operation_close_calls += 1
        lc.event.register(
            "_lc_operation_close_registered",
            builtins._lc_operation_late_listener)
)py");
    }

    mainWindow.runOperationByName("PyOperationEventTrackingProbe");
    ASSERT_EQ(mainWindow.luaInterface()->listenerCount(
                  "_lc_operation_dynamic_trigger"), 1u);
    ASSERT_EQ(mainWindow.luaInterface()->listenerCount(
                  "_lc_operation_failure_trigger"), 1u);

    std::string error;
    EXPECT_EQ(mainWindow.luaInterface()->getOperationAttribute("step").asString(),
              "pick");
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount(
                  "_lc_operation_read_attribute"), 2u);
    ASSERT_TRUE(mainWindow.luaInterface()->setOperationAttributeChecked(
        "step", lc::scripting::ScriptValue("next"), error)) << error;
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount(
                  "_lc_operation_write_attribute"), 1u);

    lc::scripting::ScriptValue result;
    ASSERT_TRUE(mainWindow.luaInterface()->callOperationMethodChecked(
        "register_from_action", {}, result, error)) << error;
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount(
                  "_lc_operation_action_registered"), 1u);

    mainWindow.luaInterface()->triggerEvent(
        "_lc_operation_dynamic_trigger", lc::scripting::ScriptValue{});
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount(
                  "_lc_operation_late_registered"), 1u);

    int unrelatedCalls = 0;
    const auto unrelatedListener = lc::scripting::nativeCallback(
        [&unrelatedCalls](const std::vector<lc::scripting::ScriptValue>&) {
            ++unrelatedCalls;
            return lc::scripting::ScriptValue{};
        });
    mainWindow.luaInterface()->registerEvent(
        "_lc_operation_failure_trigger", unrelatedListener);
    mainWindow.luaInterface()->triggerEvent(
        "_lc_operation_failure_trigger", lc::scripting::ScriptValue{});

    EXPECT_EQ(unrelatedCalls, 1);
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount(
                  "_lc_operation_dynamic_trigger"), 0u);
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount(
                  "_lc_operation_failure_trigger"), 1u);
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount(
                  "_lc_operation_read_attribute"), 0u);
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount(
                  "_lc_operation_write_attribute"), 0u);
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount(
                  "_lc_operation_action_registered"), 0u);
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount(
                  "_lc_operation_late_registered"), 0u);
    EXPECT_EQ(mainWindow.luaInterface()->listenerCount(
                  "_lc_operation_close_registered"), 0u);
    EXPECT_TRUE(mainWindow.currentOperation().isNil());
    {
        pybind11::gil_scoped_acquire gil;
        EXPECT_EQ(pybind11::module_::import("builtins")
                      .attr("_lc_operation_close_calls").cast<int>(), 1);
    }
    EXPECT_TRUE(mainWindow.luaInterface()->deleteEvent(
        "_lc_operation_failure_trigger", unrelatedListener));
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
        // Leave the outer C++ wrappers empty before releasing the GIL;
        // otherwise their final decref of Python None runs without it.
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
        pybind11::dict ns;
        pybind11::exec(R"py(
import lc
import importlib.util
names = ('PyPointOperations', 'PyRemoveOperation')
saved = {name: lc.operation_registry[name] for name in names if name in lc.operation_registry}
try:
    for name in names:
        lc.operation_registry.pop(name, None)
    # Execute the actual files under private aliases. Do not reload cached
    # production modules or leave once-only bootstrap registrations missing.
    for module_name in ('lcUIPy.create_actions.point_operations',
                        'lcUIPy.actions.remove_operation'):
        source = importlib.util.find_spec(module_name)
        assert source is not None and source.origin, module_name
        spec = importlib.util.spec_from_file_location(module_name + '_proof_test', source.origin)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
    p_cls = lc.operation_registry['PyPointOperations']
    r_cls = lc.operation_registry['PyRemoveOperation']
    assert p_cls.command_line == 'PYPOINT'
    assert p_cls.icon == 'point.svg'
    assert p_cls.description == 'Python Point'
    assert p_cls.menu_actions == {'default': 'actionPoint'}
    assert r_cls.command_line == 'PYREMOVE'
    assert r_cls.icon == 'delete.svg'
    assert r_cls.description == 'Python Remove'
finally:
    for name in names:
        lc.operation_registry.pop(name, None)
    lc.operation_registry.update(saved)
)py", ns);
    }
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
