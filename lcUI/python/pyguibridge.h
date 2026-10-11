#pragma once

// Phase 3 PR-3.2 — minimal Python GUI bindings.
//
// Second embedded pybind11 module, sibling of `lc` (which is defined by
// lcpythonscript's PYBIND11_EMBEDDED_MODULE).  The `lcgui` module holds
// bindings for QWidget-owned classes that scripts running inside
// ScriptDock need to reach: MainWindow, CadMdiChild.  Everything is
// returned by reference because Qt owns them. QtClass installs a QPointer
// lifetime guard on every wrapper; the shared call-boundary policy rejects
// destroyed receivers and arguments. Python-created widgets retain their
// default holder until a bound call gives them a Qt parent; the holder is
// then released before Qt teardown can release callback-held wrappers.
// Qt destruction invalidates retained handles and removes dead addresses
// from pybind11's wrapper cache. DialogWidget is
// parented to its supplied MainWindow. Shared kernel/viewer objects keep
// their normal shared_ptr ownership and are not Qt lifetime wrappers.
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
// Build gate: LC_WITH_PYTHONSCRIPT is propagated by the lcpythonscript
// target to consumers; lcUI links that target when Python support is enabled.

#ifdef LC_WITH_PYTHONSCRIPT

// This header intentionally has no declarations — the embedded module
// registers itself via the pybind11 macro in pyguibridge.cpp.  The
// header exists as a build-system anchor so the .cpp is picked up in
// AUTOMOC/CMake without a stray "unused header" warning.

#endif  // LC_WITH_PYTHONSCRIPT
