#pragma once

// Phase 3 PR-3.2 — minimal Python GUI bindings.
//
// Second embedded pybind11 module, sibling of `lc` (which is defined by
// lcpythonscript's PYBIND11_EMBEDDED_MODULE).  The `lcgui` module holds
// bindings for QWidget-owned classes that scripts running inside
// ScriptDock need to reach: MainWindow, CadMdiChild.  Everything is
// bound HOLDER-LESS (raw pointer / `return_value_policy::reference`)
// because Qt owns these widgets — Python must NOT try to delete them.
// This matches Lua's raw-pointer exposure of the same classes; a
// dangling Python reference after window close is the deliberate parity
// (same failure mode as Lua on the same close).  Do not "fix" with a
// holder.
//
// The module body's FIRST statement is `py::module_::import("lc")` so
// lc's kernel bindings are registered before lcgui's signatures need
// them; importing lcgui before lc would raise "Unregistered type" on
// e.g. `Document` or `CADEntity` return types.
//
// Kernel types are NOT re-bound here (duplicate registration throws).
// Scope for phase 3: MainWindow (message/cadMdiChild/undo/redo),
// CadMdiChild (document/selection/tempEntities/autoScale).  The full
// gui.* widget API is phase 5.
//
// Build gate: LC_WITH_PYTHONSCRIPT (root CMakeLists.txt adds -D
// globally when WITH_PYTHONSCRIPT=ON; lcUI/CMakeLists.txt links the
// required libraries).

#ifdef LC_WITH_PYTHONSCRIPT

// This header intentionally has no declarations — the embedded module
// registers itself via the pybind11 macro in pyguibridge.cpp.  The
// header exists as a build-system anchor so the .cpp is picked up in
// AUTOMOC/CMake without a stray "unused header" warning.

#endif  // LC_WITH_PYTHONSCRIPT
