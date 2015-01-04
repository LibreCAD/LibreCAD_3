#include "math.h"

#include "cadmdichild.h"

#include "cad/primitive/circle.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/ellipse.h"
#include "cad/primitive/text.h"
#include "cad/primitive/coordinate.h"
#include "cad/meta/color.h"
#include "cad/primitive/line.h"
#include "cad/meta/linewidth.h"
#include "cad/interface/metatype.h"
#include "cad/document/selectionmanager.h"
#include "cad/dochelpers/storagemanagerimpl.h"
#include "cad/dochelpers/undomanagerimpl.h"
#include "cad/dochelpers/documentimpl.h"

#include "drawitems/gradientbackground.h"
#include "drawitems/metricgrid.h"
#include "drawitems/cursor.h"
#include "drawitems/lcvdrawitem.h"
#include "helpers/snapmanager.h"
#include "helpers/snapmanagerimpl.h"
#include "cad/interface/snapable.h"

#include "cadmdichild.h"
#include "testform.h"
#include <QFileDialog>
#include <cad/operations/builder.h>
#include <QMessageBox>
#include <iostream>
// #include "lcDWG/dwgimpl.h"
#include <lcDXF/dxfimpl.h>
#include <QDebug>

#include <QTime>
#include <random>
#include <cad/operations/layerops.h>
#include <cad/meta/metacolor.h>

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

    viewer = new LCADViewer(this);
    viewer->setObjectName(QStringLiteral("viewer"));
    viewer->setGeometry(QRect(50, 30, 581, 401));
    viewer->setAutoFillBackground(true);
    viewer->resize(10000, 10000);

    gridLayout->addWidget(viewer, 0, 0, 1, 1);

    horizontalScrollBar->setMinimum(-1000);
    horizontalScrollBar->setMaximum(1000);
    verticalScrollBar->setMinimum(-1000);
    verticalScrollBar->setMaximum(1000);

    /*
    connect(horizontalScrollBar, SIGNAL(valueChanged(int)),
            viewer, SLOT(setHorizontalOffset(int)));
    connect(verticalScrollBar, SIGNAL(valueChanged(int)),
            viewer, SLOT(setVerticalOffset(int))); */

}

CadMdiChild::~CadMdiChild() {
    // TODO make document a shared ptr or make sure object's are deleted in the right order
    //    delete _document;
}

int CadMdiChild::randInt(int low, int high) {
    // Random number between low and high
    return qrand() % ((high + 1) - low) + low;
}


void CadMdiChild::newDocument() {

    // Entity manager add's/removes entities to layers
    _storageManager = std::make_shared<lc::StorageManagerImpl>();

    // Create a new document with required objects, all objects that are required needs to be passed into the constructor
    _document = new lc::DocumentImpl(_storageManager);

    // Add the document to a LibreCAD Viewer system so we can visualize the document
    viewer->setDocument(_document);

    // Should this be done using the events system of QT??
    viewer->addBackgroundItem(std::shared_ptr<LCVDrawItem>(new GradientBackground(lc::Color(0x06, 0x35, 0x06), lc::Color(0x07, 0x15, 0x11))));
    auto metricGrid = std::make_shared<MetricGrid>(20, lc::Color(0x40, 0x48, 0x40), lc::Color(0x80, 0x90, 0x80));
    viewer->addBackgroundItem(metricGrid);

    // Snap manager
    _snapManager = std::make_shared<SnapManagerImpl>(viewer,  std::dynamic_pointer_cast<lc::Snapable>(metricGrid), 25.);

    // Add a cursor manager, Cursor will decide the ultimate position of clicked objects
    _cursor = std::make_shared<Cursor>(40, viewer, _snapManager, lc::Color(0xff, 0x00, 0x00), lc::Color(0x00, 0xff, 0x00));

    // Undo manager takes care that we can undo/redo entities within a document
    _undoManager = std::make_shared<lc::UndoManagerImpl>(_document, 10);


    // Add operation manager
    _operationManager = std::shared_ptr<OperationManager> (new OperationManager(_document));

    // Create a default layer
    auto layer = std::make_shared<lc::Layer>("0", lc::Color(1., 0., 0., 1.));
    auto al = std::make_shared<lc::operation::AddLayer>(_document, layer);
    al->execute();

    // Create a cross at position 0,0
    auto builder = std::make_shared<lc::operation::Builder>(document());
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(-100., 100.), lc::geo::Coordinate(100., -100.), layer));
    builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(-100., -100.), lc::geo::Coordinate(100., 100.), layer));
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0.0, 0.0), 100. * sqrtf(2.0), layer));
    builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(0.0, 0.0), 50. * sqrtf(2.0), layer));
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0.0, 0.0), 300, 0, 270.0, layer));
    builder->append(std::make_shared<lc::Ellipse>(lc::geo::Coordinate(0.0, 0.0), lc::geo::Coordinate(500.0, 0.0), 100.0 , 0., 360.0 , layer));
    builder->append(std::make_shared<lc::Arc>(lc::geo::Coordinate(0.0 , 5.0), 700, 50.26, 70.26, layer));
    //    builder->append(std::make_shared<lc::Text>(lc::geo::Coordinate(300, 300), lc::geo::Coordinate(500, 500), 20.0, "Jai Sai Naath", 0.00, 45.0 * M_PI / 180. , "Style", 0, 0, 0, layer));
    //    builder->append(std::make_shared<lc::Text>(lc::geo::Coordinate(350, 300), lc::geo::Coordinate(500, 500), 20.0, "Jai Sai Naath", 0.00, 45.0 * M_PI / 180. , "Style", 0, 0, 1, layer));
    //    builder->append(std::make_shared<lc::Text>(lc::geo::Coordinate(400, 300), lc::geo::Coordinate(500, 500), 20.0, "Jai Sai Naath", 0.00, 45.0 * M_PI / 180. , "Style", 0, 0, 2, layer));
    //    builder->append(std::make_shared<lc::Text>(lc::geo::Coordinate(450, 300), lc::geo::Coordinate(500, 500), 20.0, "Jai Sai Naath", 0.00, 45.0 * M_PI / 180. , "Style", 0, 0, 3, layer));
    //    builder->append(std::make_shared<lc::Coordinate>(0., 0., layer));
    builder->execute();

    //on_actionAdd_Random_Lines_triggered();
    //on_addCircles_clicked();
}



void CadMdiChild::undo() {
    _undoManager->undo();
}

void CadMdiChild::import(std::string str) {

    // Entity manager add's/removes entities to layers
    _storageManager = std::make_shared<lc::StorageManagerImpl>();

    // Create a new document with required objects, all objects that are required needs to be passed into the constructor
    _document = new lc::DocumentImpl(_storageManager);


    // Add the document to a LibreCAD Viewer system so we can visualize the document
    viewer->setDocument(_document);

    // Should this be done using the events system of QT??
    viewer->addBackgroundItem(std::shared_ptr<LCVDrawItem>(new GradientBackground(lc::Color(0x06, 0x35, 0x06), lc::Color(0x07, 0x15, 0x11))));
    auto metricGrid = std::make_shared<MetricGrid>(20, lc::Color(0x40, 0x48, 0x40), lc::Color(0x80, 0x90, 0x80));
    viewer->addBackgroundItem(metricGrid);

    // Snap manager
    _snapManager = std::make_shared<SnapManagerImpl>(viewer,  std::dynamic_pointer_cast<lc::Snapable>(metricGrid), 25.);

    // Add a cursor manager, Cursor will decide the ultimate position of clicked objects
    _cursor = std::make_shared<Cursor>(40, viewer, _snapManager, lc::Color(0xff, 0x00, 0x00), lc::Color(0x00, 0xff, 0x00));

    // Undo manager takes care that we can undo/redo entities within a document
    _undoManager = std::make_shared<lc::UndoManagerImpl>(_document, 10);

    // Add operation manager
    _operationManager = std::shared_ptr<OperationManager> (new OperationManager(_document));


    std::string ext = str.substr(str.length() - 3, 3);

//#ifdef USE_lcDXFDWG

    if (ext == "dxf" || ext == "DXF") {
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
        std::cout << "Sorry, not compiled with DWG support";
    }

//#else
//    std::cout << "Sorry, not compiled with USE_lcDXFDWG";
//#endif
}

void CadMdiChild::redo() {
    _undoManager->redo();
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
            builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(x1, y1), lc::geo::Coordinate(x2, y2), layer, mymap));
        } else {
            builder->append(std::make_shared<lc::Line>(lc::geo::Coordinate(x1, y1), lc::geo::Coordinate(x2, y2), layer));
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

void CadMdiChild::on_addCircles_clicked() {
    auto builder = std::make_shared<lc::operation::Builder>(document());
    auto layer = _storageManager->layerByName("0");

    for (int i = 0; i < 10000; i++) {
        double x1 = randInt(-4000, 4000);
        double y1 = randInt(-4000, 4000);

        double r = randInt(0, 150);

        if (randInt(0, 2) == 0) {
            std::shared_ptr<lc::MetaInfo> mymap2 = lc::MetaInfo::create()->add(std::make_shared<const lc::MetaColor>(randInt(0, 255) / 255., randInt(0, 255) / 255., randInt(0, 255) / 255.));
            builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(x1, y1), r, layer, mymap2));
        } else {
            builder->append(std::make_shared<lc::Circle>(lc::geo::Coordinate(x1, y1), r, layer));
        }
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

        s = randInt(0, 180) / (360.0 / M_PI / 2);
        e = randInt(180, 360) / (360.0 / M_PI / 2);
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
    return viewer;
}

SnapManager_SPtr  CadMdiChild::snapManager() const {
    return  _snapManager;
}

std::shared_ptr<OperationManager>  CadMdiChild::operationManager() const {
    return _operationManager;
}

lc::Document* CadMdiChild::document() const {
    return _document;
}

lc::StorageManager_SPtr CadMdiChild::storageManager() const {
    return _storageManager;
}

void CadMdiChild::cancelCurrentOperations() {
    operationManager()->cancel();
}


