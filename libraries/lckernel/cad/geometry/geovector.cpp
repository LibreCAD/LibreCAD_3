#include "geovector.h"

#include "geoarea.h"

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

Coordinate Vector::nearestPointOnPath(const Coordinate& coord) const {
    Coordinate direction = this->end() - this->start();
    Coordinate vpc = coord - this->start();
    // At DLI: We don't test if length is 0 because we require that each vector/line is valid when it's created, test needs to be done in the constructor!
    vpc = start() + direction * Coordinate(vpc).dot(direction) / direction.squared();

    // test of the generated point is within the lines 'Area', if so return vpc
    Area area(end(), start());

    if (area.inArea(vpc)) {
        return vpc;
    }

    // Return minimum ppoint
    if (area.minP().x() < vpc.x()) {
        return area.minP();
    }

    // Return maximum point
    return area.maxP();
}


QDebug operator << (QDebug dbg, const lc::geo::Vector& c) {
    dbg.nospace() << "(" << c.start() << "," << c.end() << ")";
    return dbg.space();
}
