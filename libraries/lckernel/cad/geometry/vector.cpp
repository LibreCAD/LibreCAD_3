#include "vector.h"

using namespace lc;
using namespace geo;

Vector::Vector(const geo::Coordinate& start, const geo::Coordinate& end) {
    _start = start;
    _end = end;
}

Coordinate Vector::start() const {
    return _start;
}
Coordinate Vector::end() const {
    return _end;
}


Vector::Vector(const Vector& vector) {
    _start = vector._start;
    _end = vector._end;
}

Vector& Vector::operator= (const Vector& vector) {
    if (this != &vector) {
        _start = vector._start;
        _end = vector._end;
    }

    return *this;
}
