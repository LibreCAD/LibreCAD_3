#include "geocoordinate.h"
#include "cad/math/lcmath.h"

using namespace lc;
using namespace geo;

double Coordinate::angleBetween(const Coordinate& v1, const Coordinate& v2) const {
    Coordinate vStart(v1 - (*this));
    Coordinate vEnd(v2 - (*this));
    return Math::correctAngle(atan2(vStart.x() * vEnd.y() - vStart.y() * vEnd.x(), vStart.x() * vEnd.x() + vStart.y() * vEnd.y()));
}
