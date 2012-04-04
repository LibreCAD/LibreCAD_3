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

double Ellipse::minorRadius() {
    return _minorRadius;
}

double Ellipse::startAngle() {
    return _startAngle;
}

double Ellipse::endAngle() {
    return _endAngle;
}

