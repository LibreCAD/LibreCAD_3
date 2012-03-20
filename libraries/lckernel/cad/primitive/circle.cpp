#include "circle.h"

using namespace lc;

Circle::Circle(Point* center, double radius) : CADEntity() {
    this->_center = center;
    this->_radius = radius;
}
Circle::Circle(Point* center, double radius, METAINFO_TYPELIST metaTypes) : CADEntity(metaTypes) {
    this->_center = center;
    this->_radius = radius;
}

Point* Circle::center() const {
    return _center;
}
double Circle::radius() const {
    return _radius;
}
