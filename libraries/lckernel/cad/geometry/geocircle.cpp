#include "geocircle.h"

using namespace lc;
using namespace geo;

#include "geointersectable.h"


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
    geo::Coordinate vp = coord - center();
    double d = vp.magnitude();
    return center() + vp * (radius() / d);
}

QList<Coordinate> Circle::intersect(IntersectablePtr x) const {
    return x->intersect(*this);
}
QList<Coordinate> Circle::intersect(const Vector& x) const {
    return geoIntersectCircleLine(*this, x);
}
QList<Coordinate> Circle::intersect(const Circle& x) const {
    QList<Coordinate> points;
    return points;
}
QList<Coordinate> Circle::intersect(const Arc& x) const {
    QList<Coordinate> points;
    return points;
}
QList<Coordinate> Circle::intersect(const Ellipse& x) const {
    QList<Coordinate> points;
    return points;
}

QDebug operator << (QDebug dbg, const lc::geo::Circle& c) {
    dbg.nospace() << "(" << c.center() << "," << c.radius() << ")";
    return dbg.space();
}

