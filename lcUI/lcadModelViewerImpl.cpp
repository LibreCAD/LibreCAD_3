#include "lcadModelViewerImpl.h"

using namespace lc::ui;
using namespace lc::viewer;

// @TODO: result according to active layer
LCADModelViewerImpl::LCADModelViewerImpl(QWidget* parent):_parent(parent){
    _storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
};

void LCADModelViewerImpl::setDocument(std::shared_ptr<lc::storage::Document> document){
    _document = document;
    auto _viewer = _viewers[0];
    _viewer->setDocument(document);
}

LCADViewer* LCADModelViewerImpl::getViewer(){
    auto viewer = new LCADModelViewer(_parent);
    _viewers.push_back(viewer);
    return viewer;
}