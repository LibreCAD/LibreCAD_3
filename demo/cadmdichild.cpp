#include "math.h"

#include "cadmdichild.h"
#include "ui_cadmdichild.h"

#include "scenemanager.h"

#include "cad/primitive/circle.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/line.h"
#include "cad/primitive/ellipse.h"
#include "cad/meta/color.h"
#include "cad/meta/linewidth.h"
#include "cad/interface/metatype.h"
#include "cad/document/selectionmanager.h"
#include "cad/dochelpers/storagemanagerimpl.h"
#include "cad/dochelpers/undomanagerimpl.h"
#include "cad/dochelpers/documentimpl.h"

#include "drawitems/gradientbackground.h"
#include "drawitems/metricgrid.h"
#include "drawitems/cursor.h"
#include "helpers/snapmanager.h"
#include "helpers/snapmanagerimpl.h"
#include "cad/interface/snapable.h"

#include "cadmdichild.h"

#include <cad/operations/builder.h>



#include <helpers/selectionmanagerimpl.h>

#include <QTime>

CadMdiChild::CadMdiChild(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::CadMdiChild) {
    ui->setupUi(this);
}

CadMdiChild::~CadMdiChild() {
    delete ui;
    delete _document;
}

int CadMdiChild::randInt(int low, int high) {
    // Random number between low and high
    return qrand() % ((high + 1) - low) + low;
}


void CadMdiChild::newDocument() {


    // Should this be done using the events system of QT??
    ui->lCADCairoViewer->addBackgroundItem(std::shared_ptr<LCViewerDrawItem>(new GradientBackground(QColor(0x06, 0x15, 0x06), QColor(0x07, 0x25, 0x11))));
    std::shared_ptr<LCViewerDrawItem> metricGrid = std::shared_ptr<LCViewerDrawItem>(new MetricGrid(20, QColor(0x40, 0x48, 0x40), QColor(0x80, 0x90, 0x80)));
    ui->lCADCairoViewer->addBackgroundItem(metricGrid);


    // Entity manager add's/removes entities to layers
    _storageManager = std::make_shared<lc::StorageManagerImpl>();

    // Create a new document with required objects, all objects that are required needs to be passed into the constructor
    _document = new lc::DocumentImpl(_storageManager);

    // Selection manager allow for finding entities around a point or within areas
    // _selectionManager = std::make_shared<SelectionManagerImpl>(_storageManager, ui->lCADCairoViewer);

    // Scene manager listens to the document and takes care that the scene is changed according to what
    // is added and removed within a document
    // SceneManager* sceneManager = new SceneManager(ui->lCADCairoViewer, _document);

    // Snap manager
    // _snapManager = std::make_shared<SnapManagerImpl>(ui->lCADCairoViewer, _selectionManager,  std::dynamic_pointer_cast<lc::Snapable>(metricGrid), 25.);

    // Add a cursor manager, Cursor will decide the ultimate position of clicked objects
    // _cursor = std::make_shared<Cursor>(40, ui->lCADCairoViewer, _snapManager, QColor(0xff, 0x00, 0x00), QColor(0x00, 0xff, 0x00));

    // Undo manager takes care that we can undo/redo entities within a document
    _undoManager = std::make_shared<lc::UndoManagerImpl>(_document, 10);

    // Add the document to a LibreCAD Viewer system so we can visualize the document
    ui->lCADCairoViewer->setDocument(_document);

    // Add operation manager
    _operationManager = std::shared_ptr<OperationManager> (new OperationManager(_document));


    // Create a cross at position 0,0
    auto layer = _storageManager->layerByName("0");
    auto builder = std::make_shared<lc::operation::Builder>(document());
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(-100., 100.), lc::geo::Coordinate(100., -100.), layer));
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(-100., -100.), lc::geo::Coordinate(100., 100.), layer));
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0.0, 0.0), 100. * sqrtf(2.0), layer));
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0.0, 0.0), 50. * sqrtf(2.0), layer));

    builder->execute();

}



void CadMdiChild::undo() {
    _undoManager->undo();
}

void CadMdiChild::redo() {
    _undoManager->redo();
}

void CadMdiChild::on_actionAdd_Random_Lines_triggered() {
    auto builder = std::make_shared<lc::operation::Builder>(document());
    auto layer = _storageManager->layerByName("0");
    QTime myTimer;
    myTimer.start();

    for (int i = 0; i < 1000; i++) {
        double x1 = randInt(-4000, 4000);
        double y1 = randInt(-4000, 4000);

        double x2 = x1 + randInt(-50, 50);
        double y2 = y1 + randInt(-50, 50);
        builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(x1, y1), lc::geo::Coordinate(x2, y2), layer));
    }

    qDebug() << "Create : " << myTimer.elapsed();
    myTimer.start();
    builder->execute();
    qDebug() << "Process : " << myTimer.elapsed();
}

void CadMdiChild::on_addCircles_clicked() {
    auto builder = std::make_shared<lc::operation::Builder>(document());
    auto layer = _storageManager->layerByName("0");

    for (int i = 0; i < 1000; i++) {
        double x1 = randInt(-4000, 4000);
        double y1 = randInt(-4000, 4000);

        double r = randInt(0, 150);
        builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(x1, y1), r, layer));
    }

    builder->execute();
}


void CadMdiChild::on_clearUndoables_clicked() {
    _undoManager->removeUndoables();
}

void CadMdiChild::on_addArcs_clicked() {
    auto builder = std::make_shared<lc::operation::Builder>(document());
    auto layer = _storageManager->layerByName("0");

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

        s = (0 + 45) / (360.0 / M_PI / 2);
        e = (180 + 45) / (360.0 / M_PI / 2);
        builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(x1, y1), r, s, e, layer));
    }

    builder->execute();
}

void CadMdiChild::on_addEllipse_clicked() {
    auto builder = std::make_shared<lc::operation::Builder>(document());

    auto layer = _storageManager->layerByName("0");

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

        s = (0 + 45) / (360.0 / M_PI / 2);
        e = (180 + 45) / (360.0 / M_PI / 2);
        builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(x1, y1), lc::geo::Coordinate(x2, y2), r, s, e, layer));
    }

    builder->execute();
}


QWidget* CadMdiChild::view() const {
    return ui->lCADCairoViewer;
}

std::shared_ptr<SnapManager>  CadMdiChild::snapManager() const {
    return  _snapManager;
}

std::shared_ptr<OperationManager>  CadMdiChild::operationManager() const {
    return _operationManager;
}

lc::Document* CadMdiChild::document() const {
    return _document;
}

std::shared_ptr<lc::SelectionManager> CadMdiChild::selectionManager() const {
    return _selectionManager;
}

std::shared_ptr<lc::StorageManager> CadMdiChild::storageManager() const {
    return _storageManager;
}

void CadMdiChild::cancelCurrentOperations() {
    operationManager()->cancel();
}

