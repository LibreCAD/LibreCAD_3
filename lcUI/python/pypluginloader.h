#pragma once

// Phase 5 PR-5.5 — Python plugin loader.
//
// Iterates the Python plugin directory (set by path.py's `plugin_path`
// global) and for each subdirectory, loads `plugin.py`.  Mirrors
// lcadluascript's PluginManager but for Python: `LC_interface` is
// injected into the exec namespace with the IDENTICAL name Lua uses
// ("gui"/"cli") — no case change — so a plugin author can `import`
// the same identifier from either language.
//
// Failure policy matches PluginManager's Lua analog: missing
// `plugin_path` → silent no-op (path.py may not have been staged in
// unittest fixtures); missing `plugin.py` in a subdir → skip that
// subdir; per-plugin exceptions → log to stderr and continue with the
// next plugin (a broken plugin shouldn't abort init).

#ifdef LC_WITH_PYTHONSCRIPT

#include <string>

namespace lc {
namespace ui {
namespace python {

/// Load `plugin.py` from every immediate subdirectory of the given
/// path.  `interfaceName` (e.g. "gui" or "cli") is set as
/// `LC_interface` in the exec namespace before each plugin runs, so
/// plugin authors can dispatch on the runtime host.
void loadPythonPlugins(const std::string& pluginDir,
                       const char* interfaceName);

/// Same as above but reads `plugin_path` from the current Python
/// process's `__main__` module (populated by path.py's exec at
/// LuaInterface::initLua time).  Called from initLua right after the
/// Lua plugin load so both languages get plugins from their
/// respective path.{lua,py} globals.
void loadPythonPluginsFromPathPy(const char* interfaceName);

} // namespace python
} // namespace ui
} // namespace lc

#endif  // LC_WITH_PYTHONSCRIPT
