#pragma once

#include <cad/geometry/geocoordinate.h>
#include <vector>
#include "../documentcanvas.h"
#include "../events/dragpointsevent.h"

namespace LCViewer {
    /**
     * \brief Class which draw boxes around the points that can be dragged.
     */
    class DragPoints {
        public:
            DragPoints();
            /**
             * \brief Set the new list of points. Remove the old ones.
             * \param points Event containing the drag points.
             */
            void setPoints(DragPointsEvent const & points);

            /**
             * \brief Draw the drag points.
             */
            void onDraw(DrawEvent const & event) const;

        private:
            std::vector<lc::geo::Coordinate> _points;
            int _size;
    };

    using DragPoints_SPtr = std::shared_ptr<DragPoints>;
}