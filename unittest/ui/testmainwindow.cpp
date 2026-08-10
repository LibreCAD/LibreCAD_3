#include<QApplication>

#include <gtest/gtest.h>
#include "mainwindowtest.h"

#include "uitests.h"

#ifdef LC_WITH_PYTHONSCRIPT
#include <pybind11/embed.h>
#include <lcpython.h>
#include <lcscripting/scriptvalue.h>
#include <lcscripting/scriptobject.h>
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

#endif  // LC_WITH_PYTHONSCRIPT
