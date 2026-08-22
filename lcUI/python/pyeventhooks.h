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

class MainWindow;

namespace python {

/// Install both hooks.  Idempotent — later calls overwrite.  Safe to
/// call at MainWindow construction or from ScriptDock's Python init.
/// Phase 5 PR-5.1 fixup: MUST be called unconditionally at MainWindow
/// startup — the ScriptDock-only install site was broken because
/// Python operations registering via `lc.event.register` at startup
/// arrived BEFORE the ScriptDock had ever been opened, so the hook
/// slots were still default-constructed (falsy) and every registration
/// silently no-op'd.
void installEventHooks();

/// Return the currently-active MainWindow (most-recently-added in
/// WindowManager::mainWindows).  Returns nullptr when no window
/// exists (headless CLI mode).
///
/// Phase 5 PR-5.1 fixup: this is what CreateOperations's
/// `_get_main_window()` calls INSTEAD of frame-walking.  The frame
/// walk failed for event-driven dispatch — PythonCallbackImpl::invokeEvent
/// calls onEvent fresh from C++ (Qt slot → EventBus → invoke), so
/// there's no Python frame containing `mainWindow` to find.  The
/// direct C++-side lookup works from any calling context.
lc::ui::MainWindow* currentMainWindow();

} // namespace python
} // namespace ui
} // namespace lc

#endif  // LC_WITH_PYTHONSCRIPT
