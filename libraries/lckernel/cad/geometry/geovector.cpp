#include "geovector.h"

using namespace lc;
using namespace geo;

Vector::Vector(const Coordinate& start, const Coordinate& end) : _start(start), _end(end) {
}

const Coordinate& Vector::start() const {
    return _start;
}
const Coordinate& Vector::end() const {
    return _end;
}
