#include<QApplication>

#include <gtest/gtest.h>
#include "mainwindowtest.h"

#include "uitests.h"

#ifdef LC_WITH_PYTHONSCRIPT
#include <pybind11_qt.h>
#include <lcpython.h>
#include <lcscripting/scriptvalue.h>
#include <lcscripting/scriptobject.h>
#include <lcscripting/scriptcallback.h>  // Phase 6 PR-6.1 nativeCallback
#include <managers/luacustomentitymanager.h>  // Phase 6 PR-6.1 multi-window bug test
#include <luainterface.h>  // Phase 6 PR-6.1 — construct/destruct LuaInterface directly
#endif

TEST(MWindowTest, WindowWidgetTest) {
    QApplication app(argc, argv);
    MainWindowTest* mainWindow = new MainWindowTest();

    EXPECT_TRUE(mainWindow->testAddingOfWidgets());
    EXPECT_TRUE(mainWindow->testAddingOfSelectTools());
}

TEST(MWindowTest, PointTriggerTest) {
    QApplication app(argc, argv);
    MainWindowTest* mainWindow = new MainWindowTest();

    // test if last point is updated
    EXPECT_TRUE(mainWindow->testTriggerPoint(lc::geo::Coordinate(2,2)));
}

TEST(MWindowTest, OperationFinishedTest) {
    QApplication app(argc, argv);
    MainWindowTest* mainWindow = new MainWindowTest();

    EXPECT_TRUE(mainWindow->testOperationFinished());
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(MWindowTest, TriggerEventArrayPayloadPreserved) {
    // Phase 4 post-review regression test — verify
    // LuaInterface::triggerEvent(LuaRef) passes array-table + function
    // payloads through to Lua listeners without the lossy
    // fromLua/toLua round-trip.
    QApplication app(argc, argv);
    MainWindowTest* mainWindow = new MainWindowTest();

    EXPECT_TRUE(mainWindow->testTriggerEventArrayPayloadPreserved());
}

TEST(MWindowTest, RunOperationTest) {
    QApplication app(argc, argv);
    MainWindowTest* mainWindow = new MainWindowTest();

    EXPECT_TRUE(mainWindow->testRunOperation());
}

TEST(MWindowTest, AddOperationOptionsTest) {
    QApplication app(argc, argv);
    MainWindowTest* mainWindow = new MainWindowTest();

    EXPECT_TRUE(mainWindow->testAddOperationOptions());
}

TEST(MWindowTest, ConnectMenuTest) {
    QApplication app(argc, argv);
    MainWindowTest* mainWindow = new MainWindowTest();

    EXPECT_TRUE(mainWindow->testConnectMenuItem());
}

TEST(MWindowTest, MenuAPITest) {
    QApplication app(argc, argv);
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
    QApplication app(argc, argv);
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
    QApplication app(argc, argv);
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
    // The lcUIPy dir isn't auto-added to sys.path in this unit-test
    // binary (path.py is a Qt-only bootstrap), so we manually push
    // the source-tree lcUIPy path onto sys.path before import.  The
    // resolution is: SOURCE_DIR is set by CMake for the WITH_QT_UI
    // tests when `add_compile_definitions(SOURCE_DIR=...)` runs, but
    // that's under WITH_RENDERING_UNITTESTS only — for our purposes
    // just walk from __file__.  Or simpler: use CMAKE_CURRENT_LIST_DIR
    // via a compile-time macro.  Simplest: infer from
    // QCoreApplication::applicationDirPath() and walk up.
    QApplication app(argc, argv);
    lc::python::PythonInit::initialize();

    // The lcUIPy dir sits at LIBRECAD_SRC_ROOT/lcUIPy.  For the source
    // tree we walk up from __file__ (the test binary's applicationDirPath
    // is CMAKE_RUNTIME_OUTPUT_DIRECTORY = build/bin/, which is a sibling
    // of build/{lcUI,lcUIPy,...}; walking up 2 lands on build/, walking
    // up 3 lands on the project root).  Since the test also runs from a
    // scratchpad build tree, we use a resilient search: try  a few known
    // relative parents and pick the first that contains lcUIPy/.
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(R"py(
import os, sys
# Search upward from the applicationDirPath (bin/) for a directory
# containing lcUIPy/create_actions/point_operations.py.  This test
# needs to work in both the source-tree scratchpad and the
# canonical build.
_search_from = os.getcwd()
_found = None
for _i in range(6):
    _cand = os.path.join(_search_from, 'lcUIPy',
                         'create_actions', 'point_operations.py')
    if os.path.isfile(_cand):
        _found = os.path.dirname(os.path.dirname(_cand))
        break
    _search_from = os.path.dirname(_search_from)

if _found is not None:
    _parent = os.path.dirname(_found)
    if _parent not in sys.path:
        sys.path.insert(0, _parent)
del _found, _search_from
)py");
    }

    // Import the proof-port modules directly.  If they raise (missing
    // dep, syntax error, decorator failure), py::exec propagates the
    // exception and the test fails.
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::exec(R"py(
import lc

# The test infra may not have lcUIPy on sys.path if the scratchpad
# search above didn't find it — skip gracefully in that case.  A
# CI build with the correct working directory will exercise this test.
import importlib.util
if importlib.util.find_spec('lcUIPy') is None:
    # Cannot import — the test is a no-op in this environment.
    # Fail loudly so the discovery gap is visible.
    raise RuntimeError(
        'lcUIPy not on sys.path — the source-tree search couldn\\'t '
        'locate it.  Run this test from a directory where the '
        'source-tree lcUIPy/ is reachable within 6 parents.')

# Ensure clean slate — the tests above may have injected transient
# PyDemoOp / PyDictCmdOp classes.
if 'PyPointOperations' in lc.operation_registry:
    del lc.operation_registry['PyPointOperations']
if 'PyRemoveOperation' in lc.operation_registry:
    del lc.operation_registry['PyRemoveOperation']

# Import — triggers @lc.register_operation at module top-level.
import lcUIPy.create_actions.point_operations
import lcUIPy.actions.remove_operation

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
    QApplication app(argc, argv);
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
