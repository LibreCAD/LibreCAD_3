#pragma once

#include "cad/const.h"
#include "geocoordinate.h"
#include "cad/math/lcmath.h"
#include <memory>
#include "geobase.h"
#include <vector>
#include "cad/math/equation.h"

namespace lc {

    namespace geo {
        class Ellipse : public Base, virtual public Visitable {
            public:
                Ellipse(const Coordinate& center, const Coordinate& majorP, double minorRadius, double startAngle, double endAngle);
                Ellipse(const Coordinate& center, const Coordinate& majorP, double minorRadius, double startAngle, double endAngle, bool reversed);
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
                Coordinate nearestPointOnEntity(const Coordinate& coord) const;
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

                bool isAngleBetween(double angle) const {
                    return Math::isAngleBetween(angle, _startAngle, _endAngle, !_isReversed);
                }

                /**
                 * @brief rotate an ellipse at a center by an angle
                 * @param center of rotation
                 * @param rotation_angle
                 * @return Rotated Ellipse
                 */
                Ellipse georotate(const Coordinate& center, const double rotation_angle) const;

                /**
                 * @brief scale an ellipse at some center by some factor
                 * @param center of scaling
                 * @param factor of scaling
                 * @return scaled ellipse
                 */
                Ellipse geoscale(const Coordinate& center, const Coordinate &factor) const;

                /**
                 * @brief Returns the quadratic equation
                 * @return Equation
                 */
                const maths::Equation equation() const {
                    auto ce0 = _majorP.squared();
                    auto ce2 = this->ratio() * this->ratio() * ce0;

                    if (ce0 < LCTOLERANCE * LCTOLERANCE || ce2 < LCTOLERANCE * LCTOLERANCE) {
                        return maths::Equation();
                    }

                    return maths::Equation(1. / ce0, 0., 1. / ce2, 0., 0., -1.).rotate(getAngle()).move(_center);
                }

                virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

            private:
                friend std::ostream& operator<<(std::ostream& os, const Ellipse& e) {
                    os << "Ellipse(center=" << e._center << " majorP=" << e._majorP << " minorRadius=" << e._minorRadius << " startAngle=" << e._startAngle << " endAngle=" << e._endAngle << " isReversed=" << e._isReversed << ")";
                    return os;
                }
            private:
                const Coordinate _center;
                const Coordinate _majorP;
                const double _minorRadius;
                const double  _startAngle;
                const double _endAngle;
                const bool _isReversed;
        };
    }
}

// GEOELLIPSE_H
