#pragma once
// I think this is geo then primitive but need primitive to work with
#include "cad/const.h"
#include "cad/base/cadentity.h"
#include <vector>
#include "cad/math/intersect.h"
#include "cad/base/visitor.h"
#include "cad/geometry/geoarea.h"

// The diffrent primitive type it supports
#include "cad/primitive/lwpolyline.h"
#include "cad/primitive/line.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/ellipse.h"
#include "cad/primitive/spline.h"

namespace lc {
    namespace geo {
        /**
         * @brief Represents single loop
         * 
         */
        class Loop{
            public:
                Loop(std::vector<lc::entity::CADEntity_CSPtr>);

                /**
                 * @brief Bounding box for area
                 * 
                 * @return lc::geo::Area 
                 */
                const lc::geo::Area boundingBox() const {return _boundingBox;};
            private:
                std::vector<lc::entity::CADEntity_CSPtr> _objList;
                lc::geo::Area _boundingBox;
        };

        /**
         * @brief Represents a closed region
         * made up of multiple loops
         */
        class Region{
            public:
                /**
                 * @brief Construct a empty Region
                 * 
                 */
                Region();
                /**
                 * @brief add a loop to region
                 *
                 */
                void addLoop(Loop);
                /**
                 * @brief Return number of loops in region
                 * 
                 * @return unsigned int 
                 */
                unsigned int numLoops() const;

                /**
                 * @brief Check if point is inside region
                 * 
                 * @return bool
                 */
                bool isPointInside(geo::Coordinate&) const;
                
                /**
                 * @brief Bounding box for region
                 * 
                 * @return lc::geo::Area 
                 */
                lc::geo::Area boundingBox() const;
            private:
                std::vector<Loop> _loopList;
        };
    }
}
