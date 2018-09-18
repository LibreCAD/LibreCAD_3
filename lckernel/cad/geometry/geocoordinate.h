#pragma once

#include "cad/const.h"
#include <cmath>
#include <iostream>

namespace lc {
    namespace geo {
        /**
          * Class that represents a coordinate in space
          */
        class Coordinate {
            public:
                explicit Coordinate() : _x(0.), _y(0.), _z(0.) {}
                Coordinate(double x, double y, double z) : _x(x), _y(y), _z(z) {}
                Coordinate(double x, double y) : _x(x), _y(y), _z(0.) {}
                explicit Coordinate(double angle) : _x(std::cos(angle)), _y(std::sin(angle)), _z(0.) {}
                Coordinate(Coordinate &&c) noexcept : _x(c._x), _y(c._y), _z(c._z) {}
                Coordinate(const Coordinate &c) : _x(c._x), _y(c._y), _z(c._z) {}


                /**
                     * @brief  Returns x of Coordinate
                     * @return double x
                     */
                inline double x() const {
                    return _x;
                }

                /**
                 * @brief Returns y of Coordinate
                 * @return double y
                 */
                inline double y() const {
                    return _y;
                }

                /**
                 * @brief Returns z of Coordinate
                 * @return double z
                 */
                inline double z() const {
                    return _z;
                }


                inline Coordinate& operator = (const Coordinate& coord) {
                    if (this != &coord) {
                        _x = coord._x;
                        _y = coord._y;
                        _z = coord._z;
                    }

                    return *this;
                }

            /*
                void operator = (Coordinate&& coord) {
                    _x = coord._x;
                    _y = coord._y;
                    _z = coord._z;
                }*/

                /**
                 * @brief flips the x and y of Coordinate
                 * @return geo::Coordinate coordinate
                 */
                Coordinate flipXY() const {
                    return Coordinate(this->y(), this->x(), this->z());
                }

                /**
                 * @brief Returns angle To the coordinate
                 * @param geo::Coordinate v
                 * @return double angle
                 */
                double angleTo(const Coordinate& v) const {
                    return (v - (*this)).angle();
                }

                /**
                 * @return The angle from between two vectors using the current vector as the center
                 * return 0, if the angle is not well defined
                 */
                double angleBetween(const Coordinate& v1, const Coordinate& v2) const;

                /**
                 * @brief checks for the equality of Coordinate
                 * @param geo::coordinate
                 * @return bool equality
                 */
                inline bool operator==(const Coordinate& coord) const {
                    return this->_x == coord._x && this->_y == coord._y && this->_z == coord._z;
                }

                inline bool operator!=(const Coordinate& coord) const {
                    return this->_x != coord._x || this->_y != coord._y || this->_z != coord._z;
                }

                /**
                  * Calculate the distance between this coordinate and another coordinate
                  * @return double
                  */
                inline double distanceTo(const geo::Coordinate& c) const {
                    return (*this - c).magnitude();
                }

                /**
                  * Add two points to each other and return a new coordinate : return Coordinate(_x + coord._x, _y + coord._y, _z + coord._z);
                  * @return Coordinate  Addition of two vectors
                  */
                inline Coordinate operator + (const Coordinate& coord) const {
                    return Coordinate(_x + coord._x, _y + coord._y, _z + coord._z);
                }

                /**
                 * @brief operator + for offset addition
                 * @param double offset
                 * @return geo::Coordinate
                 */
                inline Coordinate operator + (double d) const {
                    return Coordinate(_x + d, _y + d, _z + d);
                }
                
                /**
                 * @brief operator + for offset addition
                 * @param double offset
                 * @return geo::Coordinate
                 */
                inline Coordinate operator - (double d) const {
                    return Coordinate(_x - d, _y - d, _z - d);
                }

                /**
                 * @brief operator -
                 * @param
                 * @return geo::Coordinate
                 */
                inline Coordinate operator - () const {
                    return Coordinate(-_x, -_y, -_z);
                }

                /**
                  * Substract two points from each other and return a new coordinate : return _x - coord._x, _y - coord._y, _z - coord._z
                  * returns Coordinate
                  */
                inline Coordinate operator - (const Coordinate& coord) const {
                    return Coordinate(_x - coord._x, _y - coord._y, _z - coord._z);
                }

                /**
                  * Multiplication by Coordinate : return Coordinate(_x * s, _y * s, _z * s)
                  *
                  * @param Coordinate
                  * @return Coordinate
                  */
                inline Coordinate operator * (const Coordinate& coord) const {
                    return Coordinate(_x * coord._x, _y * coord._y, _z * coord._z);
                }

                inline Coordinate operator * (double s) const {
                    return Coordinate(_x * s, _y * s, _z * s);
                }
                /**
                  * Division by double : return Coordinate(_x / s, _y / s, _z / s)
                  *
                  * @param double
                  * @return Coordinate
                  */
                inline Coordinate operator / (double s) const {
                    return Coordinate(_x / s, _y / s, _z / s);
                }

                /**
                  * Magnitude or length of the point relative to zero : return sqrtf(_x * _x + _y * _y + _z * _z)
                  *
                  * @return double  Length of the vector
                  */
                inline double magnitude() const {
                    return std::sqrt(_x * _x + _y * _y + _z * _z);
                }

                inline double angle() const {
                    return std::atan2(_y, _x);
                }


                /**
                  * Squared of this Coordinate : return _x * _x + _y * _y + _z * _z;
                  *
                  * @return double   Squared value
                  */
                inline double squared() const {
                    return _x * _x + _y * _y + _z * _z;
                }

                /**
                  * Dot product with other coordindate : return _x * coord._x + _y * coord._y + _z * coord._z;
                  *
                  * @param Coordinate
                  * @return double   Dot product value
                  */
                inline double dot(const Coordinate& coord) const {
                    return _x * coord._x + _y * coord._y + _z * coord._z;
                }

                inline double dot(const Coordinate& v1, const Coordinate& v2) const {
                    return v1.x() * v2.x() + v1.y() * v2.y() + v1.z() * v2.z();
                }

                /**
                 * @brief rotate
                 * around (0.,0.) with a given angle vector
                 * @param angleVector
                 * @return
                 */
                inline Coordinate rotate(const Coordinate& angleVector) const {
                    double x0 = _x * angleVector.x() - _y * angleVector.y();
                    double y0 = _x * angleVector.y() + _y * angleVector.x();
                    return Coordinate(x0, y0);
                }

                /**
                 * @brief rotate
                 * around (0.,0.) with a given angle
                 * @param angle
                 * @return
                 */
                inline Coordinate rotate(const double& angle) const {
                    return rotate(Coordinate(angle));
                }

                /**
                 * @brief rotate
                 * around a point with a angle vector
                 * @param point
                 * @param angleVector
                 * @return
                 */
                inline Coordinate rotate(const geo::Coordinate& point, const Coordinate& angleVector) const {
                    return point + (*this - point).rotate(angleVector);
                }

                /**
                 * @brief rotate
                 * around a point with a angle
                 * @param point
                 * @param angle
                 * @return
                 */
                inline Coordinate rotate(const geo::Coordinate& point, const double& angle) const {
                    return rotate(point, Coordinate(angle));
                }

                /**
                * @brief rotate
                * around a point where the rotation described length is known
                * Example
                * @param point
                * @param length
                * @return
                */
                inline Coordinate rotateByArcLength(const geo::Coordinate& point, double const length) const {
                    double const angle = (length / point.distanceTo(*this));
                    return rotate(point, Coordinate(angle));
                }

                /**
                 * Scales the vector by given factors with 0/0 as center
                 */
                inline Coordinate scale(const double& scale_factor) const {
                    return Coordinate(_x * scale_factor, _y * scale_factor);
                }

                /**
                 * Scales the vector by given factors with 0/0 as center
                 */
                inline  Coordinate scale(const Coordinate& scale_factor) const {
                    return Coordinate(_x * scale_factor.x(), _y * scale_factor.y());
                }

                /**
                 * Scales this vector by the given factors with the given center.
                 */
                inline Coordinate scale(const Coordinate& scale_center, const Coordinate& scale_factor) const {
                    return scale_center + ((*this - scale_center) * scale_factor);
                }

                /**
                * Midpoint between two coordinates
                */
                inline Coordinate mid(const Coordinate& other) const {
                    return (other - *this) / 2. + *this;
                }
                /**
                 * Normalised version of this coordinate
                 */
                inline Coordinate norm() const {
                    auto m = magnitude();
                    return Coordinate(_x / m, _y / m, _z / m);
                }

                /**
                 * Normalised version of this coordinate with a factor
                 * The final Coordinate has a length of f
                 */
                inline Coordinate norm(const double f) const {
                    auto m = magnitude() / f;
                    return Coordinate(_x / m, _y / m, _z / m);
                }

                /**
                 * move the coordinate by distance d over vector direction
                */
                inline Coordinate move(const Coordinate& direction, double d) const {
                    return direction.norm() * d + *this;
                }

                /**
                 * Move the coordinate in the direction of point 'to' with distance d
                 */
                inline Coordinate moveTo(const Coordinate& to, double d) const {
                    return (to - *this).norm() * d + *this;
                }

                /**
                 * Apply transformation matrix to coordinates in X/Y only
                 * Note: this is a very naive approach to transform if we see
                 * ourselves using this function on large vectors of coordinates we should consider
                 * using Eigen
                 */
                inline Coordinate transform2d(double xx, double yx, double xy, double yy, double x0, double y0) {
                    return Coordinate(xx * _x + xy * _y + x0, yx * _x + yy * _y + y0, _z);
                }

                /**
                 * @brief mirror a coordinate
                 * @param axis1 : First point of line to be mirrored about
                 * @param axis2 : Second point of line to be mirrored about
                 * @return mirrored coordinate
                 */
                inline Coordinate mirror(const Coordinate& axis1, const Coordinate& axis2) const {
                    Coordinate dir(axis2 - axis1);
                    auto a= dir.squared();
                    auto ret= axis1 + dir* dot(*this- axis1, dir)/a; //projection point
                    return ret + ret - *this;
                }

            private:
                friend std::ostream& operator<<(std::ostream& os, const Coordinate& coordinate) {
                    os << "Coordinate(x=" << coordinate._x << " y=" << coordinate._y << " z=" << coordinate._z << ")";
                    return os;
                }

            private:
                double _x;
                double _y;
                double _z;
        };

        /*!
          * \brief sort a Collection in order of distance to a specific coordinate
          *
          * \note This routine is not very fast in the sense that it would re-calculate the distance for each comparison. It should still be fast enough to sort a collection of let's say 10.000 points
          *
          * \code
          * std::vector<geo::Coordinate> myList;
          * myList.push_back(geo::Coordinate(50.,50.));
          * myList.push_back(geo::Coordinate(20.,20.));
          * myList.push_back(geo::Coordinate(120.,120.));
          * std::qsort(sp.begin(), sp.end(), geo::CoordinateDistanceSort(geo::Coordinate(60.60.)));
          * \endcode
          */
        struct CoordinateDistanceSort {
                CoordinateDistanceSort(const Coordinate& distanceFrom) : _distanceFrom(distanceFrom) {
                }

                bool operator()(const Coordinate& left, const Coordinate& right) const {
                    return left.distanceTo(_distanceFrom) < right.distanceTo(_distanceFrom);
                }

                const Coordinate _distanceFrom;
        };

    }
}

