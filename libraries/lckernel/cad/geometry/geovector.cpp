#include "geovector.h"

#include "geoarea.h"
#include "geomath.h"

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
    // At DLI: We don't test if length is 0 because we require that each vector/line  this direction.squared(); is always > 0
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

bool Vector::isCoordinateOnPath(const Coordinate& coord) const {
    return (nearestPointOnPath(coord) - coord).magnitude() < 1.0e-4;
}

QList<Coordinate> Vector::intersect(IntersectablePtr x, Intersectable::Coordinates intersect) const {
    return x->intersect(*this, intersect);
}
QList<Coordinate> Vector::intersect(const Vector& x, Intersectable::Coordinates intersect) const {
    return GeoMath::intersectVectorVector(*this, x,  GeoMath::Intersect(intersect));
}
QList<Coordinate> Vector::intersect(const Circle& x, Intersectable::Coordinates intersect) const {
    return GeoMath::intersectCircleLine(x, *this,  GeoMath::Intersect(intersect));
}
QList<Coordinate> Vector::intersect(const Arc& x, Intersectable::Coordinates intersect) const {
    return GeoMath::intersectArcLine(x, *this, GeoMath::Intersect(intersect));
}
QList<Coordinate> Vector::intersect(const Ellipse& x, Intersectable::Coordinates intersect) const {
    QList<Coordinate> points;
    return points;
}


QDebug operator << (QDebug dbg, const lc::geo::Vector& c) {
    dbg.nospace() << "(" << c.start() << "," << c.end() << ")";
    return dbg.space();
}

