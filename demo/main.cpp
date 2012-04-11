#include <QtGui/QApplication>
#include <QLibrary>
#include <QtDebug>
#include "mainwindow.h"




int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    QLibrary liblcKernel("liblckernel");
    liblcKernel.load();
    QLibrary liblcadviewer("liblcadviewerplugin");
    liblcadviewer.load();

    MainWindow w;
    w.show();
        return a.exec();
}
