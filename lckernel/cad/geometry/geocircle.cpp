#include "geocircle.h"

using namespace lc;
using namespace geo;

Circle::Circle(const Coordinate& center, double radius) : Base(), _center(center), _radius(radius) {
    if (radius < 0.0) {
        throw "Invalid radius";
    }

}

const Coordinate Circle::center() const {
    return _center;
}
double Circle::radius() const {
    return _radius;
}


Coordinate Circle::nearestPointOnPath(const Coordinate& coord) const {
    return _center + Coordinate((coord - _center).angle()) * _radius;
}
Coordinate Circle::nearestPointOnEntity(const Coordinate& coord) const {
    return _center + Coordinate((coord - _center).angle()) * _radius;
}

