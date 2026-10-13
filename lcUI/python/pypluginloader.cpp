// Phase 5 PR-5.5 — Python plugin loader implementation.

#ifdef LC_WITH_PYTHONSCRIPT

#include "pypluginloader.h"
#include "pyeventhooks.h"

#include <qt_keywords_push.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/eval.h>
#include <qt_keywords_pop.h>
#include <lcpython.h>

#include <iostream>
#include <stdexcept>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QThread>

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

const PythonPluginLoadReport& bootstrapPythonPlugins() {
    const auto* app = QCoreApplication::instance();
    if (!app || QThread::currentThread() != app->thread()) {
        throw std::runtime_error("Python plugin bootstrap requires the Qt application thread");
    }
    static const PythonPluginLoadReport report = [] {
        MainWindowExecutionScope noOwner(nullptr);
        lc::python::PythonInit::initialize();
        installEventHooks();
        const QString root = resolvePythonPackageRoot();
        if (root.isEmpty()) {
            throw std::runtime_error("Could not locate the lcUIPy application package");
        }
        py::gil_scoped_acquire gil;
        const py::str parent(QFileInfo(root).dir().absolutePath().toStdString());
        auto paths = py::module_::import("sys").attr("path");
        if (!py::cast<bool>(paths.attr("__contains__")(parent))) {
            paths.attr("insert")(0, parent);
        }
        const auto importModule = py::module_::import("importlib").attr("import_module");
        std::vector<std::string> failures;
        // Preserve the existing bounded top-level and two-subfolder scan.
        // Separate plugin namespaces and Python's module cache remain intact.
        for (const QString& subdir : {QString(), QStringLiteral("create_actions"),
                                     QStringLiteral("actions")}) {
            const QDir folder(QDir(root).filePath(subdir));
            for (const QString& file : folder.entryList({"*.py"}, QDir::Files, QDir::Name)) {
                if (file == "__init__.py" || (subdir.isEmpty() &&
                    !file.startsWith("create_") && !file.startsWith("action_"))) continue;
                const std::string module = ("lcUIPy." +
                    (subdir.isEmpty() ? QString() : subdir + ".") + file.chopped(3)).toStdString();
                try {
                    importModule(module);
                } catch (const py::error_already_set& error) {
                    failures.push_back(module + ": " + error.what());
                    std::cerr << "lcUIPy load failed: " << failures.back() << '\n';
                }
            }
        }
        auto result = loadPythonPlugins(QDir(root).filePath("plugins").toStdString(), "gui");
        result.failures.insert(result.failures.begin(), failures.begin(), failures.end());
        return result;
    }();
    return report;
}

PythonPluginLoadReport loadPythonPlugins(const std::string& pluginDir,
                                         const char* interfaceName) {
    MainWindowExecutionScope noOwner(nullptr);
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
            // because bootstrap adds the package parent to sys.path first.
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
