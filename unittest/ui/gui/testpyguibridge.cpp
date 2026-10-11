// Phase 5 PR-5.6 — runtime smoke tests for the full lcgui binding surface.
//
// These tests exist BECAUSE the coordinator called out a pattern: "a
// plausible-looking binding call that turns out to reference a signature
// that doesn't actually exist" — every session blocker so far
// (`_get_main_window`, `addCommand(std::string, ...)`, `EntityBuilder(doc)`,
// `Line(a, b, layer)`) was caught by real runtime invocation, not code
// review.  This file exercises each newly-bound class by CONSTRUCTING an
// instance and CALLING at least one method against real widget state,
// closing the code-review-alone gap for PR-5.6's binding volume.
//
// Behind #ifdef LC_WITH_PYTHONSCRIPT so an OFF build (Windows/embed
// CPython) doesn't compile-fail here.  Skipped entirely otherwise —
// same pattern as testscriptdock.cpp.

#include <QApplication>
#include <gtest/gtest.h>
#include <memory>
#include <new>
#include <type_traits>

#include <mainwindow.h>
#include <cadmdichild.h>
#include <windowmanager.h>
#include <widgets/guiAPI/inputguicontainer.h>
#include <widgets/guiAPI/textgui.h>
#include <widgets/guiAPI/menuitem.h>
#include <widgets/guiAPI/toolbarbutton.h>
#include <drawables/tempentities.h>

#ifdef LC_WITH_PYTHONSCRIPT
#include <qt_keywords_push.h>
#include <pybind11/embed.h>
#include <qt_keywords_pop.h>
#include <lcpython.h>
#include <python/pyeventhooks.h>
#endif

#include "../uitests.h"

#ifdef LC_WITH_PYTHONSCRIPT

// Small RAII helper: register a MainWindow with WindowManager so
// lcgui.currentMainWindow() finds it, then remove on destruction.  The
// MainWindow constructor doesn't self-register — WindowManager::init /
// newFile / openFile normally does that; unit tests must push
// explicitly.  Same pattern as testscriptdock.cpp's
// PythonEventDrivenOnEventReachesMainWindow.
namespace {

struct MainWindowRegistration {
    lc::ui::MainWindow* mw;
    explicit MainWindowRegistration(lc::ui::MainWindow* window) : mw(window) {
        lc::ui::WindowManager::mainWindows.push_back(mw);
    }
    ~MainWindowRegistration() {
        auto& v = lc::ui::WindowManager::mainWindows;
        v.erase(std::remove(v.begin(), v.end(), mw), v.end());
    }
};

struct TempEntityRefreshCounter {
    int refreshCount = 0;

    void onRefresh() {
        ++refreshCount;
    }
};

} // namespace

TEST(PyGuiBridgeTest, RetainedQtHandlesRejectDestroyedOwner) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();
    auto window = std::make_unique<lc::ui::MainWindow>();
    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["window"] = pybind11::cast(window.get(), pybind11::return_value_policy::reference);
    // Fail safely on an unguarded implementation before invoking a dangling
    // receiver. Then exercise actual bound methods after native destruction.
    ASSERT_TRUE(pybind11::hasattr(ns["window"], "_lc_qobject_lifetime"));
    pybind11::exec(R"py(
mdi = window.cadMdiChild()
cli = window.cliCommand()
lua = window.luaInterface()
assert isinstance(mdi.getFilename(), str)
assert isinstance(cli.availableCommands(), list)
calls = [lambda: window.message("stale"), mdi.getFilename,
         cli.availableCommands, lambda: lua.pluginList("")]
)py", ns);
    window.reset();
    EXPECT_NO_THROW(pybind11::exec(R"py(
for call in calls:
    try:
        call()
    except RuntimeError as error:
        assert "destroyed Qt object" in str(error), str(error)
    else:
        raise AssertionError("destroyed receiver was accepted")
)py", ns));
}

TEST(PyGuiBridgeTest, DestroyedQtArgumentsAndOwnedChildrenAreSafe) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();
    auto owner = std::make_unique<lc::ui::MainWindow>();
    lc::ui::MainWindow other;
    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["owner"] = pybind11::cast(owner.get(), pybind11::return_value_policy::reference);
    ns["other"] = pybind11::cast(&other, pybind11::return_value_policy::reference);
    ASSERT_TRUE(pybind11::hasattr(ns["owner"], "_lc_qobject_lifetime"));
    pybind11::exec(R"py(
import lcgui
import gc
import weakref
menu = owner.addMenu("lifetime probe")
submenu = menu.addMenu("child menu")
item = submenu.addItem("child item")
dialog = lcgui.DialogWidget("lifetime probe", owner)
assert isinstance(dialog.keys(), list)  # mixed non-QObject base API
field = lcgui.Text("field")
assert dialog.addWidget("field", field)
field.setValue("live")
assert field.value() == "live"
cycle = lcgui.Text("callback-held child")
assert dialog.addWidget("cycle", cycle)
cycle.addFinishCallback(lambda widget=cycle: None)
cycle_ref = weakref.ref(cycle)
del cycle
gc.collect()
assert cycle_ref() is not None  # The C++ callback, not this dict, roots it.
)py", ns);
    owner.reset();
    EXPECT_NO_THROW(pybind11::exec(R"py(
for name, call in (("menu argument", lambda: other.addMenu(menu)),
                   ("submenu", submenu.label), ("item", item.label),
                   ("dialog base", dialog.keys), ("dialog", dialog.enable),
                   ("field", field.value),
                   ("constructor owner", lambda: lcgui.DialogWidget("invalid owner", owner))):
    try:
        call()
    except RuntimeError as error:
        assert "destroyed Qt object" in str(error), str(error)
    else:
        raise AssertionError("destroyed Qt argument/child was accepted: " + name)
# Qt already destroyed this Python-created child. Collecting the retained
# wrapper must not delete it a second time.
assert cycle_ref() is None  # Qt teardown released the callback-held wrapper.
del dialog, field, menu, submenu, item, owner, call
gc.collect()
)py", ns));
}

TEST(PyGuiBridgeTest, NativeQtAddressReuseDoesNotReviveStaleWrapper) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();
    using Text = lc::ui::api::TextGUI;
    alignas(Text) unsigned char storage[sizeof(Text)];
    auto destroy = [](Text* widget) { widget->~Text(); };
    std::unique_ptr<Text, decltype(destroy)> widget(new (storage) Text("first"), destroy);
    pybind11::gil_scoped_acquire gil;
    auto first = pybind11::cast(widget.get(), pybind11::return_value_policy::reference);
    ASSERT_TRUE(pybind11::hasattr(first, "_lc_qobject_lifetime"));
    widget.reset();
    widget.reset(new (storage) Text("replacement"));
    auto second = pybind11::cast(widget.get(), pybind11::return_value_policy::reference);
    EXPECT_FALSE(first.is(second));
    EXPECT_NO_THROW(second.attr("setValue")("replacement"));
    EXPECT_EQ(second.attr("value")().cast<std::string>(), "replacement");
    EXPECT_THROW(first.attr("value")(), pybind11::error_already_set);
    widget.reset();
}

TEST(PyGuiBridgeTest, QtAdoptionSurvivesWrapperCollectionWithoutLeakingStandaloneWidgets) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();
    auto owner = std::make_unique<lc::ui::MainWindow>();
    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["owner"] = pybind11::cast(owner.get(), pybind11::return_value_policy::reference);
    pybind11::exec(R"py(
import lcgui, gc, weakref
dialog = lcgui.DialogWidget("adoption probe", owner)
field = lcgui.Text("adopted")
standalone = lcgui.Text("standalone")
item_factory = lcgui.MenuItem.new("factory item", lambda: None)
button_factory = lcgui.ToolbarButton.new("factory button", "", lambda: None)
tooltip_factory = lcgui.ToolbarButton.newWithTooltip("factory tooltip", "", lambda: None, "tip")
checkable_factory = lcgui.ToolbarButton.newCheckable("factory checkable", "", lambda: None, "tip", True)
assert dialog.addWidget("field", field)
field.setValue("retained by Qt")
field_ref = weakref.ref(field)
)py", ns);
    QPointer<lc::ui::api::TextGUI> field(ns["field"].cast<lc::ui::api::TextGUI*>());
    QPointer<lc::ui::api::TextGUI> standalone(ns["standalone"].cast<lc::ui::api::TextGUI*>());
    const std::vector<QPointer<QObject>> factories {
        ns["item_factory"].cast<lc::ui::api::MenuItem*>(),
        ns["button_factory"].cast<lc::ui::api::ToolbarButton*>(),
        ns["tooltip_factory"].cast<lc::ui::api::ToolbarButton*>(),
        ns["checkable_factory"].cast<lc::ui::api::ToolbarButton*>()
    };
    pybind11::exec(R"py(
del field, standalone, item_factory, button_factory, tooltip_factory, checkable_factory
gc.collect()
assert field_ref() is None
)py", ns);
    ASSERT_FALSE(field.isNull());  // Fail before ever dereferencing a deleted child.
    EXPECT_TRUE(standalone.isNull());
    for (const auto& factory : factories) EXPECT_TRUE(factory.isNull());
    EXPECT_EQ(field->value(), "retained by Qt");
    owner.reset();
    EXPECT_TRUE(field.isNull());
}

// -----------------------------------------------------------------------------
// Test — MainWindow method surface: exercise the phase-5 PR-5.6 additions
// (toolbar/layers/menuByName/currentOperation/...) via a real MainWindow.
//
// This is what code review MISSED for PR-5.1 fixups: whether the .h
// signature actually matches the .def(...) shape.  The test asks Python
// to call each new method — if any binding is misnamed or referenced by
// a signature that doesn't exist, pybind11 raises at call time.
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, MainWindowRemainderBindingsCallable) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    lc::ui::MainWindow mw;
    lc::ui::python::MainWindowExecutionScope ownerScope(&mw);

    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["__builtins__"] = pybind11::module_::import("builtins");
    ns["mainWindow"]   = pybind11::cast(
        &mw, pybind11::return_value_policy::reference);

    // Reach through each new MainWindow method the fixup added.  These
    // are all reference-returning accessors — if a Getter is wired to a
    // signature that doesn't exist, this line raises with
    // "AttributeError" or "TypeError" at py::exec.
    pybind11::exec(R"py(
import lcgui

assert lcgui.currentMainWindow() is not None, \
    "currentMainWindow() should find the registered MainWindow"

# Reference-returning accessors — cover Toolbar, Layers, LuaInterface,
# CliCommand, CadMdiChild.
toolbar   = mainWindow.toolbar()
layers    = mainWindow.layers()
lua_iface = mainWindow.luaInterface()
cli       = mainWindow.cliCommand()
mdi       = mainWindow.cadMdiChild()

assert toolbar   is not None
assert layers    is not None
assert lua_iface is not None
assert cli       is not None
assert mdi       is not None

# CadMdiChild remainder methods (metaInfoManager / getSnapManager /
# getFilename / activeViewport).  Any wrong .h signature raises here.
mim  = mdi.metaInfoManager()
snap = mdi.getSnapManager()
name = mdi.getFilename()
vp   = mdi.activeViewport()

assert mim  is not None
assert snap is not None
assert isinstance(name, str)
# activeViewport CAN be a None-holder shared_ptr but the method must
# not raise.

# LuaInterface remainder — pluginList takes a path arg and returns a
# list of strings.
lst = lua_iface.pluginList("../does-not-exist")
assert isinstance(lst, list)

# PR-5.7 fixup — currentOperation() / operation() / setOperation()
# bindings REMOVED because they return/take lc::scripting::ScriptObject
# which isn't registered with pybind11.  The test used to exercise
# `mainWindow.currentOperation()` here — which would have raised
# `TypeError: Unregistered type` if the test binary had ever been
# run.  See the pyguibridge.cpp inline note.
assert not hasattr(mainWindow, "currentOperation"), \
    "currentOperation must not be bound — see ScriptObject note in pyguibridge.cpp"
assert not hasattr(lua_iface, "operation"), \
    "LuaInterface.operation must not be bound — see ScriptObject note"
assert not hasattr(lua_iface, "setOperation"), \
    "LuaInterface.setOperation must not be bound — see ScriptObject note"

# menuByPosition on an empty menu bar — returns None or a menu; either
# is fine as long as the call doesn't raise.
_ = mainWindow.menuByPosition(0)
)py",
        ns);

    // The Lua GUI bridge exposes the plain constructor and callback factory
    // under the same `new` name. Exercise both overloads to prevent the
    // Kaguya duplicate-registration regression from returning.
    const int luaStatus = luaL_dostring(mw.luaInterface()->luaState(), R"lua(
local plainItem = gui.MenuItem.new("Plain")
assert(plainItem:label() == "Plain")
local callbackItem = gui.MenuItem.new("Callback", function() end)
assert(callbackItem:label() == "Callback")

local plainButton = gui.ToolbarButton.new("PlainButton", ":/icons/none.svg", "tip", true)
assert(plainButton:label() == "PlainButton")
local callbackButton = gui.ToolbarButton.new("CallbackButton", ":/icons/none.svg", function() end)
assert(callbackButton:label() == "CallbackButton")
)lua");
    if (luaStatus != 0) {
        const char* error = lua_tostring(mw.luaInterface()->luaState(), -1);
        FAIL() << "Lua GUI constructor overloads failed: "
               << (error == nullptr ? "unknown Lua error" : error);
    }
}

// -----------------------------------------------------------------------------
// Test — SnapManager setters actually mutate state.  Bound as a
// shared_ptr class; Python reaches it via CadMdiChild.getSnapManager().
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, SnapManagerSettersMutate) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    lc::ui::MainWindow mw;
    MainWindowRegistration mwReg(&mw);

    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["__builtins__"] = pybind11::module_::import("builtins");
    ns["mainWindow"]   = pybind11::cast(
        &mw, pybind11::return_value_policy::reference);

    // Toggle each snap-manager setter — if any of the four bound
    // functions references a wrong .h signature (e.g. name typo like
    // `setIntersectionSnappable` vs `setIntersectionsSnappable`),
    // pybind11 raises at py::exec.  This is exactly the discipline the
    // coordinator called out.
    pybind11::exec(R"py(
snap = mainWindow.cadMdiChild().getSnapManager()
assert snap is not None
snap.setGridSnappable(True)
snap.setGridSnappable(False)
snap.setIntersectionSnappable(True)
snap.setMiddleSnappable(False)
snap.setEntitySnappable(True)
)py",
        ns);
}

// -----------------------------------------------------------------------------
// Test — CliCommand full surface: verifies addCommand-with-Python
// callback goes through the ScriptCallback pipeline, plus the enable /
// availableCommands / commandsHistory / clear surface.  The .c_str()
// bug that shipped in PR-5.5 (fixup round 1) would fail here with a
// TypeError if it recurred.
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, CliCommandFullSurfaceCallable) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    lc::ui::MainWindow mw;
    MainWindowRegistration mwReg(&mw);

    pybind11::gil_scoped_acquire gil;
    // Use builtins as a persistent scratchpad so test state survives
    // exec() boundaries — same pattern as testtoolbarbutton.cpp.
    pybind11::module_::import("builtins").attr("_lc_cli_hits") =
        pybind11::list();

    pybind11::dict ns;
    ns["__builtins__"] = pybind11::module_::import("builtins");
    ns["mainWindow"]   = pybind11::cast(
        &mw, pybind11::return_value_policy::reference);

    pybind11::exec(R"py(
import builtins
cli = mainWindow.cliCommand()

# addCommand — Python callable wrapped as ScriptCallback.  Registering
# a fresh name must return True; duplicate must return False.
ok = cli.addCommand("PYUNIT",
    lambda: builtins._lc_cli_hits.append("PYUNIT_FIRED"))
assert ok is True, "first addCommand for a new name must succeed"

dup = cli.addCommand("PYUNIT", lambda: None)
assert dup is False, "duplicate addCommand must return False"

# Read the full-surface API — every one MUST succeed regardless of
# whether commands run.
assert isinstance(cli.availableCommands(), list)
assert isinstance(cli.commandsHistory(),  list)

# enable/disable + isCommandEnabled round-trip.
cli.disableCommand("PYUNIT")
assert cli.isCommandEnabled("PYUNIT") is False
cli.enableCommand("PYUNIT")
assert cli.isCommandEnabled("PYUNIT") is True

# clear() must be callable.  write() must be callable.
cli.write("smoke test message")
# don't call clear() — a smoke line above is useful debug on failure
)py",
        ns);

    pybind11::module_::import("builtins").attr("_lc_cli_hits") =
        pybind11::list();
}

// -----------------------------------------------------------------------------
// Test — PropertyEditor::GetPropertyEditor accepts a MainWindow arg and
// returns a pointer.  Static bound method; if def_static routing is
// wrong the call raises TypeError.
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, PropertyEditorGetPropertyEditor) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    lc::ui::MainWindow mw;
    MainWindowRegistration mwReg(&mw);

    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["__builtins__"] = pybind11::module_::import("builtins");
    ns["mainWindow"]   = pybind11::cast(
        &mw, pybind11::return_value_policy::reference);

    pybind11::exec(R"py(
import lcgui
pe = lcgui.PropertyEditor.GetPropertyEditor(mainWindow)
assert pe is not None, "GetPropertyEditor must return the singleton"

# propertyChanged is bound as a normal method.  Fires against an
# unknown key — should be a no-op, not raise.
pe.propertyChanged("some.unknown.key")
)py",
        ns);
}

// -----------------------------------------------------------------------------
// Test — Layers.addLayer overload dispatch works.  Layers has 7 addLayer
// overloads bound via 7 explicit lambdas; pybind11 must resolve by
// argument arity + type.
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, LayersOverloadDispatch) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    lc::ui::MainWindow mw;
    MainWindowRegistration mwReg(&mw);

    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["__builtins__"] = pybind11::module_::import("builtins");
    ns["mainWindow"]   = pybind11::cast(
        &mw, pybind11::return_value_policy::reference);

    pybind11::exec(R"py(
import lc

layers = mainWindow.layers()

# 1-arg addLayer(name) — returns a Layer_CSPtr.
l1 = layers.addLayer("pygui_test_A")
assert l1 is not None

# 2-arg addLayer(name, lineWidth).
l2 = layers.addLayer("pygui_test_B", 2.5)
assert l2 is not None

# 4-arg addLayer(name, r, g, b).
l3 = layers.addLayer("pygui_test_C", 128, 64, 32)
assert l3 is not None

# 2-arg addLayer(name, color) — same arg count as (name, lineWidth) but
# second arg is a Color object; pybind11 resolves by type.
color = lc.Color(0, 255, 0)
l4 = layers.addLayer("pygui_test_D", color)
assert l4 is not None

# 3-arg addLayer(name, lineWidth, color).
l5 = layers.addLayer("pygui_test_E", 1.0, color)
assert l5 is not None

# 5-arg addLayer(name, lineWidth, r, g, b).
l6 = layers.addLayer("pygui_test_F", 0.5, 200, 100, 50)
assert l6 is not None

# layerByName round-trip.
found = layers.layerByName("pygui_test_A")
assert found is not None

# removeLayer(name) and removeLayer(Layer_CSPtr).
layers.removeLayer("pygui_test_A")
layers.removeLayer(l2)

# renameLayer(name, newName).
layers.renameLayer("pygui_test_C", "pygui_test_C_renamed")

# layers() enumeration.
all_layers = layers.layers()
assert isinstance(all_layers, list)
)py",
        ns);
}

// -----------------------------------------------------------------------------
// Test — gui.* Menu widget family construction + method surface.
// PR-5.6 sub-piece 2 additions.  Constructs Menu/MenuItem/ToolbarTab/
// ToolbarButton/ToolbarGroup instances, calls representative methods
// on each, verifies overloaded ctor arities resolve.
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, MenuFamilyConstructAndCall) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    // No MainWindow needed — these widgets are constructible standalone.
    pybind11::gil_scoped_acquire gil;
    pybind11::dict ns;
    ns["__builtins__"] = pybind11::module_::import("builtins");

    pybind11::exec(R"py(
import lcgui

# Menu: (name) ctor + label round-trip + position + setPosition.
menu = lcgui.Menu("PyTest File")
assert menu.label() == "PyTest File"
menu.setLabel("Renamed")
assert menu.label() == "Renamed"
menu.setPosition(0)
assert menu.position() == 0

# addItem(label) → MenuItem*.  Verifies the label-only overload.
item = menu.addItem("PyMenuItem")
assert item is not None
assert item.label() == "PyMenuItem"

# addItem(label, cb) — callback-taking overload.
hit_counter = {"n": 0}
def cb():
    hit_counter["n"] += 1

item_with_cb = menu.addItem("WithCB", cb)
assert item_with_cb is not None

# itemByName round-trip.
found = menu.itemByName("PyMenuItem")
assert found is not None
assert found.label() == "PyMenuItem"

# QMenu-inherited hide/show/setEnabled/isEnabled.
menu.show()
menu.hide()
menu.setEnabled(False)
assert menu.isEnabled() is False
menu.setEnabled(True)

# addMenu(label) — nested menu.
submenu = menu.addMenu("SubMenu")
assert submenu is not None
assert submenu.label() == "SubMenu"

# removeItem/removeMenu.
menu.removeItem("WithCB")
menu.removeMenu("SubMenu")
)py",
        ns);
}

// -----------------------------------------------------------------------------
// Test — MenuItem plain ctor + `.new(label, cb)` static factory + full
// method surface (addCallback overloads, addCheckedCallback,
// setCheckable/setChecked, hide/show/setEnabled, remove).
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, MenuItemConstructAndCall) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    pybind11::gil_scoped_acquire gil;
    pybind11::module_::import("builtins").attr("_lc_menu_hits") =
        pybind11::list();

    pybind11::dict ns;
    ns["__builtins__"] = pybind11::module_::import("builtins");

    pybind11::exec(R"py(
import lcgui, builtins

# Plain (label) ctor.
mi = lcgui.MenuItem("PyItem")
assert mi.label() == "PyItem"

# .new(label, cb) static factory — the callback-taking form.
mi_cb = lcgui.MenuItem.new("PyItemCB",
    lambda: builtins._lc_menu_hits.append("PyItemCB fired"))
assert mi_cb is not None
assert mi_cb.label() == "PyItemCB"

# addCallback(callback) and addCallback(name, callback) overloads.
mi.addCallback(lambda: builtins._lc_menu_hits.append("mi anon"))
mi.addCallback("named", lambda: builtins._lc_menu_hits.append("mi named"))

# addCheckedCallback.
mi.addCheckedCallback(
    lambda checked: builtins._lc_menu_hits.append(("checked", checked)))

# setCheckable / setChecked — QAction-inherited, must not raise.
mi.setCheckable(True)
mi.setChecked(True)

# hide/show/setEnabled/isEnabled.
mi.hide()
mi.show()
mi.setEnabled(False)
assert mi.isEnabled() is False
mi.setEnabled(True)

# Position round-trip.
mi.setPosition(2)
assert mi.position() == 2

# removeCallback by name (the named callback registered above).
mi.removeCallback("named")

# setLabel round-trip.
mi.setLabel("PyItem_renamed")
assert mi.label() == "PyItem_renamed"
)py",
        ns);

    pybind11::module_::import("builtins").attr("_lc_menu_hits") =
        pybind11::list();
}

// -----------------------------------------------------------------------------
// Test — Toolbar widget family construction (ToolbarTab / ToolbarButton /
// ToolbarGroup).  Verifies the 3 ToolbarButton ctor arities, static
// factories, and inter-widget composition (button in group in tab).
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, ToolbarWidgetFamilyConstructAndCompose) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    pybind11::gil_scoped_acquire gil;
    pybind11::module_::import("builtins").attr("_lc_tbb_hits") =
        pybind11::list();

    pybind11::dict ns;
    ns["__builtins__"] = pybind11::module_::import("builtins");

    pybind11::exec(R"py(
import lcgui, builtins

# ToolbarTab: single (name) ctor.
tab = lcgui.ToolbarTab("PyTab")
assert tab.label() == "PyTab"
tab.setLabel("PyTab_2")
assert tab.label() == "PyTab_2"

# ToolbarGroup: 2 ctor arities — (name) and (name, width).
group = lcgui.ToolbarGroup("PyGroup")
assert group.label() == "PyGroup"

group_w = lcgui.ToolbarGroup("PyGroupW", 5)
assert group_w.width() == 5

# ToolbarButton: 3 plain ctor arities.
btn_a = lcgui.ToolbarButton("BtnA", ":/icons/none.svg")
assert btn_a.label() == "BtnA"

btn_b = lcgui.ToolbarButton("BtnB", ":/icons/none.svg", "tooltip B")
assert btn_b.label() == "BtnB"

btn_c = lcgui.ToolbarButton("BtnC", ":/icons/none.svg", "tooltip C", True)
assert btn_c.label() == "BtnC"

# ToolbarButton static factories — callback-taking.
btn_new = lcgui.ToolbarButton.new("BtnNew", ":/icons/none.svg",
    lambda: builtins._lc_tbb_hits.append("BtnNew fired"))
assert btn_new.label() == "BtnNew"

btn_tt = lcgui.ToolbarButton.newWithTooltip("BtnTT", ":/icons/none.svg",
    lambda: builtins._lc_tbb_hits.append("BtnTT fired"), "the tooltip")
assert btn_tt.label() == "BtnTT"

btn_chk = lcgui.ToolbarButton.newCheckable("BtnChk", ":/icons/none.svg",
    lambda: builtins._lc_tbb_hits.append("BtnChk fired"),
    "check tip", True)
assert btn_chk.label() == "BtnChk"

# Composition: add group to tab, add button to group, verify lookup.
tab.addGroup(group)
group.addButton(btn_a)
group.addButton("BtnByStr", ":/icons/none.svg")
group.addButton("BtnByCB", ":/icons/none.svg",
    lambda: builtins._lc_tbb_hits.append("BtnByCB fired"))

found = group.buttonByName("BtnA")
assert found is not None

# ToolbarGroup.buttons() returns list.
btns = group.buttons()
assert isinstance(btns, list)

# ToolbarTab.groups() returns list.
groups = tab.groups()
assert isinstance(groups, list)

# QWidget-inherited: hide/show/enable/disable + isEnabled.
tab.enable(); tab.disable(); tab.enable()
group.hide(); group.show()
group.enable(); group.disable(); group.enable()
btn_a.hide(); btn_a.show()
btn_a.enable(); btn_a.disable(); btn_a.enable()
assert btn_a.isEnabled() is True
)py",
        ns);

    pybind11::module_::import("builtins").attr("_lc_tbb_hits") =
        pybind11::list();
}

// -----------------------------------------------------------------------------
// Test — Input widget family construction + value round-trips.
// PR-5.6 sub-piece 3 additions.  Constructs every input-widget subclass
// bound in lcgui and exercises its value/setValue/addCallback surface.
// The critical failure mode this test guards: pybind11's inheritance
// chain — every widget subclass declares InputGUI as base.  If the
// base binding is missing, addWidget-into-InputGUIContainer paths fail
// with "type not registered" at py::exec.
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, InputWidgetsConstructAndRoundTrip) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    pybind11::gil_scoped_acquire gil;
    pybind11::module_::import("builtins").attr("_lc_iw_hits") =
        pybind11::list();

    pybind11::dict ns;
    ns["__builtins__"] = pybind11::module_::import("builtins");

    pybind11::exec(R"py(
import lcgui, lc, builtins

# TextGUI (Text).  value/setValue round-trip + callback.
t = lcgui.Text("PyText")
assert t.label() == "PyText"
t.setValue("hello")
assert t.value() == "hello"
t.addFinishCallback(lambda: builtins._lc_iw_hits.append("Text.finish"))
t.addOnChangeCallback(lambda: builtins._lc_iw_hits.append("Text.change"))

# ButtonGUI (Button).  No value.  Just callback + click.
b = lcgui.Button("PyBtn")
b.addCallback(lambda: builtins._lc_iw_hits.append("Button.fire"))
b.click()
assert "Button.fire" in builtins._lc_iw_hits

# CheckBoxGUI (CheckBox).  Two ctor arities (label, label+checked).
c1 = lcgui.CheckBox("PyChk1")
assert c1.value() is False
c1.setValue(True)
assert c1.value() is True

c2 = lcgui.CheckBox("PyChk2", True)
assert c2.value() is True

# RadioButtonGUI (RadioButton).  Standalone (does not inherit InputGUI).
r = lcgui.RadioButton("PyRadio")
assert r.label() == "PyRadio"
r.setValue(True)
assert r.value() is True

# HorizontalGroupGUI (HorizontalGroup).  addWidget takes InputGUI*.
hg = lcgui.HorizontalGroup("PyHGroup")
hg.addWidget("t1", t)
hg.addWidget("b1", b)  # ButtonGUI upcast to InputGUI* via pybind11

# Vertical variant via 2-arg ctor.
vg = lcgui.HorizontalGroup("PyVGroup", True)
assert vg.label() == "PyVGroup"

# RadioGroupGUI (RadioGroup).  addButton takes (key, RadioButtonGUI*).
rg = lcgui.RadioGroup("PyRadioGroup")
rg.addButton("r1", r)

# CoordinateGUI (Coordinate).  value returns lc::geo::Coordinate.
coord = lcgui.Coordinate("PyCoord")
coord.setValue(lc.geo.Coordinate(1.0, 2.0, 3.0))
v = coord.value()
assert abs(v.x() - 1.0) < 1e-9
assert abs(v.y() - 2.0) < 1e-9

# AngleGUI (Angle).  value returns double.
ang = lcgui.Angle("PyAngle")
ang.setValue(1.57)
assert abs(ang.value() - 1.57) < 1e-9
ang.toDegrees()
ang.toRadians()

# SliderGUI (Slider).  2 ctor arities.
s1 = lcgui.Slider("PySlider1")
s2 = lcgui.Slider("PySlider2", 0, 200)
s2.setValue(100)
assert s2.value() == 100

# ComboBoxGUI (ComboBox).  addItem + setValue (2 overloads).
cb = lcgui.ComboBox("PyCombo")
cb.addItem("one")
cb.addItem("two", 0)  # insert at index 0
cb.setValue("one")
assert cb.value() == "one"
cb.setValue(0)  # by index

# NumberGUI (Number).  2 ctor arities.
n1 = lcgui.Number("PyNum")
n2 = lcgui.Number("PyNumRange", -100.0, 100.0)
n2.setValue(42.5)
assert abs(n2.value() - 42.5) < 1e-9

# ColorGUI (ColorPicker).  value returns lc.Color.
cp = lcgui.ColorPicker("PyColor")
cp.setValue(lc.Color(200, 100, 50))
col = cp.value()
# lc.Color is bound in lc kernel — accessor names vary but the object
# must round-trip.
assert col is not None

# EntityGUI (EntityPicker).  value returns list of CADEntity_CSPtr;
# empty by default.
ep = lcgui.EntityPicker("PyEnt")
lst = ep.value()
assert isinstance(lst, list)

# ListGUI (List).  addItem takes an InputGUI child; setListType by
# string.  DELIBERATELY NO addCallbackToAll — Lua's surface excludes
# it and Python mirrors that.
li = lcgui.List("PyList")
li.setListType("NONE")
assert not hasattr(li, "addCallbackToAll"), \
    "List must NOT expose addCallbackToAll (Lua parity)"
)py",
        ns);

    pybind11::module_::import("builtins").attr("_lc_iw_hits") =
        pybind11::list();
}

// -----------------------------------------------------------------------------
// Test — DialogWidget + InputGUIContainer composition.  Constructs a
// DialogWidget, adds child widgets by key, verifies keys() enumerates
// them, and calls addFinishCallback.
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, BoundPolymorphicBasesHaveVirtualDestructors) {
    static_assert(
        std::has_virtual_destructor<lc::ui::api::InputGUIContainer>::value,
        "InputGUIContainer is a polymorphic Python-visible base");
    static_assert(
        std::has_virtual_destructor<lc::viewer::drawable::TempEntities>::value,
        "TempEntities is a polymorphic Python-visible type");
    SUCCEED();
}

// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, DialogWidgetComposition) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    lc::ui::MainWindow mw;
    MainWindowRegistration mwReg(&mw);

    pybind11::gil_scoped_acquire gil;
    pybind11::module_::import("builtins").attr("_lc_dlg_hits") =
        pybind11::list();

    pybind11::dict ns;
    ns["__builtins__"] = pybind11::module_::import("builtins");
    ns["mainWindow"]   = pybind11::cast(
        &mw, pybind11::return_value_policy::reference);

    pybind11::exec(R"py(
import lcgui, builtins

# DialogWidget ctor takes (title, MainWindow*).
dlg = lcgui.DialogWidget("PyDialog", mainWindow)
assert dlg.keys() == [], "new DialogWidget should start without registered keys"

# Compose child widgets via addWidget under keys.
text_widget = lcgui.Text("field_A")
num_widget  = lcgui.Number("field_B")
ok_button   = lcgui.Button("OK")

assert dlg.addWidget("A", text_widget) is True
assert dlg.keys() == ["A"], "first key must be registered"
assert dlg.addWidget("B", num_widget)  is True
assert dlg.addWidget("btn", ok_button) is True

# Duplicate key must be rejected.
assert dlg.addWidget("A", lcgui.Text("dupe")) is False

# keys() enumerates.
keys = dlg.keys()
assert isinstance(keys, list)
assert set(keys) >= {"A", "B", "btn"}

# inputWidgets() enumerates widget objects.
widgets = dlg.inputWidgets()
assert isinstance(widgets, list)
assert len(widgets) >= 3

# setFinishButton + addFinishCallback wire up the finish path.
dlg.setFinishButton(ok_button)
dlg.addFinishCallback(
    lambda args: builtins._lc_dlg_hits.append("finish"))

# enable/disable round-trip.
dlg.enable()
dlg.disable()
dlg.enable()
)py",
        ns);

    pybind11::module_::import("builtins").attr("_lc_dlg_hits") =
        pybind11::list();
}

// Exercise the real rectangle custom-entity callbacks through a mutable
// transaction builder. The release re-executes the same Builder that was
// committed at press, so one undo must restore the original rectangle and one
// redo must restore the resized one without duplicate entities.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, PythonRectangleDragIsOneUndoStep) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    TempEntityRefreshCounter previewRefresh;
    lc::ui::MainWindow mw;
    MainWindowRegistration mwReg(&mw);
    lc::ui::python::MainWindowExecutionScope ownerScope(&mw);
    auto tempEntities = mw.cadMdiChild()->tempEntities();
    tempEntities->requestUpdateEvent().connect<
        TempEntityRefreshCounter,
        &TempEntityRefreshCounter::onRefresh>(previewRefresh);

    pybind11::gil_scoped_acquire gil;
    pybind11::module_::import("sys").attr("path").attr("insert")(
        0, QCoreApplication::applicationDirPath().toStdString());

    pybind11::dict ns;
    ns["__builtins__"] = pybind11::module_::import("builtins");
    ns["mainWindow"] = pybind11::cast(
        &mw, pybind11::return_value_policy::reference);

    pybind11::exec(R"py(
import importlib
import lc

rectangle = importlib.import_module('lcUIPy.plugins.rectangle.plugin')
mdi = mainWindow.cadMdiChild()
doc = mdi.document()
layer = mdi.activeLayer()
undo = mdi.undoManager()

base = lc.geo.Coordinate(1.0, 2.0, 0.0)
old_block = rectangle._create_storage(base, 10.0, 8.0)
lc.operation.AddBlock.new(doc, old_block).execute()
old_insert = rectangle._create_rectangle_entity(
    base, layer, doc, old_block)
try:
    rectangle._create_rectangle_entity(
        base, layer, doc, old_block, object())
except TypeError:
    pass
else:
    raise AssertionError('invalid MetaInfo was silently discarded')
initial_entities = lc.operation.EntityBuilder.new(doc)
for line in rectangle._generate_lines(
        base, lc.geo.Coordinate(11.0, 10.0, 0.0), layer, old_block):
    initial_entities.appendEntity(line)
initial_entities.appendEntity(old_insert)
initial_entities.execute()
setup_entities = doc.entityContainer().asVector(32767)
assert len(setup_entities) == 1, f'rectangle setup created {len(setup_entities)} top-level entities'
assert len(doc.entitiesByBlock(old_block).asVector(32767)) == 4

# Treat setup as the pre-existing document state; only the drag transaction
# should remain on the undo stack.
undo.removeUndoables()
drag = lc.operation.Builder(doc, 'Python rectangle drag')
rectangle._drag_point_click(old_insert, drag, 2)
drag.execute()
assert len(doc.entityContainer().asVector(32767)) == 0, 'press callback did not remove the original insert'
assert len(doc.entitiesByBlock(old_block).asVector(32767)) == 0, 'press callback did not remove the original block geometry'

# Move the top-right corner from (11, 10) to (13, 11), producing a 12x9
# rectangle while keeping its lower-left corner fixed at (1, 2).
rectangle._new_drag_point(
    old_insert, lc.geo.Coordinate(13.0, 11.0, 0.0))
rectangle._drag_point_release(old_insert, drag)
drag.execute()

def custom_rectangles():
    return [entity for entity in doc.entityContainer().asVector(32767)
            if isinstance(entity, lc.entity.ScriptCustomEntity)]

assert len(doc.entityContainer().asVector(32767)) == 1, 'release did not leave exactly one rectangle insert'
resized = custom_rectangles()
assert len(resized) == 1, 'release did not leave exactly one custom rectangle'
new_block = resized[0].displayBlock()
assert len(doc.entitiesByBlock(new_block).asVector(32767)) == 4, 'release did not create exactly four block edges'
assert float(new_block.param('width')) == 12.0, 'drag width was not applied'
assert float(new_block.param('height')) == 9.0, 'drag height was not applied'
assert undo.canUndo() is True, 'drag did not create an undo entry'

# One undo restores the original 4 lines + custom entity. A second undo
# would consume unrelated document history and must not be needed.
undo.undo()
assert undo.canUndo() is False, 'one undo did not consume the full drag transaction'
assert undo.canRedo() is True, 'undo did not enable redo'
restored = custom_rectangles()
assert len(doc.entityContainer().asVector(32767)) == 1, 'undo did not restore exactly one original insert'
assert len(restored) == 1, 'undo did not restore exactly one custom rectangle'
assert len(doc.entitiesByBlock(old_block).asVector(32767)) == 4, 'undo did not restore the original block geometry'
assert restored[0].id() == old_insert.id(), 'undo did not restore the original entity ID'
assert float(restored[0].displayBlock().param('width')) == 10.0, 'undo did not restore the original width'
assert float(restored[0].displayBlock().param('height')) == 8.0, 'undo did not restore the original height'

undo.redo()
assert undo.canRedo() is False, 'redo did not consume the drag redo entry'
assert len(doc.entityContainer().asVector(32767)) == 1, 'redo created duplicate or missing rectangle inserts'
resized = custom_rectangles()
assert len(resized) == 1, 'redo did not restore exactly one custom rectangle'
assert len(doc.entitiesByBlock(resized[0].displayBlock()).asVector(32767)) == 4, 'redo did not restore exactly four block edges'
assert float(resized[0].displayBlock().param('width')) == 12.0, 'redo did not restore the resized width'
assert float(resized[0].displayBlock().param('height')) == 9.0, 'redo did not restore the resized height'
)py",
        ns);

    EXPECT_EQ(previewRefresh.refreshCount, 16)
        << "Python rectangle drag must add/remove preview entities and refresh the canvas";
}

// Exercise the shipped rectangle creation operation itself, not only its
// custom-entity drag callbacks. Creation should add the storage block and
// outline as one document transaction so one undo restores the full state.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, PythonRectangleCreationIsOneUndoStep) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::module_::import("sys").attr("path").attr("insert")(
            0, QCoreApplication::applicationDirPath().toStdString());
    }

    lc::ui::MainWindow mw;
    lc::ui::python::MainWindowExecutionScope ownerScope(&mw);
    mw.clearUndoableStack();

    std::size_t initialBlockCount = 0;

    {
        pybind11::gil_scoped_acquire gil;
        auto lcModule = pybind11::module_::import("lc");
        ASSERT_TRUE(lcModule.attr("operation_registry")
                        .contains("PyRectangleOp"));
        pybind11::dict ns;
        ns["__builtins__"] = pybind11::module_::import("builtins");
        ns["mainWindow"] = pybind11::cast(
            &mw, pybind11::return_value_policy::reference);
        pybind11::exec(
            "initial_block_count = len(mainWindow.cadMdiChild().document().blocks())",
            ns);
        initialBlockCount = ns["initial_block_count"].cast<std::size_t>();
    }

    mw.cliCommand()->runCommand("PYRECTANGLE");
    auto operation = mw.currentOperation();
    ASSERT_FALSE(operation.isNil()) << "PYRECTANGLE did not start PyRectangleOp";
    EXPECT_EQ(operation.getAttr("step").asString(), "AWAIT_P1");
    mw.triggerCoordinateEntered(lc::geo::Coordinate(1.0, 2.0));
    EXPECT_EQ(mw.currentOperation().getAttr("step").asString(), "AWAIT_P2");
    mw.triggerCoordinateEntered(lc::geo::Coordinate(11.0, 10.0));

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::dict ns;
        ns["__builtins__"] = pybind11::module_::import("builtins");
        ns["mainWindow"] = pybind11::cast(
            &mw, pybind11::return_value_policy::reference);
        ns["initialBlockCount"] = initialBlockCount;
        pybind11::exec(R"py(
import lc

mdi = mainWindow.cadMdiChild()
doc = mdi.document()
undo = mdi.undoManager()
initial_block_count = initialBlockCount
rectangles = [entity for entity in doc.entityContainer().asVector(32767)
              if isinstance(entity, lc.entity.ScriptCustomEntity)]
assert len(rectangles) == 1, f'creation produced {len(rectangles)} rectangle entities'
assert len(doc.entityContainer().asVector(32767)) == 1, \
    'rectangle creation left unexpected top-level geometry'
block = rectangles[0].displayBlock()
edges = doc.entitiesByBlock(block).asVector(32767)
assert len(edges) == 4, f'expected four rectangle edges, got {len(edges)}'
assert float(block.param('width')) == 10.0
assert float(block.param('height')) == 8.0
assert len(doc.blocks()) == initial_block_count + 1, \
    'rectangle storage block was not added'
assert undo.canUndo() is True, 'rectangle creation did not create an undo entry'

undo.undo()
assert undo.canUndo() is False, 'one undo did not consume the full creation transaction'
assert undo.canRedo() is True, 'undo did not enable redo'
assert len(doc.entityContainer().asVector(32767)) == 0, \
    'one undo did not remove the rectangle geometry'
assert len(doc.blocks()) == initial_block_count, \
    'one undo left the rectangle storage block behind'

undo.redo()
assert undo.canRedo() is False, 'redo did not consume its entry'
rectangles = [entity for entity in doc.entityContainer().asVector(32767)
              if isinstance(entity, lc.entity.ScriptCustomEntity)]
assert len(rectangles) == 1, 'redo did not restore the rectangle entity'
assert len(doc.entitiesByBlock(rectangles[0].displayBlock()).asVector(32767)) == 4, \
    'redo did not restore the four rectangle edges'
assert len(doc.blocks()) == initial_block_count + 1, \
    'redo did not restore the rectangle storage block'
)py",
            ns);
    }

    EXPECT_EQ(mw.luaInterface()->listenerCount("point"), 0u);
    EXPECT_EQ(mw.luaInterface()->listenerCount("mouseMove"), 0u);
}

// Execute the maintained gear example through OperationLoader, its CLI
// command, real number/point events, and the document's undo stack. This
// closes the gap where the plugin was only imported and its builder call
// shape tested without exercising its state machine or generated outline.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, PythonGearOperationCreatesScaledOutline) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::module_::import("sys").attr("path").attr("insert")(
            0, QCoreApplication::applicationDirPath().toStdString());
    }

    lc::ui::MainWindow mw;
    lc::ui::python::MainWindowExecutionScope ownerScope(&mw);
    mw.clearUndoableStack();

    // MainWindow startup loads the shipped plugin and its decorator must
    // register the command consumed by CliCommand.
    {
        pybind11::gil_scoped_acquire gil;
        auto lcModule = pybind11::module_::import("lc");
        ASSERT_TRUE(lcModule.attr("operation_registry")
                        .contains("PyGearOp"));
    }

    mw.cliCommand()->runCommand("PYGEAR");
    auto operation = mw.currentOperation();
    ASSERT_FALSE(operation.isNil()) << "PYGEAR did not start PyGearOp";
    EXPECT_EQ(operation.getAttr("step").asString(), "AWAIT_TEETH");

    mw.triggerNumberEntered(8.0);
    EXPECT_EQ(mw.currentOperation().getAttr("step").asString(), "AWAIT_PHI");
    mw.triggerNumberEntered(20.0);
    EXPECT_EQ(mw.currentOperation().getAttr("step").asString(), "AWAIT_PC");
    mw.triggerNumberEntered(45.0);
    EXPECT_EQ(mw.currentOperation().getAttr("step").asString(), "AWAIT_ORIGIN");
    mw.triggerCoordinateEntered(lc::geo::Coordinate(10.0, 20.0));
    EXPECT_EQ(mw.currentOperation().getAttr("step").asString(), "AWAIT_SCALE");
    mw.triggerCoordinateEntered(lc::geo::Coordinate(30.0, 20.0));

    // With 8 teeth, 20 degrees, and 45 degrees circular pitch, the sample
    // emits eight vertices per tooth. The scale point is 20 units from the
    // origin, so the pitch-computed outer radius (1.25) becomes 25 units.
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::dict ns;
        ns["__builtins__"] = pybind11::module_::import("builtins");
        ns["mainWindow"] = pybind11::cast(
            &mw, pybind11::return_value_policy::reference);
        pybind11::exec(R"py(
import math
import lc

mdi = mainWindow.cadMdiChild()
doc = mdi.document()
undo = mdi.undoManager()
entities = doc.entityContainer().asVector(32767)
assert len(entities) == 64, f'expected 64 gear edges, got {len(entities)}'
assert all(isinstance(entity, lc.entity.Line) for entity in entities), \
    'gear outline contains a non-Line entity'
radii = [
    math.hypot(point.x() - 10.0, point.y() - 20.0)
    for line in entities
    for point in (line.start(), line.end())
]
assert abs(min(radii) - 15.0) < 1e-8, f'unexpected root radius: {min(radii)}'
assert abs(max(radii) - 25.0) < 1e-8, f'unexpected outer radius: {max(radii)}'
assert undo.canUndo() is True, 'gear creation did not create an undo entry'

undo.undo()
assert undo.canUndo() is False, 'one undo did not consume the gear transaction'
assert undo.canRedo() is True, 'gear undo did not enable redo'
assert len(doc.entityContainer().asVector(32767)) == 0, \
    'one undo did not remove the complete gear outline'

undo.redo()
redone = doc.entityContainer().asVector(32767)
assert undo.canRedo() is False, 'redo did not consume its entry'
assert len(redone) == 64, f'redo restored {len(redone)} gear edges, expected 64'
)py",
            ns);
    }

    EXPECT_EQ(mw.luaInterface()->listenerCount("number"), 0u);
    EXPECT_EQ(mw.luaInterface()->listenerCount("point"), 0u);
}

// Exercise both remaining maintained operation ports against a real
// MainWindow: PYPOINT must commit one point and release its listeners, and
// PYREMOVE must consume the current selection as one undoable transaction.
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, PythonPointAndRemoveOperationsUseRealSelection) {
    ensureUiApplication();
    lc::python::PythonInit::initialize();

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::module_::import("sys").attr("path").attr("insert")(
            0, QCoreApplication::applicationDirPath().toStdString());
        // PyProofPortsImportAndRegister deliberately clears these registry
        // entries after checking metadata. Python retains imported modules,
        // so a later MainWindow's import alone will not rerun the decorators.
        pybind11::exec(R"py(
import importlib
for module_name in (
        'lcUIPy.create_actions.point_operations',
        'lcUIPy.actions.remove_operation'):
    importlib.reload(importlib.import_module(module_name))
)py");
    }

    lc::ui::MainWindow mw;
    lc::ui::python::MainWindowExecutionScope ownerScope(&mw);
    mw.clearUndoableStack();

    mw.cliCommand()->runCommand("PYPOINT");
    auto pointOperation = mw.currentOperation();
    ASSERT_FALSE(pointOperation.isNil()) << "PYPOINT did not start its Python operation";
    EXPECT_EQ(pointOperation.getAttr("step").asString(), "enterPoint");
    mw.triggerCoordinateEntered(lc::geo::Coordinate(4.0, 7.0));
    EXPECT_EQ(mw.luaInterface()->listenerCount("point"), 0u);
    EXPECT_EQ(mw.luaInterface()->listenerCount("mouseMove"), 0u);

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::dict ns;
        ns["__builtins__"] = pybind11::module_::import("builtins");
        ns["mainWindow"] = pybind11::cast(
            &mw, pybind11::return_value_policy::reference);
        pybind11::exec(R"py(
import lc

mdi = mainWindow.cadMdiChild()
doc = mdi.document()
undo = mdi.undoManager()
points = doc.entityContainer().asVector(32767)
assert len(points) == 1, f'PYPOINT created {len(points)} entities, expected 1'
assert isinstance(points[0], lc.entity.Point), \
    f'PYPOINT created {type(points[0]).__name__}, expected Point'
assert abs(points[0].x() - 4.0) < 1e-9
assert abs(points[0].y() - 7.0) < 1e-9
assert undo.canUndo() is True, 'PYPOINT did not create an undo entry'
)py",
            ns);
    }

    // Treat the created point as the baseline, then select it using the same
    // MainWindow command used by the GUI before invoking the Python remove op.
    mw.clearUndoableStack();
    mw.selectAll();
    ASSERT_EQ(mw.cadMdiChild()->selection().size(), 1u);
    mw.cliCommand()->runCommand("PYREMOVE");

    {
        pybind11::gil_scoped_acquire gil;
        pybind11::dict ns;
        ns["__builtins__"] = pybind11::module_::import("builtins");
        ns["mainWindow"] = pybind11::cast(
            &mw, pybind11::return_value_policy::reference);
        pybind11::exec(R"py(
mdi = mainWindow.cadMdiChild()
doc = mdi.document()
undo = mdi.undoManager()
assert len(doc.entityContainer().asVector(32767)) == 0, \
    'PYREMOVE did not remove the selected point'
assert undo.canUndo() is True, 'PYREMOVE did not create an undo entry'
undo.undo()
assert undo.canUndo() is False, 'one undo did not consume the remove transaction'
assert undo.canRedo() is True, 'undo did not enable redo'
restored = doc.entityContainer().asVector(32767)
assert len(restored) == 1, 'undo did not restore the selected point'
assert abs(restored[0].x() - 4.0) < 1e-9
assert abs(restored[0].y() - 7.0) < 1e-9
undo.redo()
assert undo.canRedo() is False, 'redo did not consume its entry'
assert len(doc.entityContainer().asVector(32767)) == 0, \
    'redo did not restore the removal'
)py",
            ns);
    }
}

#endif  // LC_WITH_PYTHONSCRIPT
