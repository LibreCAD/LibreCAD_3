#ifndef VECTOR_H
#define VECTOR_H

#include "coordinate.h"

namespace lc {
    namespace geo {
        class Vector {
            public:
                Vector() {};
                Vector(const geo::Coordinate& start, const geo::Coordinate& end);

                geo::Coordinate start() const;
                geo::Coordinate end() const;

                Vector(const Vector& vector);
                Vector& operator = (const Vector& vector);

            private:
                geo::Coordinate _start;
                geo::Coordinate _end;
        };

    }
}
#endif // VECTOR_H
