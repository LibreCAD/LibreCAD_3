#pragma once

// Phase 5 PR-5.1 — lcUI-side installer for the lc.event.register /
// lc.event.deregister hooks defined in lcadpythonscript's
// bridge/py_lc_event.h.
//
// The hooks route py::object callbacks into the ACTIVE MainWindow's
// LuaInterface EventBus via `makePythonCallback`.  Installed once at
// MainWindow construction (or lazily from ScriptDock's Python setup) —
// re-install replaces the previous hook (matches the OpaquePtr encoder
// registry semantics in lcadluascript).
//
// Which MainWindow the hook picks up is defined by
// `WindowManager::getMainWindows().back()` — most recently added, which
// is the "current" one in the single-window-at-a-time use case.  Multi-
// window is out of scope for this phase; phase 6 revisits window
// lifecycle.

#ifdef LC_WITH_PYTHONSCRIPT

namespace lc {
namespace ui {
namespace python {

/// Install both hooks.  Idempotent — later calls overwrite.  Safe to
/// call at MainWindow construction or from ScriptDock's Python init.
void installEventHooks();

} // namespace python
} // namespace ui
} // namespace lc

#endif  // LC_WITH_PYTHONSCRIPT
