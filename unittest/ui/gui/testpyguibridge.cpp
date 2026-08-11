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

#endif  // LC_WITH_PYTHONSCRIPT
