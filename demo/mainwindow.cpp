#include "mainwindow.h"
#include "listener.h"
#include "ui_mainwindow.h"

#include "cad/primitive/circle.h"
#include "cad/primitive/line.h"
#include "cad/meta/color.h"
#include "cad/meta/linewidth.h"
#include "cad/interface/metatype.h"
#include "cad/dochelpers/documentlayerimpl.h"
#include "cad/document/layermanager.h"
#include "cad/document/selectionmanager.h"
#include "cad/dochelpers/layermanagerimpl.h"
#include "cad/dochelpers/selectionmanagerimpl.h"
#include "cad/document/documentimpl.h"
#include "cad/operations/createentities.h"


MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);


    QList<lc::MetaType*> metaTypes;

    // Create new LibreCAD Document
    lc::LayerManager* newLayerManager = dynamic_cast< lc::LayerManager*>(new lc::LayerManagerImpl());
    lc::SelectionManager* newSelectionManager = dynamic_cast< lc::SelectionManager*>(new lc::SelectionManagerImpl(newLayerManager, 5.0));
    _document =  dynamic_cast< lc::Document*>(new lc::DocumentImpl(newLayerManager));

    // Listener that listens for document events
    Listener *l = new Listener(_document);

    lc::Line* l1 = new lc::Line(lc::geo::Coordinate(1.0, 1.0), lc::geo::Coordinate(1.0, 9.0));

    lc::CreateEntities * foo = new  lc::CreateEntities();
    foo->add(l1);
    _document->operateOn(foo);

    // Create a line with meta attributes, Color Width defaults to BYLAYER, Line set to 1.5mm
    // lc::Circle* c1 = new lc::Circle(lc::Coordinate(10.0, 10.0), 12.4, metaTypes << new lc::LineWidth(2));



    // This won't work, Line MUST be defined properly
    // lc::Line* l4 = new lc::Line();

    // This won't work, Color cannot be given by Entity without specifying a color
    // lc::Color* c = new lc::Color(lc::Color::ByEntity);
    // Sorry no drawing yet



    ui->lCADViewer->setDocument(_document);

}

MainWindow::~MainWindow() {
    delete ui;
    delete _document;
}
