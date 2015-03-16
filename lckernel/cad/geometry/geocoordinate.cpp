#include "geocoordinate.h"
#include "cad/math/lcmath.h"

using namespace lc;
using namespace geo;

double Coordinate::angleBetween(const Coordinate& v1, const Coordinate& v2) const {
    Coordinate vStart(v1 - (*this));
    Coordinate vEnd(v2 - (*this));
    return Math::correctAngle(atan2(vStart.x() * vEnd.y() - vStart.y() * vEnd.x(), vStart.x() * vEnd.x() + vStart.y() * vEnd.y()));
}

Coordinate::Coordinate(const std::initializer_list<double> p) : Base() {

    _x = _y = _z = 0;

    auto i = p.begin();

    // FIXME add debug log if the size is < 2
    if (p.size() >= 2) {
        _x = *i;
        _y = *++i;

        if (p.size() >= 3) {
            _z = *++i;
        }
    }

}
