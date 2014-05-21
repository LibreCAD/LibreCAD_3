#ifndef GEOCOORDINATE_H
#define GEOCOORDINATE_H

#include "cad/const.h"

namespace lc {
    namespace geo {
        /**
          * Class that represent a coordinate in space
          */
        class Coordinate {
            public:
                Coordinate() {
                    _x = _y = _z = 0.0;
                }
                Coordinate(double x, double y, double z);
                Coordinate(double x, double y);
                Coordinate(double angle) ;
                double x() const;
                double y() const;
                double z() const;

            public:

                Coordinate& operator = (const Coordinate& coord) {
                    if (this != &coord) {
                        _x = coord._x;
                        _y = coord._y;
                        _z = coord._z;
                    }

                    return *this;
                }

                bool operator==(const Coordinate& coord) const {
                    return this->_x == coord._x && this->_y == coord._y && this->_z == coord._z;
                }
                bool operator!=(const Coordinate& coord) const {
                    return !(*this == coord);
                }

                /**
                  * Calculate the distance between this coordinate and a other coordinate
                  * @return double
                  */
                double distanceTo(const Coordinate& coordinate) const;

                /**
                  * Add two points to each other and return a new coordinate : return Coordinate(_x + coord._x, _y + coord._y, _z + coord._z);
                  * @return Coordinate  Addition of two vectors
                  */
                Coordinate operator + (const Coordinate& coordinate) const;

                /**
                  * Substract two points from each other and return a new coordinate : return _x - coord._x, _y - coord._y, _z - coord._z
                  * returns Coordinate
                  */
                Coordinate operator - (const Coordinate& coordinate) const;

                /**
                  * Multiplication by Coordinate : return Coordinate(_x * s, _y * s, _z * s)
                  *
                  * @param Coordinate
                  * @return Coordinate
                  */
                Coordinate operator * (const Coordinate& coord) const;

                Coordinate operator * (double s) const;
                /**
                  * Division by double : return Coordinate(_x / s, _y / s, _z / s)
                  *
                  * @param double
                  * @return Coordinate
                  */
                Coordinate operator / (double s) const;

                /**
                  * Magnitude or lenth of the point relative to zero : return sqrtf(_x * _x + _y * _y + _z * _z)
                  *
                  * @return double  Length of the vector
                  */
                double magnitude() const;

                double angle() const;

                /**
                  * Quared of this Coordinate : return _x * _x + _y * _y + _z * _z;
                  *
                  * @return double   Quared value
                  */
                double squared() const;

                /**
                  * Dot product with other coordindate : return _x * coord._x + _y * coord._y + _z * coord._z;
                  *
                  * @param Coordinate
                  * @return double   Dot product value
                  */
                double dot(const Coordinate& coord) const;

                Coordinate rotate(const Coordinate& angleVector) const;
                Coordinate rotate(const double& angle) const;
                Coordinate rotate(const geo::Coordinate& point, const Coordinate& angleVector) const;
                Coordinate rotate(const geo::Coordinate& point, const double& angle) const;
                Coordinate scale(const double& scale_factor) const;
                Coordinate scale(const Coordinate& scale_center, const Coordinate& scale_factor) const;
                Coordinate scale(const Coordinate& scale_factor) const;

            private:
                double _x;
                double _y;
                double _z;
        };

        /*!
          * \brief sort a Collection in order of distance to a specific coordinate
          *
          * \note This routine is not very fast in the sense that it would re-calculate the distance for each comparison. It shouls still be fast enough to sort a collection of let's say 10.000 points
          *
          * \code
          * std::vector<geo::Coordinate> myList;
          * myList.push_back(geo::Coordinate(50.,50.));
          * myList.push_back(geo::Coordinate(20.,20.));
          * myList.push_back(geo::Coordinate(120.,120.));
          * std::qsort(sp.begin(), sp.end(), geo::CoordinateDistanceSort(geo::Coordinate(60.60.)));
          * \endcode
          */
        class CoordinateDistanceSort {
            public:
                CoordinateDistanceSort(const Coordinate& distanceFrom);
                bool operator()(const Coordinate& left, const Coordinate& right) const;

            private:
                Coordinate _distanceFrom;
        };

    }
}

#endif // GEOCOORDINATE_H
