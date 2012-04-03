#include "geoarc.h"

using namespace lc;
using namespace geo;


Arc::Arc() {
    _radius = 0;
    _startAngle = 0;
    _endAngle = 0;
}

Arc::Arc(const Coordinate& center, double radius, double startAngle, double endAngle) {
    _center = center;
    _radius = radius;
    _startAngle = startAngle;
    _endAngle = endAngle;
}


double Arc::startAngle()const {
    return _startAngle;
}

double Arc::endAngle() const {
    return _endAngle;
}

Coordinate Arc::center() const {
    return _center;
}
