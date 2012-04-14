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

Coordinate::Coordinate(const QPointF& qtPoint) {
    this->_x = qtPoint.x();
    this->_y = qtPoint.y();
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



/**
  * Math functions for a coordinate
  */
double Coordinate::distanceTo(const geo::Coordinate& c) const {
    return (*this - c).magnitude();
}
Coordinate Coordinate::operator + (const Coordinate& coord) const {
    return Coordinate(_x + coord._x, _y + coord._y, _z + coord._z);
}
Coordinate Coordinate::operator - (const Coordinate& coord) const {
    return Coordinate(_x - coord._x, _y - coord._y, _z - coord._z);
}
double Coordinate::magnitude() const {
    return sqrt(_x * _x + _y * _y + _z * _z);
}
double Coordinate::squared() const {
    return _x * _x + _y * _y + _z * _z;
}
double Coordinate::dot(const Coordinate& coord) const {
    return _x * coord._x + _y * coord._y + _z * coord._z;
}
Coordinate Coordinate::operator * (double s) const {
    return Coordinate(_x * s, _y * s, _z * s);
}
Coordinate Coordinate::operator / (double s) const {
    return Coordinate(_x / s, _y / s, _z / s);
}



QDebug operator << (QDebug dbg, const lc::geo::Coordinate& c) {
    dbg.nospace() << "(" << c.x() << "," << c.y() << "," << c.z() << ")";
    return dbg.space();
}
