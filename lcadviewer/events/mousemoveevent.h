#ifndef MOUSEMOVEEVENT_H
#define MOUSEMOVEEVENT_H

#include <QPointF>
#include <QRectF>
#include "cad/geometry/geocoordinate.h"
#include "lcpainter.h"


/**
  * Event that get's dispatched each time the mouse is moved.
  */
class MouseMoveEvent  {
    public:
        MouseMoveEvent(LcPainter* painter, const QPointF& mousePosition) : _painter(painter) {
            _mousePosition = lc::geo::Coordinate(mousePosition.x(), mousePosition.y());
        }

        lc::geo::Coordinate  mousePosition() const {
            return _mousePosition;
        }

        LcPainter* painter() const {
            return _painter;
        }

    private:
        LcPainter* _painter;
        lc::geo::Coordinate _mousePosition;
};

#endif // MOUSEMOVEEVENT_H
