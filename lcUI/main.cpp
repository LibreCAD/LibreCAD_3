#include "luainterface.h"
#include "setupCAD/itemStyle.h"
#include "setupCAD/setupini.h"
#include <QApplication>
#include <QtCore/QDebug>
#include <QtWidgets/QStyleFactory>
#include <boost/log/trivial.hpp>
#include <QSettings>

int main(int argc, char* argv[]) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
#endif
    // check Setup.ini file
    SetupIni setupFile = SetupIni("setup.ini");
    // set global style
    ItemStyle style = ItemStyle(setupFile.getFilePath(), "Global");

    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create(style.getStyle()));

    BOOST_LOG_TRIVIAL(warning) << "A warning severity message";

    lc::ui::LuaInterface li;
    li.initLua();

    return QApplication::exec();
}
