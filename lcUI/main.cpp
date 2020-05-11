#include <QApplication>
#include <QtCore/QDebug>
#include <QtWidgets/QStyleFactory>
#include <QSettings>
#include "cad/logger/logger.h"
#include "luainterface.h"
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
        QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    #endif
    
    QApplication a(argc, argv);

    LOGGER;//Init logger

    lc::ui::LuaInterface li;
    lc::ui::MainWindow* mainWindow = new lc::ui::MainWindow(&li);
    li.initLua(mainWindow);

    return QApplication::exec();
}
