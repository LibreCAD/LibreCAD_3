#include "lcadModelViewerImpl.h"

using namespace lc::ui;

LCADModelViewerImpl::LCADModelViewerImpl(QWidget* parent):_parent(parent),_document(nullptr){
    _storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
};

void LCADModelViewerImpl::setDocument(std::shared_ptr<lc::storage::Document> document){
    _document = document;
    for(auto _viewer : _viewers){
    	_viewer->setDocument(document);
    };
}

LCADViewer* LCADModelViewerImpl::getViewer(){
    auto viewer = new LCADModelViewer(_parent);
    _viewers.push_back(viewer);
    if(_document){
    	viewer->setDocument(_document);
    }
    return viewer;
}