#ifndef GEOCOORDINATE_H
#define GEOCOORDINATE_H

#include "cad/const.h"

namespace lc {
    namespace geo {
        /**
          * Class that represent a coordinate in space
          */
        class Coordinate {
            public:
                Coordinate(double x, double y, double z);
                Coordinate(double x, double y);

                double x() const;
                double y() const;
                double z() const;

            public:

                /**
                  * Calculate teh distance between this coordinate and a other coordinate
                  */

                double distanceTo(const Coordinate& coordinate) const;

                /**
                  * Add two points to each other and return a new coordinate
                  */
                Coordinate operator + (const Coordinate& coordinate) const;

                /**
                  * Substract two points from each other and return a new coordinate
                  */
                Coordinate operator - (const Coordinate& coordinate) const;

                /**
                  * Magnitude or lenth of the point relative to zero
                  */
                double magnitude() const;

            private:
                double _x;
                double _y;
                double _z;
        };
    }
}
#endif // GEOCOORDINATE_H
