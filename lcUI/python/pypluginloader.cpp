// Phase 5 PR-5.5 — Python plugin loader implementation.

#ifdef LC_WITH_PYTHONSCRIPT

#include "pypluginloader.h"

#include <pybind11_qt.h>

#include <dirent.h>
#include <sys/stat.h>
#include <iostream>

namespace py = pybind11;

namespace lc {
namespace ui {
namespace python {

namespace {

// True iff `<dir>/plugin.py` exists as a regular file.  Skips symlink
// weirdness intentionally — matches the Lua path's `dofile` behavior
// which just calls `fopen`.
bool hasPluginPy(const std::string& dir) {
    std::string candidate = dir + "/plugin.py";
    struct stat st;
    if (stat(candidate.c_str(), &st) != 0) return false;
    return S_ISREG(st.st_mode);
}

} // namespace

void loadPythonPlugins(const std::string& pluginDir,
                       const char* interfaceName) {
    // Silent no-op on missing dir — same graceful-fail policy as
    // PluginManager::loadPluginsFrom() / phase 5 PR-5.3's path.py.
    DIR* dir = opendir(pluginDir.c_str());
    if (dir == nullptr) return;

    py::gil_scoped_acquire gil;

    struct dirent* ent;
    while ((ent = readdir(dir)) != nullptr) {
        if (strcmp(ent->d_name, ".") == 0
            || strcmp(ent->d_name, "..") == 0) {
            continue;
        }

        std::string subdir = pluginDir + "/" + ent->d_name;

        // Only descend into directories.  d_type isn't portable so
        // stat() the entry.
        struct stat st;
        if (stat(subdir.c_str(), &st) != 0) continue;
        if (!S_ISDIR(st.st_mode)) continue;

        if (!hasPluginPy(subdir)) continue;

        std::string pluginFile = subdir + "/plugin.py";
        try {
            // Fresh namespace per plugin so `LC_interface` doesn't
            // leak between plugins and one plugin can't stomp another's
            // globals.  Import `lc` + `lcgui as gui` first for parity
            // with what ScriptDock's PyNamespace ships (see
            // scriptdock.cpp).  Plugins can `import lcUIPy.<module>`
            // via the sys.path insertion path.py performs.
            py::dict ns;
            ns["__builtins__"] = py::module_::import("builtins");
            py::exec("import lc\ntry:\n    import lcgui as gui\nexcept ImportError:\n    pass\n", ns);
            ns["LC_interface"] = interfaceName;
            py::eval_file(pluginFile, ns);
        } catch (const py::error_already_set& e) {
            // Broken plugin — log and continue with the next.
            std::cerr << "Plugin " << pluginFile
                      << " load failed: " << e.what() << std::endl;
        }
    }
    closedir(dir);
}

void loadPythonPluginsFromPathPy(const char* interfaceName) {
    py::gil_scoped_acquire gil;
    try {
        py::dict main =
            py::reinterpret_borrow<py::dict>(
                py::module_::import("__main__").attr("__dict__"));
        if (!main.contains("plugin_path")) return;
        py::object obj = main["plugin_path"];
        if (obj.is_none()) return;
        std::string path = py::str(obj);
        if (path.empty()) return;
        loadPythonPlugins(path, interfaceName);
    } catch (const py::error_already_set& e) {
        std::cerr << "loadPythonPluginsFromPathPy failed: "
                  << e.what() << std::endl;
    }
}

} // namespace python
} // namespace ui
} // namespace lc

#endif  // LC_WITH_PYTHONSCRIPT
