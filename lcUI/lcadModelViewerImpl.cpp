#include "lcadModelViewerImpl.h"

using namespace lc::ui;
using namespace lc::viewer;

// @TODO: result according to active layer
LCADModelViewerImpl::LCADModelViewerImpl(QWidget* parent):_parent(parent){
    _storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    _gradientBackground = std::make_shared<drawable::GradientBackground>(lc::Color(0x07, 0x15, 0x11), lc::Color(0x06, 0x35, 0x06));
    _grid = std::make_shared<drawable::Grid>(20, lc::Color(0x40, 0x48, 0x40), lc::Color(0x80, 0x90, 0x80));
};

void LCADModelViewerImpl::setDocument(std::shared_ptr<lc::storage::Document> document){
    _document = document;
    auto _viewer = _viewers[0];
    _viewer->setDocument(document);
    _viewer->documentCanvas()->background().connect<drawable::GradientBackground, &drawable::GradientBackground::draw>(_gradientBackground.get());
    _viewer->documentCanvas()->background().connect<drawable::Grid, &drawable::Grid::draw>(_grid.get());

    // Snap manager
    _snapManager = std::make_shared<manager::SnapManagerImpl>(_viewer->documentCanvas(),  _grid, 25.);
    _viewer->setSnapManager(_snapManager);

        // Add a cursor manager, Cursor will decide the ultimate position of clicked objects
    _cursor = std::make_shared<drawable::Cursor>(40, _viewer->documentCanvas(), lc::Color(0xff, 0x00, 0x00), lc::Color(0x00, 0xff, 0x00));
    _viewer->documentCanvas()->foreground().connect<drawable::Cursor, &drawable::Cursor::onDraw>(_cursor.get());
    _snapManager->snapPointEvents().connect<drawable::Cursor, &drawable::Cursor::onSnapPointEvent>(_cursor.get());

    _tempEntities = std::make_shared<drawable::TempEntities>(_viewer->documentCanvas());
    _viewer->documentCanvas()->foreground().connect<drawable::TempEntities, &drawable::TempEntities::onDraw>(_tempEntities.get());
    _tempEntities->requestUpdateEvent().connect<ui::LCADViewer, &ui::LCADViewer::updateHelper>(*_viewer);

    //Drag manager
    _dragManager = std::make_shared<manager::DragManager>(_viewer->documentCanvas(), _cursor, _tempEntities, 10);
    _viewer->setDragManager(_dragManager);

    _dragPoints = std::make_shared<drawable::DragPoints>();
    _dragManager->dragPointsEvent().connect<drawable::DragPoints, &drawable::DragPoints::setPoints>(_dragPoints.get());
    _viewer->documentCanvas()->foreground().connect<drawable::DragPoints, &drawable::DragPoints::onDraw>(_dragPoints.get());

    // Undo manager takes care that we can undo/redo entities within a document
    _undoManager = std::make_shared<lc::storage::UndoManagerImpl>(10);
    _document->commitProcessEvent().connect<lc::storage::UndoManagerImpl, &lc::storage::UndoManagerImpl::on_CommitProcessEvent>(_undoManager.get());
}

LCADViewer* LCADModelViewerImpl::getViewer(){
    auto viewer = new LCADViewer(_parent);
    _viewers.push_back(viewer);
    return viewer;
}