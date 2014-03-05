#ifndef GEOARC_H
#define GEOARC_H

#include "cad/const.h"
#include "geocoordinate.h"

namespace lc {
    namespace geo {
        class Arc  {
            public:
                Arc(const Coordinate& center, double radius, double startAngle, double endAngle);

                const Coordinate& center() const;
                double radius() const;
                double startAngle() const;
                double endAngle() const;

                Coordinate nearestPointOnPath(const Coordinate& coord) const;
                bool isCoordinateOnPath(const Coordinate& coord) const;

            private:
                const Coordinate _center;
                double _radius;
                double _startAngle;
                double _endAngle;
        };
    }
}
#endif // GEOARC_H
