#include "luainterface.h"
#include <QApplication>

#include <log4cxx/xml/domconfigurator.h>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    //log4cxx::xml::DOMConfigurator::configure("Log4cxxConfig.xml");

    LuaInterface li;
    li.initLua();
    
    return a.exec();
}
