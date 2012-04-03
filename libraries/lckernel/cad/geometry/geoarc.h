#ifndef GEOARC_H
#define GEOARC_H

#include "cad/const.h"

#include "geocoordinate.h"

namespace lc {
    namespace geo {
        class Arc {
            public:
                Arc();
                Arc(const Coordinate& center, double radius, double startAngle, double endAngle);
                Coordinate center() const;
                double radius() const;
                double startAngle() const;
                double endAngle() const;


            private:
                geo::Coordinate _center;
                double _radius;
                double _startAngle;
                double _endAngle;
        };

    }
}
#endif // GEOARC_H
