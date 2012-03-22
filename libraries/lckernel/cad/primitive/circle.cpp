#include "circle.h"

using namespace lc;

Circle::Circle(Coordinate* center, double radius) : CADEntity() {
    this->_center = center;
    this->_radius = radius;
}
Circle::Circle(Coordinate* center, double radius, METAINFO_TYPELIST metaTypes) : CADEntity(metaTypes) {
    this->_center = center;
    this->_radius = radius;
}

Coordinate* Circle::center() const {
    return _center;
}
double Circle::radius() const {
    return _radius;
}
