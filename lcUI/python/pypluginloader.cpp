// Phase 5 PR-5.5 — Python plugin loader implementation.

#ifdef LC_WITH_PYTHONSCRIPT

#include "pypluginloader.h"

#include <qt_keywords_push.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/eval.h>
#include <qt_keywords_pop.h>

#include <iostream>
#include <map>
#include <mutex>

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

struct PluginLoadOutcome {
    bool loaded = false;
    std::string failure;
};

std::mutex& pluginLoadMutex() {
    static std::mutex mutex;
    return mutex;
}

std::map<std::string, PluginLoadOutcome>& pluginLoadOutcomes() {
    // Plugin definitions and global registrations are process-scoped. Keep
    // only paths and plain diagnostics here; Python objects remain owned by
    // their registries and are never destroyed by this cache without the GIL.
    static std::map<std::string, PluginLoadOutcome> outcomes;
    return outcomes;
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
    // Acquire the GIL before the cache mutex: plugin code may invoke Python
    // callbacks, and all concurrent callers must follow the same lock order.
    std::lock_guard<std::mutex> cacheLock(pluginLoadMutex());
    auto& outcomes = pluginLoadOutcomes();

    const QFileInfoList entries = dir.entryInfoList(
        QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo& entry : entries) {
        if (!hasPluginPy(entry.absoluteFilePath())) continue;

        const QString pluginPath = QDir(entry.absoluteFilePath()).filePath("plugin.py");
        const QFileInfo pluginInfo(pluginPath);
        QString identityPath = pluginInfo.canonicalFilePath();
        if (identityPath.isEmpty()) {
            identityPath = pluginInfo.absoluteFilePath();
        }
        const std::string pluginIdentity = identityPath.toStdString();
        const auto cached = outcomes.find(pluginIdentity);
        if (cached != outcomes.end()) {
            // Report cached outcomes to callers (including runtime smoke) as
            // if this invocation discovered each plugin, without evaluating
            // its definition or repeating process-global registrations.
            if (cached->second.loaded) {
                report.loadedPlugins.emplace_back(entry.fileName().toStdString());
            } else {
                report.failures.emplace_back(
                    entry.fileName().toStdString() + ": " +
                    cached->second.failure);
            }
            continue;
        }

        const std::string pluginFile = pluginIdentity;
        PluginLoadOutcome outcome;
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
            outcome.loaded = true;
        } catch (const py::error_already_set& e) {
            // Broken plugin — log and continue with the next.
            outcome.failure = e.what();
        } catch (const std::exception& e) {
            outcome.failure = e.what();
        } catch (...) {
            outcome.failure = "unknown plugin load failure";
        }

        outcomes.emplace(pluginIdentity, outcome);
        if (outcome.loaded) {
            report.loadedPlugins.emplace_back(entry.fileName().toStdString());
        } else {
            std::cerr << "Plugin " << pluginFile
                      << " load failed: " << outcome.failure << std::endl;
            report.failures.emplace_back(
                entry.fileName().toStdString() + ": " + outcome.failure);
        }
    }
    return report;
}

} // namespace python
} // namespace ui
} // namespace lc

#endif  // LC_WITH_PYTHONSCRIPT
