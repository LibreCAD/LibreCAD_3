#include "lcadviewer.h"

#include <QtGui>
#include <QGLWidget>

LCADViewer::LCADViewer(QWidget* parent) :
    QCachedGraphicsView(parent) {

    /******/
    QGLFormat fmt;
    fmt.setSampleBuffers(true);
    setViewport(new QGLWidget(fmt));
    /****/


    // setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    this->setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing);
    setWindowTitle(tr("LC Viewer"));

    scale(qreal(1), qreal(1));
    centerOn(0.0, 0.0);
    setDragMode(QGraphicsView::RubberBandDrag);

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    this->_altKeyActive = false;

    // Track mouse without it beeing pressed down
    setMouseTracking(true);
    parent->setMouseTracking(true);
}

void LCADViewer::setAbstractDocument(lc::AbstractDocument* document) {
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
void LCADViewer::addBackgroundItem(LCViewerDrawItemPtr item) {
    this->_backgroundItems.append(item);
}

/**
  * Add a foreground render item to the viewer.
  *
  */
void LCADViewer::addForegroundItem(LCViewerDrawItemPtr item) {
    this->_foregroundItems.append(item);
}

/**
  * Add cursors to the scene
  *
  */
void LCADViewer::addCursorItem(LCViewerCursorItemPtr item) {
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

    // Render & PaintEvent
    // PaintEvent calls drawForeground, background and items
    /*
    drawBackground(painter, sourceSceneRect);
    drawItems(painter, numItems, itemArray, styleOptionArray);
    drawForeground(painter, sourceSceneRect);
    */
    /*
    _cursor = getPixmapForView(_cursor);
    QPainter p(_cursor);
    _cursor->fill(Qt::transparent);
    p.setMatrix(painter->matrix());
    for (int i = 0; i < _cursorItems.size(); ++i) {
        this->_cursorItems.at(i)->draw(this, &p, rect, _lastMousePosition);
    }*/
}

