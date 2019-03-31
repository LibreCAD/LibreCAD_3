#include "lcadmodelviewer.h"

using namespace lc::ui;
using namespace lc::viewer;

LCADModelViewer::LCADModelViewer(QWidget* parent,int id = 0):LCADViewer(parent),_id(id)
{
    qDebug( "lcadmodelviewer--- Constructor()--- object= %u parent=%u id=%d",this,parent,id );
    _gradientBackground = std::make_shared<drawable::GradientBackground>(lc::Color(0x07, 0x15, 0x11), lc::Color(0x06, 0x35, 0x06));
    _grid = std::make_shared<drawable::Grid>(20, lc::Color(0x40, 0x48, 0x40), lc::Color(0x80, 0x90, 0x80));
    _cursor = std::make_shared<drawable::Cursor>(40, this->documentCanvas(), lc::Color(0xff, 0x00, 0x00), lc::Color(0x00, 0xff, 0x00));
    connect(this, SIGNAL(mouseMoveEvent()) , this, SLOT(onMouseMoveEvent()));
}

void LCADModelViewer::setDocument(std::shared_ptr<lc::storage::Document> document){
    qDebug( "lcadmodelviewer--- setDocument()--- object= %u",this);
    
    LCADViewer::setDocument(document,nullptr);
    this->documentCanvas()->background().connect<drawable::GradientBackground, &drawable::GradientBackground::draw>(_gradientBackground.get());
    this->documentCanvas()->background().connect<drawable::Grid, &drawable::Grid::draw>(_grid.get());

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

void LCADModelViewer::onMouseMoveEvent(){
    emit setActive(_id);
};