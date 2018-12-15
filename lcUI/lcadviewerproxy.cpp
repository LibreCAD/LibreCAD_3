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
    connect(_activeView, SIGNAL(keyPressEvent(int)), this, SIGNAL(keyPress(int)));
}

void LCADViewerProxy::setDocument(std::shared_ptr<lc::storage::Document> document){
    _modelViewerImpl->setDocument(document);
    _paperViewers->setDocument(document);
    _viewport = document->viewportByName("MODEL");

    auto viewports = document->allViewports();
    for(auto view: viewports){
        if(view.first!="MODEL"){
            auto x = _paperViewers->getViewer();
            _tabWidget->addTab(x,tr(view.first.c_str()));
        }
    }
}

void LCADViewerProxy::setActive(LCADViewer* view,bool isModel){
    if(view!=_activeView){
        _isModel = isModel;
        disconnect(_activeView, SIGNAL(mouseMoveEvent()), this, SIGNAL(mouseMoveEvent()));
        disconnect(_activeView, SIGNAL(mousePressEvent()), this, SIGNAL(mousePressEvent()));
        disconnect(_activeView, SIGNAL(mouseReleaseEvent()), this, SIGNAL(mouseReleaseEvent()));
        disconnect(_activeView, SIGNAL(keyPressEvent(int)), this, SIGNAL(keyPress(int)));

        _activeView = view;
        connect(_activeView, SIGNAL(mouseMoveEvent()), this, SIGNAL(mouseMoveEvent()));
        connect(_activeView, SIGNAL(mousePressEvent()), this, SIGNAL(mousePressEvent()));
        connect(_activeView, SIGNAL(mouseReleaseEvent()), this, SIGNAL(mouseReleaseEvent()));
        connect(_activeView, SIGNAL(keyPressEvent(int)), this, SIGNAL(keyPress(int)));
    }
}