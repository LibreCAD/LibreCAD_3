#include "luainterface.h"
#include <QApplication>

#include <log4cxx/xml/domconfigurator.h>

int main(int argc, char* argv[]) {
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
        QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    #endif
    
    QApplication a(argc, argv);

    //log4cxx::xml::DOMConfigurator::configure("Log4cxxConfig.xml");

    LuaInterface li;
    li.initLua();
    
    return a.exec();
}
