#ifndef GEOELLIPSE_H
#define GEOELLIPSE_H

#include "cad/const.h"
#include "geocoordinate.h"

namespace lc {
    namespace geo {
        class Ellipse  {
            public:
                Ellipse(const Coordinate& center, const Coordinate& majorP, double minorRadius, double startAngle, double endAngle, bool isArc = false);

                const Coordinate& center() const;
                const Coordinate& majorP() const;
                double minorRadius() const;
                double startAngle() const;
                double endAngle() const;

                /**
                 * @brief getPoint, return a point on ellipse with given elliptic angle
                 * @param angle, elliptic angle in radians
                 * @return coordinates of a point on ellipse with given elliptic angle
                 */
                Coordinate getPoint(const double& angle) const;
                /**
                 * @brief startPoint, start point of ellipse
                 * @return for elliptic arc, return the startPoint, otherwise return end of majorP
                 */
                Coordinate startPoint() const;
                /**
                 * @brief endPoint, end point of ellipse
                 * @return for elliptic arc, return the endPoint, otherwise return end of majorP
                 */
                Coordinate endPoint() const;

                Coordinate nearestPointOnPath(const Coordinate& coord) const;
                bool isCoordinateOnPath(const Coordinate& coord) const;
                /**
                 * @brief isArc
                 * @return true if entity is an elliptic arc
                 */
                bool isArc() const;
                /**
                 * @brief getEllipseAngle
                 * @param coord, a point on ellipse, not actually required to be on ellipse
                 * @return elliptic angle to the position of coord
                 */
                double getEllipseAngle(const Coordinate& coord) const;

            private:
                const Coordinate _center;
                const Coordinate _majorP;
                double _minorRadius;
                double _startAngle;
                double _endAngle;
                bool _isArc;
        };
    }
}

#endif // GEOELLIPSE_H
