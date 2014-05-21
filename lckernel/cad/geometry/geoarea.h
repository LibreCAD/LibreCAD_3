#ifndef GEOAREA_H
#define GEOAREA_H

#include "cad/const.h"

#include "geocoordinate.h"

namespace lc {
    namespace geo {
        /**
          * Class that describes a area or window.
          */
        class Area {
            public:
                /**
                  * Create a new Area. The coordinates coordA and coordB will be ordered so that minP wil always be < maxP
                  * The corordinates are not allowed to describe a volume
                  *
                  * @param CoordA First coordinate of a area
                  * @param CoordB Second coordinate of a area
                  */
                Area(const Coordinate& coordA, const Coordinate& coordB) ;

                /**
                  * Return the smalles corner (closest to (0,0,0) )
                  */
                const Coordinate& minP() const;

                /**
                  * Return the heigest corner
                  */
                const Coordinate& maxP() const;

                /**
                 * @brief width
                 * Returns the wid th of this area
                 * @return
                 */
                double width() const;
                /**
                 * @brief height
                 * Returns the height f this area
                 * @return
                 */
                double height() const;

                /**
                  * Test of a specific point lies within a area
                  * @param point Point to test agains
                  * @return boolean true of the point is within the area
                  */
                bool inArea(const Coordinate& point) const;

                /**
                 * @brief merge
                 * two area's and expend if required to largest area
                 * @param other
                 * @return
                 */
                Area merge(const Area & other) const;
            private:
                const Coordinate _minP;
                const Coordinate _maxP;
        };
    }
}
#endif // GEOAREA_H
