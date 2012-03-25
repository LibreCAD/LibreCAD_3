#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {

    QList<lc::MetaType*> metaTypes;

    // Create new LibreCAD Document
    lc::LayerManager* newLayerManager = dynamic_cast< lc::LayerManager*>(new lc::LayerManagerImpl());
    lc::SelectionManager* newSelectionManager = dynamic_cast< lc::SelectionManager*>(new lc::SelectionManagerImpl(newLayerManager, 5.0));
    lc::Document* document =  dynamic_cast< lc::Document*>(new lc::DocumentImpl(newLayerManager));


    // Somewhere else we get the layer Manager
    lc::LayerManager* layerManager = document->layerManager();

    // Add a new layer
    layerManager->addLayer("Layer 2");

    // get layer 0
    lc::DocumentLayer* documentLayer = layerManager->layer("0");

    for (int i = 0; i < 100; i++) {
        lc::Line* l1 = new lc::Line(lc::geo::Coordinate(i, 1.0), lc::geo::Coordinate(i, 9.0));
        documentLayer->addEntity(l1);
    }

    ui->lCADViewer->setDocument(document);


    // Create a line with meta attributes, Color Width defaults to BYLAYER, Line set to 1.5mm
    // lc::Circle* c1 = new lc::Circle(lc::Coordinate(10.0, 10.0), 12.4, metaTypes << new lc::LineWidth(2));



    // This won't work, Line MUST be defined properly
    // lc::Line* l4 = new lc::Line();

    // This won't work, Color cannot be given by Entity without specifying a color
    // lc::Color* c = new lc::Color(lc::Color::ByEntity);
    // Sorry no drawing yet


    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete _document;
    delete ui;
}
