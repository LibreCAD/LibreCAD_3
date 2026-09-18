#include "cmath"

#include "cadmdichild.h"

#include "cad/storage/documentimpl.h"

#include <QMenu>
#include <cad/operations/layerops.h>
#include <file.h>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>

using namespace lc::ui;
using namespace lc::viewer;

CadMdiChild::CadMdiChild(QWidget* parent) :
    QWidget(parent),
    _activeLayer(nullptr) {

    if (this->objectName().isEmpty()) {
        this->setObjectName(QStringLiteral("CadMdiChild"));
    }

    this->resize(1078, 736);

    auto gridLayout = new QGridLayout(this);
    gridLayout->setHorizontalSpacing(0);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));
    gridLayout->setContentsMargins(0, 0, 0, 0);

    _viewerProxy = new LCADViewerProxy(this);
    gridLayout->addWidget(_viewerProxy, 0, 0, 1, 1);
    _metaInfoManager = std::make_shared<lc::ui::MetaInfoManager>();
    connect(this,SIGNAL(keyPressEventx(int)),_viewerProxy,SIGNAL(keyPressEvent(int)));
    // it CadMdi knows about proxy
    QObject::connect(_viewerProxy, &LCADViewerProxy::mousePressEvent, this, &CadMdiChild::mousePressEvent);
    QObject::connect(_viewerProxy, &LCADViewerProxy::mouseReleaseEvent, this, &CadMdiChild::mouseReleaseEvent);
    QObject::connect(_viewerProxy, &LCADViewerProxy::mouseMoveEvent, this, &CadMdiChild::mouseMoveEvent);
    QObject::connect(_viewerProxy, &LCADViewerProxy::selectionChangeEvent, this, &CadMdiChild::selectionChangeEvent);
}

CadMdiChild::~CadMdiChild() {
    // Phase 4 PR-10 — _destroyCallback is ScriptCallback now; invoke()
    // is nil-safe (returns Nil for a default-constructed callback), so
    // no explicit isNil check needed.  Matches the pre-refactor
    // behavior where an unset LuaRef converted to false in the bool
    // context.
    _destroyCallback.invoke();
}


void CadMdiChild::newDocument() {
    // Create a new document with required objects, all objects that are required needs to be passed into the constructor
    _document = std::make_shared<lc::storage::DocumentImpl>(storageManager());

    // Add the document to a LibreCAD Viewer system so we can visualize the document
    _viewerProxy->setDocument(_document);

    _activeLayer = _document->layerByName("0");
}


bool CadMdiChild::openFile() {
    auto availableTypes = lc::persistence::File::getSupportedFileExtensions();

    if(availableTypes.empty()) {
        QMessageBox::critical(nullptr, "Save error", "No library available for file opening.");
        return false;
    }

    //Build format string
    QString filterList="All Supported Formats(";
    auto it = availableTypes.begin();
    filterList += (" *."+it->first).c_str();
    it++;
    while(it != availableTypes.end()) {
        filterList += (" *."+it->first).c_str();
        it++;
    }
    filterList+=");;";

    it = availableTypes.begin();
    filterList += (it->second+"(*."+it->first+")").c_str();
    it++;
    while(it != availableTypes.end()) {
        filterList += (";;"+it->second+"(*."+it->first+")").c_str();
        it++;
    }
    filterList+=";;All Files(*.*)";

    auto file = QFileDialog::getOpenFileName(nullptr,"Open document",nullptr,filterList);

    if(file == "") {
        return false;
    }

    auto fileInfo = QFileInfo(file);

    auto ext = fileInfo.suffix().toStdString();
    auto availableLibraries = lc::persistence::File::getAvailableLibrariesForFormat(ext);

    if(!availableLibraries.empty()) {
        //TODO: if more than once, ask which one to choose
        newDocument();
        _filename = file.toStdString();

        const auto result = lc::persistence::File::importFile(
            _document, _filename, availableLibraries.begin()->first);
        _source = lc::persistence::sourceFromImport(_filename, result);

        if (!result.ok) {
            // The document holds whatever arrived before the failure, which is
            // worth showing -- but the user has to know it is not the drawing.
            QMessageBox::warning(nullptr, tr("Open"),
                                 tr("%1 could not be read completely; %2 entities were "
                                    "recovered. Saving will ask for a new file so the "
                                    "original is not replaced by the part that was readable.")
                                 .arg(QString::fromStdString(_filename))
                                 .arg(static_cast<qulonglong>(result.entitiesDelivered)));
        } else if (!result.loss.empty()) {
            QString dropped;
            for (const auto& kind : result.loss.droppedByType) {
                if (!dropped.isEmpty()) {
                    dropped += ", ";
                }
                dropped += QString::number(static_cast<qulonglong>(kind.second)) + " "
                           + QString::fromStdString(kind.first);
            }
            QMessageBox::information(nullptr, tr("Open"),
                                     tr("%1 contains records LibreCAD has no equivalent for, "
                                        "which were not imported: %2.")
                                     .arg(QString::fromStdString(_filename)).arg(dropped));
        }
    }
    else {
        QMessageBox::critical(nullptr, "Open error", "Unknown file extension ." + fileInfo.suffix());
        return false;
    }

    return true;
}

void CadMdiChild::saveFile() {
    // Whether Save may overwrite the file this document came from is decided
    // outside this class, where it can be tested without a window: see
    // persistence/documentsource.cpp.
    const auto decision = lc::persistence::decideSave(_source);

    if (decision.action != lc::persistence::SaveAction::WriteToPath) {
        if (!decision.reason.empty() && _source.hasPath) {
            QMessageBox::information(nullptr, tr("Save"),
                                     tr("%1 Choose where to save it.")
                                     .arg(QString::fromStdString(decision.reason)));
        }
        saveAsFile();
        return;
    }

    lc::persistence::File::Type type = lc::persistence::File::Type::LIBDXFRW_DXF_R2000;
    if (!lc::persistence::File::typeForVariantId(decision.variantId, type)) {
        saveAsFile();
        return;
    }

    reportSaveFailure(lc::persistence::File::save(_document, decision.path, type), decision.path);
}

// A refused write leaves the target untouched, so the user must be told: the
// document is still unsaved.
void CadMdiChild::reportSaveFailure(bool saved, const std::string& path) {
    if (saved) {
        return;
    }

    QMessageBox::critical(nullptr, tr("Save error"),
                          tr("%1 could not be written in the selected format. "
                             "The file was not changed. See the log for details.")
                          .arg(QString::fromStdString(path)));
}

void CadMdiChild::saveAsFile() {
    QString filterList;
    QString selectedFilter;
    lc::persistence::File::Type type = lc::persistence::File::Type::LIBDXFRW_DXF_R2000;
    auto availableTypes = lc::persistence::File::getAvailableFileTypes();

    if(availableTypes.empty()) {
        QMessageBox::critical(nullptr, "Save error", "No library available for file saving.");
        return;
    }

    auto it = availableTypes.begin();
    filterList = (it->second+"(*."+lc::persistence::File::getExtensionForFileType(it->first)+")").c_str();
    it++;
    while(it != availableTypes.end()) {
        filterList += (";;"+it->second+"(*."+lc::persistence::File::getExtensionForFileType(it->first)+")").c_str();
        it++;
    }

    auto file = QFileDialog::getSaveFileName(nullptr, "Save file", "", filterList, &selectedFilter);

    if (file.isEmpty())
    {
        return;
    }

    auto selectedType = selectedFilter.toStdString();
    //Removing extension part
    std::size_t fpos = selectedType.rfind("(*");

    selectedType = selectedType.substr(0,fpos);

    bool chosen = false;
    for(auto availableType : availableTypes) {
        if(selectedType == availableType.second) {
            type = availableType.first;
            chosen = true;
            break;
        }
    }

    // `type` was read uninitialised when no filter matched -- the format the
    // document was then saved in, and remembered as, was whatever happened to
    // be on the stack.
    if (!chosen) {
        QMessageBox::critical(nullptr, tr("Save error"),
                              tr("%1 is not a format LibreCAD can write.")
                              .arg(QString::fromStdString(selectedType)));
        return;
    }

    //Add extension if not present
    auto fileInfo = QFileInfo(file);
    auto ext = fileInfo.suffix().toStdString();
    if(ext=="")file+=("."+lc::persistence::File::getExtensionForFileType(type)).c_str();
    _filename = file.toStdString();

    const auto result = lc::persistence::File::exportFile(_document, _filename, type);
    reportSaveFailure(result.ok, _filename);

    if (result.ok) {
        // The document now *is* this file, completely: a later Save writes
        // straight back here, whatever it was opened from.
        _source.hasPath = true;
        _source.path = _filename;
        _source.variantId = result.variantId;
        _source.partial = false;
        _source.writable = true;
    }
}

void CadMdiChild::ctxMenu(const QPoint& pos) {
    auto menu = new QMenu;
    menu->addAction(tr("Test Item"), this, SLOT(test_slot()));
    menu->exec(_viewerProxy->mapToGlobal(pos));
}

manager::SnapManager_SPtr  CadMdiChild::snapManager() const {
    return _viewerProxy->snapManager();
}

std::shared_ptr<lc::storage::Document> CadMdiChild::document() const {
    return _viewerProxy->document();
}

lc::storage::StorageManager_SPtr CadMdiChild::storageManager() const {
    return _viewerProxy->storageManager();
}

lc::storage::UndoManager_SPtr CadMdiChild::undoManager() const {
    return _viewerProxy->undoManager();
}

std::shared_ptr<drawable::Cursor> CadMdiChild::cursor() const {
    return _viewerProxy->cursor();
}

void CadMdiChild::setDestroyCallback(lc::scripting::ScriptCallback destroyCallback) {
    _destroyCallback = std::move(destroyCallback);
}

void CadMdiChild::keyPressEvent(QKeyEvent *event) {
    QWidget::keyPressEvent(event);
    emit keyPressed(event);
    emit keyPressEventx(event->key());
}

drawable::TempEntities_SPtr CadMdiChild::tempEntities() {
    return _viewerProxy->tempEntities();
}

std::vector<lc::entity::CADEntity_CSPtr> CadMdiChild::selection() {
    return viewer()->documentCanvas()->selectedEntities().asVector();
}

lc::meta::Layer_CSPtr CadMdiChild::activeLayer() const {
    return _activeLayer;
}

void CadMdiChild::setActiveLayer(const lc::meta::Layer_CSPtr& activeLayer) {
    _activeLayer = activeLayer;
}

lc::ui::MetaInfoManager_SPtr CadMdiChild::metaInfoManager() const {
    return _metaInfoManager;
}

const manager::SnapManagerImpl_SPtr CadMdiChild::getSnapManager() const {
    return _viewerProxy->snapManager();
}
