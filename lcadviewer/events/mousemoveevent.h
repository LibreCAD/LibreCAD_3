#ifndef MOUSEMOVEEVENT_H
#define MOUSEMOVEEVENT_H

#include <QPointF>
#include <QRectF>
#include "cad/geometry/geocoordinate.h"

class QWidget;
class LcPainter;

/**
  * Event that get's dispatched each time the mouse is moved.
  */
class MouseMoveEvent  {
    public:
        MouseMoveEvent(const QRectF& rect, const QPointF& mousePosition) : _rect(rect), _mousePosition(mousePosition) {
        }

        lc::geo::Coordinate  mousePosition() const {
            return _mousePosition;
        }

        QRectF rect() const {
            return _rect;
        }

    private:
        QRectF _rect;
        lc::geo::Coordinate _mousePosition;
};

#endif // MOUSEMOVEEVENT_H
