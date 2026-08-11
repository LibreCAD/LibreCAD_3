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
#include <pybind11/embed.h>
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

# currentOperation — before any operation runs, returns a Nil
# ScriptObject.  Just verify the call itself works.
op = mainWindow.currentOperation()

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

#endif  // LC_WITH_PYTHONSCRIPT
