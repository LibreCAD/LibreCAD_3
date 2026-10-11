#pragma once

// Phase 5 PR-5.1 — lcUI-side installer for the lc.event.register /
// lc.event.deregister hooks defined in lcadpythonscript's
// bridge/py_lc_event.h.
//
// The hooks route py::object callbacks into the owning MainWindow's
// LuaInterface EventBus via `makePythonCallback`. Process-wide hook functions
// consult a nested execution scope rather than guessing from window insertion
// order, so constructor-time registration and multiple windows are supported.

#ifdef LC_WITH_PYTHONSCRIPT

#include <QPointer>

namespace lc {
namespace ui {

class MainWindow;

namespace python {

/// Temporarily bind Python's current-window lookup to the UI owner whose
/// code is executing. Scopes nest and restore their predecessor.
class MainWindowExecutionScope {
public:
    explicit MainWindowExecutionScope(MainWindow* window);
    ~MainWindowExecutionScope();

    MainWindowExecutionScope(const MainWindowExecutionScope&) = delete;
    MainWindowExecutionScope& operator=(const MainWindowExecutionScope&) = delete;

private:
    QPointer<MainWindow> _previous;
};

/// Install the process hook functions. Idempotent; safe to call at startup.
/// Bootstrap installs these before process-global plugin definitions and
/// per-window operation init; installing only when ScriptDock opens is too late.
void installEventHooks();

/// Return the MainWindow bound to the current execution scope, or nullptr
/// when no UI owner is active (including headless calls). Do not infer an
/// owner from vector insertion order: a window runs Python during its
/// constructor, before it can be inserted into WindowManager.
lc::ui::MainWindow* currentMainWindow();

} // namespace python
} // namespace ui
} // namespace lc

#endif  // LC_WITH_PYTHONSCRIPT
