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
                  * @param Coordinate   First coordinate of a area
                  * @param Coordinate   Second coordinate of a area
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
                  * Test of a specific point lies within a area
                  * @param Coordinate   Point to test agains
                  * @return boolean     true of the point is within the area
                  */
                bool inArea(const Coordinate& point) const;

            private:
                const Coordinate _minP;
                const Coordinate _maxP;
        };
    }
}
#endif // GEOAREA_H
