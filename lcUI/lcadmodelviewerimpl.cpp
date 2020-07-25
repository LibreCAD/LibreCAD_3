#include "lcadmodelviewerimpl.h"
#include <QVBoxLayout>
#include <QSplitter>

using namespace lc::ui;

LCADModelViewerImpl::LCADModelViewerImpl(QWidget* parent):QWidget(parent),_document(nullptr){
    _storageManager = std::make_shared<lc::storage::StorageManagerImpl>();

    auto gridLayout = new QGridLayout(this);
    gridLayout->setHorizontalSpacing(0);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));
    gridLayout->setContentsMargins(0, 0, 0, 0);

    //For now splitter to two views
    auto splitter = new QSplitter(this);
    gridLayout->addWidget(splitter, 0, 0, 1, 1);
    auto v=getViewer();
    v->setFocused(true);
    splitter->addWidget(v);
    //splitter->addWidget(getViewer());
    //Hide the second view, can be accessed by splitter
    //splitter->setSizes(QList<int>() << 1 << 0); 
    // add these lines only when requested for second viewer
};

void LCADModelViewerImpl::setDocument(std::shared_ptr<lc::storage::Document> document){
    _document = document;
    for(auto _viewer : _viewers){
    	_viewer->setDocument(document);
    };
}

LCADModelViewer* LCADModelViewerImpl::getViewer(){
    auto viewer = new LCADModelViewer(this,_viewers.size());
    _viewers.push_back(viewer);
    if(_document){
    	viewer->setDocument(_document);
    }
    connect(viewer, SIGNAL(setActive(int)), this, SLOT(setActive(int)));
    //connect(viewer, SIGNAL(customContextMenuRequested(const QPoint&)), this->parent(), SLOT(ctxMenu(const QPoint&)));
    return viewer;
}

void LCADModelViewerImpl::setActive(int v){
    _viewers[_activeView]->setFocused(false);
    _activeView=v;
    _viewers[_activeView]->setFocused(true);
    emit setActiveView(_viewers[v],true);
};
