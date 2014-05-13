#include "LCADViewer.h"
#include "documentrenderer.h"
#include "lccairopainter.h"

#include <QtGui>
#include <QGLWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTime>

#include <cad/dochelpers/entitycontainer.h>
#include <cad/primitive/line.h>
#include <cad/primitive/circle.h>
#include <drawitems/lcvcircle.h>
#include <drawitems/lcvline.h>

LCADViewer::LCADViewer(QWidget* parent) :
    QWidget(parent), _scale(1.0), _zoom_min(0.1), _zoom_max(10.0), _scaleLineWidth(false), _posX(0.), _posY(0.) {

    setMouseTracking(true);
    this->_altKeyActive = false;
}

void LCADViewer::setDocument(lc::Document* document) {
    _document = document;

    connect(_document, SIGNAL(addEntityEvent(const lc::AddEntityEvent&)),
            this, SLOT(on_addEntityEvent(const lc::AddEntityEvent&)));
    connect(_document, SIGNAL(removeEntityEvent(const lc::RemoveEntityEvent&)),
            this, SLOT(on_removeEntityEvent(const lc::RemoveEntityEvent&)));
}


/**
  * Function to add entities to the graphics scene
  */
void LCADViewer::on_addEntityEvent(const lc::AddEntityEvent& event) {

    // Add a line
    const std::shared_ptr<const lc::Line> line = std::dynamic_pointer_cast<const lc::Line>(event.entity());

    if (line != NULL) {
        _entityContainer.insert(std::make_shared<LCVLine>(line));
        return;
    }

    // Add a circle
    const std::shared_ptr<const lc::Circle> circle = std::dynamic_pointer_cast<const lc::Circle>(event.entity());

    if (circle != NULL) {
        auto newCircle = std::make_shared<LCVCircle>(circle);
        newCircle->selected(true);
        _entityContainer.insert(newCircle);
        return;
    }

    /*

    // Add a Arc
    const std::shared_ptr<const lc::Arc> arc = std::dynamic_pointer_cast<const lc::Arc>(event.entity());

    if (arc != NULL) {
        LCArcItem* foo = new LCArcItem(arc);
        foo->setFlags(QGraphicsItem::ItemIsSelectable);
        scene->addItem(foo);
        _activeGraphicsItems.insert(arc->id(), foo);
        return;
    }

    // Add Ellipse
    const std::shared_ptr<const lc::Ellipse> ellipse = std::dynamic_pointer_cast<const lc::Ellipse>(event.entity());

    if (ellipse != NULL) {
        LCEllipseItem* foo = new LCEllipseItem(ellipse);
        foo->setFlags(QGraphicsItem::ItemIsSelectable);
        scene->addItem(foo);
        _activeGraphicsItems.insert(ellipse->id(), foo);
        return;
    }
    */
}

/**
  * Function to remove a entity from the graphics scene on request
  */
void LCADViewer::on_removeEntityEvent(const lc::RemoveEntityEvent& event) {
    _entityContainer.remove(event.entity());
}



/**
  * Handle key pressing and release to add additional states to this view
  *
  */
void LCADViewer::keyPressEvent(QKeyEvent* event) {
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
        _scale = std::min(_scale * 1.2, _zoom_max);
    } else if (event->angleDelta().y() < 0) {
        _scale = std::max(_scale * 0.83, _zoom_min);
    }

    this->update();
}

void LCADViewer::mouseMoveEvent(QMouseEvent* event) {
    QWidget::mouseMoveEvent(event);

    /*

    _lastMousePosition = this->mapToScene(event->pos());

    QPolygonF p = mapToScene(this->rect());

    // Invalidate screen so we can draw the mouse
    // FIXME: See QCachedGraphicsView performance is quite bad because we need to update all layers
    invalidateScene(QRectF(_lastMousePosition.x() - 2, _lastMousePosition.y() - 2, 4, 4), QGraphicsScene::AllLayers);

    // Emit a mouse move event
    MouseMoveEvent e(this, QRectF(p.at(0), p.at(2)), _lastMousePosition);
    emit mouseMoveEvent(e);

    update();
    */
}

void LCADViewer::mousePressEvent(QMouseEvent* event) {
    QWidget::mousePressEvent(event);

}

void LCADViewer::mouseReleaseEvent(QMouseEvent* event) {
    QList<lc::EntityDistance> emptyList;
    //  MouseReleaseEvent e(this, _lastMousePosition, event, emptyList);
    //  emit mouseReleaseEvent(e);
}

void LCADViewer::paintEvent(QPaintEvent* p) {

    if (p->rect().width() == 0 || p->rect().height() == 0) {
        return;
    }

    // Here we decide somehow what 'painter' to use, currently hardcoded to Cairo
    QImage m_image(p->rect().width(), p->rect().height(), QImage::Format_ARGB32);
    LcPainter* lcPainter = LcCairoPainter::createImagePainter(m_image.bits(), p->rect().width(), p->rect().height());

    // Position document
    double posx = _posX;
    double posy = _posY;
    lcPainter->user_to_device(&posx, &posy);

    double swx = this->size().width() / 2;
    double swy = this->size().height() / 2;
    lcPainter->user_to_device(&swx, &swy);
    lcPainter->translate(posx  + swx, posy + swy);

    // Set scaling
    lcPainter->scale(_scale);

    double pxSize=1.;
    lcPainter->device_to_user(&pxSize, &pxSize);

    // Calculate background size
    posx = -1; posy = -1;
    swx = this->size().width() * 2+2;
    swy = this->size().height() * 2+2;

    lcPainter->device_to_user(&posx, &posy);
    lcPainter->device_to_user(&swx, &swy);
    QRectF visible = QRectF(posx, posy, swx, swy);

    if (_backgroundItems.size()==0) {
        lcPainter->clear(0., 0.1, 0.);
    }
//    lcPainter->clear(0., 0.1, 0.);

    // Background
    drawBackground(lcPainter, visible);

    // Paint Document
    DocumentRenderer render = DocumentRenderer(&_entityContainer, lcPainter);
    render.render(QRectF(0., 0., 0., 0.));

    // Draw on this widget
    QPainter painter(this);
    painter.drawImage(QPoint(0, 0), m_image);
    painter.end();

    delete lcPainter;
}




/**
  * Add a background render item to the viewer.
  *
  */
void LCADViewer::addBackgroundItem(std::shared_ptr<LCVDrawItem> item) {
    this->_backgroundItems.append(item);
}

/**
  * Add a foreground render item to the viewer.
  *
  */
void LCADViewer::addForegroundItem(std::shared_ptr<LCVDrawItem> item) {
    this->_foregroundItems.append(item);
}


void LCADViewer::drawBackground(LcPainter* lcPainter, const QRectF& rect) {

    for (int i = 0; i < _backgroundItems.size(); ++i) {
        _backgroundItems.at(i)->draw(lcPainter, NULL, rect);
    }

}
void LCADViewer::drawForeground(LcPainter* lcPainter, const QRectF& rect) {
    for (int i = 0; i < _foregroundItems.size(); ++i) {
        //        _foregroundItems.at(i)->draw(this, painter, rect);
    }

    //    for (int i = 0; i < _cursorItems.size(); ++i) {
    //        this->_cursorItems.at(i)->draw(this, painter, rect, lastMousePosition());
    //    }

    // FIXME: move to cachedgraphicsview class some day
    //    DrawEvent event(painter, rect, this->lastMousePosition());
    //    emit drawEvent(event);
}

void LCADViewer::setVerticalOffset(int v) {
    _posY = v;
    this->repaint();
}

void LCADViewer::setHorizontalOffset(int v) {
    _posX = v;
    this->repaint();
}

lc::EntityContainer* LCADViewer::entityContainer() {
    return &_entityContainer;
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
