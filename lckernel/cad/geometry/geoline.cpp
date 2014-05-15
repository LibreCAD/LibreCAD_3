#include "geoline.h"

#include "geoarea.h"
#include "cad/math/geointersect.h"

using namespace lc;
using namespace geo;

Line::Line(const Coordinate& start, const Coordinate& end) : _start(start), _end(end) {
}

Line::Line(const Line& v) : _start(v._start), _end(v._end)  {
}

const Coordinate& Line::start() const {
    return _start;
}
const Coordinate& Line::end() const {
    return _end;
}

Coordinate Line::nearestPointOnPath(const Coordinate& coord) const {
    Coordinate direction = this->end() - this->start();
    Coordinate vpc = coord - this->start();
    // At DLI: We don't test if length is 0 because we require that each vector/line  this direction.squared(); is always > 0
    vpc = start() + direction * Coordinate(vpc).dot(direction) / direction.squared();

    return vpc;
}

bool Line::isCoordinateOnPath(const Coordinate& coord) const {
    return (nearestPointOnPath(coord) - coord).magnitude() < 1.0e-4;
}

void Line::start(const Coordinate& coord) {
    _start = coord;
}
void Line::end(const Coordinate& coord) {
    _end = coord;
}


QDebug operator << (QDebug dbg, const geo::Line& c) {
    dbg.nospace() << "(" << c.start() << "," << c.end() << ")";
    return dbg.space();
}

