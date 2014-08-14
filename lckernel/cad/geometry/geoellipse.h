#pragma once

#include "cad/const.h"
#include "geocoordinate.h"
#include "cad/math/lcmath.h"
#include <memory>
namespace lc {

    class CADEntity;
    typedef std::shared_ptr<CADEntity> CADEntity_SPtr;
    typedef std::shared_ptr<const CADEntity> CADEntity_CSPtr;
    namespace geo {
        class Ellipse  {
            public:
                Ellipse(const Coordinate& center, const Coordinate& majorP, double minorRadius, double startAngle, double endAngle, bool isArc = false);

                /**
                 * @brief center, Returns Center point of Ellipse
                 * @return geo::Coordinate center
                 */
                const Coordinate center() const;

                /**
                 * @brief majorP, Returns major point of the ellipse
                 * @return geo::Coordinate majorP
                 */
                const Coordinate majorP() const;

                /**
                 * @brief minorRadius, Returns the minor radius of ellipse
                 * @return double minor radius
                 */
                double minorRadius() const;

                /**
                 * @brief startAngle, Returns Startangle of ellipse
                 * @return double startangle
                 */
                double startAngle() const;

                /**
                 * @brief endAngle, Return the end angle of ellipse
                 * @return double endangle
                 */
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
                bool isCoordinateOnPath(const Coordinate& coord, double tolerance) const;
                /**
                 * @brief isArc
                 * @return true if entity is an elliptic arc
                 */
                bool isArc() const;

                /**
                 * @retval true if the arc is reversed (clockwise),
                 * @retval false otherwise
                 */
                bool isReversed() const;

                /**
                 * @brief ratio of major radius to minor radius
                 * @return double ratio
                 */
                double ratio() const;

                /**
                 * @brief Major Radius
                 * @return double Major radius
                 */
                double majorRadius() const;

                /**
                 * @brief getAngle of MajorP
                 * @return double majorP angle
                 */
                double getAngle() const;

                /**
                 * @brief getEllipseAngle
                 * @param coord, a point on ellipse, not actually required to be on ellipse
                 * @return elliptic angle to the position of coord
                 */
                double getEllipseAngle(const Coordinate& coord) const;

            private:
                const Coordinate _center, _majorP;
                const double _minorRadius, _startAngle, _endAngle;
                const bool _isArc, _isReversed;
        };
    }
}

// GEOELLIPSE_H
