#include "geoarc.h"

using namespace lc;
using namespace geo;

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

Coordinate Arc::nearestPointOnPath(const Coordinate& coord) const {
    Coordinate vp = coord - center();
    double d = vp.magnitude();
    return center() + vp * (radius() / d);
}

bool Arc::isCoordinateOnPath(const Coordinate& coord) const {
    return (nearestPointOnPath(coord) - coord).magnitude() < 1.0e-4;
}

