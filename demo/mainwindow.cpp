
#include <memory>

#include <QTime>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "scenemanager.h"

#include "cad/primitive/circle.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/line.h"
#include "cad/primitive/ellipse.h"
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
#include "cad/dochelpers/documentimpl.h"
#include "cad/operations/createentities.h"

#include "drawitems/gradientbackground.h"
#include "drawitems/metricgrid.h"
#include "drawitems/cursor.h"

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

    // Should this be done using the events system of QT??
    ui->lCADViewer->addBackgroundItem(LCViewerDrawItemPtr(new GradientBackground(QColor(0x06, 0x15, 0x06), QColor(0x07, 0x25, 0x11))));
    ui->lCADViewer->addBackgroundItem(LCViewerDrawItemPtr(new MetricGrid(20, QColor(0x40, 0x48, 0x40), QColor(0x80, 0x90, 0x80))));
    ui->lCADViewer->addCursorItem(LCViewerCursorItemPtr(new Cursor(40, QColor(0xff, 0x00, 0x00), QColor(0x00, 0xff, 0x00))));

    // Create a scene for this document, each document will have only one scene, but can have multiple views
    QGraphicsScene* scene = new QGraphicsScene(this);
    ui->lCADViewer->setScene(scene);
    scene->setSceneRect(-15000, -15000, 30000, 30000);
    scene->setItemIndexMethod(QGraphicsScene::BspTreeIndex);

    // Create a new document with required objects, all objects that are required needs to be passed into the constructor
    _document = new lc::DocumentImpl();

    // Layer Manager takes care of creating and removing layers
    lc::LayerManager* layerManager = new lc::LayerManagerImpl(_document);

    // A layer manager is required for a document to work, but chicken and the egg problem prevents making these referencing them
    // May be the document should create it somehow???
    _document->setLayerManager(layerManager);

    // Entity manager add's/removes entities to layers
    lc::EntityManager* entityManager = new lc::EntityManagerImpl(_document);

    // Selection manager keeps a list of current selected entities
    lc::SelectionManager* newSelectionManager = new lc::SelectionManagerImpl(layerManager, 5.0);

    // Scene manager listens to the document and takes care that the scene is changed according to what
    // is added and removed within a document
    SceneManager* sceneManager = new SceneManager(ui->lCADViewer, _document);

    // Undo manager takes care that we can undo/redo entities within a document
    _undoManager = new lc::UndoManagerImpl(_document, 10);

    // Add the document to a LibreCAD Viewer system so we can visualize the document
    ui->lCADViewer->setAbstractDocument(_document);

    // Create a cross at position 0,0
    lc::CreateEntities* foo = new  lc::CreateEntities(_document, "0");
    foo->append(lc::CADEntityPtr(new lc::Line(lc::geo::Coordinate(-100, 100), lc::geo::Coordinate(100, -100))));
    foo->append(lc::CADEntityPtr(new lc::Line(lc::geo::Coordinate(-100, -100), lc::geo::Coordinate(100, 100))));
    _document->operateOn(shared_ptr<lc::Operation>(foo));
}

MainWindow::~MainWindow() {
    delete ui;
    delete _document;
}


void MainWindow::wheelEvent(QWheelEvent* event) {
    //   ui->lCADViewer->scaleView(pow((double)2, -event->delta() / 240.0));
}

void MainWindow::on_undoButton_clicked() {
    _undoManager->undo();
}

void MainWindow::on_redoButtom_clicked() {
    _undoManager->redo();
}

void MainWindow::on_addEntities_clicked() {
    lc::CreateEntities* foo = new  lc::CreateEntities(_document, "0");

    QTime myTimer;
    myTimer.start();

    for (int i = 0; i < 1000; i++) {
        double x1 = randInt(-4000, 4000);
        double y1 = randInt(-4000, 4000);

        double x2 = x1 + randInt(-50, 50);
        double y2 = y1 + randInt(-50, 50);
        foo->append(lc::CADEntityPtr(new lc::Line(lc::geo::Coordinate(x1, y1), lc::geo::Coordinate(x2, y2))));
    }

    qDebug() << "Create : " << myTimer.elapsed();
    myTimer.start();
    _document->operateOn(shared_ptr<lc::Operation>(foo));
    qDebug() << "Process : " << myTimer.elapsed();
}

void MainWindow::on_addCircles_clicked() {
    lc::CreateEntities* foo = new  lc::CreateEntities(_document, "0");

    for (int i = 0; i < 1000; i++) {
        double x1 = randInt(-4000, 4000);
        double y1 = randInt(-4000, 4000);

        double r = randInt(0, 150);
        foo->append(lc::CADEntityPtr(new lc::Circle(lc::geo::Coordinate(x1, y1), r)));
    }

    _document->operateOn(shared_ptr<lc::Operation>(foo));
}


void MainWindow::on_clearUndoables_clicked() {
    _undoManager->removeUndoables();
}

void MainWindow::on_addArcs_clicked() {
    lc::CreateEntities* foo = new  lc::CreateEntities(_document, "0");

    for (int i = 0; i < 1000; i++) {
        double x1 = randInt(-4000, 4000);
        double y1 = randInt(-4000, 4000);

        double r = randInt(0, 150);
        double s = randInt(0, 6283) / 1000.0;
        double e = randInt(0, 6283) / 1000.0;

        if (s < e) {
            double t = e;
            e = s;
            s = t;
        }

        s = (0 + 45) / (360.0 / PI / 2);
        e = (180 + 45) / (360.0 / PI / 2);
        foo->append(lc::CADEntityPtr(new lc::Arc(lc::geo::Coordinate(x1, y1), r, s, e)));
    }

    _document->operateOn(shared_ptr<lc::Operation>(foo));
}

void MainWindow::on_addEllipse_clicked() {
    lc::CreateEntities* foo = new  lc::CreateEntities(_document, "0");

    for (int i = 0; i < 1000; i++) {
        double x1 = randInt(-4000, 4000);
        double y1 = randInt(-4000, 4000);

        double x2 = x1 + randInt(-50, 50);
        double y2 = y1 + randInt(-50, 50);

        double r = randInt(0, 150);
        double s = randInt(0, 6283) / 1000.0;
        double e = randInt(0, 6283) / 1000.0;

        if (s < e) {
            double t = e;
            e = s;
            s = t;
        }

        s = (0 + 45) / (360.0 / PI / 2);
        e = (180 + 45) / (360.0 / PI / 2);
        foo->append(lc::CADEntityPtr(new lc::Ellipse(lc::geo::Coordinate(x1, y1), lc::geo::Coordinate(x2, y2), r, s, e)));
    }

    _document->operateOn(shared_ptr<lc::Operation>(foo));
}
