#pragma once

#ifdef LC_WITH_PYTHONSCRIPT

namespace lc {
namespace ui {
namespace python {

/// Run a deterministic installed/build-tree Python runtime smoke and return
/// zero only when interpreter isolation, imports, plugin registration,
/// document mutation, undo/redo, and (when enabled) DXF round-trip pass.
int runPythonRuntimeSmoke();

} // namespace python
} // namespace ui
} // namespace lc

#endif // LC_WITH_PYTHONSCRIPT
