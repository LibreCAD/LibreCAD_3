#ifndef DRAWEVENT_H
#define DRAWEVENT_H

#include <QGraphicsView>
#include <QPainter>
#include <QRectF>
#include <QPointF>

/**
  * Draw event for a object to draw something.
  *
  */
class DrawEvent {
    public:
        DrawEvent(QGraphicsView* view, QPainter* painter, const QRectF& rect, const QPointF& mousePosition) :
            _view(view), _painter(painter), _rect(rect), _mousePosition(mousePosition) {

        }

        QGraphicsView* view() const {
            return _view;
        }
        QPainter* painter() const {
            return _painter;
        }
        QRectF rect() const {
            return _rect;
        }
        QPointF mousePosition() const {
            return _mousePosition;
        }

    private:
        QGraphicsView* _view;
        QPainter* _painter;
        QRectF _rect;
        QPointF _mousePosition;
};

#endif // DRAWEVENT_H
