#include "lcadviewer.h"
#include "glewsupport.h"
#include <QtGui>
#include <QVBoxLayout>
#include <iostream>
#include <QLabel>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <cad/logger/logger.h>
#include "widgets/guiAPI/menu.h"
#include "managers/contextmenumanager.h"

using namespace lc;
using namespace lc::ui;

QString LCADViewer::glInitializationFailureMessage(const QString& detail)
{
    QString message = tr("The drawing canvas could not initialize OpenGL.");
    if (!detail.isEmpty()) {
        message += QStringLiteral("\n\n") + detail;
    }
    message += QStringLiteral("\n\n")
        + tr("If this system uses a Wayland session, start LibreCAD from a "
             "terminal with QT_QPA_PLATFORM=xcb set in the environment and "
             "try again.");
    return message;
}

LCADViewer::LCADViewer(QWidget *parent) :
    QOpenGLWidget(parent),
    _docCanvas(nullptr),
    _mouseScrollKeyActive(false),
    _operationActive(false),
    _scale(1.0),
    _zoomMin(0.05),
    _zoomMax(20.0),
    _scaleLineWidth(false),
    _autoScalePending(false),
    _glFailureOverlay(nullptr),
    _backgroundPainter(nullptr),
    _documentPainter(nullptr),
    _foregroundPainter(nullptr)
{
    setMouseTracking(true);
    this->_altKeyActive = false;
    this->_ctrlKeyActive = false;
    setCursor(Qt::BlankCursor);

    _glFailureOverlay = new QLabel(this);
    _glFailureOverlay->setWordWrap(true);
    _glFailureOverlay->setAlignment(Qt::AlignCenter);
    _glFailureOverlay->setTextFormat(Qt::PlainText);
    _glFailureOverlay->setTextInteractionFlags(Qt::TextSelectableByMouse);
    _glFailureOverlay->setStyleSheet(
        QStringLiteral("QLabel { background-color: #1e1e1e; color: #f0f0f0; padding: 24px; }"));
    _glFailureOverlay->hide();

    QSurfaceFormat format;
    format.setMajorVersion(3);
    format.setMinorVersion(2);
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
        // AnySource/AnyType/AnySeverity are Qt's filter sentinels and
        // Invalid* cannot arrive on a real message; nothing to name.
        default: break;
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
        // AnySource/AnyType/AnySeverity are Qt's filter sentinels and
        // Invalid* cannot arrive on a real message; nothing to name.
        default: break;
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
    default:
        // AnySeverity is a filter sentinel and InvalidSeverity cannot arrive on
        // a real message; log them at debug rather than dropping them silently.
        LOG_DEBUG << error.toStdString() << std::endl;
        break;
    }
}

LCADViewer::~LCADViewer()
{
    deletePainters();
    if (_document)
        _document->commitProcessEvent().disconnect<LCADViewer, &LCADViewer::on_commitProcessEvent>(this);
}

void LCADViewer::initializeGL()
{
    makeCurrent();
    QOpenGLContext* glContext = QOpenGLContext::currentContext();
    if (glContext == nullptr) {
        reportGlInitializationFailure(tr("No current OpenGL context is available."));
        return;
    }

    QOpenGLDebugLogger* logger = new QOpenGLDebugLogger(this);
    if (logger->initialize()) {
        connect(logger, &QOpenGLDebugLogger::messageLogged, this, &LCADViewer::messageLogged);
        logger->startLogging();
    }
    else {
        LOG_WARNING << "OpenGL debug logger could not be initialized" << std::endl;
    }


    int width = size().width();
    int height = size().height();

    std::string glewError;
    if (!initialiseGlew(glewError)) {
        reportGlInitializationFailure(QString::fromStdString(glewError));
        return;
    }

    // A recreated context may succeed after an earlier failure.
    _glInitializationFailure.clear();
    _glFailureOverlay->hide();

    deletePainters();
    createPainters(width, height);
    _documentPainter->create_resources();
    if (_docCanvas != nullptr) {
        _docCanvas->autoScale(*_documentPainter);
    }
}

void LCADViewer::reportGlInitializationFailure(const QString& detail)
{
    if (!_glInitializationFailure.isEmpty()) {
        // initializeGL() runs again when the context is recreated; report the
        // failure once instead of repeating the same user-facing error.
        return;
    }

    LOG_ERROR << "OpenGL initialization failed: " << detail.toStdString() << std::endl;
    _glInitializationFailure = glInitializationFailureMessage(detail);
    _hookManager.cancel();
    startSelectPos = QPoint();
    _mouseScrollKeyActive = false;

    // Show the explanation inside the viewer instead of opening a dialog.
    // initializeGL() runs while the widget is being realized, and this path is
    // only reachable when the platform's GL setup is already failing; an
    // in-canvas overlay needs neither a second platform window nor a nested
    // event loop to become visible.
    _glFailureOverlay->setText(_glInitializationFailure);
    _glFailureOverlay->setGeometry(rect());
    _glFailureOverlay->show();
    _glFailureOverlay->raise();
}

void LCADViewer::resizeEvent(QResizeEvent* event)
{
    QOpenGLWidget::resizeEvent(event);
    if (_glFailureOverlay != nullptr) {
        _glFailureOverlay->setGeometry(rect());
    }
}


void LCADViewer::setDocument(std::shared_ptr<lc::storage::Document> document, meta::Block_CSPtr viewport)
{
    if (_document)
        _document->commitProcessEvent().disconnect<LCADViewer, &LCADViewer::on_commitProcessEvent>(this);
    if (_docCanvas)
        _docCanvas->selectionChanged().disconnect<LCADViewer, &LCADViewer::_selectionChanged>(this);
    QOpenGLContext* widgetContext = context();
    const bool hasValidContext = widgetContext != nullptr && widgetContext->isValid();
    if (hasValidContext) {
        makeCurrent();
    }
    if (_glInitializationFailure.isEmpty()) {
        deletePainters();
    }
    else if (_docCanvas != nullptr) {
        // Keep old-context painters alive until a working context can release
        // them. The new canvas must not retain their pointers in the meantime.
        _docCanvas->setPainter(nullptr);
    }

    // Use this canvas's painter, not a closure borrowing a future/dead viewer.
    _docCanvas = std::make_shared<lc::viewer::DocumentCanvas>(document, nullptr, viewport);
    _docCanvas->newDeviceSize(size().width(), size().height());
    _autoScalePending = true;

    _document = document;
    _document->commitProcessEvent().connect<LCADViewer, &LCADViewer::on_commitProcessEvent>(this);
    _docCanvas->selectionChanged().connect<LCADViewer, &LCADViewer::_selectionChanged>(this);

    if (_glInitializationFailure.isEmpty() && hasValidContext
        && QOpenGLContext::currentContext() == widgetContext) {
        createPainters(size().width(), size().height());
        _documentPainter->create_resources();
        doneCurrent();
    }
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
    if (!_glInitializationFailure.isEmpty() || _docCanvas == nullptr || _documentPainter == nullptr) {
        return;
    }

    _docCanvas->newDeviceSize(width,height);
    _documentPainter->new_device_size(width,height);
    updateBackground();
    updateDocument();
}

void LCADViewer::wheelEvent(QWheelEvent *event) {
    if (!_glInitializationFailure.isEmpty() || _docCanvas == nullptr || _documentPainter == nullptr) {
        event->accept();
        return;
    }

    // Make 10% for 15deg turn, enable slow zoom for mouse with more precision
    // May be use accumulation if very sensitive
    QOpenGLWidget::makeCurrent();
    QPoint numDegrees = event->angleDelta()/8;
    double zoom;

    if(numDegrees.isNull()) return;
    zoom = 1.+(double)numDegrees.y()/150.;

    for(auto pair : imagemaps)
    {
        // pos() is deprecated; position() is QPointF, and zoom() takes
        // integral device coordinates, so round back to a QPoint.
        const QPoint wheelPos = event->position().toPoint();
        _docCanvas->zoom(*pair.first, zoom, true, wheelPos.x(), wheelPos.y());
    }

    updateBackground();
    updateDocument();
    this->update();
}

bool LCADViewer::dragHandler(lc::ui::HookEvent& e) {
    if (!_glInitializationFailure.isEmpty() || _docCanvas == nullptr || _documentPainter == nullptr) {
        return true;
    }

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
    if (!_glInitializationFailure.isEmpty() || _docCanvas == nullptr || _documentPainter == nullptr) {
        return true;
    }

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
    if (!_glInitializationFailure.isEmpty() || _docCanvas == nullptr || _documentPainter == nullptr) {
        return true;
    }

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
    if (!_glInitializationFailure.isEmpty()) {
        event->accept();
        return;
    }

    QWidget::mouseMoveEvent(event);
    _snapManager->setDeviceLocation(event->pos().x(), event->pos().y());
    _hookManager.onMouseEvent("move", event);
    emit mouseMoveEvent();
    update();
}

void LCADViewer::mousePressEvent(QMouseEvent *event)
{
    if (!_glInitializationFailure.isEmpty()) {
        event->accept();
        return;
    }

    QWidget::mousePressEvent(event);
    _hookManager.onMouseEvent("press", event);
    if (event->buttons() != Qt::RightButton) {
        emit mousePressEvent();
    }
}


void LCADViewer::mouseReleaseEvent(QMouseEvent *event) {
    if (!_glInitializationFailure.isEmpty()) {
        event->accept();
        return;
    }

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
    if (!_glInitializationFailure.isEmpty() || _docCanvas == nullptr || _documentPainter == nullptr) {
        return;
    }

    // A document installed while the widget was not yet shown cannot be
    // fitted until it has a real device size. Keep the request pending if the
    // first paint still has no usable dimensions.
    if (_autoScalePending && width() > 0 && height() > 0) {
        _docCanvas->newDeviceSize(static_cast<unsigned int>(width()),
                                  static_cast<unsigned int>(height()));
        _docCanvas->autoScale(*_documentPainter);
        _autoScalePending = false;
    }

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
    if (_docCanvas) _docCanvas->setPainter(nullptr);
    for(auto pair : imagemaps) {
        delete pair.first;
        delete pair.second;
    }
    imagemaps.clear();
    _backgroundPainter = nullptr;
    _documentPainter = nullptr;
    _foregroundPainter = nullptr;
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
    if (!_glInitializationFailure.isEmpty() || _docCanvas == nullptr) {
        event->accept();
        return;
    }

    lc::ui::api::Menu menu("ContextMenu", this);
    ContextMenuManager::GetContextMenuManager(_contextMenuManagerId)->generateMenu(&menu, documentCanvas()->selectedEntities().asVector());
    menu.exec(event->globalPos());
}
