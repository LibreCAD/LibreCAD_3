#pragma once

#include <cad/geometry/geocoordinate.h>
#include <vector>
#include "../documentcanvas.h"
#include "../events/dragpointsevent.h"

namespace lc {
    namespace viewer {
        namespace drawable {
            /**
             * \brief Class which draw boxes around the points that can be dragged.
             */
            class DragPoints {
                public:
                    DragPoints() = default;

                    /**
                     * \brief Set the new list of points. Remove the old ones.
                     * \param points Event containing the drag points.
                     */
                    void setPoints(event::DragPointsEvent const& points);

                    /**
                     * \brief Draw the drag points.
                     */
                    void onDraw(event::DrawEvent const& event) const;

                private:
                    std::vector<lc::geo::Coordinate> _points;
                    int _size;
            };

            DECLARE_SHORT_SHARED_PTR(DragPoints)
        }
    }
}