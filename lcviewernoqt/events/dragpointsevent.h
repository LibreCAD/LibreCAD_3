#pragma once

#include <vector>
#include <cad/geometry/geocoordinate.h>

/**
  * Event that get's dispatched each time we want to have the cursor snapping to this point
  *
  */

namespace LCViewer {
    class DragPointsEvent {
        public:
            DragPointsEvent(const std::vector<lc::geo::Coordinate>& dragPoints) : _dragPoints(dragPoints)
            {}

            std::vector<lc::geo::Coordinate> dragPoints() const {
                return _dragPoints;
            }

        private:
            std::vector<lc::geo::Coordinate> _dragPoints;
    };
}
