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
#include "cad/dochelpers/documentlayerimpl.h"
#include "cad/document/layermanager.h"
#include "cad/document/selectionmanager.h"
#include "cad/dochelpers/layermanagerimpl.h"
#include "cad/dochelpers/entitymanagerimpl.h"
#include "cad/dochelpers/undomanagerimpl.h"
#include "cad/dochelpers/documentimpl.h"

#include "drawitems/gradientbackground.h"
#include "drawitems/metricgrid.h"
#include "drawitems/cursor.h"
#include "helpers/snapmanager.h"
#include "helpers/snapmanagerimpl.h"
#include "helpers/selectionmanagerimpl.h"
#include "cad/interface/snapable.h"

#include "cadmdichild.h"

#include <cad/operations/builder.h>

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
    ui->lCADViewer->addBackgroundItem(shared_ptr<LCViewerDrawItem>(new GradientBackground(QColor(0x06, 0x15, 0x06), QColor(0x07, 0x25, 0x11))));
    shared_ptr<LCViewerDrawItem> metricGrid = shared_ptr<LCViewerDrawItem>(new MetricGrid(20, QColor(0x40, 0x48, 0x40), QColor(0x80, 0x90, 0x80)));
    ui->lCADViewer->addBackgroundItem(metricGrid);


    // Create a scene for this document, each document will have only one scene, but can have multiple views
    QGraphicsScene* scene = new QGraphicsScene(this);
    ui->lCADViewer->setScene(scene);
    ui->lCADViewer->setSceneRect(-15000, -15000, 30000, 30000);
    scene->setItemIndexMethod(QGraphicsScene::BspTreeIndex);

    // Create a new document with required objects, all objects that are required needs to be passed into the constructor
    _document = new lc::DocumentImpl();

    // Layer Manager takes care of creating and removing layers
    _layerManager = shared_ptr<lc::LayerManager>(new lc::LayerManagerImpl(_document));

    // A layer manager is required for a document to work, but chicken and the egg problem prevents making these referencing them
    // May be the document should create it somehow???
    _document->setLayerManager(_layerManager);


    // Entity manager add's/removes entities to layers
    lc::EntityManager* entityManager = new lc::EntityManagerImpl(_document);

    // Selection manager allow for finding entities around a point or within areas
    _selectionManager = shared_ptr<lc::SelectionManager>(new SelectionManagerImpl(_layerManager, ui->lCADViewer));

    // Scene manager listens to the document and takes care that the scene is changed according to what
    // is added and removed within a document
    SceneManager* sceneManager = new SceneManager(ui->lCADViewer, _document);

    // Snap manager
    _snapManager = shared_ptr<SnapManager> (new SnapManagerImpl(ui->lCADViewer, _selectionManager,  dynamic_pointer_cast<lc::Snapable>(metricGrid), 25.));

    // Add a cursor manager, Cursor will decide the ultimate position of clicked objects
    _cursor = shared_ptr<const Cursor> (new Cursor(40, ui->lCADViewer, _snapManager, QColor(0xff, 0x00, 0x00), QColor(0x00, 0xff, 0x00)));

    // Undo manager takes care that we can undo/redo entities within a document
    _undoManager = new lc::UndoManagerImpl(_document, 10);

    // Add the document to a LibreCAD Viewer system so we can visualize the document
    ui->lCADViewer->setAbstractDocument(_document);

    // Add operation manager
    _operationManager = shared_ptr<OperationManager> (new OperationManager(_document));


    // Create a cross at position 0,0
    auto builder = make_shared<lc::operation::Builder>(document());
    builder->append(make_shared<lc::Line>(lc::geo::Coordinate(-100., 100.), lc::geo::Coordinate(100., -100.)));
    builder->append(make_shared<lc::Line>(lc::geo::Coordinate(-100., -100.), lc::geo::Coordinate(100., 100.)));
    builder->append(make_shared<lc::Circle>(lc::geo::Coordinate(0.0, 0.0), 100. * sqrtf(2.0)));
    builder->append(make_shared<lc::Circle>(lc::geo::Coordinate(0.0, 0.0), 50. * sqrtf(2.0)));

    builder->execute();

}



void CadMdiChild::undo() {
    _undoManager->undo();
}

void CadMdiChild::redo() {
    _undoManager->redo();
}

void CadMdiChild::on_actionAdd_Random_Lines_triggered() {
    auto builder = make_shared<lc::operation::Builder>(document());

    QTime myTimer;
    myTimer.start();

    for (int i = 0; i < 1000; i++) {
        double x1 = randInt(-4000, 4000);
        double y1 = randInt(-4000, 4000);

        double x2 = x1 + randInt(-50, 50);
        double y2 = y1 + randInt(-50, 50);
        builder->append(make_shared<const lc::Line>(lc::geo::Coordinate(x1, y1), lc::geo::Coordinate(x2, y2)));
    }

    qDebug() << "Create : " << myTimer.elapsed();
    myTimer.start();
    builder->execute();
    qDebug() << "Process : " << myTimer.elapsed();
}

void CadMdiChild::on_addCircles_clicked() {
    auto builder = make_shared<lc::operation::Builder>(document());

    for (int i = 0; i < 1000; i++) {
        double x1 = randInt(-4000, 4000);
        double y1 = randInt(-4000, 4000);

        double r = randInt(0, 150);
        builder->append(make_shared<const lc::Circle>(lc::geo::Coordinate(x1, y1), r));
    }

    builder->execute();
}


void CadMdiChild::on_clearUndoables_clicked() {
    _undoManager->removeUndoables();
}

void CadMdiChild::on_addArcs_clicked() {
    auto builder = make_shared<lc::operation::Builder>(document());

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
        builder->append(make_shared<const lc::Arc>(lc::geo::Coordinate(x1, y1), r, s, e));
    }

    builder->execute();
}

void CadMdiChild::on_addEllipse_clicked() {
    auto builder = make_shared<lc::operation::Builder>(document());

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
        builder->append(make_shared<const lc::Ellipse>(lc::geo::Coordinate(x1, y1), lc::geo::Coordinate(x2, y2), r, s, e));
    }

    builder->execute();
}


QCachedGraphicsView* CadMdiChild::view() const {
    return ui->lCADViewer;
}

shared_ptr<SnapManager>  CadMdiChild::snapManager() const {
    return  _snapManager;
}

shared_ptr<OperationManager>  CadMdiChild::operationManager() const {
    return _operationManager;
}

lc::AbstractDocument* CadMdiChild::document() const {
    return _document;
}

shared_ptr<lc::SelectionManager> CadMdiChild::selectionManager() const {
    return _selectionManager;
}


void CadMdiChild::cancelCurrentOperations() {
    operationManager()->cancel();
}

