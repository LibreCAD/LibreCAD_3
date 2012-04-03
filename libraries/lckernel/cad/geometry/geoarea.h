#ifndef GEOAREA_H
#define GEOAREA_H

#include "cad/const.h"

#include "geocoordinate.h"

namespace lc {
    namespace geo {
        class Area {
            public:
                Area(const geo::Coordinate& coordA, const geo::Coordinate& coordB) ;

                geo::Coordinate coordA() const;
                geo::Coordinate coordB() const;

                Area(const Area& area);
                Area& operator = (const Area& area);
            public:

            private:
                geo::Coordinate _coordA;
                geo::Coordinate _coordB;
        };
    }
}
#endif // GEOAREA_H
