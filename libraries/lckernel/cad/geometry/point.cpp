#include "point.h"

using namespace lc;

Point::Point(double x, double y, double z) {
    this->_x = x;
    this->_y = y;
    this->_z = z;
}
Point::Point(double x, double y) {
    this->_x = x;
    this->_y = y;
    this->_z = 0;
}

double Point::x() const {
    return _x;
}
double Point::y() const {
    return _y;
}
double Point::z() const {
    return _z;
}
