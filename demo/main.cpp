#include <QtGui/QApplication>
#include <QLibrary>
#include <QtDebug>
#include "mainwindow.h"




int main(int argc, char* argv[]) {
    QApplication a(argc, argv);


    MainWindow w;
    w.show();
    return a.exec();
}



/*
    lc::Line* l1 = new lc::Line(lc::Coordinate(1.0, 1.0), lc::Coordinate(9.0, 9.0));
    documentLayer->addEntity(l1);

    l1 = new lc::Line(lc::Coordinate(1.0, 9.0), lc::Coordinate(1.0, 9.0));
    documentLayer->addEntity(l1);

    l1 = new lc::Line(lc::Coordinate(11.0, 11.0), lc::Coordinate(19.0, 19.0));
    documentLayer->addEntity(l1);

    l1 = new lc::Line(lc::Coordinate(11.0, 19.0), lc::Coordinate(11.0, 19.0));
    documentLayer->addEntity(l1);

    l1 = new lc::Line(lc::Coordinate(5.0, 5.0), lc::Coordinate(15.0, 15.0));
    documentLayer->addEntity(l1);



    // Create a line with meta attributes, Color Width defaults to BYLAYER, Line set to 1.5mm
    lc::Circle* c1 = new lc::Circle(lc::Coordinate(10.0, 10.0), 12.4, metaTypes << new lc::LineWidth(2));

*/
