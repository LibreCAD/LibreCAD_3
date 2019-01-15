#include "lcadviewer.h"
#include <QtGui>
#include <QVBoxLayout>

using namespace lc;
using namespace lc::ui;

LCADViewer::LCADViewer(QWidget *parent) :
    QWidget(parent),
    _docCanvas(nullptr),
    _mouseScrollKeyActive(false),
    _operationActive(false),
    _scale(1.0),
    _zoomMin(0.05),
    _zoomMax(20.0),
    _scaleLineWidth(false),
    _backgroundPainter(nullptr),
    _documentPainter(nullptr),
    _foregroundPainter(nullptr) {

    setMouseTracking(true);
    this->_altKeyActive = false;
    this->_ctrlKeyActive = false;
	setCursor(Qt::BlankCursor);
}

LCADViewer::~LCADViewer() {
    deletePainters();

    _document->commitProcessEvent().disconnect<LCADViewer, &LCADViewer::on_commitProcessEvent>(this);
}


void LCADViewer::setDocument(std::shared_ptr<lc::storage::Document> document, meta::Block_CSPtr viewport) {
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

void LCADViewer::resizeEvent(QResizeEvent * event) {
    deletePainters();
    createPainters(event->size().width(), event->size().height());
    _docCanvas->newDeviceSize(event->size().width(), event->size().height());

    updateBackground();
    updateDocument();
}

void LCADViewer::wheelEvent(QWheelEvent *event) {

    if (event->angleDelta().y() > 0) {
        for(auto pair : imagemaps) {
            _docCanvas->zoom(*pair.first, 1.1, true, event->pos().x(), event->pos().y());
        }
    } else if (event->angleDelta().y() < 0) {
        for(auto pair : imagemaps) {
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

void LCADViewer::mousePressEvent(QMouseEvent *event) {
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

void LCADViewer::paintEvent(QPaintEvent *p) {
    if (p->rect().width() == 0 || p->rect().height() == 0) {
        return;
    }

    QPainter painter(this);

    _foregroundPainter->clear(1.0, 1.0, 1.0, 0.0);
    _docCanvas->render(*_foregroundPainter, lc::viewer::VIEWER_FOREGROUND);

    painter.drawImage(QPoint(0, 0), *imagemaps.at(_backgroundPainter));
    painter.drawImage(QPoint(0, 0), *imagemaps.at(_documentPainter));
    painter.drawImage(QPoint(0, 0), *imagemaps.at(_foregroundPainter));

    painter.end();
}

void LCADViewer::createPainters(unsigned int width, unsigned int height) {
    QImage *m_image;

    m_image = new QImage(width, height, QImage::Format_ARGB32);
    _backgroundPainter = lc::viewer::createCairoImagePainter(m_image->bits(), width, height);
    imagemaps.insert(std::make_pair(_backgroundPainter, m_image));

    m_image = new QImage(width, height, QImage::Format_ARGB32);
    _documentPainter = lc::viewer::createCairoImagePainter(m_image->bits(), width, height);
    imagemaps.insert(std::make_pair(_documentPainter, m_image));

    m_image = new QImage(width, height, QImage::Format_ARGB32);
    _foregroundPainter = lc::viewer::createCairoImagePainter(m_image->bits(), width, height);
    imagemaps.insert(std::make_pair(_foregroundPainter, m_image));
}

void LCADViewer::deletePainters() {
    for(auto pair : imagemaps) {
        delete pair.first;
        delete pair.second;
    }

    imagemaps.clear();
}

void LCADViewer::updateBackground() {
    _backgroundPainter->clear(1.0, 1.0, 1.0, 0.0);
    _docCanvas->render(*_backgroundPainter, lc::viewer::VIEWER_BACKGROUND);
}

void LCADViewer::updateDocument() {
    _documentPainter->clear(1.0, 1.0, 1.0, 0.0);
    _docCanvas->render(*_documentPainter, lc::viewer::VIEWER_DOCUMENT);
}

const std::shared_ptr<lc::viewer::DocumentCanvas>& LCADViewer::docCanvas() const {
    return _docCanvas;
}
