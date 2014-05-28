
#include "geodimlinear.h"

using namespace lc;
using namespace geo;

DimLinear::DimLinear(const Dimension& dimension, const Coordinate& extension_point1, const Coordinate& extension_point2, const double oblique) : _extension_point1(extension_point1), _extension_point2(extension_point2), _oblique(oblique) {
}

Coordinate DimLinear::extension_point1() const {
    return _extension_point1;
}

Coordinate DimLinear::extension_point2() const {
    return _extension_point2;
}

double DimLinear::oblique() const {
    return _oblique;
}
