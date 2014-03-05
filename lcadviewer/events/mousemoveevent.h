#ifndef MOUSEMOVEEVENT_H
#define MOUSEMOVEEVENT_H

#include <QPointF>
#include <QRectF>
#include <QGraphicsView>
#include "cad/geometry/geocoordinate.h"

/**
  * Event that get's dispatched each time the mouse is moved.
  */
class MouseMoveEvent  {
    public:
        MouseMoveEvent(QGraphicsView* view, const QRectF& rect, const QPointF& mousePosition) : _view(view), _rect(rect), _mousePosition(mousePosition) {
        }

        lc::geo::Coordinate  mousePosition() const {
            return _mousePosition;
        }

        QGraphicsView* view() const {
            return _view;
        }
        QRectF rect() const {
            return _rect;
        }

    private:
        QGraphicsView* _view;
        QRectF _rect;
        lc::geo::Coordinate _mousePosition;
};

#endif // MOUSEMOVEEVENT_H
