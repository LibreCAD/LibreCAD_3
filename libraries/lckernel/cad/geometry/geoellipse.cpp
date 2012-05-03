#include "geoellipse.h"

using namespace lc;
using namespace geo;



Ellipse::Ellipse(const Coordinate& center, const Coordinate& majorP, double minorRadius, double startAngle, double endAngle)  :
    _center(center),
    _majorP(majorP) {
    _minorRadius = minorRadius;
    _startAngle = startAngle;
    _endAngle = endAngle;
}

const Coordinate& Ellipse::center() const {
    return _center;
}

const Coordinate& Ellipse::majorP() const {
    return _majorP;
}

double Ellipse::minorRadius() const {
    return _minorRadius;
}

double Ellipse::startAngle() const {
    return _startAngle;
}

Coordinate Ellipse::nearestPointOnPath(const Coordinate& coord) const {
    return center();
}

bool Ellipse::isCoordinateOnPath(const Coordinate& coord) const {
    return false;
}

double Ellipse::endAngle() const {
    return _endAngle;
}

QList<Coordinate> Ellipse::intersect(IntersectablePtr x, Intersectable::Coordinates intersect) const {
    QList<Coordinate> points;
    return points;
}
QList<Coordinate> Ellipse::intersect(const Vector& x, Intersectable::Coordinates intersect) const {
    QList<Coordinate> points;
    return points;
}
QList<Coordinate> Ellipse::intersect(const Circle& x, Intersectable::Coordinates intersect) const {
    QList<Coordinate> points;
    return points;
}
QList<Coordinate> Ellipse::intersect(const Arc& x, Intersectable::Coordinates intersect) const {
    QList<Coordinate> points;
    return points;
}
QList<Coordinate> Ellipse::intersect(const Ellipse& x, Intersectable::Coordinates intersect) const {
    QList<Coordinate> points;
    return points;
}
