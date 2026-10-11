#include <QApplication>
#include <QByteArray>
#include <QDir>
#include <QtCore/QDebug>
#include <QStandardPaths>
#include <QtWidgets/QStyleFactory>
#include <QSettings>
#include "cad/logger/logger.h"
#include "windowmanager.h"
#include <iostream>
#include <patternLoader/patternProvider.h>
#include <build_constants.h>
#ifdef LC_WITH_PYTHONSCRIPT
#include <lcpython.h>
#include "python/pyruntimesmoke.h"
#endif

#ifndef debug
#define debug
#endif

namespace {

QByteArray g_bundleSettingsPath;

void configureMacOSAppBundleResources() {
#ifdef Q_OS_MACOS
    QDir applicationDirectory(QCoreApplication::applicationDirPath());
    if (applicationDirectory.dirName() != "MacOS") return;

    QDir contentsDirectory = applicationDirectory;
    if (!contentsDirectory.cdUp() || contentsDirectory.dirName() != "Contents") return;
    QDir bundleDirectory = contentsDirectory;
    if (!bundleDirectory.cdUp() || !bundleDirectory.dirName().endsWith(".app")) return;

    const QStringList patternCandidates = {
        applicationDirectory.filePath("../Resources/librecad/hatch"),
        applicationDirectory.filePath("../share/librecad/hatch"),
    };
    for (const QString& candidate : patternCandidates) {
        const QString resolved = QDir::cleanPath(candidate);
        if (QDir(resolved).exists()) {
            lc::persistence::PatternProvider::setResourcePath(resolved.toStdString());
            break;
        }
    }

    QCoreApplication::setOrganizationName("LibreCAD");
    QCoreApplication::setApplicationName("LibreCAD");
    const QString settingsDirectoryOverride = qEnvironmentVariable("LIBRECAD_SETTINGS_DIR");
    const QString settingsDirectory = settingsDirectoryOverride.isEmpty()
        ? QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
        : QDir::cleanPath(settingsDirectoryOverride);
    if (settingsDirectory.isEmpty() || !QDir::isAbsolutePath(settingsDirectory) ||
        !QDir().mkpath(settingsDirectory)) {
        throw std::runtime_error(
            "Could not create the LibreCAD app-bundle settings directory: " +
            settingsDirectory.toStdString());
    }
    g_bundleSettingsPath = settingsDirectory.toUtf8();
    SETTINGS_PATH = g_bundleSettingsPath.constData();
#endif
}

} // namespace

int main2(int argc, char* argv[]) {
    // The guard this used to carry asked for Qt 5.4 or newer. lcUI requires
    // Qt6, so it was always taken -- and the last place in the tree that still
    // spoke of Qt5.
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QApplication a(argc, argv);
    configureMacOSAppBundleResources();

#ifdef LC_WITH_PYTHONSCRIPT
    // Select the build's Python before any window/plugin setup can initialize
    // the interpreter through ScriptDock or LuaInterface.
    lc::python::PythonInit::configureForApplication(
        QCoreApplication::applicationDirPath().toStdString());
#endif

    if (a.arguments().contains("--python-smoke")) {
#ifdef LC_WITH_PYTHONSCRIPT
        return lc::ui::python::runPythonRuntimeSmoke();
#else
        std::cerr << "Python runtime smoke is unavailable: this build has Python scripting disabled\n";
        return 2;
#endif
    }

    LOGGER;//Init logger
    lc::persistence::PatternProvider::Instance();//Init pattern loader

    lc::ui::WindowManager::init();

    return QApplication::exec();
}

int main(int argc, char* argv[]) {
    try {
        return main2(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "LibreCAD startup failed: " << e.what() << std::endl;
        return 1;
    } catch (const char* e) {
        std::cerr << "LibreCAD startup failed: " << e << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "LibreCAD startup failed with an unknown exception" << std::endl;
        return 1;
    }
}
