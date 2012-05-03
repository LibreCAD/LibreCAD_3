#include "geocircle.h"

using namespace lc;
using namespace geo;

#include "geointersectable.h"
#include "geomath.h"


Circle::Circle(const Coordinate& center, double radius) : _center(center) {
    if (radius < 0.0) {
        throw "Invalid radius";
    }

    _radius = radius;
}

const Coordinate& Circle::center() const {
    return _center;
}
double Circle::radius() const {
    return _radius;
}


Coordinate Circle::nearestPointOnPath(const Coordinate& coord) const {
    Coordinate vp = coord - center();
    double d = vp.magnitude();
    return center() + vp * (radius() / d);
}

bool Circle::isCoordinateOnPath(const Coordinate& coord) const {
    return (nearestPointOnPath(coord) - coord).magnitude() < 1.0e-4;
}

QList<Coordinate> Circle::intersect(IntersectablePtr x, Intersectable::Coordinates intersect) const {
    return x->intersect(*this, intersect);
}
QList<Coordinate> Circle::intersect(const Vector& x, Intersectable::Coordinates intersect) const {
    return GeoMath::intersectCircleLine(*this, x,  GeoMath::Intersect(intersect));
}
QList<Coordinate> Circle::intersect(const Circle& x, Intersectable::Coordinates intersect) const {
    QList<Coordinate> points;
    return points;
}
QList<Coordinate> Circle::intersect(const Arc& x, Intersectable::Coordinates intersect) const {
    QList<Coordinate> points;
    return points;
}
QList<Coordinate> Circle::intersect(const Ellipse& x, Intersectable::Coordinates intersect) const {
    QList<Coordinate> points;
    return points;
}

QDebug operator << (QDebug dbg, const lc::geo::Circle& c) {
    dbg.nospace() << "(" << c.center() << "," << c.radius() << ")";
    return dbg.space();
}

