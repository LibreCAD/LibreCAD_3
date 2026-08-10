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
#include <gtest/gtest.h>

#include <mainwindow.h>
#include <widgets/scriptdock.h>
#include <cad/storage/document.h>

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
    QApplication app(argc, argv);
    lc::ui::MainWindow mainWindow;
    ScriptDock dock(&mainWindow);
    ASSERT_EQ(entityCount(mainWindow.cadMdiChild()->document()), 0u);

    dock.setInputText(QStringLiteral(R"lua(
        local start_pt = lc.geo.Coordinate(0, 0, 0)
        local end_pt = lc.geo.Coordinate(10, 0, 0)
        local layer = mainWindow:cadMdiChild():activeLayer()
        local line = lc.entity.Line(start_pt, end_pt, layer)
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
    QApplication app(argc, argv);
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
layer    = mdi.document().activeLayer()
line     = lc.entity.Line(start_pt, end_pt, layer)
builder  = lc.operation.EntityBuilder(mdi.document())
builder.appendEntity(line)
builder.execute()
)py"));
    dock.run();

    EXPECT_EQ(entityCount(mainWindow.cadMdiChild()->document()), 1u)
        << "Python leg of ScriptDock must create exactly one Line entity "
           "— proves the mainWindow/document injection + lcgui bindings "
           "reach the kernel correctly";
}

#endif  // LC_WITH_PYTHONSCRIPT
