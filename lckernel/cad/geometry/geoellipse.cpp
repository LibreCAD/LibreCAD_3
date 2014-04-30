#include "geoellipse.h"

#include "math.h"

using namespace lc;
using namespace geo;



Ellipse::Ellipse(const Coordinate& center, const Coordinate& majorP, double minorRadius, double startAngle, double endAngle, bool isArc)  :
    _center(center),
    _majorP(majorP),
    _isArc(isArc) {
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

Coordinate Ellipse::nearestPointOnPath(const Coordinate& /*coord*/) const {
    return center();
}

bool Ellipse::isCoordinateOnPath(const Coordinate& /*coord*/) const {
    return false;
}

double Ellipse::endAngle() const {
    return _endAngle;
}

Coordinate Ellipse::getPoint(const double& angle) const {
    return _center + Coordinate(majorP().magnitude() * cos(angle), minorRadius() * sin(angle)).rotate(Coordinate(0., 0.), majorP().angle());
}

Coordinate Ellipse::startPoint() const {
    return getPoint(startAngle());
}
Coordinate Ellipse::endPoint() const {
    return getPoint(endAngle());
}

bool Ellipse::isArc() const {
    return _isArc;
}

double Ellipse::getEllipseAngle(const Coordinate& coord) const {
    Coordinate offset = coord - _center;
    Coordinate point = offset.rotate(-_majorP.angle());
    return atan2(point.y() * _majorP.magnitude(), point.x() * _minorRadius);
}
