#include "lcadpaperviewerimpl.h"

using namespace lc::ui;

#include <QVBoxLayout>
#include <QSplitter>

using namespace lc::ui;

LCADPaperViewerImpl::LCADPaperViewerImpl(QWidget* parent):_document(nullptr),_parent(parent){
    _storageManager = std::make_shared<lc::storage::StorageManagerImpl>();
    qDebug( "lcadpaperviewerimpl--- Constructor()--- object= %u parent=%u",this,parent );
    qDebug( "lcadpaperviewerimpl---constructor()--- size of _viewer= %d",_viewers.size() );
   
   
};

void LCADPaperViewerImpl::setDocument(std::shared_ptr<lc::storage::Document> document){
    _document = document;
    qDebug( "lcadpaperviewerimpl---setDocument()--- object= %u",this );
    
}

LCADPaperViewer* LCADPaperViewerImpl::getViewer()
{
    qDebug( "lcadpaperviewerimpl---getViewer()--- object= %u",this );
    auto viewer = new LCADPaperViewer(_parent,_viewers.size());
    _viewers.push_back(viewer);
    qDebug( "lcadpaperviewerimpl---getviewer()--- size of _viewer= %d",_viewers.size() );
    
    connect(viewer, SIGNAL(setActive(int)), this, SLOT(setActive(int)));
    //connect(viewer, SIGNAL(customContextMenuRequested(const QPoint&)), this->parent(), SLOT(ctxMenu(const QPoint&)));
    return viewer;
}

void LCADPaperViewerImpl::setActive(int v){
    _viewers[_activeView]->setFocused(false);
    _activeView=v;
    _viewers[_activeView]->setFocused(true);
    emit setActiveView(_viewers[v],false);
};