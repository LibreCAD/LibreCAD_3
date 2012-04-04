#ifndef GEOCOORDINATE_H
#define GEOCOORDINATE_H

#include "cad/const.h"

namespace lc {
    namespace geo {
        class Coordinate {
            public:
                Coordinate(double x, double y, double z);
                Coordinate(double x, double y);

                double x() const;
                double y() const;
                double z() const;

            private:
                double _x;
                double _y;
                double _z;
        };
    }
}
#endif // GEOCOORDINATE_H
