#include <QtGui/QApplication>
#include <QLibrary>
#include <QtDebug>
#include "mainwindow.h"

#include "cad/primitive/circle.h"
#include "cad/primitive/line.h"
#include "cad/meta/color.h"
#include "cad/meta/linewidth.h"
#include "cad/interface/metatype.h"

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    QList<lc::MetaType*> metaTypes;
    // Create a line with no meta attributes, Line Width and color defaults to BYLAYER
    lc::Line* l1 = new lc::Line(new lc::Point(10.0, 10.0), new lc::Point(10.0, 10.0));
    // Create a line with meta attributes, Line Width defaults to BYLAYER, Color set to 10,10,10
    lc::Line* l2 = new lc::Line(new lc::Point(10.0, 10.0), new lc::Point(10.0, 10.0), metaTypes << new lc::Color(10, 10, 10));
    // Create a line with meta attributes, Color Width defaults to BYLAYER, Line set to 1.5mm
    lc::Line* l3 = new lc::Line(new lc::Point(10.0, 10.0), new lc::Point(10.0, 10.0), metaTypes << new lc::LineWidth(1.5));

    // Create a line with meta attributes, Color Width defaults to BYLAYER, Line set to 1.5mm
    lc::Circle* c1 = new lc::Circle(new lc::Point(10.0, 10.0), 12.4, metaTypes << new lc::LineWidth(2));

    // This won't work, Line MUST be defined properly
    // lc::Line* l4 = new lc::Line();

    // This won't work, Color cannot be given by Entity without specifying a color
    // lc::Color* c = new lc::Color(lc::Color::ByEntity);
    // Sorry no drawing yet
    MainWindow w;
    w.show();
    return a.exec();
}
