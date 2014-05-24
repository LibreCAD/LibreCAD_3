#ifndef GEOAREA_H
#define GEOAREA_H

#include "cad/const.h"

#include "geocoordinate.h"

namespace lc {
    namespace geo {
        /**
          * Class that describes a area or window.
          */
        class Area {
            public:
                /**
                  * Create a new Area. The coordinates coordA and coordB will be ordered so that minP wil always be < maxP
                  * The corordinates are not allowed to describe a volume
                  *
                  * @param CoordA First coordinate of a area
                  * @param CoordB Second coordinate of a area
                  */
                explicit Area(const Coordinate& coordA, const Coordinate& coordB) :
                    _minP(Coordinate(std::min(coordA.x(), coordB.x()), std::min(coordA.y(), coordB.y()), std::min(coordA.z(), coordB.z()))),
                    _maxP(Coordinate(std::max(coordA.x(), coordB.x()), std::max(coordA.y(), coordB.y()), std::max(coordA.z(), coordB.z()))) {
                    if (coordA.x() != coordB.x() && coordA.y() != coordB.y() && coordA.z() != coordB.z()) {
                        throw "Points describe a volume, not a area.";
                    }

                }

                explicit Area() : _minP(0., 0.), _maxP(0., 0.) {
                }

                /**
                 * @brief Area
                 * given at a coordinate with a given width and height
                 * @param coordA
                 * @param width
                 * @param height
                 */
                explicit Area(const Coordinate& coord, double width, double height) : _minP(coord), _maxP(coord.x() + std::abs(width), coord.y() + std::abs(height)) {
                }

                //explicit Area(const Area &a) : _minP(a._minP), _maxP(a._maxP) {}

                inline Area& operator = (const Area& other) {
                    if (this != &other) {
                        _minP = other._minP;
                        _maxP = other._maxP;
                    }

                    return *this;
                }

                /**
                  * Return the smalles corner (closest to (0,0,0) )
                  */
                inline const Coordinate minP() const {
                    return _minP;
                }

                /**
                  * Return the heigest corner
                  */
                inline const Coordinate maxP() const {
                    return _maxP;
                }

                /**
                 * @brief width
                 * Returns the wid th of this area
                 * @return
                 */
                inline double width() const {
                    return _maxP.x() - _minP.x();
                }

                /**
                 * @brief height
                 * Returns the height f this area
                 * @return
                 */
                inline double height() const {
                    return _maxP.y() - _minP.y();
                }

                /**
                  * Test of a specific point lies within a area
                  * @param point Point to test agains
                  * @return boolean true of the point is within the area
                  */
                inline bool inArea(const Coordinate& point) const {
                    return (point.x() >= _minP.x() && point.x() <= _maxP.x() && point.y() >= _minP.y() && point.y() <= _maxP.y());
                }

                /**
                 * @brief merge
                 * two area's and expend if required to largest area
                 * @param other
                 * @return
                 */
                inline Area merge(const Area& other) const {
                    return Area(
                               Coordinate(std::min(other.minP().x(), this->minP().x()), std::min(other.minP().y(), this->minP().y())),
                               Coordinate(std::max(other.maxP().x(), this->maxP().x()), std::max(other.maxP().y(), this->maxP().y())));

                }

            private:
                Coordinate _minP;
                Coordinate _maxP;
        };
    }
}
#endif // GEOAREA_H
