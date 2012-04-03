#include "geocircle.h"

using namespace lc;
using namespace geo;

Circle::Circle() {
    _radius = 0;
}

Circle::Circle(const Coordinate& center, double radius) {
    _center = center;
    _radius = radius;
}

Circle::Circle(const Circle& Circle) {
    _center = Circle._center;
    _radius = Circle._radius;
}

Circle& Circle::operator= (const Circle& Circle) {
    if (this != &Circle) {
        _center = Circle._center;
        _radius = Circle._radius;
    }

    return *this;
}

Coordinate Circle::center() const {
    return _center;
}
double Circle::radius() const {
    return _radius;
}
