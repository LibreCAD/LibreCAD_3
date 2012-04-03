#include "geocoordinate.h"

using namespace lc;
using namespace geo;

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

Coordinate::Coordinate(const Coordinate& coordinate) {
    _x = coordinate._x;
    _y = coordinate._y;
    _z = coordinate._z;
}

Coordinate& Coordinate::operator= (const Coordinate& coordinate) {
    if (this != &coordinate) {
        _x = coordinate._x;
        _y = coordinate._y;
        _z = coordinate._z;
    }

    return *this;
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
