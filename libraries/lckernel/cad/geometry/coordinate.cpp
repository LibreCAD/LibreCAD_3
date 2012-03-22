#include "Coordinate.h"

using namespace lc;

Coordinate::Coordinate(double x, double y, double z) {
    this->_x = x;
    this->_y = y;
    this->_z = z;
}
Coordinate::Coordinate(double x, double y) {
    this->_x = x;
    this->_y = y;
    this->_z = 0;
}

double Coordinate::x() const {
    return _x;
}
double Coordinate::y() const {
    return _y;
}
double Coordinate::z() const {
    return _z;
}
