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


    // setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setWindowTitle(tr("LC Viewer"));

    scale(qreal(1), qreal(1));
    centerOn(0.0, 0.0);
    setDragMode(QGraphicsView::RubberBandDrag);

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    this->_altKeyActive = false;
}

void LCADViewer::setAbstractDocument(lc::AbstractDocument* document) {
    _document = document;
}


void LCADViewer::drawBackground(QPainter* painter, const QRectF& rect) {

    for (int i = 0; i < _backgroundItems.size(); ++i) {
        _backgroundItems.at(i)->draw(this, painter, rect);
    }
}
void LCADViewer::drawForeground(QPainter* painter, const QRectF& rect) {

    for (int i = 0; i < _foregroundItems.size(); ++i) {
        _foregroundItems.at(i)->draw(this, painter, rect);
    }
}


/**
  * Handle key pressing and release to add additional states to this view
  *
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
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
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

        // FIXME: see if we can create scalefactors so that the grid will always be perfectly alligned with the view
        qreal scaleFactor = pow((double)2, -event->delta() / 240.0);
        qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();

        // TODO: See if these scale factors are good for us. We have to set a limit else there is a chance
        // that people zoom in way to much, or zoomout way to much
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
void LCADViewer::mouseMoveEvent(QMouseEvent* event) {
    QGraphicsView::mouseMoveEvent(event);
}


/**
  * Add a background render item to the viewer.
  *
  */
void LCADViewer::addBackgroundItem(LCADViewerDrawItemPtr item) {
    this->_backgroundItems.append(item);
}

/**
  * Add a foreground render item to the viewer.
  *
  */
void LCADViewer::addForegroundItem(LCADViewerDrawItemPtr item) {
    this->_foregroundItems.append(item);
}
