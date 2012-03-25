
#include <algorithm>
#include "area.h"

using namespace lc;
using namespace geo;


Area::Area(const geo::Coordinate& coordA, const geo::Coordinate& coordB) {
    if (coordA.x() != coordB.x() && coordA.y() != coordB.y() && coordA.z() != coordB.z()) {
        throw "Points describe a volume, not a coordinate.";
    }

    _coordA = geo::Coordinate(std::min(coordA.x(), coordB.x()), std::min(coordA.y(), coordB.y()), std::min(coordA.z(), coordB.z()));
    _coordB = geo::Coordinate(std::max(coordA.x(), coordB.x()), std::max(coordA.y(), coordB.y()), std::max(coordA.z(), coordB.z()));
}

Coordinate Area::coordA() const {
    return _coordA;
}
Coordinate Area::coordB() const {
    return _coordB;
}


Area::Area(const Area& area) {
    _coordA = area._coordA;
    _coordB = area._coordB;
}

Area& Area::operator= (const Area& area) {
    if (this != &area) {
        _coordA = area._coordA;
        _coordB = area._coordB;
    }

    return *this;
}
