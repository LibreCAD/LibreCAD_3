#ifndef AREA_H
#define AREA_H

#include "coordinate.h"

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
#endif // AREA_H
