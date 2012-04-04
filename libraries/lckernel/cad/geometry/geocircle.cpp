#include "geocircle.h"

using namespace lc;
using namespace geo;


Circle::Circle(const Coordinate& center, double radius) : _center(center) {
    _radius = radius;
}

const Coordinate& Circle::center() const {
    return _center;
}
double Circle::radius() const {
    return _radius;
}
