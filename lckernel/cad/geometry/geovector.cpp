#include "geovector.h"

#include "geoarea.h"

using namespace lc;
using namespace geo;

Vector::Vector(const Coordinate& start, const Coordinate& end) : _start(start), _end(end) {
}

Vector::Vector(const Vector& v) : _start(v._start), _end(v._end)  {
}

const Coordinate& Vector::start() const {
    return _start;
}
const Coordinate& Vector::end() const {
    return _end;
}

Coordinate Vector::nearestPointOnPath(const Coordinate& coord) const {
    Coordinate direction = this->end() - this->start();
    Coordinate vpc = coord - this->start();
    // At DLI: We don't test if length is 0 because we require that each vector/line  this direction.squared(); is always > 0
    vpc = start() + direction * Coordinate(vpc).dot(direction) / direction.squared();

    return vpc;
}

bool Vector::isCoordinateOnPath(const Coordinate& coord) const {
    return (nearestPointOnPath(coord) - coord).magnitude() < 1.0e-4;
}

void Vector::start(const Coordinate& coord) {
    _start = coord;
}
void Vector::end(const Coordinate& coord) {
    _end = coord;
}
