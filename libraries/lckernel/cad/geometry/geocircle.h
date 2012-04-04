#ifndef GEOCIRCLE_H
#define GEOCIRCLE_H

#include "cad/const.h"

#include "geocoordinate.h"

namespace lc {
    namespace geo {
        class Circle {
            public:
                Circle(const Coordinate& center, double radius);

                const Coordinate& center() const;
                double radius() const;

            private:
                const Coordinate _center;
                double _radius;
        };
    }
}

#endif // GEOCIRCLE_H
