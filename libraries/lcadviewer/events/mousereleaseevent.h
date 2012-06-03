#ifndef MouseReleaseEvent_H
#define MouseReleaseEvent_H

#include <QPointF>
#include <QRectF>
#include <QGraphicsView>
#include <QMouseEvent>
#include "cad/geometry/geocoordinate.h"
#include "cad/vo/entitydistance.h"

/**
  * Event that get's dispatched each time the mouse is moved.
  */
class MouseReleaseEvent  {
    public:
        MouseReleaseEvent() {
            _view = NULL;
            _mouseEvent = NULL;
        }
        MouseReleaseEvent(QGraphicsView* view, const lc::geo::Coordinate& mousePosition, QMouseEvent* mouseEvent, QList<lc::EntityDistance> entities) : _view(view), _mousePosition(mousePosition), _mouseEvent(mouseEvent), _entities(entities) {
        }

        lc::geo::Coordinate  mousePosition() const {
            return _mousePosition;
        }

        QGraphicsView* view() const {
            return _view;
        }
        QMouseEvent* mouseEvent() const {
            return _mouseEvent;
        }
        QList<lc::EntityDistance> entities() const {
            return _entities;
        }

    private:
        QGraphicsView* _view;
        lc::geo::Coordinate _mousePosition;
        QMouseEvent* _mouseEvent;
        QList<lc::EntityDistance> _entities;
};
Q_DECLARE_METATYPE(MouseReleaseEvent)

#endif // MouseReleaseEvent_H
