#pragma once

#include <cad/geometry/geocoordinate.h>
#include <vector>
#include "../documentcanvas.h"
#include "../events/dragpointsevent.h"

namespace LCViewer {
    class DragPoints {
        public:
            DragPoints(int size);
            void setPoints(DragPointsEvent const & points);

            void onDraw(DrawEvent const & event) const;

        private:
            std::vector<lc::geo::Coordinate> _points;
            int _size;
    };

    using DragPoints_SPtr = std::shared_ptr<DragPoints>;
}