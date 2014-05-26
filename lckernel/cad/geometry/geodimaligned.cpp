#include "geodimaligned.h"

using namespace lc;
using namespace geo;

DimAligned::DimAligned(const Dimension& dimension, const Coordinate& extension_point1, const Coordinate& extension_point2) : _extension_point1(extension_point1), _extension_point2(extension_point2) {
}

Coordinate DimAligned::extension_point1() const {
    return _extension_point1;
}

Coordinate DimAligned::extension_point2() const {
    return _extension_point2;
}

