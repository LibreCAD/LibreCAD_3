// Phase 5 PR-5.5 — Python plugin loader implementation.

#ifdef LC_WITH_PYTHONSCRIPT

#include "pypluginloader.h"

#include <qt_keywords_push.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/eval.h>
#include <qt_keywords_pop.h>

#include <iostream>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStringList>

namespace py = pybind11;

namespace lc {
namespace ui {
namespace python {

QString resolvePythonPackageRoot() {
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
        QDir(appDir).filePath("lcUIPy"),
        QDir(appDir).filePath("../lcUIPy"),
        QDir(appDir).filePath("../share/librecad/lcUIPy"),
        QDir(appDir).filePath("../Resources/lcUIPy"),
        QDir(appDir).filePath("../Resources/librecad/lcUIPy"),
    };

    for (const QString& candidate : candidates) {
        const QString root = QDir::cleanPath(candidate);
        if (QFileInfo(QDir(root).filePath("__init__.py")).isFile()) {
            return QFileInfo(root).absoluteFilePath();
        }
    }
    return {};
}

namespace {

// True iff `<dir>/plugin.py` exists as a regular file.  Skips symlink
// weirdness intentionally — matches the Lua path's `dofile` behavior
// which just calls `fopen`.
bool hasPluginPy(const QString& dir) {
    return QFileInfo(QDir(dir).filePath("plugin.py")).isFile();
}

} // namespace

PythonPluginLoadReport loadPythonPlugins(const std::string& pluginDir,
                                         const char* interfaceName) {
    PythonPluginLoadReport report;
    // Silent no-op on missing dir — same graceful-fail policy as
    // PluginManager::loadPluginsFrom().
    const QDir dir(QString::fromStdString(pluginDir));
    if (!dir.exists()) {
        report.failures.push_back("plugin directory not found: " + pluginDir);
        return report;
    }
    report.directoryFound = true;

    py::gil_scoped_acquire gil;

    const QFileInfoList entries = dir.entryInfoList(
        QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo& entry : entries) {
        if (!hasPluginPy(entry.absoluteFilePath())) continue;

        const QString pluginPath = QDir(entry.absoluteFilePath()).filePath("plugin.py");
        const std::string pluginFile = pluginPath.toStdString();
        try {
            // Fresh namespace per plugin so `LC_interface` doesn't
            // leak between plugins and one plugin can't stomp another's
            // globals.  Import `lc` + `lcgui as gui` first for parity
            // with what ScriptDock's PyNamespace ships (see
            // scriptdock.cpp).  Plugins can `import lcUIPy.<module>`
            // because LuaInterface adds the package parent to sys.path first.
            py::dict ns;
            ns["__builtins__"] = py::module_::import("builtins");
            py::exec("import lc\ntry:\n    import lcgui as gui\nexcept ImportError:\n    pass\n", ns);
            ns["LC_interface"] = interfaceName;
            py::eval_file(pluginFile, ns);
            report.loadedPlugins.emplace_back(entry.fileName().toStdString());
        } catch (const py::error_already_set& e) {
            // Broken plugin — log and continue with the next.
            std::cerr << "Plugin " << pluginFile
                      << " load failed: " << e.what() << std::endl;
            report.failures.emplace_back(
                entry.fileName().toStdString() + ": " + e.what());
        }
    }
    return report;
}

} // namespace python
} // namespace ui
} // namespace lc

#endif  // LC_WITH_PYTHONSCRIPT
