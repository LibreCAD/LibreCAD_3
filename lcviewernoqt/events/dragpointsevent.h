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
            DragPointsEvent(const std::vector<lc::geo::Coordinate>& dragPoints, const unsigned int size) :
                    _dragPoints(dragPoints),
                    _size(size)
            {}

            std::vector<lc::geo::Coordinate> dragPoints() const {
                return _dragPoints;
            }

            unsigned int size() const {
                return _size;
            }

        private:
            std::vector<lc::geo::Coordinate> _dragPoints;
            unsigned int _size;
    };
}
