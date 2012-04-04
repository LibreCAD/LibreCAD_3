#ifndef GEOAREA_H
#define GEOAREA_H

#include "cad/const.h"

#include "geocoordinate.h"

namespace lc {
    namespace geo {
        class Area {
            public:
                Area(const Coordinate& coordA, const Coordinate& coordB) ;

                const Coordinate& coordA() const;
                const Coordinate& coordB() const;

            private:
                const Coordinate _coordA;
                const Coordinate _coordB;
        };
    }
}
#endif // GEOAREA_H
