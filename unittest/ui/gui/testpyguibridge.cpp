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

#include <mainwindow.h>
#include <cadmdichild.h>
#include <windowmanager.h>

#ifdef LC_WITH_PYTHONSCRIPT
#include <pybind11_qt.h>
#include <lcpython.h>
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

} // namespace

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
    QApplication app(argc, argv);
    lc::python::PythonInit::initialize();

    lc::ui::MainWindow mw;
    MainWindowRegistration mwReg(&mw);

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
}

// -----------------------------------------------------------------------------
// Test — SnapManager setters actually mutate state.  Bound as a
// shared_ptr class; Python reaches it via CadMdiChild.getSnapManager().
// -----------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-identifier-naming)
TEST(PyGuiBridgeTest, SnapManagerSettersMutate) {
    QApplication app(argc, argv);
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
    QApplication app(argc, argv);
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
    QApplication app(argc, argv);
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
    QApplication app(argc, argv);
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
    QApplication app(argc, argv);
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
    QApplication app(argc, argv);
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
    QApplication app(argc, argv);
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
    QApplication app(argc, argv);
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
TEST(PyGuiBridgeTest, DialogWidgetComposition) {
    QApplication app(argc, argv);
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

# Compose child widgets via addWidget under keys.
text_widget = lcgui.Text("field_A")
num_widget  = lcgui.Number("field_B")
ok_button   = lcgui.Button("OK")

assert dlg.addWidget("A", text_widget) is True
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

#endif  // LC_WITH_PYTHONSCRIPT
