#include "lcadviewer.h"
#include <QtGui>
#include <QVBoxLayout>
#include <iostream>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <cad/logger/logger.h>
#include "widgets/guiAPI/menu.h"
#include "managers/contextmenumanager.h"
#include "viewersettings.h"

using namespace lc;
using namespace lc::ui;

LCADViewer::LCADViewer(QWidget *parent) :
    QOpenGLWidget(parent),
    _docCanvas(nullptr),
    _mouseScrollKeyActive(false),
    _operationActive(false),
    _scale(1.0),
    _zoomMin(0.05),
    _zoomMax(20.0),
    _scaleLineWidth(false),
    _backgroundPainter(nullptr),
    _documentPainter(nullptr),
    _foregroundPainter(nullptr)
{
    setMouseTracking(true);
    this->_altKeyActive = false;
    this->_ctrlKeyActive = false;
    setCursor(Qt::BlankCursor);

    QSurfaceFormat format;
    format.setMajorVersion(3);
    format.setMinorVersion(0);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOption(QSurfaceFormat::DebugContext);
    setFormat(format);

    // This list work in reverse lower loads first
    _hookManager.append("select", [&](lc::ui::HookEvent& e)->bool{return selectHandler(e);});
    _hookManager.append("drag", [&](lc::ui::HookEvent& e)->bool{return dragHandler(e);});
    _hookManager.append("pan", [&](lc::ui::HookEvent& e)->bool{return panHandler(e);});
    // Inside lambdas to can capture context
    //   bind would be better here but not sure
}

void LCADViewer::messageLogged(const QOpenGLDebugMessage &msg)
{
    QString error;
    error += " (";

    // Format based on source
#define CASE(c) case QOpenGLDebugMessage::c: error += #c; break
    switch (msg.source())
    {
        CASE(APISource);
        CASE(WindowSystemSource);
        CASE(ShaderCompilerSource);
        CASE(ThirdPartySource);
        CASE(ApplicationSource);
        CASE(OtherSource);
        CASE(InvalidSource);
    }
#undef CASE

    error += " : ";

    // Format based on type
#define CASE(c) case QOpenGLDebugMessage::c: error += #c; break
    switch (msg.type())
    {
        CASE(ErrorType);
        CASE(DeprecatedBehaviorType);
        CASE(UndefinedBehaviorType);
        CASE(PortabilityType);
        CASE(PerformanceType);
        CASE(OtherType);
        CASE(MarkerType);
        CASE(GroupPushType);
        CASE(GroupPopType);
    }
#undef CASE

    error += ")";

    switch (msg.severity())
    {
    case QOpenGLDebugMessage::NotificationSeverity:
        LOG_DEBUG << error.toStdString() << std::endl;
        break;
    case QOpenGLDebugMessage::HighSeverity:
        LOG_ERROR << error.toStdString() << std::endl;
        break;
    case QOpenGLDebugMessage::MediumSeverity:
        LOG_WARNING << error.toStdString() << std::endl;
        break;
    case QOpenGLDebugMessage::LowSeverity:
        LOG_INFO << error.toStdString() << std::endl;
        break;
    }
}

LCADViewer::~LCADViewer()
{
    deletePainters();
    _document->commitProcessEvent().disconnect<LCADViewer, &LCADViewer::on_commitProcessEvent>(this);
}

void LCADViewer::initializeGL()
{
        
    
    QString sPathToShaders = QCoreApplication::applicationDirPath() + "/../resources/shaders/";
    QString sPathToFonts = QCoreApplication::applicationDirPath() + "/../resources/fonts/";

    //const char* cPathToShaders = sPathToShaders.toLocal8Bit().constData();
    //const char* cPathToFonts = sPathToFonts.toLocal8Bit().constData();

    auto pathToShaders = lc::storage::settings::StringSettingValue(sPathToShaders.toStdString());
    auto pathToFonts = lc::storage::settings::StringSettingValue(sPathToFonts.toStdString());

    (lc::viewer::setShader(pathToShaders));
    (lc::viewer::setFont(pathToFonts));

    //lc::viewer::opengl::SHADER_PATH = cPathToShaders;
    //lc::viewer::opengl::FONT_PATH = cPathToFonts;


    QOpenGLWidget::makeCurrent();
    QOpenGLContext *CC= QOpenGLContext::currentContext();

    QOpenGLDebugLogger *logger = new QOpenGLDebugLogger(this);
    logger->initialize();

    connect(logger, &QOpenGLDebugLogger::messageLogged, this, &LCADViewer::messageLogged);
    logger->startLogging();


    int width = size().width();
    int height = size().height();

    if (CC != 0)
    {
        GLenum err = glewInit();

        if (err != GLEW_OK) {
            LOG_ERROR << "GLEW Error: " << glewGetErrorString(err) << std::endl;
            exit(1);
        }
        if (!GLEW_VERSION_2_1) {
            LOG_ERROR << "OpenGL version 2.1 is not available" << std::endl;
            exit(1);
        }

        deletePainters();
        createPainters(width, height);
        _documentPainter->create_resources();
    }

    else
    {
        deletePainters();
        createPainters(width, height);
    }

    
}


void LCADViewer::setDocument(std::shared_ptr<lc::storage::Document> document, meta::Block_CSPtr viewport)
{
    int width = size().width();
    int height = size().height();

    deletePainters();
    createPainters(width, height);

    _docCanvas = std::make_shared<lc::viewer::DocumentCanvas>(document, [this](double* x, double* y) {
        _documentPainter->device_to_user(x, y);
    }, viewport);

    _document = document;
    _document->commitProcessEvent().connect<LCADViewer, &LCADViewer::on_commitProcessEvent>(this);
    _docCanvas->selectionChanged().connect<LCADViewer, &LCADViewer::_selectionChanged>(this);

    if(_docCanvas != nullptr)
        _docCanvas->setPainter(_documentPainter);  //passing pointer to painter to doc canvas
}

void LCADViewer::setSnapManager(std::shared_ptr<lc::viewer::manager::SnapManager> snapmanager) {
    _snapManager = std::move(snapmanager);
}

void LCADViewer::setDragManager(lc::viewer::manager::DragManager_SPtr dragManager) {
    _dragManager = std::move(dragManager);
}

void LCADViewer::updateHelper() {
    update();
}

void LCADViewer::on_commitProcessEvent(const lc::event::CommitProcessEvent& event) {
    updateDocument();
    update();
}

void LCADViewer::_selectionChanged() {
    _dragManager->onSelectionChanged();
    emit selectionChangeEvent();
}

/**
  * Handle key pressing and release to add additional states to this view
  *
  */
void LCADViewer::keyPressEvent(QKeyEvent* event) {
    QWidget::keyPressEvent(event);

    switch (event->key()) {
    case Qt::Key_Shift:
        // When shift key is released we switch back to rubber band
        //     setDragMode(QGraphicsView::ScrollHandDrag);
        break;

    case Qt::Key_Alt:
        //
        _altKeyActive = true;
        break;

    case Qt::Key_Control:
        _ctrlKeyActive = true;
        break;

    case Qt::Key_Return:
        //Change focus to command line
        break;

    default:
        emit keyPressEvent(event->key());
        break;
    }
}

void LCADViewer::keyReleaseEvent(QKeyEvent* event) {
    QWidget::keyReleaseEvent(event);

    switch (event->key()) {
    case Qt::Key_Shift:
        // When shift key is released we switch back to rubber band
        //       setDragMode(QGraphicsView::RubberBandDrag);
        break;

    case Qt::Key_Alt:
        //
        _altKeyActive = false;
        break;

    case Qt::Key_Control:
        _ctrlKeyActive = false;
        break;
    }
}

void LCADViewer::resizeGL(int width, int height)
{
    _docCanvas->newDeviceSize(width,height);
    _documentPainter->new_device_size(width,height);
    updateBackground();
    updateDocument();
}

void LCADViewer::wheelEvent(QWheelEvent *event) {
    // Make 10% for 15deg turn, enable slow zoom for mouse with more precision
    // May be use accumulation if very sensetive
    QOpenGLWidget::makeCurrent();
    QPoint numDegrees = event->angleDelta()/8;
    double zoom;

    if(numDegrees.isNull()) return;
    zoom = 1.+(double)numDegrees.y()/150.;

    for(auto pair : imagemaps)
    {
        _docCanvas->zoom(*pair.first, zoom, true, event->pos().x(), event->pos().y());
    }

    updateBackground();
    updateDocument();
    this->update();
}

bool LCADViewer::dragHandler(lc::ui::HookEvent& e) {
    //It shouldn't happen during operation
    // And should be strong check
    // Drag
    // Left click check if clicked in any drag point
    // After that only move
    // And finally release leaves hook
    if(_operationActive)return false;
    if(e.eventType=="pressMouseEvent") {
        if (e.mouseEvent->buttons() != Qt::LeftButton)return false;
        _dragManager->onMousePress();
        if(_dragManager->entityDragged())
            e.grab();// Grab we are dragging now no interference
    } else if(e.eventType=="moveMouseEvent") {
        _dragManager->onMouseMove();
    } else if(e.eventType=="releaseMouseEvent") {
        bool grabbed=_dragManager->entityDragged();
        _dragManager->onMouseRelease();
        if(grabbed) { //If we have grabbed
            e.free();
            return true;
        }
    }
    return false;
};

// Select should always be at last
bool LCADViewer::selectHandler(lc::ui::HookEvent& e) {
    /**
    * Select works this way
    * if we click and release mouse, no move point mode
    * else area mode
    */
    // We need to add stronger check if some handler come after it
    if(e.eventType=="pressMouseEvent") {
        if (e.mouseEvent->buttons() != Qt::LeftButton)return false;
        startSelectPos = e.mouseEvent->pos();// Save it
        e.grab();// Grab we are selecting now no interference
    } else if(e.eventType=="moveMouseEvent") {
        if (!startSelectPos.isNull()) {
            bool occopies = startSelectPos.x() < e.mouseEvent->pos().x();
            _docCanvas->makeSelectionDevice(
                *_documentPainter,
                std::min(startSelectPos.x(), e.mouseEvent->pos().x()), std::min(startSelectPos.y(), e.mouseEvent->pos().y()),
                std::abs(startSelectPos.x() - e.mouseEvent->pos().x()),
                std::abs(startSelectPos.y() - e.mouseEvent->pos().y()), occopies);

            updateDocument();
        }
    } else if(e.eventType=="releaseMouseEvent") {
        if (startSelectPos.isNull())return false;
        if(startSelectPos==e.mouseEvent->pos()) { //Point select
            double x = startSelectPos.x();
            double y = startSelectPos.y();
            _documentPainter->device_to_user(&x, &y);
            docCanvas()->selectPoint(x, y);
        } else { //Rectangular select
            _docCanvas->closeSelection();
            _docCanvas->removeSelectionArea();
        }
        updateDocument();
        startSelectPos=QPoint();
        e.free();
        return true;
    }
    return false;
};

bool LCADViewer::panHandler(lc::ui::HookEvent& e) {
    /**
    * Pan, work with middle mouse only
    * Check click, move and release
    */
    if(e.eventType=="pressMouseEvent") {
        if (e.mouseEvent->buttons() != Qt::MiddleButton)return false;
        startSelectPos = e.mouseEvent->pos();// Save it
        _mouseScrollKeyActive = true;
        e.grab();// Grab we are selecting now no interference
    } else if(e.eventType=="moveMouseEvent") {
        if (startSelectPos.isNull())return false;// No info about it
        if (_altKeyActive || _mouseScrollKeyActive) {
            if (!startSelectPos.isNull()) {
                auto translateX = e.mouseEvent->pos().x()-startSelectPos.x();
                auto translateY = e.mouseEvent->pos().y()-startSelectPos.y();
                startSelectPos = e.mouseEvent->pos();
                for(auto pair : imagemaps) {
                    _docCanvas->pan(*pair.first, translateX, translateY);
                }

                updateBackground();
                updateDocument();
            }
        }
    } else if(e.eventType=="releaseMouseEvent") {
        if (startSelectPos.isNull())return false;// No info about it
        startSelectPos=QPoint();
        e.free();
        _mouseScrollKeyActive = false;
        return true;
    }
    return false;
};

void LCADViewer::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);
    _snapManager->setDeviceLocation(event->pos().x(), event->pos().y());
    _hookManager.onMouseEvent("move", event);
    emit mouseMoveEvent();
    update();
}

void LCADViewer::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    _hookManager.onMouseEvent("press", event);
    if (event->buttons() != Qt::RightButton) {
        emit mousePressEvent();
    }
}


void LCADViewer::mouseReleaseEvent(QMouseEvent *event) {
    _hookManager.onMouseEvent("release", event);
    emit mouseReleaseEvent();
    update();
}

std::shared_ptr<lc::viewer::DocumentCanvas> LCADViewer::documentCanvas() const {
    return _docCanvas;
}

void LCADViewer::setOperationActive(bool operationActiveIn) {
    _operationActive = operationActiveIn;

    if(!operationActiveIn) {
        documentCanvas()->removeSelection();
    }
}

bool LCADViewer::operationActive() const {
    return _operationActive;
}

void LCADViewer::paintGL()
{
    _docCanvas->render(*_documentPainter, lc::viewer::VIEWER_BACKGROUND);
    _docCanvas->render(*_documentPainter, lc::viewer::VIEWER_DOCUMENT);
    _docCanvas->render(*_documentPainter, lc::viewer::VIEWER_FOREGROUND);
}

void LCADViewer::createPainters(unsigned int width, unsigned int height) {
    QImage *m_image;

    m_image = new QImage(width, height, QImage::Format_ARGB32);
    _documentPainter = lc::viewer::createOpenGLPainter(m_image->bits(), width, height);
    imagemaps.insert(std::make_pair(_documentPainter, m_image));

    if(_docCanvas != nullptr)
        _docCanvas->setPainter(_documentPainter);  //passing pointer to painter to doc canvas
}

void LCADViewer::deletePainters()
{
    for(auto pair : imagemaps) {
        delete pair.first;
        delete pair.second;
    }
    imagemaps.clear();
}

void LCADViewer::updateBackground()
{
}

void LCADViewer::updateDocument()
{
}

const std::shared_ptr<lc::viewer::DocumentCanvas>& LCADViewer::docCanvas() const {
    return _docCanvas;
}

void LCADViewer::setContextMenuManagerId(int contextMenuManagerId) {
    _contextMenuManagerId = contextMenuManagerId;
}

void LCADViewer::contextMenuEvent(QContextMenuEvent* event) {
    lc::ui::api::Menu menu("ContextMenu", this);
    ContextMenuManager::GetContextMenuManager(_contextMenuManagerId)->generateMenu(&menu, documentCanvas()->selectedEntities().asVector());
    menu.exec(event->globalPos());
}
