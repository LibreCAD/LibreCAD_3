#include<QApplication>
#include <gtest/gtest.h>

#include <luainterface.h>
#include <mainwindow.h>

#include <widgets/toolbar.h>
#include <widgets/guiAPI/toolbartab.h>
#include <widgets/guiAPI/toolbargroup.h>
#include <widgets/guiAPI/toolbarbutton.h>

// Phase 4 PR-3 — ToolbarButton takes ScriptCallback; tests wrap the
// LuaRef via the adapter.
#include <scriptadapter/luacallback.h>

// Phase 4 exit-criterion regression test — Python callback attached to
// a ToolbarButton (see PythonCallbackFires + PythonBoundMethodEquality
// below).  Only exercised when built with LC_WITH_PYTHONSCRIPT.
#ifdef LC_WITH_PYTHONSCRIPT
#include <scriptadapter/pythoncallback.h>
#include <lcpython.h>
#include <pybind11/embed.h>
#endif

#include "../uitests.h"

using namespace lc::ui::widgets;
using namespace lc::ui::api;

TEST(ToolbarButtonTest, LabelAndInitTest) {
    QApplication app(argc, argv);
    Toolbar toolbar(nullptr);
    ToolbarTab* testtab = toolbar.addTab("TestTab");
    ToolbarGroup* testgroup = testtab->addGroup("TestGroup");

    ToolbarButton* testbutton1 = new ToolbarButton("TestButton1", "");
    testgroup->addButton(testbutton1);
    ToolbarButton* testbutton2 = testgroup->addButton("TestButton2", "");

    EXPECT_EQ("TestButton1", testbutton1->label());
    EXPECT_EQ("TestButton2", testbutton2->label());

    EXPECT_EQ(testbutton1, testgroup->buttonByName("TestButton1"));
    EXPECT_EQ(testbutton2, testgroup->buttonByName("TestButton2"));

    testbutton1->setLabel("NewTestButton");

    EXPECT_EQ(nullptr, testgroup->buttonByName("TestButton1"));
    EXPECT_EQ(testbutton1, testgroup->buttonByName("NewTestButton"));
}

TEST(ToolbarButtonTest, TooltipAndRemoveTest) {
    QApplication app(argc, argv);
    Toolbar toolbar(nullptr);
    ToolbarTab* testtab = toolbar.addTab("TestTab");
    ToolbarGroup* testgroup = testtab->addGroup("TestGroup");

    ToolbarButton* testbutton = new ToolbarButton("TestButton1", "", "TestTooltip", true);
    testgroup->addButton(testbutton);

    EXPECT_EQ("TestTooltip", testbutton->toolTip().toStdString());
    EXPECT_TRUE(testbutton->icon().isNull());

    testbutton->setToolTip("NewToolTip");
    EXPECT_EQ("NewToolTip", testbutton->toolTip().toStdString());
    EXPECT_TRUE(testbutton->checkable());

    EXPECT_EQ(testbutton, testgroup->buttonByName("TestButton1"));
    testbutton->remove();
    EXPECT_EQ(nullptr, testgroup->buttonByName("TestButton1"));
}

TEST(ToolbarButtonTest, CallbackTest) {
    QApplication app(argc, argv);
    MainWindow* mainWindow = new MainWindow();

    lc::ui::LuaInterface* luaInterface = mainWindow->luaInterface();
    kaguya::State state(luaInterface->luaState());

    state.dostring("testbutton1 = function() doesThisExist1=true end");
    state.dostring("testbutton2 = function() doesThisExist2=true end");
    state.dostring("testbutton3 = function() doesThisExist3=true end");
    state.dostring("testbutton4 = function() doesThisExist4=true end");
    kaguya::LuaRef cb1 = state["testbutton1"];
    kaguya::LuaRef cb2 = state["testbutton2"];
    kaguya::LuaRef cb3 = state["testbutton3"];
    kaguya::LuaRef cb4 = state["testbutton4"];

    Toolbar* toolbar = mainWindow->toolbar();
    ToolbarTab* testtab = toolbar->addTab("TestTab");
    ToolbarGroup* testgroup = testtab->addGroup("TestGroup");
    // Phase 4 PR-3 — wrap the LuaRef callbacks in ScriptCallback via
    // the Lua adapter.  Behavior identical: the Lua function still gets
    // invoked when the button is clicked.
    ToolbarButton* testbutton = new ToolbarButton("TestButton", "",
                                                   lc::lua::makeLuaCallback(cb1));
    testbutton->addCallback(lc::lua::makeLuaCallback(cb2));

    EXPECT_FALSE(state["doesThisExist1"].get<bool>());
    EXPECT_FALSE(state["doesThisExist2"].get<bool>());

    testgroup->addButton(testbutton);
    testbutton->clicked();

    EXPECT_TRUE(state["doesThisExist1"].get<bool>());
    EXPECT_TRUE(state["doesThisExist2"].get<bool>());

    // Phase 4 PR-3 — getCallback now returns ScriptCallback&, which we
    // compare via operator== (LuaCallbackImpl::equals delegates to
    // kaguya::LuaRef::operator==; same equality as before the refactor).
    EXPECT_EQ(lc::lua::makeLuaCallback(cb2), testbutton->getCallback(1));

    ToolbarButton* testbutton2 = new ToolbarButton("TestButton2", "");
    testgroup->addButton(testbutton2);
    testbutton2->addCallback("Callback1", lc::lua::makeLuaCallback(cb3));
    testbutton2->addCallback("Callback2", lc::lua::makeLuaCallback(cb4));

    EXPECT_FALSE(state["doesThisExist3"].get<bool>());
    EXPECT_FALSE(state["doesThisExist4"].get<bool>());

    testbutton2->removeCallback("Callback1");

    testbutton2->clicked();

    EXPECT_FALSE(state["doesThisExist3"].get<bool>());
    EXPECT_TRUE(state["doesThisExist4"].get<bool>());

    mainWindow->close();
}

#ifdef LC_WITH_PYTHONSCRIPT

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(ToolbarButtonTest, PythonCallbackFires) {
    // Phase 4 exit-criterion regression test — see the phase-4 sub-plan
    // lines 178-179/190 + master plan "Exit:" line: "a py::object
    // wrapped as ScriptCallback attached to a ToolbarButton fires".
    //
    // Establishes the acceptance test that was implicitly required by
    // phase 4 but never written until the coordinator's exit-bar
    // review.  Companion for the register/deregister-with-bound-method
    // property covered by the next test.
    QApplication app(argc, argv);
    lc::python::PythonInit::initialize();

    Toolbar toolbar(nullptr);
    ToolbarTab* testtab = toolbar.addTab("PyTab");
    ToolbarGroup* testgroup = testtab->addGroup("PyGroup");

    // Build a Python callable via the adapter.  Record hits into a
    // builtins-level list so its lifetime spans the register + click +
    // assert windows (a per-exec dict `ns` would be dropped between
    // gil_scoped_acquire scopes).
    lc::scripting::ScriptCallback py_cb;
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::module_::import("builtins").attr("_lc_tbb_hits") =
            pybind11::list();
        pybind11::dict ns;
        ns["__builtins__"] = pybind11::module_::import("builtins");
        pybind11::exec(
            "import builtins\n"
            "_fn = lambda: builtins._lc_tbb_hits.append(1)\n",
            ns);
        pybind11::object fn = ns["_fn"];
        py_cb = lc::python::makePythonCallback(fn);
    }

    ToolbarButton* testbutton = new ToolbarButton("PyButton", "", py_cb);
    testgroup->addButton(testbutton);
    testbutton->clicked();
    testbutton->clicked();

    // Python callback fired twice (once per click).  The Python adapter
    // acquires the GIL inside invoke() — the main thread has been
    // GIL-free since the test setup released it.
    pybind11::gil_scoped_acquire gil;
    pybind11::list hits =
        pybind11::module_::import("builtins").attr("_lc_tbb_hits");
    EXPECT_EQ(pybind11::len(hits), 2u);

    // Cleanup.
    pybind11::module_::import("builtins").attr("_lc_tbb_hits") =
        pybind11::list();
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(ToolbarButtonTest, PythonBoundMethodRegisterDeregister) {
    // Phase 4 exit-criterion regression test — the OTHER half of the
    // sub-plan's requirement: "register/deregister with a bound method
    // works."  This is the "is-or-equal" equality trap:
    //   * bound methods are FRESH py::object per attribute access
    //     (`a.m is a.m` is False in Python);
    //   * but `a.m == a.m` is True because Python's method wrapper
    //     compares equal by (self, function) pair.
    // If the adapter's operator== used only `is`, then
    // `deleteEvent(name, obj.method)` after `registerEvent(name,
    // obj.method)` would never match — leaking a callback per
    // register+deregister cycle.  The Python adapter's operator==
    // therefore does `is OR __eq__`; verify that here by attaching a
    // named callback, removing it by name, and confirming subsequent
    // clicks don't fire the removed slot.
    QApplication app(argc, argv);
    lc::python::PythonInit::initialize();

    Toolbar toolbar(nullptr);
    ToolbarTab* testtab = toolbar.addTab("PyTab");
    ToolbarGroup* testgroup = testtab->addGroup("PyGroup");

    lc::scripting::ScriptCallback cb_first, cb_second;
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::module_::import("builtins").attr("_lc_tbb_bm_hits") =
            pybind11::list();
        pybind11::dict ns;
        ns["__builtins__"] = pybind11::module_::import("builtins");
        pybind11::exec(
            "import builtins\n"
            "class Handler:\n"
            "    def method(self):\n"
            "        builtins._lc_tbb_bm_hits.append(1)\n"
            "_h = Handler()\n",
            ns);
        pybind11::object h = ns["_h"];
        // Access the same bound method TWICE — these are DIFFERENT
        // py::object instances (bound-method identity gotcha).
        cb_first  = lc::python::makePythonCallback(h.attr("method"));
        cb_second = lc::python::makePythonCallback(h.attr("method"));
    }

    // The two ScriptCallbacks SHOULD compare equal despite being backed
    // by distinct py::object bound-method wrappers — Python's `==`
    // succeeds via `is-or-equal` fallback.  This is the design
    // guarantee that lets register+deregister work for Python bound
    // methods; the sub-plan's Python-adapter equality note explicitly
    // documents this.
    EXPECT_TRUE(cb_first == cb_second)
        << "bound-method py::object accessed twice must compare equal "
           "(is-or-equal semantics); otherwise register/deregister "
           "would never match and Python listeners would leak.";

    // Attach via the named-callback API, verify it fires, then remove
    // by name and verify subsequent clicks do NOT fire it.
    ToolbarButton* button = new ToolbarButton("PyBMButton", "");
    testgroup->addButton(button);
    button->addCallback("handler", cb_first);

    button->clicked();
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::list hits =
            pybind11::module_::import("builtins").attr("_lc_tbb_bm_hits");
        EXPECT_EQ(pybind11::len(hits), 1u);
    }

    button->removeCallback("handler");

    button->clicked();
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::list hits =
            pybind11::module_::import("builtins").attr("_lc_tbb_bm_hits");
        EXPECT_EQ(pybind11::len(hits), 1u)
            << "removed named callback must not fire on subsequent "
               "click — the register+deregister roundtrip only works "
               "correctly if the ScriptCallback equality respects "
               "Python's __eq__ semantics.";
    }

    // Cleanup.
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::module_::import("builtins").attr("_lc_tbb_bm_hits") =
            pybind11::list();
    }
}

#endif  // LC_WITH_PYTHONSCRIPT
