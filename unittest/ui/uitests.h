#pragma once

#include <luainterface.h>
#include <cad/storage/documentimpl.h>
#include <QApplication>
#include <cad/storage/storagemanagerimpl.h>
#include <cadmdichild.h>

inline int& uiTestArgc() {
    static int argc = 1;
    return argc;
}

inline char** uiTestArgv() {
    static char appName[] = "lcunittest";
    static char* argv[] = {appName, nullptr};
    return argv;
}

inline QApplication& ensureUiApplication() {
    static QApplication* application = [] {
        // Qt owns process-global platform resources; keep QApplication alive
        // until process exit instead of running teardown after test widgets
        // and platform-plugin singletons have begun static destruction.
        return new QApplication(uiTestArgc(), uiTestArgv());
    }();
    return *application;
}

using namespace lc;
using namespace ui;

storage::Document_SPtr createDocument();
