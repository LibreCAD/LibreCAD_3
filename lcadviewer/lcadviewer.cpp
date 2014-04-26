#include "lcadviewer.h"

#include <QtGui>
#include <QGLWidget>

LCADViewer::LCADViewer(QWidget* parent) :
    QCachedGraphicsView(parent) {

    this->_altKeyActive = false;

    /******/
    QGLFormat fmt;
    fmt.setSampleBuffers(true);
    setViewport(new QGLWidget(fmt));
    /****/


    setCacheMode(CacheBackground); // Currently creates artifacts
    // setViewportUpdateMode(BoundingRectViewportUpdate);
    // this->setOptimizationFlag(DontAdjustForAntialiasing, true);
    this->setRenderHint(QPainter::Antialiasing, true);

    this->setOptimizationFlag(QGraphicsView::IndirectPainting, true);
    setWindowTitle(tr("LC Viewer"));

    this->setFrameStyle(0);
    this->setAttribute(Qt::WA_TranslucentBackground);

    //   scale(qreal(1), qreal(1));
    centerOn(0.0, 0.0);
    // setDragMode(QGraphicsView::RubberBandDrag);

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    this->_altKeyActive = false;

    // Track mouse without it beeing pressed down
    setMouseTracking(true);
    parent->setMouseTracking(true);

    this->setDragMode(RubberBandDrag);

    // We might need to change this based on if we move the rubberband 'negative' or positive
    this->setRubberBandSelectionMode(Qt::IntersectsItemBoundingRect);

}

void LCADViewer::setAbstractDocument(lc::Document* document) {
    _document = document;
}


/**
  * Handle key pressing and release to add additional states to this view
  *
  */
void LCADViewer::keyPressEvent(QKeyEvent* event) {
    QCachedGraphicsView::keyReleaseEvent(event);

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
    QCachedGraphicsView::keyReleaseEvent(event);

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
  * Add a background render item to the viewer.
  *
  */
void LCADViewer::addBackgroundItem(shared_ptr<LCViewerDrawItem> item) {
    this->_backgroundItems.append(item);
}

/**
  * Add a foreground render item to the viewer.
  *
  */
void LCADViewer::addForegroundItem(shared_ptr<LCViewerDrawItem> item) {
    this->_foregroundItems.append(item);
}

/**
  * Add cursors to the scene
  *
  */
void LCADViewer::addCursorItem(shared_ptr<LCViewerCursorItem>  item) {
    this->_cursorItems.append(item);
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

    for (int i = 0; i < _cursorItems.size(); ++i) {
        this->_cursorItems.at(i)->draw(this, painter, rect, lastMousePosition());
    }

    // FIXME: move to cachedgraphicsview class some day
    DrawEvent event(this, painter, rect, this->lastMousePosition());
    emit drawEvent(event);
}

