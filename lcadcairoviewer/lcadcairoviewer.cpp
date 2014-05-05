#include "lcadcairoviewer.h"

#include <QtGui>
#include <QGLWidget>

LCADCairoViewer::LCADCairoViewer(QWidget* parent) :
    QWidget(parent) {

    this->_altKeyActive = false;



}

void LCADCairoViewer::setAbstractDocument(lc::Document* document) {
    _document = document;
}


/**
  * Handle key pressing and release to add additional states to this view
  *
  */
void LCADCairoViewer::keyPressEvent(QKeyEvent* event) {
    QWidget::keyReleaseEvent(event);

    switch (event->key()) {
        case Qt::Key_Shift:
            // When shift key is release we switch back to rubber band
       //     setDragMode(QGraphicsView::ScrollHandDrag);
            break;

        case Qt::Key_Alt:
            //
            _altKeyActive = true;
            break;
    }

}

void LCADCairoViewer::keyReleaseEvent(QKeyEvent* event) {
    QWidget::keyReleaseEvent(event);

    switch (event->key()) {
        case Qt::Key_Shift:
            // When shift key is release we switch back to rubber band
     //       setDragMode(QGraphicsView::RubberBandDrag);
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
void LCADCairoViewer::addBackgroundItem(std::shared_ptr<LCViewerDrawItem> item) {
    this->_backgroundItems.append(item);
}

/**
  * Add a foreground render item to the viewer.
  *
  */
void LCADCairoViewer::addForegroundItem(std::shared_ptr<LCViewerDrawItem> item) {
    this->_foregroundItems.append(item);
}

/**
  * Add cursors to the scene
  *
  */
void LCADCairoViewer::addCursorItem(std::shared_ptr<LCViewerCursorItem>  item) {
    this->_cursorItems.append(item);
}


void LCADCairoViewer::drawBackground(QPainter* painter, const QRectF& rect) {

    for (int i = 0; i < _backgroundItems.size(); ++i) {
//        _backgroundItems.at(i)->draw(this, painter, rect);
    }

}
void LCADCairoViewer::drawForeground(QPainter* painter, const QRectF& rect) {
    for (int i = 0; i < _foregroundItems.size(); ++i) {
//        _foregroundItems.at(i)->draw(this, painter, rect);
    }

    for (int i = 0; i < _cursorItems.size(); ++i) {
//        this->_cursorItems.at(i)->draw(this, painter, rect, lastMousePosition());
    }

    // FIXME: move to cachedgraphicsview class some day
//    DrawEvent event(this, painter, rect, this->lastMousePosition());
//    emit drawEvent(event);
}

