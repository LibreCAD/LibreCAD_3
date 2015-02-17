#include "lcadviewer.h"
#include "documentcanvas.h"
#include "lccairopainter.h"

#include <map>

#include <QtGui>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTime>

#include <cad/dochelpers/entitycontainer.h>
#include <cad/primitive/line.h>
#include <cad/primitive/circle.h>
#include <drawitems/lcvcircle.h>
#include <drawitems/lcvline.h>

#include "nano-signal-slot/nano_signal_slot.hpp"


LCADViewer::LCADViewer(QWidget* parent) :
    QWidget(parent), _scale(1.0), _zoomMin(0.05), _zoomMax(20.0), _scaleLineWidth(false), _docRenderer(nullptr) {

    setMouseTracking(true);
    this->_altKeyActive = false;

}

LCADViewer::~LCADViewer() {
    delete _docRenderer;
    _document->commitProcessEvent().disconnect<LCADViewer, &LCADViewer::on_commitProcessEvent>(this);
}


void LCADViewer::setDocument(lc::Document* document) {
    _docRenderer = new DocumentCanvas(document);
    _document = document;
    _document->commitProcessEvent().connect<LCADViewer, &LCADViewer::on_commitProcessEvent>(this);

    _docRenderer->createPainterFunctor(
    [this](const unsigned int width, const unsigned int height) {
        QImage* m_image = new QImage(width, height, QImage::Format_ARGB32);
        LcCairoPainter* lcPainter = LcCairoPainter::createImagePainter(m_image->bits(), width, height);
        imagemaps.insert(std::make_pair(lcPainter, m_image));
        return lcPainter;
    });

    _docRenderer->deletePainterFunctor([this]
    (LcPainter * painter) {
        QImage* m_image = imagemaps.at(painter);
        delete painter;
        delete m_image;
        imagemaps.erase(painter);
    });

    _docRenderer->newDeviceSize(size().width(), size().height());

}

void LCADViewer::on_commitProcessEvent(const lc::CommitProcessEvent&) {
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
            // When shift key is release we switch back to rubber band
            //     setDragMode(QGraphicsView::ScrollHandDrag);
            break;

        case Qt::Key_Alt:
            //
            _altKeyActive = true;
            break;
    }

}

void LCADViewer::keyReleaseEvent(QKeyEvent* event) {
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

void LCADViewer::wheelEvent(QWheelEvent* event) {

    if (event->angleDelta().y() > 0) {
        this->_docRenderer->zoom(1.1, event->pos().x(), event->pos().y()); //1.2
    } else if (event->angleDelta().y() < 0) {
        this->_docRenderer->zoom(0.9, event->pos().x(), event->pos().y()); // 0.83
    }

    this->update();
}

void LCADViewer::setVerticalOffset(int v) {
    int val = v_ - v;
    this->_docRenderer->transY(val * 10);
    v_ = v;
    update();
}

void LCADViewer::setHorizontalOffset(int v) {
    int val = h_ - v;
    this->_docRenderer->transX(val * 20);
    h_ = v;
    update();
}



void LCADViewer::mouseMoveEvent(QMouseEvent* event) {
    QWidget::mouseMoveEvent(event);

    // Selection by area
    if (_altKeyActive) {
        this->_docRenderer->pan(event->pos().x(), event->pos().y());
    } else {
        if (!startSelectPos.isNull()) {
            bool occopies = startSelectPos.x() < event->pos().x();
            _docRenderer->makeSelectionDevice(
                std::min(startSelectPos.x(), event->pos().x()) , std::min(startSelectPos.y(), event->pos().y()),
                std::abs(startSelectPos.x() - event->pos().x()),
                std::abs(startSelectPos.y() - event->pos().y()), occopies);
        }
    }

    update();
}

void LCADViewer::mousePressEvent(QMouseEvent* event) {
    QWidget::mousePressEvent(event);

    startSelectPos = event->pos();
}

void LCADViewer::mouseReleaseEvent(QMouseEvent* event) {
    std::vector<lc::EntityDistance> emptyList;
    //  MouseReleaseEvent e(this, _lastMousePosition, event, emptyList);
    //  emit mouseReleaseEvent(e);
    startSelectPos = QPoint();
    _docRenderer->removeSelectionArea();
    update();
}




void LCADViewer::paintEvent(QPaintEvent* p) {
    if (p->rect().width() == 0 || p->rect().height() == 0) {
        return;
    }

    _docRenderer->newDeviceSize(size().width(), size().height());

    QPainter painter(this);
    _docRenderer->render([&](LcPainter * lcPainter) {
        lcPainter->clear(1., 1., 1., 0.0);

    }, [&](LcPainter * lcPainter) {
        QImage* i = imagemaps.at(lcPainter);
        painter.drawImage(QPoint(0, 0), *i);

    });
    painter.end();

    /*
        // Emit a mouse move event
        MouseMoveEvent e(painterImage->painter(), this->pos());
        emit mouseMoveEvent(e);

        // Emit a general draw event
        DrawEvent event(painterImage->painter(), this->pos());
        emit drawEvent(event);

        // Draw on this widget
        QPainter painter(this);

        if (_cachedPainters.count(VIEWER_BACKGROUND) > 0) {
            painterImage = _cachedPainters[VIEWER_BACKGROUND];
            painter.drawImage(QPoint(0, 0), *painterImage->image());
        }

        if (_cachedPainters.count(VIEWER_DOCUMENT) > 0) {
            painterImage = _cachedPainters[VIEWER_DOCUMENT];
            painter.drawImage(QPoint(0, 0), *painterImage->image());
        }

        if (_cachedPainters.count(VIEWER_DRAWING) > 0) {
            painterImage = _cachedPainters[VIEWER_DRAWING];
            painter.drawImage(QPoint(0, 0), *painterImage->image());
        }



    */



}




/**
  * Add a background render item to the viewer.
  *
  */
void LCADViewer::addBackgroundItem(std::shared_ptr<LCVDrawItem> item) {
    this->_docRenderer->addBackgroundItem(item);
}

/**
  * Add a foreground render item to the viewer.
  *
  */
void LCADViewer::addForegroundItem(std::shared_ptr<LCVDrawItem> item) {
    this->_docRenderer->addForegroundItem(item);
}




/*
void LCADViewer::paintEvent(QPaintEvent* p) {


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

/*
void LCADViewer::paintEvent(QPaintEvent* p) {
    QPainter painter(this);

    cairo_surface_t* quartzSurface = cairo_quartz_surface_create(CAIRO_FORMAT_ARGB32, this->size().width(), this->size().height());

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


    QImage m_image(this->size().width(), this->size().height(), QImage::Format_ARGB32);
    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, this->size().width());
    cairo_surface_t* imageSurface = cairo_image_surface_create_for_data(m_image.bits(), CAIRO_FORMAT_ARGB32, this->size().width(), this->size().height(), stride);
    cairo_t* imageContext = cairo_create(imageSurface);

    cairo_set_source_surface(imageContext, quartzSurface, 0.0, 0.0);
    cairo_paint(imageContext);

    painter.drawImage(QPoint(0, 0), m_image);
    cairo_destroy(quartzContext);
    cairo_surface_destroy(quartzSurface);

    cairo_destroy(imageContext);
    cairo_surface_destroy(imageSurface);
}
*/
