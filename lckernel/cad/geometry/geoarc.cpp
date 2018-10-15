#include <cad/math/lcmath.h>
#include "geoarc.h"

using namespace lc;
using namespace geo;

Arc::Arc(Coordinate center, double radius, double startAngle, double endAngle, bool isCCW) :
        Base(),
        _center(std::move(center)),
        _radius(radius),
        _startAngle(maths::Math::correctAngle(startAngle)),
        _endAngle(maths::Math::correctAngle(endAngle)),
        _CCW(isCCW) {

    if (radius <= 0.0) {
        throw std::runtime_error("Invalid radius");
    }
}

Arc Arc::createArc3P(const Coordinate &p1, const Coordinate &p2, const Coordinate &p3) {
    geo::Coordinate vra = p2 - p1;
    geo::Coordinate vrb = p3 - p1;
    double ra2 = vra.squared() * 0.5;
    double rb2 = vrb.squared() * 0.5;
    double crossp = vra.x() * vrb.y() - vra.y() * vrb.x();
    if (std::abs(crossp) <= 0.0) {
        throw std::runtime_error("Cannot create a arc with radius 0.0.");
    }
    crossp = 1. / crossp;

    auto center = geo::Coordinate((ra2 * vrb.y() - rb2 * vra.y()) * crossp, (rb2 * vra.x() - ra2 * vrb.x()) * crossp);
    auto radius = center.magnitude();
    center = center + p1;
    auto angle1 = center.angleTo(p1);
    auto angle2 = center.angleTo(p3);

    return Arc(center, radius, angle1, angle2, false);
}

Arc Arc::createArcBulge(const Coordinate &p1, const Coordinate &p2, const double bulge) {
    auto isCCW = bulge>0.;
    auto delta = atan(bulge) * 4.0;

    auto middle = p1.mid(p2);
    auto dist = p1.distanceTo(p2)/2.0;

    auto radius = std::abs(dist / std::sin(delta/2.0));

    auto wu = std::abs(std::pow(radius, 2.0) - std::pow(dist, 2.0));
    auto h = std::sqrt(wu);
    auto angle = p1.angleTo(p2);

    if (isCCW) {
        angle -= M_PI/2.0;
    }
    else {
        angle += M_PI/2.0;
    }

    if (std::abs(delta) < M_PI) {
        h*=-1.0;
    }

    auto center = geo::Coordinate(angle) * h + middle;

    return Arc(center, radius, center.angleTo(p1), center.angleTo(p2), isCCW);
}

double Arc::radius() const {
    return _radius;
}

double Arc::startAngle() const {
    return _startAngle;
}

double Arc::endAngle() const {
    return _endAngle;
}

const Coordinate Arc::center() const {
    return _center;
}

Coordinate Arc::nearestPointOnPath(const Coordinate &coord) const {
    return center() + Coordinate((coord - center()).angle()) * radius();
}

Coordinate Arc::nearestPointOnEntity(const Coordinate &coord) const {
    const auto angle = (coord - center()).angle();

    // if the angle is between start and stop then calculate the nearest point
    // on it's entity
    if (isAngleBetween(angle)) {
        return center() + angle * radius();
    }

    // Find out if angle is closer to start or end and return the appropriate coordinate
    const auto ad1 = std::abs(angle - _startAngle);
    const auto ad2 = std::abs(angle - _endAngle);

    if (ad1 <= ad2) {
        return startP();
    }
    else {
        return endP();
    }
}

double Arc::length() const {
    if (_startAngle > _endAngle) {
        return std::abs((2. * M_PI + endAngle() - startAngle()) * radius());
    }
    else {
        return std::abs((endAngle() - startAngle()) * radius());
    }
}

bool Arc::CCW() const {
    return _CCW;
}

Coordinate Arc::startP() const {
    return _center + Coordinate(_startAngle) * _radius;
}

Coordinate Arc::endP() const {
    return _center + Coordinate(_endAngle) * _radius;
}

Area Arc::boundingBox() const {
    geo::Area area(startP(), endP());

    const double startAngle = _startAngle;
    const double endAngle = _endAngle;
    const double p0 = 0.0 * M_PI;
    const double p1 = 0.5 * M_PI;
    const double p2 = 1.0 * M_PI;
    const double p3 = 1.5 * M_PI;

    if ((maths::Math::isAngleBetween(p0, startAngle, endAngle, _CCW))) {
        area = area.merge(Coordinate(_center.x() + _radius, _center.y()));
    }
    if ((maths::Math::isAngleBetween(p1, startAngle, endAngle, _CCW))) {
        area = area.merge(Coordinate(_center.x(), _center.y() + _radius));
    }
    if ((maths::Math::isAngleBetween(p2, startAngle, endAngle, _CCW))) {
        area = area.merge(Coordinate(_center.x() - _radius, _center.y()));
    }
    if ((maths::Math::isAngleBetween(p3, startAngle, endAngle, _CCW))) {
        area = area.merge(Coordinate(_center.x(), _center.y() - _radius));
    }
    return area;
}

bool Arc::isAngleBetween(double angle) const {
    return maths::Math::isAngleBetween(angle, _startAngle, _endAngle, _CCW);
}

double Arc::angle() const {
    return maths::Math::correctAngle(std::abs(_endAngle - _startAngle));
}

double Arc::bulge() const {
    double bulge = std::tan(angle()/4.0);

    if(!CCW()) {
        bulge *= -1;
    }

    return bulge;
}