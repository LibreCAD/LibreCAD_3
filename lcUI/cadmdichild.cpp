#include "math.h"

#include "cadmdichild.h"

#include "cad/dochelpers/storagemanagerimpl.h"
#include "cad/dochelpers/documentimpl.h"

#include <QMenu>
#include <cad/operations/layerops.h>
#include <file.h>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>


using namespace LCViewer;

CadMdiChild::CadMdiChild(QWidget* parent) :
    QWidget(parent),
    _activeLayer(nullptr) {

    if (this->objectName().isEmpty()) {
        this->setObjectName(QStringLiteral("CadMdiChild"));
    }

    this->resize(1078, 736);

    QGridLayout* gridLayout = new QGridLayout(this);
    gridLayout->setHorizontalSpacing(0);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));
    gridLayout->setContentsMargins(0, 0, 0, 0);
    horizontalScrollBar = new QScrollBar(this);
    horizontalScrollBar->setObjectName(QStringLiteral("horizontalScrollBar"));
    horizontalScrollBar->setOrientation(Qt::Horizontal);

    gridLayout->addWidget(horizontalScrollBar, 1, 0, 1, 1);

    verticalScrollBar = new QScrollBar(this);
    verticalScrollBar->setObjectName(QStringLiteral("verticalScrollBar"));
    verticalScrollBar->setOrientation(Qt::Vertical);

    gridLayout->addWidget(verticalScrollBar, 0, 1, 1, 1);

    _viewer = new LCADViewer(this);
    _viewer->setObjectName(QStringLiteral("viewer"));
    _viewer->setGeometry(QRect(50, 30, 581, 401));
    _viewer->setAutoFillBackground(true);
    _viewer->setContextMenuPolicy(Qt::CustomContextMenu);
    _viewer->setFocusPolicy(Qt::StrongFocus);
    connect(_viewer, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ctxMenu(const QPoint&)));

    gridLayout->addWidget(_viewer, 0, 0, 1, 1);

    horizontalScrollBar->setMinimum(-1000);
    horizontalScrollBar->setMaximum(1000);
    verticalScrollBar->setMinimum(-1000);
    verticalScrollBar->setMaximum(1000);

    _metaInfoManager = std::make_shared<lc::ui::MetaInfoManager>();

    connect(horizontalScrollBar, SIGNAL(valueChanged(int)),
            _viewer, SLOT(setHorizontalOffset(int)));
    connect(verticalScrollBar, SIGNAL(valueChanged(int)),
            _viewer, SLOT(setVerticalOffset(int)));
}

CadMdiChild::~CadMdiChild() {
	if(_destroyCallback) {
		_destroyCallback(_id);
	}
}


void CadMdiChild::newDocument() {
    // Entity manager add's/removes entities to layers
    _storageManager = std::make_shared<lc::StorageManagerImpl>();

    // Create a new document with required objects, all objects that are required needs to be passed into the constructor
    _document = std::make_shared<lc::DocumentImpl>(_storageManager);

    // Add the document to a LibreCAD Viewer system so we can visualize the document
    _viewer->setDocument(_document);

    _gradientBackground = std::make_shared<GradientBackground>(lc::Color(0x07, 0x15, 0x11), lc::Color(0x06, 0x35, 0x06));
    _viewer->documentCanvas()->background().connect<GradientBackground, &GradientBackground::draw>(_gradientBackground.get());
    _grid = std::make_shared<Grid>(20, lc::Color(0x40, 0x48, 0x40), lc::Color(0x80, 0x90, 0x80));
    _viewer->documentCanvas()->background().connect<Grid, &Grid::draw>(_grid.get());

    // Snap manager
    _snapManager = std::make_shared<SnapManagerImpl>(_viewer->documentCanvas(),  _grid, 25.);
    _viewer->setSnapManager(_snapManager);

    // Add a cursor manager, Cursor will decide the ultimate position of clicked objects
    _cursor = std::make_shared<LCViewer::Cursor>(40, _viewer->documentCanvas(), lc::Color(0xff, 0x00, 0x00), lc::Color(0x00, 0xff, 0x00));
    _viewer->documentCanvas()->foreground().connect<LCViewer::Cursor, &LCViewer::Cursor::onDraw>(_cursor.get());
    _snapManager->snapPointEvents().connect<LCViewer::Cursor, &LCViewer::Cursor::onSnapPointEvent>(_cursor.get());

    _tempEntities = std::make_shared<TempEntities>(_viewer->documentCanvas());
    _viewer->documentCanvas()->foreground().connect<LCViewer::TempEntities, &LCViewer::TempEntities::onDraw>(_tempEntities.get());

    //Drag manager
    _dragManager = std::make_shared<DragManager>(_viewer->documentCanvas(), _cursor, _tempEntities, 10);
    _viewer->setDragManager(_dragManager);

    _dragPoints = std::make_shared<LCViewer::DragPoints>();
    _dragManager->dragPointsEvent().connect<LCViewer::DragPoints, &LCViewer::DragPoints::setPoints>(_dragPoints.get());
    _viewer->documentCanvas()->foreground().connect<LCViewer::DragPoints, &LCViewer::DragPoints::onDraw>(_dragPoints.get());

    // Undo manager takes care that we can undo/redo entities within a document
    _undoManager = std::make_shared<lc::UndoManagerImpl>(10);
    _document->commitProcessEvent().connect<lc::UndoManagerImpl, &lc::UndoManagerImpl::on_CommitProcessEvent>(_undoManager.get());

    _activeLayer = _document->layerByName("0");
}


bool CadMdiChild::openFile() {
    auto file = QFileDialog::getOpenFileName();

    if(file == "") {
        return false;
    }

    auto fileInfo = QFileInfo(file);

    auto ext = fileInfo.suffix().toStdString();
    auto availableLibraries = lc::File::getAvailableLibrariesForFormat(ext);

    if(availableLibraries.size() > 0) {
        //TODO: if more than once, ask which one to choose
        newDocument();
        lc::File::open(_document, file.toStdString(), availableLibraries.begin()->first);
    }
    else {
        QMessageBox::critical(nullptr, "Open error", "Unknown file extension ." + fileInfo.suffix());
        return false;
    }

    return true;
}


void CadMdiChild::saveFile() {
    QString filterList;
    QString selectedFilter;
    lc::File::Type type;
    auto availableTypes = lc::File::getAvailableFileTypes();

    if(availableTypes.size() == 0) {
        QMessageBox::critical(nullptr, "Save error", "No library available for file saving.");
        return;
    }

    auto it = availableTypes.begin();
    type = it->first;
    filterList = it->second.c_str();
    it++;

    while(it != availableTypes.end()) {
        filterList += ";;";
        filterList += it->second.c_str();

        it++;
    }

    auto file = QFileDialog::getSaveFileName(nullptr, "Save file", "", filterList, &selectedFilter);

    auto selectedType = selectedFilter.toStdString();

    for(auto availableType : availableTypes) {
        if(selectedType == availableType.second) {
            type = availableType.first;
            break;
        }
    }

    lc::File::save(_document, file.toStdString(), type);
}

void CadMdiChild::ctxMenu(const QPoint& pos) {
    QMenu* menu = new QMenu;
    menu->addAction(tr("Test Item"), this, SLOT(test_slot()));
    menu->exec(_viewer->mapToGlobal(pos));
}

SnapManager_SPtr  CadMdiChild::snapManager() const {
    return  _snapManager;
}

std::shared_ptr<lc::Document> CadMdiChild::document() const {
    return _document;
}

lc::StorageManager_SPtr CadMdiChild::storageManager() const {
    return _storageManager;
}
lc::UndoManager_SPtr CadMdiChild::undoManager() const {
    return _undoManager;
}

std::shared_ptr<LCViewer::Cursor> CadMdiChild::cursor() const {
	return _cursor;
}

unsigned int CadMdiChild::id() {
	return _id;
}
void CadMdiChild::setId(unsigned int id) {
	_id = id;
}

void CadMdiChild::setDestroyCallback(LuaIntf::LuaRef destroyCallback) {
	_destroyCallback = destroyCallback;
}

void CadMdiChild::keyPressEvent(QKeyEvent *event) {
    QWidget::keyPressEvent(event);
    emit keyPressed(event);
}

LCViewer::TempEntities_SPtr CadMdiChild::tempEntities() {
    return _tempEntities;
}

std::vector<lc::entity::CADEntity_SPtr> CadMdiChild::selection() {
    return viewer()->documentCanvas()->selection().asVector();
}

lc::Layer_CSPtr CadMdiChild::activeLayer() const {
    return _activeLayer;
}

void CadMdiChild::setActiveLayer(const lc::Layer_CSPtr& activeLayer) {
    _activeLayer = activeLayer;
}

lc::ui::MetaInfoManager_SPtr CadMdiChild::metaInfoManager() const {
    return _metaInfoManager;
}
