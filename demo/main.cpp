#include <QtGui/QApplication>
#include <QLibrary>
#include <QtDebug>
#include "mainwindow.h"

#include "cad/primitive/circle.h"
#include "cad/primitive/line.h"
#include "cad/meta/color.h"
#include "cad/meta/linewidth.h"
#include "cad/interface/metatype.h"
#include "cad/document/documentlayerimpl.h"
#include "cad/document/layermanagerimpl.h"


int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    QList<lc::MetaType*> metaTypes;

    lc::LayerManagerImpl* layerManager = new lc::LayerManagerImpl();

    layerManager->addLayer("Layer 2");


    lc::Line* l1 = new lc::Line(new lc::Coordinate(10.0, 10.0), new lc::Coordinate(10.0, 10.0));
    layerManager->addEntity("Layer 2", l1);

    l1 = new lc::Line(new lc::Coordinate(10.0, 10.0), new lc::Coordinate(10.0, 10.0), metaTypes << new lc::Color(10, 10, 10));
    layerManager->addEntity("0", l1);

    l1 = new lc::Line(new lc::Coordinate(10.0, 10.0), new lc::Coordinate(10.0, 10.0), metaTypes << new lc::LineWidth(1.5));
    layerManager->addEntity("0", l1);

    // Create a line with meta attributes, Color Width defaults to BYLAYER, Line set to 1.5mm
    lc::Circle* c1 = new lc::Circle(new lc::Coordinate(10.0, 10.0), 12.4, metaTypes << new lc::LineWidth(2));

    // Remove entity with id 1
    layerManager->removeEntity("0", 3);


    // This won't work, Line MUST be defined properly
    // lc::Line* l4 = new lc::Line();

    // This won't work, Color cannot be given by Entity without specifying a color
    // lc::Color* c = new lc::Color(lc::Color::ByEntity);
    // Sorry no drawing yet
    MainWindow w;
    w.show();
    return a.exec();
}
