
#include <algorithm>
#include "geoarea.h"

using namespace lc;
using namespace geo;


Area::Area(const Coordinate& coordA, const Coordinate& coordB) :
    _coordA(Coordinate(std::min(coordA.x(), coordB.x()), std::min(coordA.y(), coordB.y()), std::min(coordA.z(), coordB.z()))),
    _coordB(Coordinate(std::max(coordA.x(), coordB.x()), std::max(coordA.y(), coordB.y()), std::max(coordA.z(), coordB.z()))) {
    if (coordA.x() != coordB.x() && coordA.y() != coordB.y() && coordA.z() != coordB.z()) {
        throw "Points describe a volume, not a coordinate.";
    }

}

const Coordinate& Area::coordA() const {
    return _coordA;
}
const Coordinate& Area::coordB() const {
    return _coordB;
}
