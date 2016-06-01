#include "geoellipse.h"

#include "math.h"

using namespace lc;
using namespace geo;

Ellipse::Ellipse(const Coordinate& center, const Coordinate& majorP, double minorRadius, double startAngle, double endAngle) :
    Base(),
    _center(center),
    _majorP(majorP),
    _minorRadius(minorRadius),
    _startAngle(startAngle),
    _endAngle(endAngle),
    _isReversed(false){

}

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

Ellipse Ellipse::geoscale(const Coordinate& center, const Coordinate &factor) const {
    geo::Coordinate vp1(this->majorP());
    double a(vp1.magnitude());
    geo::Coordinate vp2(vp1.x() * 1. / a, vp1.y() * 1. / a);
    geo::Coordinate startPoint;
    geo::Coordinate endPoint;

    if (isArc()) {
        startPoint = this->startPoint();
        endPoint = this->endPoint();
    }

    double ct = vp2.x();
    double ct2 = ct * ct; // cos^2 angle
    double st = vp2.y();
    double st2 = 1.0 - ct2; // sin^2 angle
    double kx2 = factor.x() * factor.x();
    double ky2 = factor.y() * factor.y();
    double b = (this->minorRadius() / this->majorP().magnitude()) * a;
    double cA = 0.5 * a * a * (kx2 * ct2 + ky2 * st2);
    double cB = 0.5 * b * b * (kx2 * st2 + ky2 * ct2);
    double cC = a * b * ct * st * (ky2 - kx2);
    geo::Coordinate vp(cA - cB, cC);
    geo::Coordinate vp3(a, b);
    geo::Coordinate vp4(vp3.scale(geo::Coordinate(vp.angle() * 0.5)));
    geo::Coordinate vp5(vp4.rotate(geo::Coordinate(ct, st)));
    geo::Coordinate vp6(vp5.scale(factor));
    double z = cA + cB;
    double x = vp.magnitude();
    double ratio = std::sqrt((z - x) / (z + x));
    double minor_ = vp6.magnitude() * ratio;

    return Ellipse(this->center().scale(center, factor), vp6,
                                                    minor_,
                                                    isArc() ? this->getEllipseAngle(startPoint) : 0.,
                                                    isArc() ? this->getEllipseAngle(endPoint) : 2.*M_PI);
}

Coordinate Ellipse::nearestPointOnPath(const Coordinate& coord) const {
    /* TODO implement
     * fix compiler warning
     */
    return Coordinate();
}

Coordinate Ellipse::nearestPointOnEntity(const Coordinate& coord) const {
    /* TODO implement
     * fix compiler warning
     */
    return Coordinate();
}

/*
 * DEPRECATED, but keeping source for reference
bool Ellipse::isCoordinateOnPath(const Coordinate& coord, double tolerance) const {
    double t = fabs(tolerance);
    double a = majorRadius();
    double b = a * ratio();

    Coordinate vp((coord - center()).rotate(-getAngle()));

    if (a < LCTOLERANCE) {
        //radius treated as zero
        if (fabs(vp.x()) < LCTOLERANCE && fabs(vp.y()) < b) {
            return true;
        }

        return false;
    }

    if (b < LCTOLERANCE) {
        //radius treated as zero
        if (fabs(vp.y()) < LCTOLERANCE && fabs(vp.x()) < a) {
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
 */

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
    return Math::getAngleDifferenceShort(_startAngle, _endAngle, _isReversed) < LCARCTOLERANCE;
}

double Ellipse::getEllipseAngle(const Coordinate& coord) const {
    Coordinate offset = coord - _center;
    Coordinate point = offset.rotate(-_majorP.angle());
    return atan2(point.y() * _majorP.magnitude(), point.x() * _minorRadius);
}
