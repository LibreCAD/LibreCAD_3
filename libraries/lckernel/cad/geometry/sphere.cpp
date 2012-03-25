#include "Sphere.h"

using namespace lc;
using namespace geo;

Sphere::Sphere(const geo::Coordinate& center, double radius) {
    _center = center;
    _radius = radius;
}

Sphere::Sphere(const Sphere& Sphere) {
    _center = Sphere._center;
    _radius = Sphere._radius;
}

Sphere& Sphere::operator= (const Sphere& Sphere) {
    if (this != &Sphere) {
        _center = Sphere._center;
        _radius = Sphere._radius;
    }

    return *this;
}

Coordinate Sphere::center() const {
    return _center;
}
double Sphere::radius() const {
    return _radius;
}
