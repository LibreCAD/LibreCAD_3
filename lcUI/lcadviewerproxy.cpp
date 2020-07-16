#include "lcadviewerproxy.h"

using namespace lc::ui;

#include <QVBoxLayout>

using namespace lc::ui;

LCADViewerProxy::LCADViewerProxy(QWidget* parent=0){
    auto gridLayout = new QGridLayout(this);
    gridLayout->setHorizontalSpacing(0);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));
    gridLayout->setContentsMargins(0, 0, 0, 0);

    _tabWidget = new QTabWidget(this);

    _modelViewerImpl = new LCADModelViewerImpl(this);
    _modelViewerImpl->setObjectName(QStringLiteral("viewer"));
    _modelViewerImpl->setGeometry(QRect(50, 30, 581, 401));
    _modelViewerImpl->setAutoFillBackground(true);
    _modelViewerImpl->setContextMenuPolicy(Qt::CustomContextMenu);
    _modelViewerImpl->setFocusPolicy(Qt::StrongFocus);

    gridLayout->addWidget(_tabWidget, 0, 0, 1, 1);

    _tabWidget->addTab(_modelViewerImpl, tr("Model"));
    _tabWidget->addTab(_modelViewerImpl, tr("Paper 1"));//Just for init, removed later
    _tabWidget->addTab(_modelViewerImpl, tr("Paper 2"));

    _paperViewers = new LCADPaperViewerImpl(this);
    
    //Connections to receive active View
    connect(_modelViewerImpl, SIGNAL(setActiveView(LCADViewer*,bool)), this, SLOT(setActive(LCADViewer*,bool)));
    connect(_paperViewers, SIGNAL(setActiveView(LCADViewer*,bool)), this, SLOT(setActive(LCADViewer*,bool)));

    //Init connections now
    _isModel = true;
    _activeView = _modelViewerImpl->viewer();
    connect(_activeView, SIGNAL(mouseMoveEvent()), this, SIGNAL(mouseMoveEvent()));
    connect(_activeView, SIGNAL(mousePressEvent()), this, SIGNAL(mousePressEvent()));
    connect(_activeView, SIGNAL(mouseReleaseEvent()), this, SIGNAL(mouseReleaseEvent()));
    connect(_activeView, SIGNAL(keyPressEvent(int)), this, SIGNAL(keyPressEvent(int)));
}

void LCADViewerProxy::setDocument(std::shared_ptr<lc::storage::Document> document){
    _modelViewerImpl->setDocument(document);
    _paperViewers->setDocument(document);

    _tabWidget->removeTab(1);
    _tabWidget->removeTab(1);

    //For each block create viewport
    auto view = document->blockByName("*Paper_Space");
    auto x = _paperViewers->getViewer();
    x->setDocument(document,view);
    _tabWidget->addTab(x,tr("Paper 1"));

    view = document->blockByName("*Paper_Space0");
    x = _paperViewers->getViewer();
    x->setDocument(document,view);
    _tabWidget->addTab(x,tr("Paper 2"));
}

void LCADViewerProxy::setActive(LCADViewer* view,bool isModel){
    if(view!=_activeView){
        _isModel = isModel;
        disconnect(_activeView, SIGNAL(mouseMoveEvent()), this, SIGNAL(mouseMoveEvent()));
        disconnect(_activeView, SIGNAL(mousePressEvent()), this, SIGNAL(mousePressEvent()));
        disconnect(_activeView, SIGNAL(mouseReleaseEvent()), this, SIGNAL(mouseReleaseEvent()));
        disconnect(_activeView, SIGNAL(keyPressEvent(int)), this, SIGNAL(keyPressEvent(int)));

        _activeView = view;
        connect(_activeView, SIGNAL(mouseMoveEvent()), this, SIGNAL(mouseMoveEvent()));
        connect(_activeView, SIGNAL(mousePressEvent()), this, SIGNAL(mousePressEvent()));
        connect(_activeView, SIGNAL(mouseReleaseEvent()), this, SIGNAL(mouseReleaseEvent()));
        connect(_activeView, SIGNAL(keyPressEvent(int)), this, SIGNAL(keyPressEvent(int)));
    }
}
