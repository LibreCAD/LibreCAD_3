#ifndef MouseReleaseEvent_H
#define MouseReleaseEvent_H

#include <QPointF>
#include <QRectF>
#include <QGraphicsView>
#include "cad/geometry/geocoordinate.h"

/**
  * Event that get's dispatched each time the mouse is moved.
  */
class MouseReleaseEvent  {
    public:
        MouseReleaseEvent() {
            _view = NULL;
        }
        MouseReleaseEvent(QGraphicsView* view, const lc::geo::Coordinate& mousePosition) : _view(view), _mousePosition(mousePosition) {
        }

        lc::geo::Coordinate  mousePosition() const {
            return _mousePosition;
        }

        QGraphicsView* view() const {
            return _view;
        }

    private:
        QGraphicsView* _view;
        lc::geo::Coordinate _mousePosition;
};
Q_DECLARE_METATYPE(MouseReleaseEvent)

#endif // MouseReleaseEvent_H
