#include <QApplication>
#include <QLibrary>
#include <QtDebug>
#include "mainwindow.h"


#include <log4cxx/xml/domconfigurator.h>


int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    log4cxx::xml::DOMConfigurator::configure("Log4cxxConfig.xml");

    MainWindow w;
    w.show();
    return a.exec();
}
