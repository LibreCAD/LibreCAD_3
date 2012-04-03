#ifndef GEOCIRCLE_H
#define GEOCIRCLE_H

#include "cad/const.h"

#include "geocoordinate.h"

namespace lc {
    namespace geo {
        class Circle {
            public:
                Circle();

                Circle(const Coordinate& center, double radius);
                Coordinate center() const;
                double radius() const;

                Circle(const Circle& Circle);
                Circle& operator = (const Circle& Circle);

            private:
                Coordinate _center;
                double _radius;
        };
    }
}

#endif // GEOCIRCLE_H
