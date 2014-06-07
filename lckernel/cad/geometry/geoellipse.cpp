#include "geoellipse.h"

#include "math.h"

using namespace lc;
using namespace geo;

Ellipse::Ellipse(const Coordinate& center, const Coordinate& majorP, double minorRadius, double startAngle, double endAngle, bool isArc) :
    _center(center),
    _majorP(majorP),
    _isArc(isArc),
    _minorRadius(minorRadius),
    _startAngle(startAngle),
    _endAngle(endAngle) {}

const Coordinate Ellipse::center() const {
    return _center;
}

const Coordinate Ellipse::majorP() const {
    return _majorP;
}

double Ellipse::minorRadius() const {
    return _minorRadius;
}

double Ellipse::startAngle() const {
    return _startAngle;
}

double Ellipse::majorRadius() const {
    return _majorP.magnitude();
}

double Ellipse::getAngle() const {
    return _majorP.angle();
}

Coordinate Ellipse::nearestPointOnPath(const Coordinate& coord) const {
}

bool Ellipse::isCoordinateOnPath(const Coordinate& coord, double tolerance) const {
    double t = fabs(tolerance);
    double a = majorRadius();
    double b = a * ratio();

    Coordinate vp((coord - center()).rotate(-getAngle()));

    if (a < TOLERANCE) {
        //radius treated as zero
        if (fabs(vp.x()) < TOLERANCE && fabs(vp.y()) < b) {
            return true;
        }

        return false;
    }

    if (b < TOLERANCE) {
        //radius treated as zero
        if (fabs(vp.y()) < TOLERANCE && fabs(vp.x()) < a) {
            return true;
        }

        return false;
    }

    auto cord = Coordinate(vp.scale(Coordinate(1. / a, 1. / b)));

    if (fabs(cord.squared() - 1.) > t) {
        return false;
    }

    return Math::isAngleBetween(cord.angle(), startAngle(), endAngle(), isReversed());
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

bool Ellipse::isReversed() const {
    return _isReversed;
}

double Ellipse::ratio() const {
    return _majorP.magnitude() / _minorRadius;
}

bool Ellipse::isArc() const {
    return _startAngle != _endAngle;
}

double Ellipse::getEllipseAngle(const Coordinate& coord) const {
    Coordinate offset = coord - _center;
    Coordinate point = offset.rotate(-_majorP.angle());
    return atan2(point.y() * _majorP.magnitude(), point.x() * _minorRadius);
}
