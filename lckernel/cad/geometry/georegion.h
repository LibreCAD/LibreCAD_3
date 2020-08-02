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

                /**
                 * @brief return entities
                 * 
                 * @return const std::vector<lc::entity::CADEntity_CSPtr>& 
                 */
                const std::vector<lc::entity::CADEntity_CSPtr>& entities() const {return _objList;};
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
                bool isPointInside(const geo::Coordinate&) const;


                /**
                 * @brief Get intersection with line
                 * 
                 * @return std::vector<lc::geo::Coordinate> 
                 */
                std::vector<lc::geo::Coordinate> getLineIntersection(const lc::geo::Vector&) const;

                /**
                 * @brief Bounding box for region
                 * 
                 * @return lc::geo::Area 
                 */
                lc::geo::Area boundingBox() const;
                
                //For entity operation
		Region move(const geo::Coordinate &offset) const;
		Region copy(const geo::Coordinate &offset) const;
		Region rotate(const geo::Coordinate &rotation_center, const double rotation_angle) const;
		Region scale(const geo::Coordinate &scale_center,
                                          const geo::Coordinate &scale_factor) const;
		Region mirror(const geo::Coordinate& axis1,
                    const geo::Coordinate& axis2) const;
                    
               const std::vector<Loop>& loopList() const {return _loopList;}
               
            private:
            	std::vector<Loop> _loopList;
        };
    }
}
