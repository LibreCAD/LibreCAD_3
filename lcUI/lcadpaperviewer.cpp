#include "lcadpaperviewer.h"

using namespace lc;
using namespace lc::ui;
using namespace lc::viewer;

#define PAPER_WIDTH 300
#define PAPER_HEIGHT -200

LCADPaperViewer::LCADPaperViewer(QWidget* parent,int id = 0):LCADViewer(parent),_id(id){
    _gradientBackground = std::make_shared<drawable::PaperBackground>(PAPER_WIDTH, PAPER_HEIGHT);
    _grid = std::make_shared<drawable::Grid>(20, lc::Color(0x40, 0x48, 0x40), lc::Color(0x80, 0x90, 0x80));
    _cursor = std::make_shared<drawable::Cursor>(40, this->documentCanvas(), lc::Color(0xff, 0x00, 0x00), lc::Color(0x00, 0xff, 0x00));
    connect(this, SIGNAL(mouseMoveEvent()) , this, SLOT(onMouseMoveEvent()));
}

void LCADPaperViewer::setDocument(std::shared_ptr<lc::storage::Document> document, meta::Block_CSPtr viewport){
	LCADViewer::setDocument(document, viewport);
    _viewport = viewport;
    this->documentCanvas()->background().connect<drawable::PaperBackground, &drawable::PaperBackground::draw>(_gradientBackground.get());
//    this->documentCanvas()->background().connect<drawable::Grid, &drawable::Grid::draw>(_grid.get()); don't draw grid

    // Snap manager
    _snapManager = std::make_shared<manager::SnapManagerImpl>(this->documentCanvas(),  _grid, 25.);
    this->setSnapManager(_snapManager);

    // Add a cursor manager, Cursor will decide the ultimate position of clicked objects
    this->documentCanvas()->foreground().connect<drawable::Cursor, &drawable::Cursor::onDraw>(_cursor.get());
    _snapManager->snapPointEvents().connect<drawable::Cursor, &drawable::Cursor::onSnapPointEvent>(_cursor.get());

    _tempEntities = std::make_shared<drawable::TempEntities>(this->documentCanvas());
    this->documentCanvas()->foreground().connect<drawable::TempEntities, &drawable::TempEntities::onDraw>(_tempEntities.get());
    _tempEntities->requestUpdateEvent().connect<ui::LCADViewer, &ui::LCADViewer::updateHelper>(*this);

    //Drag manager
    _dragManager = std::make_shared<manager::DragManager>(this->documentCanvas(), this->cursor(), _tempEntities, 10);
    this->setDragManager(_dragManager);

    _dragPoints = std::make_shared<drawable::DragPoints>();
    _dragManager->dragPointsEvent().connect<drawable::DragPoints, &drawable::DragPoints::setPoints>(_dragPoints.get());
    this->documentCanvas()->foreground().connect<drawable::DragPoints, &drawable::DragPoints::onDraw>(_dragPoints.get());

    // Undo manager takes care that we can undo/redo entities within a document
    _undoManager = std::make_shared<lc::storage::UndoManagerImpl>(10);
    document->commitProcessEvent().connect<lc::storage::UndoManagerImpl, &lc::storage::UndoManagerImpl::on_CommitProcessEvent>(_undoManager.get());
}

void LCADPaperViewer::onMouseMoveEvent(){
    emit setActive(_id);
};
