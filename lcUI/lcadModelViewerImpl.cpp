#include "lcadModelViewerImpl.h"
#include <QVBoxLayout>

using namespace lc::ui;

LCADModelViewerImpl::LCADModelViewerImpl(QWidget* parent):QWidget(parent),_document(nullptr){
    _storageManager = std::make_shared<lc::storage::StorageManagerImpl>();

    auto gridLayout = new QGridLayout(this);
    gridLayout->setHorizontalSpacing(0);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));
    gridLayout->setContentsMargins(0, 0, 0, 0);

    gridLayout->addWidget(getViewer(), 0, 0, 1, 1);
};

void LCADModelViewerImpl::setDocument(std::shared_ptr<lc::storage::Document> document){
    _document = document;
    for(auto _viewer : _viewers){
    	_viewer->setDocument(document);
    };
}

LCADViewer* LCADModelViewerImpl::getViewer(){
    auto viewer = new LCADModelViewer(this);
    _viewers.push_back(viewer);
    if(_document){
    	viewer->setDocument(_document);
    }
    connect(viewer, SIGNAL(customContextMenuRequested(const QPoint&)), this->parent(), SLOT(ctxMenu(const QPoint&)));
    return viewer;
}