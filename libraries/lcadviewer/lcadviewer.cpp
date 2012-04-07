#include "lcadviewer.h"

#include <QtGui>
#include <QGLWidget>

LCADViewer::LCADViewer(QWidget* parent) :
    QGraphicsView(parent) {

    /******/
    QGLFormat fmt;
    fmt.setSampleBuffers(true);
    setViewport(new QGLWidget(fmt));
    /****/


    QGraphicsScene* scene = new QGraphicsScene(this);

    setScene(scene);

    scene->setItemIndexMethod(QGraphicsScene::BspTreeIndex);
    // setCacheMode(CacheBackground);
    // SetCenter(QPointF(0.0, 0.0));
    setViewportUpdateMode(BoundingRectViewportUpdate);
    // setRenderHint(QPainter::Antialiasing);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setWindowTitle(tr("LC Viewer"));
    //setDragMode(QGraphicsView::RubberBandDrag);

    scale(qreal(1), qreal(1));
    // setSceneRect(-500, -500, 500, 500); // DOn't set this, for some reason it goes 'weird'
    // SetCenter(QPointF(0.0, 0.0));

    // Set default drag mode
    setDragMode(QGraphicsView::RubberBandDrag);

    //setTransformationAnchor(AnchorUnderMouse);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    setResizeAnchor(QGraphicsView::AnchorUnderMouse); // anchir under the cursor
    this->_altKeyActive = false;
}

void LCADViewer::setAbstractDocument(lc::AbstractDocument* document) {
    _document = document;
}


void LCADViewer::drawBackground(QPainter* painter, const QRectF& rect) {

    /**
      * Draw a linear gradient backgound
      *
      */
    QRectF sceneRect = rect.normalized();
    QLinearGradient gradient(
        (sceneRect.right() - sceneRect.left()) / 2.0, sceneRect.top(),
        (sceneRect.right() - sceneRect.left()) / 2.0, sceneRect.bottom());
    gradient.setColorAt(0, QColor(0x07, 0x25, 0x11));
    gradient.setColorAt(1, QColor(0x06, 0x15, 0x06));
    painter->fillRect(rect.intersect(sceneRect), gradient);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(sceneRect);
}

/*
void LCADViewer::mousePressEvent(QMouseEvent* event) {
    QGraphicsView::mousePressEvent(event);
}
*/
void LCADViewer::keyPressEvent(QKeyEvent* event) {
    QGraphicsView::keyReleaseEvent(event);

    switch (event->key()) {
        case Qt::Key_Shift:
            // When shift key is release we switch back to rubber band
            setDragMode(QGraphicsView::ScrollHandDrag);
            break;

        case Qt::Key_Alt:
            //
            _altKeyActive = true;
            break;
    }

}

void LCADViewer::keyReleaseEvent(QKeyEvent* event) {
    QGraphicsView::keyReleaseEvent(event);

    switch (event->key()) {
        case Qt::Key_Shift:
            // When shift key is release we switch back to rubber band
            setDragMode(QGraphicsView::RubberBandDrag);
            break;

        case Qt::Key_Alt:
            //
            _altKeyActive = false;
            break;
    }
}

/**
  * Need to update the center so there is no jolt in the
  * interaction after resizing the widget.
  */
void LCADViewer::resizeEvent(QResizeEvent* event) {
    setResizeAnchor(QGraphicsView::AnchorViewCenter); // anchir under the cursor
    QGraphicsView::resizeEvent(event);
    return;
}


/**
  * Zoom the view in and out.
  */
void LCADViewer::wheelEvent(QWheelEvent* event) {

    // FIXME: Need to have configurable KeyModifier
    if (event->modifiers().testFlag(Qt::AltModifier)) {
        QWheelEvent* e = event;
        qreal scaleFactor = pow((double)2, -event->delta() / 240.0);
        qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();

        if (factor < 0.01 || factor > 100) {
            return;
        }

        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        QMatrix mat = matrix();
        mat.scale(scaleFactor, scaleFactor);
        setMatrix(mat);
        event->accept();

    } else {
        QGraphicsView::wheelEvent(event);
    }
}

/**
*Handles the mouse move event
*/
/**
*Handles the mouse move event
*/
void LCADViewer::mouseMoveEvent(QMouseEvent* event) {
    QGraphicsView::mouseMoveEvent(event);
}
