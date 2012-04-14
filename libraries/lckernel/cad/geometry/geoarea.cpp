
#include <algorithm>
#include "geoarea.h"

using namespace lc;
using namespace geo;


Area::Area(const Coordinate& coordA, const Coordinate& coordB) :
    _minP(Coordinate(std::min(coordA.x(), coordB.x()), std::min(coordA.y(), coordB.y()), std::min(coordA.z(), coordB.z()))),
    _maxP(Coordinate(std::max(coordA.x(), coordB.x()), std::max(coordA.y(), coordB.y()), std::max(coordA.z(), coordB.z()))) {
    if (coordA.x() != coordB.x() && coordA.y() != coordB.y() && coordA.z() != coordB.z()) {
        throw "Points describe a volume, not a coordinate.";
    }

}

const Coordinate& Area::minP() const {
    return _minP;
}
const Coordinate& Area::maxP() const {
    return _maxP;
}

bool Area::inArea(const Coordinate& point) const {
    return (point.x() >= _minP.x() && point.x() <= _maxP.x() && point.y() >= _minP.y() && point.y() <= _maxP.y());
}

