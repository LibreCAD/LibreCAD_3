#include <cad/math/lcmath.h>
#include "geoarc.h"
#include "geoarea.h"

using namespace lc;
using namespace geo;

Arc::Arc(const Coordinate &center, double radius, double startAngle, double endAngle)
        : Base(), _center(center), _radius(radius), _startAngle(startAngle), _endAngle(endAngle), _reversed(false) {
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

}

Arc::Arc(const Coordinate &center, double radius, double startAngle, double endAngle, bool reversed)
        : Base(), _center(center), _radius(radius), _startAngle(startAngle), _endAngle(endAngle), _reversed(reversed) {
}

Arc Arc::createArc3P(const Coordinate &p1, const Coordinate &p2, const Coordinate &p3) {

    geo::Coordinate vra = p2 - p1;
    geo::Coordinate vrb = p3 - p1;
    double ra2 = vra.squared() * 0.5;
    double rb2 = vrb.squared() * 0.5;
    double crossp = vra.x() * vrb.y() - vra.y() * vrb.x();
    if (std::abs(crossp) <= 0.0) {
        // "Cannot create a arc with radius 0.0.");
        // TODO add exception handling of some sort
//        throw;
        return Arc(geo::Coordinate(0., 0.), 0., 0., 0.);
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
    auto reversed = bulge<0.;
    auto alpha = atan(bulge)*4.;

    auto middle = p1.mid(p2);
    auto dist = p1.distanceTo(p2)/2.0;

    auto radius = std::abs(dist / std::sin(alpha/2.0));

    auto wu = std::abs(std::pow(radius, 2.0) - std::pow(dist, 2.0));
    auto h = std::sqrt(wu);
    auto angle = p1.angleTo(p2);

    if (reversed) {
        angle-=M_PI/2.0;
    } else {
        angle+=M_PI/2.0;
    }

    if (std::abs(alpha)>M_PI) {
        h*=-1.0;
    }

    auto center = geo::Coordinate(angle) * h + middle;

    return Arc(center, radius, center.angleTo(p1), center.angleTo(p2), reversed);
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

bool Arc::isCoordinateOnPath(const Coordinate &coord) const {
    return (nearestPointOnPath(coord) - coord).magnitude() < 1.0e-4;
}

double Arc::length() const {
    if (startAngle() > endAngle()) {
        return std::abs((2. * M_PI + endAngle() - startAngle()) * radius());
    } else {
        return std::abs((endAngle() - startAngle()) * radius());
    }
}

double Arc::reversed() const {
    return _reversed;
}

Coordinate Arc::startP() const {
    return _center + Coordinate(_startAngle) * _radius;
}

Coordinate Arc::endP() const {
    return _center + Coordinate(_endAngle) * _radius;
}

Area Arc::boundingBox() const {
    geo::Area area(startP(), endP());

    const double startAngle = Math::correctAngle(_startAngle);
    const double endAngle = Math::correctAngle(_endAngle);
    const double p0 = 0.0 * M_PI;
    const double p1 = 0.5 * M_PI;
    const double p2 = 1.0 * M_PI;
    const double p3 = 1.5 * M_PI;

    if ((Math::isAngleBetween(p0, startAngle, endAngle, _reversed))) {
        area = area.merge(Coordinate(_center.x() + _radius, _center.y()));
    }
    if ((Math::isAngleBetween(p1, startAngle, endAngle, _reversed))) {
        area = area.merge(Coordinate(_center.x(), _center.y() + _radius));
    }
    if ((Math::isAngleBetween(p2, startAngle, endAngle, _reversed))) {
        area = area.merge(Coordinate(_center.x() - _radius, _center.y()));
    }
    if ((Math::isAngleBetween(p3, startAngle, endAngle, _reversed))) {
        area = area.merge(Coordinate(_center.x(), _center.y() - _radius));
    }
    return area;
}