#ifndef GEOCOORDINATE_H
#define GEOCOORDINATE_H

#include <QPointF>
#include "qdebug.h"

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
                Coordinate(const QPointF& qtPoint);  // Convenience

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
                  * Magnitude or lenth of the point relative to zero : return sqrt(_x * _x + _y * _y + _z * _z)
                  *
                  * @return double  Length of the vector
                  */
                double magnitude() const;

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

                /**
                  * Multiplication by double : return Coordinate(_x * s, _y * s, _z * s)
                  *
                  * @param double
                  * @return Coordinate
                  */
                Coordinate operator * (double s) const;

                /**
                  * Division by double : return Coordinate(_x / s, _y / s, _z / s)
                  *
                  * @param dobule
                  * @return Coordinate
                  */
                Coordinate operator / (double s) const;


            private:
                double _x;
                double _y;
                double _z;
        };
    }
}

/**
  * Used for debugging for example qDebug() << "my Coordinate:" << point;
  * outputs:
  * My Coordinate:(1,2,3)
  *
  */
QDebug operator << (QDebug dbg, const lc::geo::Coordinate& c);

#endif // GEOCOORDINATE_H
