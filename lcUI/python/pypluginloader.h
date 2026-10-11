#pragma once

// Phase 5 PR-5.5 — Python plugin loader.
//
// Iterates the resolved Python plugin directory and for each subdirectory,
// loads `plugin.py`. Mirrors
// lcadluascript's PluginManager but for Python: `LC_interface` is
// injected into the exec namespace with the IDENTICAL name Lua uses
// ("gui"/"cli") — no case change — so a plugin author can `import`
// the same identifier from either language.
//
// Failure policy matches PluginManager's Lua analog: missing
// missing directory → silent no-op; missing `plugin.py` in a subdir → skip that
// subdir; per-plugin exceptions → log to stderr and continue with the
// next plugin (a broken plugin shouldn't abort init).

#ifdef LC_WITH_PYTHONSCRIPT

#include <QString>

#include <string>
#include <vector>

namespace lc {
namespace ui {
namespace python {

struct PythonPluginLoadReport {
    bool directoryFound = false;
    std::vector<std::string> loadedPlugins;
    std::vector<std::string> failures;
};

/// Resolve `lcUIPy` from the executable, Unix install prefix, or app bundle.
/// Returns an empty string when none of those supported layouts contains it.
QString resolvePythonPackageRoot();

/// Load `plugin.py` from every immediate subdirectory of the given
/// path.  `interfaceName` (e.g. "gui" or "cli") is set as
/// `LC_interface` in the exec namespace before each plugin runs, so
/// plugin authors can dispatch on the runtime host. Per-plugin errors remain
/// non-fatal for normal startup but are included in the returned report so
/// smoke/tests can require successful discovery and loading.
PythonPluginLoadReport loadPythonPlugins(const std::string& pluginDir,
                                        const char* interfaceName);

} // namespace python
} // namespace ui
} // namespace lc

#endif  // LC_WITH_PYTHONSCRIPT
