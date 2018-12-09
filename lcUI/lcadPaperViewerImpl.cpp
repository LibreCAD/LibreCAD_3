#include "lcadPaperViewerImpl.h"

using namespace lc::ui;

#include <QVBoxLayout>
#include <QSplitter>

using namespace lc::ui;

LCADPaperViewerImpl::LCADPaperViewerImpl(QWidget* parent):_document(nullptr),_parent(parent){
    _storageManager = std::make_shared<lc::storage::StorageManagerImpl>();

};

void LCADPaperViewerImpl::setDocument(std::shared_ptr<lc::storage::Document> document){
    _document = document;
    for(auto _viewer : _viewers){
    	_viewer->setDocument(document);
    };
}

LCADPaperViewer* LCADPaperViewerImpl::getViewer(){
    auto viewer = new LCADPaperViewer(_parent,_viewers.size());
    _viewers.push_back(viewer);
    if(_document){
    	viewer->setDocument(_document);
    }
    connect(viewer, SIGNAL(setActive(int)), this, SLOT(setActive(int)));
    //connect(viewer, SIGNAL(customContextMenuRequested(const QPoint&)), this->parent(), SLOT(ctxMenu(const QPoint&)));
    return viewer;
}

void LCADPaperViewerImpl::setActive(int v){
    _viewers[_activeView]->setFocused(false);
    _activeView=v;
    _viewers[_activeView]->setFocused(true);
    emit setActiveView(_viewers[v]);
};