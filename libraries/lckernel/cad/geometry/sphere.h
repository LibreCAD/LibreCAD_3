#ifndef Sphere_H
#define Sphere_H

#include "coordinate.h"

namespace lc {
    namespace geo {
        class Sphere {
            public:
                Sphere() {
                    _radius = 0;
                }

                Sphere(const Coordinate& center, double radius);
                Coordinate center() const;
                double radius() const;

                Sphere(const Sphere& Sphere);
                Sphere& operator = (const Sphere& Sphere);

            private:
                geo::Coordinate _center;
                double _radius;
        };
    }
}

#endif // Sphere_H
