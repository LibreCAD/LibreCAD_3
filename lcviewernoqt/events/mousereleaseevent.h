#pragma once

#include "cad/geometry/geocoordinate.h"
#include "cad/vo/entitydistance.h"

/**
  * Event that gets dispatched each time the mouse is moved.
  */

namespace LCViewer {
class LcPainter;
class MouseReleaseEvent  {
    public:
        MouseReleaseEvent() {
        }
        MouseReleaseEvent(const lc::geo::Coordinate& mousePosition, std::vector<lc::EntityDistance> entities) : _mousePosition(mousePosition), _entities(entities) {
        }

        lc::geo::Coordinate  mousePosition() const {
            return _mousePosition;
        }

        std::vector<lc::EntityDistance> entities() const {
            return _entities;
        }

    private:
        lc::geo::Coordinate _mousePosition;
        std::vector<lc::EntityDistance> _entities;
};
}
