#ifndef GEOVECTOR_H
#define GEOVECTOR_H

#include "cad/const.h"

#include "geocoordinate.h"

namespace lc {
    namespace geo {
        class Vector {
            public:
                Vector(const Coordinate& start, const Coordinate& end);

                const Coordinate& start() const;
                const Coordinate& end() const;

            private:
                const Coordinate _start;
                const Coordinate _end;
        };

    }
}
#endif // GEOVECTOR_H
