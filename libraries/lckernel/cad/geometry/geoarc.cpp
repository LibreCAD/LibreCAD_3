#include "geoarc.h"

using namespace lc;
using namespace geo;




Arc::Arc(const Coordinate& center, double radius, double startAngle, double endAngle) : _center(center) {
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

const Coordinate& Arc::center() const {
    return _center;
}
