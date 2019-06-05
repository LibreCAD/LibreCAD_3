#include "lcadviewer.h"
#include <QtGui>
#include <QVBoxLayout>
#include <iostream>
#include <QtDebug>
#include <QOpenGLContext>

using namespace lc;
using namespace lc::ui;

QOpenGLContext *m_context;

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


    qDebug("==========================lcadviewer constructor========================");
    QOpenGLContext *CC= QOpenGLContext::currentContext();
    qDebug("Current context=%u",CC);
    qDebug("This widget object=%u",this);
    qDebug("This widget parent=%u",parent);
    qDebug("=====================================================================");
   
    setMouseTracking(true);
    this->_altKeyActive = false;
    this->_ctrlKeyActive = false;
    setCursor(Qt::BlankCursor);

}

LCADViewer::~LCADViewer() {
    deletePainters();

    _document->commitProcessEvent().disconnect<LCADViewer, &LCADViewer::on_commitProcessEvent>(this);
}

void LCADViewer::initializeGL()
{
  QOpenGLWidget::makeCurrent();
  qDebug("#########################init_GL###########################");
  QOpenGLContext *CC= QOpenGLContext::currentContext();
  qDebug("Generated context=%u",CC);
  qDebug("This widget object=%u",this);
  qDebug("###########################################################");

  if(CC!=0)
  {
     GLenum err = glewInit();
          
          qDebug("GLEW---------->>>>%d",err);
         if (err != GLEW_OK)
          exit(1); // or handle the error in a nicer way
          if (!GLEW_VERSION_2_1)  // check that the machine supports the 2.1 API.
         exit(1); // or handle the error in a nicer way

  }

  
}


void LCADViewer::setDocument(std::shared_ptr<lc::storage::Document> document, meta::Block_CSPtr viewport) 
{
   qDebug( "lcadviewer---setDocument()--- object= %u",this );
    int width = size().width();
    int height = size().height();

          
    createPainters(width, height);

   

    _docCanvas = std::make_shared<lc::viewer::DocumentCanvas>(document, [this](double* x, double* y) {
        _documentPainter->device_to_user(x, y);
    }, viewport);

    _document = document;
    _document->commitProcessEvent().connect<LCADViewer, &LCADViewer::on_commitProcessEvent>(this);
   
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
/*
void LCADViewer::resizeEvent(QResizeEvent * event) 
{
  
  //  deletePainters();
   // createPainters(event->size().width(), event->size().height());
    _docCanvas->newDeviceSize(event->size().width(), event->size().height());

    updateBackground();
    updateDocument();
}
*/

void LCADViewer::resizeGL(int width, int height)
{
  
   deletePainters();
   createPainters(width, height);
    _docCanvas->newDeviceSize(width,height);

    updateBackground();
    updateDocument();
}

float Tx=0,Ty=0;
void LCADViewer::wheelEvent(QWheelEvent *event) {
 
  QOpenGLWidget::makeCurrent();
 // qDebug("++++++++++++++++++++++++++_MOUSE_WHEEL_++++++++++++++++++++++++++");
 // QOpenGLContext *CC= QOpenGLContext::currentContext();
 // qDebug("Current context=%u",CC);
  //qDebug("This widget object=%u",this);
  //qDebug("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    if (event->angleDelta().y() > 0) 
    {
        for(auto pair : imagemaps) 
        {
            _docCanvas->zoom(*pair.first, 1.1, true, event->pos().x(), event->pos().y());
           
        } 
        Ty+=0.1f;
    } 

    else if (event->angleDelta().y() < 0)
    {
        for(auto pair : imagemaps) 
        {
            _docCanvas->zoom(*pair.first, 0.9, true, event->pos().x(), event->pos().y()); 
                 
        } 
       Ty-=0.1f;
    }
 
    updateBackground();
    updateDocument();

    this->update();
}

void LCADViewer::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);

//=========================checking coordinate system=================
/*    double tX = event->pos().x();
    double tY = event->pos().y();
qDebug("mouse device position=>   x=%f   y=%f",tX,tY);
 _documentPainter->device_to_user(&tX,&tY);
qDebug("device_to_user position=> x=%f   y=%f",tX,tY);
   tX = event->pos().x();
   tY = event->pos().y();
_documentPainter->device_to_user_distance(&tX, &tY);
qDebug("device_to_user_distance=> x=%f   y=%f",tX,tY);
qDebug("-----------------------------------------------");
*/
//====================================================================
    _snapManager->setDeviceLocation(event->pos().x(), event->pos().y());
    _dragManager->onMouseMove();

  

    // Selection by area
    if (_altKeyActive || _mouseScrollKeyActive) {
        if (!startSelectPos.isNull()) {
            //qDebug("lcadviewer pan to px=%f py=%f",event->pos().x(),event->pos().y());
            auto translateX = event->pos().x()-startSelectPos.x();
            auto translateY = event->pos().y()-startSelectPos.y();
            startSelectPos = event->pos();
            //qDebug("lcadviewer PAN TX=%f TY=%f",translateX,translateY);
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
    QOpenGLWidget::makeCurrent();
 // qDebug("0000000000000000000_MOUSE_PRESS_0000000000000000000000000");
  //QOpenGLContext *CC= QOpenGLContext::currentContext();
  //qDebug("Current context=%u",CC);
  //qDebug("This widget object=%u",this);
  //qDebug("000000000000000000000000000000000000000000000000000000000");
    QWidget::mousePressEvent(event);

    startSelectPos = event->pos();
    qDebug("Mouse Button clicked at Sx=%f Sy=%f",event->pos().x(),event->pos().y());
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

/*
void LCADViewer::paintEvent(QPaintEvent *p) 
{

    if (p->rect().width() == 0 || p->rect().height() == 0) 
    {
        return;
    }

   // _docCanvas->render(*_documentPainter, lc::viewer::VIEWER_DOCUMENT);
    _documentPainter->SAMPLE_OPENGL(Ty);

}
*/

void LCADViewer::paintGL()
{

   // if (p->rect().width() == 0 || p->rect().height() == 0) 
  //  {
   //     return;
   // }

    _docCanvas->render(*_documentPainter, lc::viewer::VIEWER_BACKGROUND);
    _docCanvas->render(*_documentPainter, lc::viewer::VIEWER_DOCUMENT);
    _docCanvas->render(*_documentPainter, lc::viewer::VIEWER_FOREGROUND);
   

}

void LCADViewer::createPainters(unsigned int width, unsigned int height) {
    QImage *m_image;

  //  m_image = new QImage(width, height, QImage::Format_ARGB32);
  //  _backgroundPainter = lc::viewer::createOpenGLPainter(m_image->bits(), width, height);
  //  imagemaps.insert(std::make_pair(_backgroundPainter, m_image));

    m_image = new QImage(width, height, QImage::Format_ARGB32);
    _documentPainter = lc::viewer::createOpenGLPainter(m_image->bits(), width, height);
    imagemaps.insert(std::make_pair(_documentPainter, m_image));

 //   m_image = new QImage(width, height, QImage::Format_ARGB32);
 //   _foregroundPainter = lc::viewer::createOpenGLPainter(m_image->bits(), width, height);
 //   imagemaps.insert(std::make_pair(_foregroundPainter, m_image));
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
   // qDebug( "~update_background" );
   // _backgroundPainter->clear(1.0, 1.0, 1.0, 0.0);
   // _docCanvas->render(*_backgroundPainter, lc::viewer::VIEWER_BACKGROUND);
     
}

void LCADViewer::updateDocument() 
{
    // qDebug( "~update_document" );
   // _documentPainter->clear(1.0, 1.0, 1.0, 0.0);
   // _docCanvas->render(*_documentPainter, lc::viewer::VIEWER_DOCUMENT);
     
}

const std::shared_ptr<lc::viewer::DocumentCanvas>& LCADViewer::docCanvas() const {
    return _docCanvas;
}
