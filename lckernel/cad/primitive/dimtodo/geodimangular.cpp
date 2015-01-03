#include "geodimangular.h"

using namespace lc;
using namespace geo;

DimAngular::DimAngular(const Dimension& dimension, const Coordinate& definition_point1, const Coordinate& definition_point2, const Coordinate& definition_point3, const Coordinate& definition_point4) : _definition_point1(definition_point1), _definition_point2(definition_point2), _definition_point3(definition_point3), _definition_point4(definition_point4)  {
}

Coordinate DimAngular::definition_point1() const {
    return _definition_point1;
}

Coordinate DimAngular::definition_point2() const {
    return _definition_point2;
}

Coordinate DimAngular::definition_point3() const {
    return _definition_point3;
}

Coordinate DimAngular::definition_point4() const {
    return _definition_point4;
}

