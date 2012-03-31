
#include <QTime>
#include "mainwindow.h"
#include "listener.h"
#include "ui_mainwindow.h"

#include "scenemanager.h"

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
#include "cad/dochelpers/entitymanagerimpl.h"
#include "cad/dochelpers/undomanagerimpl.h"
#include "cad/document/documentimpl.h"
#include "cad/operations/createentities.h"

#include <QtGui>

#include <math.h>

int MainWindow::randInt(int low, int high) {
    // Random number between low and high
    return qrand() % ((high + 1) - low) + low;
}

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);



    // Create new LibreCAD AbstractDocument
    lc::LayerManager* newLayerManager = new lc::LayerManagerImpl();
    lc::SelectionManager* newSelectionManager = new lc::SelectionManagerImpl(newLayerManager, 5.0);

    lc::EntityManager* entityManager = new lc::EntityManagerImpl();

    // Create a new document
    _document = new lc::DocumentImpl(newLayerManager, entityManager);

    SceneManager* sceneManager = new SceneManager(ui->lCADViewer, _document);
    _undoManager = new lc::UndoManagerImpl(10);
    _undoManager->setDocument(_document);

    // Listener that listens for document events
    Listener* l = new Listener(_document);

    // Create a line with meta attributes, Color Width defaults to BYLAYER, Line set to 1.5mm
    // QList<lc::MetaType*> metaTypes
    // lc::Circle* c1 = new lc::Circle(lc::Coordinate(10.0, 10.0), 12.4, metaTypes << new lc::LineWidth(2));

    // This won't work, Line MUST be defined properly
    // lc::Line* l4 = new lc::Line();

    // This won't work, Color cannot be given by Entity without specifying a color
    // lc::Color* c = new lc::Color(lc::Color::ByEntity);
    // Sorry no drawing yet



    ui->lCADViewer->setAbstractDocument(_document);

}

MainWindow::~MainWindow() {
    delete ui;
    delete _document;
}


void MainWindow::wheelEvent(QWheelEvent* event) {
    ui->lCADViewer->scaleView(pow((double)2, -event->delta() / 240.0));
}

void MainWindow::on_addEntities_clicked() {
    lc::CreateEntities* foo = new  lc::CreateEntities(_document, "0");

    for (int i = 0; i < 10000; i++) {
        double x1 = randInt(-4000, 4000);
        double y1 = randInt(-4000, 4000);

        double x2 = x1 + randInt(-50, 50);
        double y2 = y1 + randInt(-50, 50);
        lc::Line* l1 = new lc::Line(lc::geo::Coordinate(x1, y1), lc::geo::Coordinate(x2, y2));
        foo->add(l1);
    }

    _document->operateOn(foo);
}

void MainWindow::on_addCircles_clicked() {
    lc::CreateEntities* foo = new  lc::CreateEntities(_document, "0");

    for (int i = 0; i < 10000; i++) {
        double x1 = randInt(-4000, 4000);
        double y1 = randInt(-4000, 4000);

        double r = randInt(0, 150);
        lc::Circle* c1 = new lc::Circle(lc::geo::Coordinate(x1, y1), r);
        foo->add(c1);
    }

    _document->operateOn(foo);
}

void MainWindow::on_undoButton_clicked() {
    _undoManager->undo();
}

void MainWindow::on_redoButtom_clicked() {
    _undoManager->redo();
}
