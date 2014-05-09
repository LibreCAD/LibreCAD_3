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
#include <cairo-quartz.h>
#include <cairo-quartz-image.h>

#include <QtMacExtras/QtMac>

LCADCairoViewer::LCADCairoViewer(QWidget* parent) :
    QWidget(NULL), _scale(1.0), _zoom_min(0.1), _zoom_max(10.0), _scaleLineWidth(true) {

    this->_altKeyActive = false;
//          setAttribute(Qt::WA_NativeWindow);
//          setAttribute(Qt::WA_PaintOnScreen);
//          setAttribute(Qt::WA_TranslucentBackground);
//                 setAttribute(Qt::WA_NoSystemBackground);
//                 setAutoFillBackground(true);
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

/*
void LCADCairoViewer::paintEvent(QPaintEvent* p) {


    CGContextRef dc = QtMac::currentCGContext();

    if (dc == 0) {
        qDebug() << "Invalid CGContextRef";
        return;
    }

    cairo_surface_t* quartzSurface = cairo_quartz_surface_create_for_cg_context((CGContextRef)dc, this->size().width(), this->size().height());

    if (quartzSurface == 0x0) {
        qDebug() << "Invalid surface";
        return;
    }

    cairo_t* quartzContext = cairo_create(quartzSurface);


    // cairo_set_fill_rule (quartzContext, CAIRO_FILL_RULE_EVEN_ODD);
    // cairo_set_operator(quartzContext, CAIRO_OPERATOR_OVER);
    // cairo_set_tolerance( quartzContext, 0.5 );

    QElapsedTimer timer;
    timer.start();

    cairo_scale(quartzContext, 1., 1.);
    drawBackground(quartzContext, QRectF(QPointF(0., 0.), this->size()));

    cairo_translate(quartzContext, 500., 500.);
    cairo_scale(quartzContext, _scale, _scale);
    drawDocument(quartzContext, QRectF(QPointF(0., 0.), this->size()));

    cairo_scale(quartzContext, 1., 1.);
    cairo_translate(quartzContext, 0., 0.);
    drawForeground(quartzContext, QRectF(QPointF(0., 0.), this->size()));

    qDebug() << "draw time total " << (timer.nsecsElapsed() / 1000000000.0);

    cairo_surface_flush(quartzSurface);
//    CGContextFlush(dc);

//    CGImageRef imgRef = CGBitmapContextCreateImage(dc);
 //   QPixmap pm = QtMac::fromCGImageRef(imgRef);

   // painter.drawPixmap(QPoint(0, 0), myPixmap);

    cairo_destroy(quartzContext);
    cairo_surface_destroy(quartzSurface);
}
*/

void LCADCairoViewer::paintEvent(QPaintEvent* p) {
    QPainter painter(this);

    cairo_surface_t* quartzSurface = cairo_quartz_surface_create(CAIRO_FORMAT_ARGB32, this->size().width(), this->size().height());

    cairo_t *quartzContext = cairo_create(quartzSurface);


    // cairo_set_fill_rule (quartzContext, CAIRO_FILL_RULE_EVEN_ODD);
    // cairo_set_operator(quartzContext, CAIRO_OPERATOR_OVER);
    // cairo_set_tolerance( quartzContext, 0.5 );

    QElapsedTimer timer;
    timer.start();

    cairo_scale(quartzContext, 1.,1.);
    drawBackground(quartzContext, QRectF(QPointF(0., 0.), this->size()));

    cairo_translate(quartzContext, 500.,500.);
    cairo_scale(quartzContext, _scale, _scale);
    drawDocument(quartzContext, QRectF(QPointF(0., 0.), this->size()));

    cairo_scale(quartzContext, 1.,1.);
    cairo_translate(quartzContext, 0.,0.);
    drawForeground(quartzContext, QRectF(QPointF(0., 0.), this->size()));

    qDebug() << "draw time total " << (timer.nsecsElapsed() / 1000000000.0);


    QImage m_image(this->size().width(), this->size().height(), QImage::Format_ARGB32);
    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, this->size().width());
    cairo_surface_t *imageSurface = cairo_image_surface_create_for_data(m_image.bits(), CAIRO_FORMAT_ARGB32, this->size().width(), this->size().height(), stride);
    cairo_t *imageContext = cairo_create(imageSurface);

    cairo_set_source_surface(imageContext, quartzSurface, 0.0, 0.0);
    cairo_paint(imageContext);

    painter.drawImage(QPoint(0, 0), m_image);
    cairo_destroy(quartzContext);
    cairo_surface_destroy(quartzSurface);

    cairo_destroy(imageContext);
    cairo_surface_destroy(imageSurface);
}


/*
void LCADCairoViewer::paintEvent(QPaintEvent* p) {
    QPainter painter(this);

    cairo_surface_t* surface;
    cairo_t* cr;
    QImage m_image(this->size().width(), this->size().height(), QImage::Format_ARGB32);
    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, this->size().width());
    surface = cairo_image_surface_create_for_data(m_image.bits(), CAIRO_FORMAT_ARGB32, this->size().width(), this->size().height(), stride);
    cr = cairo_create(surface);
    cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    cairo_set_tolerance( cr, 0.5 );

    QElapsedTimer timer;
    timer.start();

    cairo_scale(cr, 1.,1.);
    drawBackground(cr, QRectF(QPointF(0., 0.), this->size()));

    cairo_translate(cr, 500.,500.);
    cairo_scale(cr, _scale, _scale);
    drawDocument(cr, QRectF(QPointF(0., 0.), this->size()));

    cairo_scale(cr, 1.,1.);
    cairo_translate(cr, 0.,0.);
    drawForeground(cr, QRectF(QPointF(0., 0.), this->size()));

    qDebug() << "draw time total " << (timer.nsecsElapsed() / 1000000000.0);


    painter.drawImage(QPoint(0, 0), m_image);
    cairo_surface_destroy(surface);

    qDebug() << "btblk time " << (timer.nsecsElapsed() / 1000000000.0);

} */


void LCADCairoViewer::drawDocument(cairo_t* cr, const QRectF& rect) {
    QList<std::shared_ptr<const lc::CADEntity>> all = _document->entityContainer().allEntities().values();

    if (_scaleLineWidth) {
        cairo_set_line_width(cr, 1.0 / _scale);
    } else {
        cairo_set_line_width(cr, _scale);
    }

    cairo_set_source_rgba(cr, 1., 1., 1., 1.);

    foreach(auto item, all) {
        const std::shared_ptr<const lc::Line> line = std::dynamic_pointer_cast<const lc::Line>(item);

        if (line != NULL) {
            cairo_move_to(cr, line->start().x(), line->start().y());
            cairo_line_to(cr, line->end().x(), line->end().y());
        }

        // Add a circle
        const std::shared_ptr<const lc::Circle> circle = std::dynamic_pointer_cast<const lc::Circle>(item);

        if (circle != NULL) {
            cairo_arc(cr, circle->center().x(), circle->center().y() , circle->radius(), 0, 2 * M_PI);
        }

    }
    cairo_stroke(cr);


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

