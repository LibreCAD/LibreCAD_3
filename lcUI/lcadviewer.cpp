#include "lcadviewer.h"
#include "documentcanvas.h"

#include <map>

#include <QtGui>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTime>

#include <cad/dochelpers/entitycontainer.h>

#include <managers/snapmanager.h>

using namespace LCViewer;

LCADViewer::LCADViewer(QWidget *parent) :
    QWidget(parent), _docCanvas(nullptr), _mouseScrollKeyActive(false), _operationActive(false), _scale(1.0), _zoomMin(0.05), _zoomMax(20.0), _scaleLineWidth(false) {

    setMouseTracking(true);
    this->_altKeyActive = false;
    this->_ctrlKeyActive = false;
	setCursor(Qt::BlankCursor);
}

LCADViewer::~LCADViewer() {
    // Remove the painters here because this object gets deleted before the document
    _docCanvas->removePainters();
    _document->commitProcessEvent().disconnect<LCADViewer, &LCADViewer::on_commitProcessEvent>(this);
}


void LCADViewer::setDocument(std::shared_ptr<lc::Document> document) {
    _docCanvas = std::make_shared<DocumentCanvas>(document);
    _document = document;
    _document->commitProcessEvent().connect<LCADViewer, &LCADViewer::on_commitProcessEvent>(this);

    _docCanvas->createPainterFunctor(
    [this](const unsigned int width, const unsigned int height) {
        QImage *m_image = new QImage(width, height, QImage::Format_ARGB32);
        LcPainter* lcPainter = createCairoImagePainter(m_image->bits(), width, height);
        imagemaps.insert(std::make_pair(lcPainter, m_image));
        return lcPainter;
    });

    _docCanvas->deletePainterFunctor([this]
    (LcPainter * painter) {
        // If you get an exception here and you are destroying this object, you migth need to call _docCanvas->removePainters();
        // in your destructor
        QImage *m_image = imagemaps.at(painter);
        delete painter;
        delete m_image;
        imagemaps.erase(painter);
    });

    _docCanvas->newDeviceSize(size().width(), size().height());

}

void LCADViewer::setSnapManager(std::shared_ptr<SnapManager> snapmanager) {
    _snapManager = snapmanager;
}

void LCADViewer::setDragManager(DragManager_SPtr dragManager) {
    _dragManager = dragManager;
}


void LCADViewer::on_commitProcessEvent(const lc::CommitProcessEvent &) {
    update();
}



/**
  * Handle key pressing and release to add additional states to this view
  *
  */
void LCADViewer::keyPressEvent(QKeyEvent *event) {

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
    }

}

void LCADViewer::keyReleaseEvent(QKeyEvent *event) {
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
    _docCanvas->newDeviceSize(event->size().width(), event->size().height());
}

void LCADViewer::wheelEvent(QWheelEvent *event) {

    if (event->angleDelta().y() > 0) {
        this->_docCanvas->zoom(1.1, true, event->pos().x(), event->pos().y()); //1.2
    } else if (event->angleDelta().y() < 0) {
        this->_docCanvas->zoom(0.9, true, event->pos().x(), event->pos().y()); // 0.83
    }

    this->update();
}

void LCADViewer::setVerticalOffset(int v) {
    int val = v_ - v;
    this->_docCanvas->transY(val * 10);
    v_ = v;
    update();
}

void LCADViewer::setHorizontalOffset(int v) {
    int val = h_ - v;
    this->_docCanvas->transX(val * 20);
    h_ = v;
    update();
}


void LCADViewer::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);

    _snapManager->setDeviceLocation(event->pos().x(), event->pos().y());
    _dragManager->onMouseMove();

    // Selection by area
    if (_altKeyActive || _mouseScrollKeyActive) {
        if (!startSelectPos.isNull()) {
            this->_docCanvas->pan(event->pos().x(), event->pos().y());
        }
    } else {
        if (!startSelectPos.isNull()) {
            bool occopies = startSelectPos.x() < event->pos().x();
            _docCanvas->makeSelectionDevice(
                std::min(startSelectPos.x(), event->pos().x()) , std::min(startSelectPos.y(), event->pos().y()),
                std::abs(startSelectPos.x() - event->pos().x()),
                std::abs(startSelectPos.y() - event->pos().y()), occopies, _ctrlKeyActive);
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

    if (!_ctrlKeyActive) {
        _docCanvas->removeSelection();
    }

    if(_dragManager->entityDragged()) {
        startSelectPos = QPoint();
    }

    switch (event->buttons()) {
        case Qt::MiddleButton: {
            _mouseScrollKeyActive = true;
        }
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

    emit mouseReleaseEvent();

    update();
}

std::shared_ptr<DocumentCanvas> LCADViewer::documentCanvas() const {
    return _docCanvas;
}

void LCADViewer::setOperationActive(bool operationActive) {
    _operationActive = operationActive;

    if(operationActive == false) {
        documentCanvas()->removeSelection();
    }
}

void LCADViewer::paintEvent(QPaintEvent *p) {
    if (p->rect().width() == 0 || p->rect().height() == 0) {
        return;
    }

    QPainter painter(this);
    _docCanvas->render([&](LcPainter & lcPainter) {
        lcPainter.clear(1., 1., 1., 0.0);

    }, [&](LcPainter & lcPainter) {
        QImage *i = imagemaps.at(&lcPainter);
        painter.drawImage(QPoint(0, 0), *i);

    });
    painter.end();
}