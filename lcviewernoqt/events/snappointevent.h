#pragma once

#include <cad/geometry/geocoordinate.h>

/**
  * Event that get's dispatched each time we want to have the cursor snapping to this point
  *
  */

namespace LCViewer {
class SnapPointEvent {
    public:
        SnapPointEvent(const lc::geo::Coordinate& snapPoint) : _snapPoint(snapPoint), _snapped(true) {
        }



    SnapPointEvent() : _snapped(false) {
        }

        lc::geo::Coordinate snapPoint() const {
            return _snapPoint;
        }

        bool status() const {
            return _snapped;
        }

    private:
        lc::geo::Coordinate _snapPoint;
        bool _snapped;
};
}
