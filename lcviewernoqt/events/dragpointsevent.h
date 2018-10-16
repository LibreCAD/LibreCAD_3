#pragma once

#include <vector>
#include <cad/geometry/geocoordinate.h>

/**
  * Event that gets dispatched each time we want to have the cursor snapping to this point
  *
  */

namespace lc {
    namespace viewer {
        namespace event {
            /**
             * \brief Event emitted when drag points are near the cursor and need to be shown.
             */
            class DragPointsEvent {
                public:
                    /**
                     * \brief Create event
                     * \param dragPoints List of coordinate of points
                     * \param size Size of box around the point (required to make the selection tolerance match the box size)
                     */
                    DragPointsEvent(const std::vector<lc::geo::Coordinate>& dragPoints, const unsigned int size) :
                            _dragPoints(dragPoints),
                            _size(size) {}

                    /**
                     * \brief Return list of drag points
                     * \return Vector of Coordinate
                     */
                    std::vector<lc::geo::Coordinate> dragPoints() const {
                        return _dragPoints;
                    }

                    /**
                     * \brief Return size of box around the drag point
                     * \return int
                     */
                    unsigned int size() const {
                        return _size;
                    }

                private:
                    std::vector<lc::geo::Coordinate> _dragPoints;
                    unsigned int _size;
            };
        }
    }
}
