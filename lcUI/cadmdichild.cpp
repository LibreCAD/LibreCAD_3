#include "math.h"

#include "cadmdichild.h"

#include "cad/primitive/circle.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/ellipse.h"
#include "cad/primitive/text.h"
#include <cad/primitive/point.h>
#include "cad/meta/color.h"
#include "cad/primitive/line.h"
#include "cad/meta/metalinewidth.h"
#include "cad/interface/metatype.h"
#include "cad/dochelpers/storagemanagerimpl.h"
#include "cad/dochelpers/documentimpl.h"

#include <drawables/lccursor.h>
#include "drawitems/lcvdrawitem.h"
#include <managers/snapmanager.h>
#include <managers/snapmanagerimpl.h>
#include "cad/interface/snapable.h"

#include "cadmdichild.h"
#include <QFileDialog>
#include <cad/operations/builder.h>
#include <QMessageBox>
#include <iostream>
// #include "lcDWG/dwgimpl.h"
#include <QDebug>
#include <QMenu>
#include <QTime>
#include <random>
#include <cad/operations/layerops.h>
#include <cad/meta/metacolor.h>
#include <drawables/gradientbackground.h>
#include <drawables/grid.h>
#include <cad/math/lcmath.h>

using namespace LCViewer;

CadMdiChild::CadMdiChild(QWidget* parent) :
    QWidget(parent) {



    if (this->objectName().isEmpty()) {
        this->setObjectName(QStringLiteral("CadMdiChild"));
    }

    this->resize(1078, 736);

    QGridLayout* gridLayout = new QGridLayout(this);
    gridLayout->setHorizontalSpacing(0);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));
    gridLayout->setContentsMargins(0, 0, 0, 0);
    horizontalScrollBar = new QScrollBar(this);
    horizontalScrollBar->setObjectName(QStringLiteral("horizontalScrollBar"));
    horizontalScrollBar->setOrientation(Qt::Horizontal);

    gridLayout->addWidget(horizontalScrollBar, 1, 0, 1, 1);

    verticalScrollBar = new QScrollBar(this);
    verticalScrollBar->setObjectName(QStringLiteral("verticalScrollBar"));
    verticalScrollBar->setOrientation(Qt::Vertical);

    gridLayout->addWidget(verticalScrollBar, 0, 1, 1, 1);

    _viewer = new LCADViewer(this);
    _viewer->setObjectName(QStringLiteral("viewer"));
    _viewer->setGeometry(QRect(50, 30, 581, 401));
    _viewer->setAutoFillBackground(true);
    _viewer->setContextMenuPolicy(Qt::CustomContextMenu);
    _viewer->setFocusPolicy(Qt::StrongFocus);
    connect(_viewer, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ctxMenu(const QPoint&)));

    gridLayout->addWidget(_viewer, 0, 0, 1, 1);

    horizontalScrollBar->setMinimum(-1000);
    horizontalScrollBar->setMaximum(1000);
    verticalScrollBar->setMinimum(-1000);
    verticalScrollBar->setMaximum(1000);

    connect(horizontalScrollBar, SIGNAL(valueChanged(int)),
            _viewer, SLOT(setHorizontalOffset(int)));
    connect(verticalScrollBar, SIGNAL(valueChanged(int)),
            _viewer, SLOT(setVerticalOffset(int)));
}

CadMdiChild::~CadMdiChild() {
	if(_destroyCallback) {
		_destroyCallback(_id);
	}
}

int CadMdiChild::randInt(int low, int high) {
    // Random number between low and high
    return qrand() % ((high + 1) - low) + low;
}


void CadMdiChild::newDocument() {

    // Entity manager add's/removes entities to layers
    _storageManager = std::make_shared<lc::StorageManagerImpl>();

    // Create a new document with required objects, all objects that are required needs to be passed into the constructor
    _document = std::make_shared<lc::DocumentImpl>(_storageManager);

    // Add the document to a LibreCAD Viewer system so we can visualize the document
    _viewer->setDocument(_document);

    _gradientBackground = std::make_shared<GradientBackground>(lc::Color(0x07, 0x15, 0x11), lc::Color(0x06, 0x35, 0x06));
    _viewer->documentCanvas()->background().connect<GradientBackground, &GradientBackground::draw>(_gradientBackground.get());
    _grid = std::make_shared<Grid>(20, lc::Color(0x40, 0x48, 0x40), lc::Color(0x80, 0x90, 0x80));
    _viewer->documentCanvas()->background().connect<Grid, &Grid::draw>(_grid.get());

    // Snap manager
    _snapManager = std::make_shared<SnapManagerImpl>(_viewer->documentCanvas(),  _grid, 25.);
    _viewer->setSnapManager(_snapManager);

    // Add a cursor manager, Cursor will decide the ultimate position of clicked objects
    _cursor = std::make_shared<LCViewer::Cursor>(40, _viewer->documentCanvas(), lc::Color(0xff, 0x00, 0x00), lc::Color(0x00, 0xff, 0x00));
    _viewer->documentCanvas()->background().connect<LCViewer::Cursor, &LCViewer::Cursor::onDraw>(_cursor.get());
    _snapManager->snapPointEvents().connect<LCViewer::Cursor, &LCViewer::Cursor::onSnapPointEvent>(_cursor.get());

    _tempEntities = std::make_shared<TempEntities>(_viewer->documentCanvas());
    _viewer->documentCanvas()->background().connect<LCViewer::TempEntities, &LCViewer::TempEntities::onDraw>(_tempEntities.get());

    //Drag manager
    _dragManager = std::make_shared<DragManager>(_viewer->documentCanvas(), _cursor, _tempEntities, 10);
    _viewer->setDragManager(_dragManager);

    _dragPoints = std::make_shared<LCViewer::DragPoints>();
    _dragManager->dragPointsEvent().connect<LCViewer::DragPoints, &LCViewer::DragPoints::setPoints>(_dragPoints.get());
    _viewer->documentCanvas()->background().connect<LCViewer::DragPoints, &LCViewer::DragPoints::onDraw>(_dragPoints.get());

    // Undo manager takes care that we can undo/redo entities within a document
    _undoManager = std::make_shared<lc::UndoManagerImpl>(10);
    _document->commitProcessEvent().connect<lc::UndoManagerImpl, &lc::UndoManagerImpl::on_CommitProcessEvent>(_undoManager.get());


    // Add operation manager
    _operationManager = std::make_shared<OperationManager>(_document);

    // Create a default layer
    auto layer = std::make_shared<lc::Layer>("0", lc::Color(1., 1., 1., 1.));
    auto al = std::make_shared<lc::operation::AddLayer>(_document, layer);
    al->execute();
    _undoManager->removeUndoables(); //Prevent undo of default layer creation

    // Create a cross at position 0,0
//    auto builder = std::make_shared<lc::operation::Builder>(document());
//    builder->append(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(-100., 100.), lc::geo::Coordinate(100., -100.), layer));
//    builder->append(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(-100., -100.), lc::geo::Coordinate(100., 100.), layer));

//    builder->append(std::make_shared<lc::entity::Circle>(lc::geo::Coordinate(0.0, 0.0), 100. * sqrtf(2.0), layer));
//    builder->append(std::make_shared<lc::entity::Circle>(lc::geo::Coordinate(0.0, 0.0), 1000. * sqrtf(2.0), layer));
//    builder->append(std::make_shared<lc::entity::Circle>(lc::geo::Coordinate(0.0, 0.0), 50. * sqrtf(2.0), layer));
//    builder->append(std::make_shared<lc::entity::Arc>(lc::geo::Coordinate(0.0, 0.0), 300, 0, 2.*M_PI-0.1, true, layer));
//    builder->append(std::make_shared<lc::entity::Ellipse>(lc::geo::Coordinate(0.0, 0.0), lc::geo::Coordinate(500.0, 0.0), 100.0 , 0., 360.0 , layer));
//    builder->append(std::make_shared<lc::entity::Arc>(lc::geo::Coordinate(0.0 , 5.0), 700, 0, 1.*M_PI, true, layer));

//        builder->append(std::make_shared<lc::Text>(lc::geo::Coordinate(300, 300), lc::geo::Coordinate(500, 500), 20.0, "Jai Sai Naath", 0.00, 45.0 * M_PI / 180. , "Style", 0, 0, 0, layer));
//        builder->append(std::make_shared<lc::Text>(lc::geo::Coordinate(350, 300), lc::geo::Coordinate(500, 500), 20.0, "Jai Sai Naath", 0.00, 45.0 * M_PI / 180. , "Style", 0, 0, 1, layer));
//        builder->append(std::make_shared<lc::Text>(lc::geo::Coordinate(400, 300), lc::geo::Coordinate(500, 500), 20.0, "Jai Sai Naath", 0.00, 45.0 * M_PI / 180. , "Style", 0, 0, 2, layer));
//        builder->append(std::make_shared<lc::Text>(lc::geo::Coordinate(450, 300), lc::geo::Coordinate(500, 500), 20.0, "Jai Sai Naath", 0.00, 45.0 * M_PI / 180. , "Style", 0, 0, 3, layer));
//        builder->append(std::make_shared<lc::Point>(0., 0., layer));
//   builder->execute();

    //on_actionAdd_Random_Lines_triggered();
    //on_addCircles_clicked();
//    std::string path ="/home/gagan/lol.dxf";
//    exportDXF(path, DXF::version::R2010);
}


void CadMdiChild::import(std::string str) {

    // Entity manager add's/removes entities to layers
    _storageManager = std::make_shared<lc::StorageManagerImpl>();

    // Create a new document with required objects, all objects that are required needs to be passed into the constructor
    _document = std::make_shared<lc::DocumentImpl>(_storageManager);


    // Add the document to a LibreCAD Viewer system so we can visualize the document
    _viewer->setDocument(_document);

    _gradientBackground = std::make_shared<GradientBackground>(lc::Color(0x07, 0x15, 0x11), lc::Color(0x06, 0x35, 0x06));
    _viewer->documentCanvas()->background().connect<GradientBackground, &GradientBackground::draw>(_gradientBackground.get());
    _grid = std::make_shared<Grid>(20, lc::Color(0x40, 0x48, 0x40), lc::Color(0x80, 0x90, 0x80));
    _viewer->documentCanvas()->background().connect<Grid, &Grid::draw>(_grid.get());

//    SnapManagerImpl(DocumentCanvas_SPtr view, lc::Snapable_CSPtr grid, double distanceToSnap);
    // Snap manager
    _snapManager = std::make_shared<SnapManagerImpl>(_viewer->documentCanvas(),  _grid, 25.);
    _viewer->setSnapManager(_snapManager);

    // Add a cursor manager, Cursor will decide the ultimate position of clicked objects
    _cursor = std::make_shared<LCViewer::Cursor>(40, _viewer->documentCanvas(), lc::Color(0xff, 0x00, 0x00), lc::Color(0x00, 0xff, 0x00));
    _viewer->documentCanvas()->background().connect<LCViewer::Cursor, &LCViewer::Cursor::onDraw>(_cursor.get());
    _snapManager->snapPointEvents().connect<LCViewer::Cursor, &LCViewer::Cursor::onSnapPointEvent>(_cursor.get());

    _tempEntities = std::make_shared<TempEntities>(_viewer->documentCanvas());
    _viewer->documentCanvas()->background().connect<LCViewer::TempEntities, &LCViewer::TempEntities::onDraw>(_tempEntities.get());

    //Drag manager
    _dragManager = std::make_shared<DragManager>(_viewer->documentCanvas(), _cursor, _tempEntities, 10);
    _viewer->setDragManager(_dragManager);

    _dragPoints = std::make_shared<LCViewer::DragPoints>();
    _dragManager->dragPointsEvent().connect<LCViewer::DragPoints, &LCViewer::DragPoints::setPoints>(_dragPoints.get());
    _viewer->documentCanvas()->background().connect<LCViewer::DragPoints, &LCViewer::DragPoints::onDraw>(_dragPoints.get());

    // Undo manager takes care that we can undo/redo entities within a document
    _undoManager = std::make_shared<lc::UndoManagerImpl>(10);
	_document->commitProcessEvent().connect<lc::UndoManagerImpl, &lc::UndoManagerImpl::on_CommitProcessEvent>(_undoManager.get());

    // Add operation manager
    _operationManager = std::make_shared<OperationManager>(_document);

    // This might not always work on operating system's that doesn't support extends well, like OS/X
    std::string ext = str.substr(str.length() - 3, 3);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == "dxf") {
        auto builder = std::make_shared<lc::operation::Builder>(document());
        DXFimpl* F = new DXFimpl(_document, builder);
        dxfRW R(str.c_str());
        R.read(F, true);
        builder->execute();
    } else if (ext == "dwg" || ext == "DWG") {
        // DWGimpl* F = new DWGimpl(_storageManager, _document);
        // F->readFile((char*)str.c_str());
        // _document = F->document();
        //_storageManager = F->storageManager();
        std::cerr << "Sorry, not compiled with DWG support";
    }
}


void CadMdiChild::exportDXF(std::string& str, DXF::version lcv) {
    DXFimpl* F = new DXFimpl(_document);
    F->writeDXF(str, lcv);
}

void CadMdiChild::on_actionAdd_Random_Lines_triggered() {
    QTime myTimer;
    myTimer.start();


    auto builder = std::make_shared<lc::operation::Builder>(document());
    auto layer = _storageManager->layerByName("0");

    std::mt19937 rng;
    rng.seed(0);

    for (int i = 0; i < 1000; i++) {
        double x1 = randInt(-4000, 4000);
        double y1 = randInt(-4000, 4000);

        double x2 = x1 + randInt(-50, 50);
        double y2 = y1 + randInt(-50, 50);

        if (randInt(0, 2) == 0) {
            auto mymap = lc::MetaInfo::create()->add(std::make_shared<const lc::MetaColor>(randInt(0, 255) / 255., randInt(0, 255) / 255., randInt(0, 255) / 255.));
            builder->append(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(x1, y1), lc::geo::Coordinate(x2, y2), layer, mymap));
        } else {
            builder->append(std::make_shared<lc::entity::Line>(lc::geo::Coordinate(x1, y1), lc::geo::Coordinate(x2, y2), layer));
        }

    }

    qDebug() << "Create : " << myTimer.elapsed();
    myTimer.start();
    builder->execute();
    qDebug() << "Process : " << myTimer.elapsed();

    /*
    // This retreival is currently expensive because it makes a copy.
    // The idea is to not make a copy by returning it 'const'. To decides yet

    lc::EntityContainer container=document()->entityContainer();
    myTimer.restart();
    qDebug() << "Entities Total " << container.asVector().size();
    qDebug() << "Selection Time : " << myTimer.elapsed() << "ms\n";

    myTimer.restart();
    lc::EntityContainer itemsInArea = container.entitiesByArea(lc::geo::Area(lc::geo::Coordinate(200,200), lc::geo::Coordinate(350,350)));
    qDebug() << "Entities in area : " << itemsInArea.asVector().size();
    qDebug() << "Selection Time : " << myTimer.elapsed() << "ms\n";

    myTimer.restart();
    lc::EntityContainer  itemsOnLayerWithinArea = itemsInArea.entitiesByLayer(storageManager()->layerByName("1"));
    qDebug() << "Entities on layer within above selection : " << itemsOnLayerWithinArea.asVector().size();
    qDebug() << "Selection Time : " << myTimer.elapsed() << "ms\n";

    myTimer.restart();
    lc::EntityContainer  itemsOnDocument = container.entitiesByLayer(storageManager()->layerByName("1"));
    qDebug() << "Entities on this layer within document : " << itemsOnDocument.asVector().size();
    qDebug() << "Selection Time : " << myTimer.elapsed() << "ms\n";
    */

}

void CadMdiChild::ctxMenu(const QPoint& pos) {
    QMenu* menu = new QMenu;
    menu->addAction(tr("Test Item"), this, SLOT(test_slot()));
    menu->exec(_viewer->mapToGlobal(pos));
}

void CadMdiChild::on_addCircles_clicked() {
    auto builder = std::make_shared<lc::operation::Builder>(document());
    auto layer = _storageManager->layerByName("0");

    for (int i = 0; i < 10000; i++) {
        double x1 = randInt(-4000, 4000);
        double y1 = randInt(-4000, 4000);

        double r = randInt(0, 150);

        if (randInt(0, 2) == 0) {
            std::shared_ptr<lc::MetaInfo> mymap2 = lc::MetaInfo::create()->add(std::make_shared<const lc::MetaColor>(randInt(0, 255) / 255., randInt(0, 255) / 255., randInt(0, 255) / 255.));
            builder->append(std::make_shared<lc::entity::Circle>(lc::geo::Coordinate(x1, y1), r, layer, mymap2));
        } else {
            builder->append(std::make_shared<lc::entity::Circle>(lc::geo::Coordinate(x1, y1), r, layer));
        }
    }

    builder->execute();
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

        s = randInt(0, 180) / (360.0 / M_PI / 2);
        e = randInt(180, 360) / (360.0 / M_PI / 2);
        builder->append(std::make_shared<lc::entity::Arc>(lc::geo::Coordinate(x1, y1), r, s, e, false, layer));
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
        builder->append(std::make_shared<lc::entity::Ellipse>(lc::geo::Coordinate(x1, y1), lc::geo::Coordinate(x2, y2), r, s, e, layer));
    }

    builder->execute();
}


QWidget* CadMdiChild::view() const {
    return _viewer;
}

SnapManager_SPtr  CadMdiChild::snapManager() const {
    return  _snapManager;
}

std::shared_ptr<OperationManager>  CadMdiChild::operationManager() const {
    return _operationManager;
}

std::shared_ptr<lc::Document> CadMdiChild::document() const {
    return _document;
}

lc::StorageManager_SPtr CadMdiChild::storageManager() const {
    return _storageManager;
}

void CadMdiChild::cancelCurrentOperations() {
    operationManager()->cancel();
}

lc::UndoManager_SPtr CadMdiChild::undoManager() const {
    return _undoManager;
}

std::shared_ptr<LCViewer::Cursor> CadMdiChild::cursor() const {
	return _cursor;
}

unsigned int CadMdiChild::id() {
	return _id;
}
void CadMdiChild::setId(unsigned int id) {
	_id = id;
}

void CadMdiChild::setDestroyCallback(LuaIntf::LuaRef destroyCallback) {
	_destroyCallback = destroyCallback;
}

void CadMdiChild::keyPressEvent(QKeyEvent *event) {
    QWidget::keyPressEvent(event);
    emit keyPressed(event);
}

LCViewer::TempEntities_SPtr CadMdiChild::tempEntities() {
    return _tempEntities;
}

std::vector<lc::entity::CADEntity_SPtr> CadMdiChild::selection() {
    return viewer()->documentCanvas()->selection().asVector();
}
