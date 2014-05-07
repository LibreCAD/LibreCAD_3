#include "lcadcairoviewer.h"

#include <QtGui>
#include <QGLWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTime>
#include <cairo.h>
#include <cairo-gl.h>

#include <cad/dochelpers/entitycontainer.h>
#include <cad/primitive/line.h>
#include <cad/primitive/circle.h>

LCADCairoViewer::LCADCairoViewer(QWidget* parent) :
    QGLWidget(parent), _scale(1.0), _zoom_min(0.1), _zoom_max(10.0), _scaleLineWidth(false) {

    this->_altKeyActive = false;

}

void LCADCairoViewer::setDocument(lc::Document* document) {
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

void LCADCairoViewer::wheelEvent(QWheelEvent* event) {

    if (event->angleDelta().y() > 0) {
        _scale = std::min(_scale * 1.2, _zoom_max);
    } else if (event->angleDelta().y() < 0) {
        _scale = std::max(_scale * 0.83, _zoom_min);
    }

    this->update();
}

static cairo_status_t write_png_stream_to_byte_array(void* chars, const unsigned char* data, unsigned int length) {
    QByteArray* byteArray = (QByteArray*) chars;
    byteArray->append((const char*)data, length);
    return CAIRO_STATUS_SUCCESS;
}

void LCADCairoViewer::paintEvent(QPaintEvent* p) {
    QPainter painter(this);
    QOpenGLContext m_context = new QOpenGLContext;

    cairo_surface_t* surface;
    cairo_t* cr;

//     surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, this->size().width(), this->size().height());


    Display *display=glXGetCurrentDisplay();
    qDebug () << "Diplay : " << display;
    GLXContext context;
    cairo_device_t * device = cairo_glx_device_create(display, context);
    cairo_status_t status = cairo_device_status(device);//return success
    cairo_surface_set_device_offset(surface, 225.0,225.0);



    cr = cairo_create(surface);

    cairo_scale(cr, _scale, _scale);



    QElapsedTimer timer;
    timer.start();

    drawBackground(cr, QRectF(QPointF(0.,0.), this->size()));
    drawDocument(cr, QRectF(QPointF(0.,0.), this->size()));
    drawForeground(cr, QRectF(QPointF(0.,0.), this->size()));

    qDebug() << "draw time total " << (timer.nsecsElapsed() / 1000000000.0);
    QByteArray byteArray;

    // We might want to use cairo_image_surface_get_data  and use a more direct approach instead if using a PNG
    cairo_surface_write_to_png_stream(surface, write_png_stream_to_byte_array, &byteArray);
    cairo_surface_destroy(surface);

    /*
    timer.restart();
    QBuffer buf(&byteArray);
    buf.open(QIODevice::ReadOnly);
    QImage image;

    image.load(&buf, "PNG");
    painter.drawImage(QPoint(0, 0), image);
    qDebug() << "btblk time " << (timer.nsecsElapsed() / 1000000000.0);
    */
}

void LCADCairoViewer::drawDocument(cairo_t* cr, const QRectF& rect) {
    lc::EntityContainer e = _document->entityContainer();

    QList<std::shared_ptr<const lc::CADEntity>> all = e.allEntities().values();

    if (_scaleLineWidth) {
        cairo_set_line_width(cr, 1.0 / _scale );
    } else {
        cairo_set_line_width(cr, _scale );
    }
    cairo_set_source_rgba(cr, 1., 1.,1.,1.);

    foreach( auto item, e.allEntities().values() ) {
        const std::shared_ptr<const lc::Line> line = std::dynamic_pointer_cast<const lc::Line>(item);

        if (line != NULL) {
            cairo_move_to(cr, line->start().x(), line->start().y());
            cairo_line_to(cr, line->end().x(), line->end().y());
        }

        // Add a circle
        const std::shared_ptr<const lc::Circle> circle = std::dynamic_pointer_cast<const lc::Circle>(item);

        if (circle != NULL) {
            cairo_arc (cr, circle->center().x(), circle->center().y() , circle->radius(), 0, 2*M_PI);
        }
        cairo_stroke(cr);
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


void LCADCairoViewer::drawBackground(cairo_t* cr, const QRectF& rect) {

    for (int i = 0; i < _backgroundItems.size(); ++i) {
        _backgroundItems.at(i)->draw(cr, rect);
    }

}
void LCADCairoViewer::drawForeground(cairo_t* cr, const QRectF& rect) {
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

