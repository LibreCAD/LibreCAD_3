#ifndef GEOELLIPSE_H
#define GEOELLIPSE_H

#include "cad/const.h"

#include "geocoordinate.h"

namespace lc {
    namespace geo {
        class Ellipse {
            public:
                Ellipse(const Coordinate& center, const Coordinate& majorP, double minorRadius, double startAngle, double endAngle);

                const Coordinate& center() const;
                const Coordinate& majorP() const;
                double minorRadius() const;
                double startAngle() const;
                double endAngle() const;

            private:
                const Coordinate _center;
                const Coordinate _majorP;
                double _minorRadius;
                double _startAngle;
                double _endAngle;
        };
    }
}


#endif // GEOELLIPSE_H
