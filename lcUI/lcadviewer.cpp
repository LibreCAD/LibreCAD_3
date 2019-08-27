#include "lcadviewer.h"
#include <QtGui>
#include <QVBoxLayout>
#include <iostream>
#include <QOpenGLContext>
#include <QSurfaceFormat>

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
    QOpenGLContext *CC= QOpenGLContext::currentContext();
    
    setMouseTracking(true);
    this->_altKeyActive = false;
    this->_ctrlKeyActive = false;
    setCursor(Qt::BlankCursor);

   QSurfaceFormat format;
    format.setMajorVersion(4);
    format.setMinorVersion(6);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOption(QSurfaceFormat::DebugContext);
    setFormat(format);
}

void LCADViewer::messageLogged(const QOpenGLDebugMessage &msg)
{
    qCritical() << "OpenGL log:\n";
  QString error;

  // Format based on severity
  switch (msg.severity())
  {
  case QOpenGLDebugMessage::NotificationSeverity:
    error += "--";
    break;
  case QOpenGLDebugMessage::HighSeverity:
    error += "!!";
    break;
  case QOpenGLDebugMessage::MediumSeverity:
    error += "!~";
    break;
  case QOpenGLDebugMessage::LowSeverity:
    error += "~~";
    break;
  }

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
  qCritical() << qPrintable(error) << "\n" << qPrintable(msg.message()) << "\n";
}

LCADViewer::~LCADViewer() 
{
    deletePainters();
    _document->commitProcessEvent().disconnect<LCADViewer, &LCADViewer::on_commitProcessEvent>(this);
}

void LCADViewer::initializeGL()
{
  QOpenGLWidget::makeCurrent();
  QOpenGLContext *CC= QOpenGLContext::currentContext();
  
  /*QOpenGLDebugLogger *logger = new QOpenGLDebugLogger(this);
    logger->initialize();

    connect(logger, &QOpenGLDebugLogger::messageLogged, this, &LCADViewer::messageLogged);
    logger->startLogging();
    */

  int width = size().width();
  int height = size().height();

  if(CC!=0)
  {
    GLenum err = glewInit();
          
    if (err != GLEW_OK)
      exit(1); // or handle the error in a nicer way
    if (!GLEW_VERSION_2_1)  // check that the machine supports the 2.1 API.
      exit(1); // or handle the error in a nicer way

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
 
    if(_docCanvas != nullptr)
   _docCanvas->setPainter(_documentPainter);  //passing pointer to painter to doc canvas
}

void LCADViewer::setSnapManager(std::shared_ptr<lc::viewer::manager::SnapManager> snapmanager) {
    _snapManager = std::move(snapmanager);
}

void LCADViewer::setDragManager(lc::viewer::manager::DragManager_SPtr dragManager) {
    _dragManager = std::move(dragManager);
}

void LCADViewer::updateHelper(){
    update();
}

void LCADViewer::on_commitProcessEvent(const lc::event::CommitProcessEvent& event) {
    updateDocument();
    update();
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
  QOpenGLWidget::makeCurrent();
   if (event->angleDelta().y() > 0) 
    {
        for(auto pair : imagemaps) 
        {
            _docCanvas->zoom(*pair.first, 1.1, true, event->pos().x(), event->pos().y());
        } 
    } 
    else if (event->angleDelta().y() < 0)
    {
        for(auto pair : imagemaps) 
        {
            _docCanvas->zoom(*pair.first, 0.9, true, event->pos().x(), event->pos().y());         
        } 
    }
 
    updateBackground();
    updateDocument();
    this->update();
}

void LCADViewer::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);

 _snapManager->setDeviceLocation(event->pos().x(), event->pos().y());
    _dragManager->onMouseMove();

    // Selection by area
    if (_altKeyActive || _mouseScrollKeyActive) {
        if (!startSelectPos.isNull()) {
            auto translateX = event->pos().x()-startSelectPos.x();
            auto translateY = event->pos().y()-startSelectPos.y();
            startSelectPos = event->pos();
            for(auto pair : imagemaps) {
                _docCanvas->pan(*pair.first, translateX, translateY);
            }  
            
            updateBackground();
            updateDocument();
            update();
        }
    } else {
        if (!startSelectPos.isNull()) {
            bool occopies = startSelectPos.x() < event->pos().x();
            _docCanvas->makeSelectionDevice(
                *_documentPainter,
                std::min(startSelectPos.x(), event->pos().x()) , std::min(startSelectPos.y(), event->pos().y()),
                std::abs(startSelectPos.x() - event->pos().x()),
                std::abs(startSelectPos.y() - event->pos().y()), occopies);

            updateDocument();
        }
    }
    emit mouseMoveEvent();
    update();
}

void LCADViewer::mousePressEvent(QMouseEvent *event) 
{
    QWidget::mousePressEvent(event);

    startSelectPos = event->pos();
   if(!_operationActive) {
        _dragManager->onMousePress();
    }

    if(_dragManager->entityDragged()) {
        startSelectPos = QPoint();
    }

    switch (event->buttons()) {
        case Qt::MiddleButton: {
            _mouseScrollKeyActive = true;
        }
            break;

        default:
            break;
    }
  
    emit mousePressEvent();
}


void LCADViewer::mouseReleaseEvent(QMouseEvent *event) {
    startSelectPos = QPoint();

    _dragManager->onMouseRelease();

    _docCanvas->closeSelection();

    std::vector<lc::EntityDistance> emptyList;
    //  MouseReleaseEvent e(this, _lastMousePosition, event, emptyList);
    //  emit mouseReleaseEvent(e);
    switch (event->button()) {
        case Qt::MiddleButton: {
            _mouseScrollKeyActive = false;
        } break;
        default: {    
        } break;
    }

    _docCanvas->removeSelectionArea();
    updateDocument();

    emit mouseReleaseEvent();

    update();
}

std::shared_ptr<lc::viewer::DocumentCanvas> LCADViewer::documentCanvas() const {
    return _docCanvas;
}

void LCADViewer::setOperationActive(bool operationActive) {
    _operationActive = operationActive;

    if(!operationActive) {
        documentCanvas()->removeSelection();
    }
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
