#include "geocoordinate.h"

#include <math.h>

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

Coordinate::Coordinate(double angle) {
    this->_x = cos(angle);
    this->_y = sin(angle);
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
Coordinate Coordinate::operator * (const Coordinate& coord) const {
    return Coordinate(_x * coord._x, _y * coord._y, _z * coord._z);
}
double Coordinate::magnitude() const {
    return sqrtf(_x * _x + _y * _y + _z * _z);
}

double Coordinate::angle() const {
    return atan2(_y, _x);
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

Coordinate Coordinate::rotate(const Coordinate& angleVector) const {
    double x0 = _x * angleVector.x() - _y * angleVector.y();
    double y0 = _x * angleVector.y() + _y * angleVector.x();
    return Coordinate(x0, y0);
}

Coordinate Coordinate::rotate(const double& angle) const {
    return rotate(Coordinate(angle));
}

Coordinate Coordinate::rotate(const geo::Coordinate& point, const Coordinate& angleVector) const {
    Coordinate new_center = point + (*this - point).rotate(angleVector);
    return new_center;
}

Coordinate Coordinate::rotate(const geo::Coordinate& point, const double& angle) const {
    return rotate(point, Coordinate(angle));
}

/**
 * Scales the vector by given factors with 0/0 as center
 */
Coordinate Coordinate::scale(const double& scale_factor) const {
    double x0 = _x * scale_factor;
    double y0 = _y * scale_factor;
    return Coordinate(x0, y0);
}

/**
 * Scales this vector by the given factors with the given center.
 */
Coordinate Coordinate::scale(const Coordinate& scale_center, const Coordinate& scale_factor) const {
    Coordinate newscale = scale_center + (*this - scale_center) * scale_factor;
    return newscale;
}

CoordinateDistanceSort::CoordinateDistanceSort(const Coordinate& distanceFrom) : _distanceFrom(distanceFrom) {
}

bool CoordinateDistanceSort::operator()(const Coordinate& left, const Coordinate& right) const {
    double d1 = (this->_distanceFrom - left).squared();
    double d2 = (this->_distanceFrom - right).squared();
    return d1 < d2;
}
