#include "geoarc.h"

using namespace lc;
using namespace geo;

#include "geointersectable.h"

Arc::Arc(const Coordinate& center, double radius, double startAngle, double endAngle) : _center(center) {
    /*
    if (startAngle<0.0 || startAngle>PI2 || startAngle<endAngle) {
        throw "Invalid start angle";
    }
    if (endAngle<0.0 || endAngle> PI2) {
        throw "Invalid end angle";
    }
    if (radius<0.0) {
        throw "Invalid radius";
    }*/

    _radius = radius;
    _startAngle = startAngle;
    _endAngle = endAngle;


}

double Arc::radius() const {
    return _radius;
}

double Arc::startAngle()const {
    return _startAngle;
}

double Arc::endAngle() const {
    return _endAngle;
}

const Coordinate& Arc::center() const {
    return _center;
}


QList<Coordinate> Arc::intersect(IntersectablePtr x) const {
    return x->intersect(*this);
}
QList<Coordinate> Arc::intersect(const Vector& x) const {
    return geoIntersectArcLine(*this, x);
}
QList<Coordinate> Arc::intersect(const Circle& x) const {
    QList<Coordinate> points;
    return points;
}
QList<Coordinate> Arc::intersect(const Arc& x) const {
    QList<Coordinate> points;
    return points;
}
QList<Coordinate> Arc::intersect(const Ellipse& x) const {
    QList<Coordinate> points;
    return points;
}
