#pragma once

#include "cad/const.h"
#include "geoarea.h"
#include "geocoordinate.h"
#include "geobase.h"
#include <vector>
#include "cad/math/equation.h"

namespace lc {
    namespace geo {

        class Arc  : public Base, virtual public Visitable {
            public:

                /**
                 * Create a new Arc
                 * center center location
                 * radius radius of the arc
                 * startAngle, will be corrected to 0..2*PI
                 * endAngle, will be corrected to 0..2*PI
                 *
                 * Arc will always be setup such that startAngle will increase or decrease to endAngle
                 * So it's perfectly possible to have a startAngle > endAngle
                 */
                Arc(const Coordinate& center, double radius, double startAngle, double endAngle);
                /**
                 * Create a new Arc
                 * center center location
                 * radius radius of the arc
                 * startAngle, will be corrected to 0..2*PI
                 * endAngle, will be corrected to 0..2*PI
                 * CCW, setup arc to be drawn ClockWise or counter clockwise
                 */
                Arc(const Coordinate& center, double radius, double startAngle, double endAngle, bool CCW);

                static Arc createArc3P(const Coordinate& p1, const Coordinate& p2, const Coordinate& p3);
                static Arc createArcBulge(const Coordinate& p1, const Coordinate& p2, const double bulge);

                Arc(const Arc &c) : _center(c._center), _radius(c._radius), _startAngle(c._startAngle), _endAngle(c._endAngle), _CCW(c._CCW) {}
                Arc(Arc &&c) noexcept {
                    std::swap(_center, c._center);
                    std::swap(_radius, c._radius);
                    std::swap(_startAngle, c._startAngle);
                    std::swap(_endAngle, c._endAngle);
                    std::swap(_CCW, c._CCW);
                }

                Arc& operator = (Arc&& c) noexcept {
                    std::swap(_center, c._center);
                    std::swap(_radius, c._radius);
                    std::swap(_startAngle, c._startAngle);
                    std::swap(_endAngle, c._endAngle);
                    std::swap(_CCW, c._CCW);
                    return *this;
                }


            /**
                 * @brief Returns center of Arc
                 * @return geo::Coordinate Center
                 */
                const Coordinate center() const;

                /*!
                 * \brief Returns the radius of Arc.
                 * \return Double Radius.
                 */
                double radius() const;
                /*!
                 * \brief Returns the startAngle
                 * \return Double startAngle.
                 */
                double startAngle() const;
                /*!
                 * \brief Returns the EndAngle
                 * \return Double EndAngle.
                 */
                double endAngle() const;

                /**
                * Calculates arc length
                * This assumes that startAngle and endAngle are between -M_PI and M_PI and the difference is not bigger then 2.*M_PI
                * TODO: Should we make this more reliable, for example to have angle is 4*MPI it's still correctly calculated?
                */
                double length() const;

                /**
                * Coordinate of the start point
                */
                Coordinate startP() const;

                /**
                * Coordinate of the end point
                */
                Coordinate endP() const;

                /**
                * Tight bounding box for the area
                */
                Area boundingBox() const;

                /*!
                 * \brief Returns of the arc is in reversed direction
                 * \return Double reversed.
                 */
                bool CCW() const;

                Coordinate nearestPointOnPath(const Coordinate& coord) const;
                Coordinate nearestPointOnEntity(const Coordinate& coord) const;

                const maths::Equation equation() const {
                    return maths::Equation(1., 0.,1., 0., 0., -_radius* _radius).move(_center);
                }


                // test of the given angle is between the start and end angle of this arc.
                bool isAngleBetween(double angle) const;

                virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }
            private:
                friend std::ostream& operator<<(std::ostream& os, const Arc& a) {
                    os << "Arc(center=" << a._center << " radius=" << a._radius << " startAngle=" << a._startAngle << " endAngle=" << a._endAngle << " ccw=" << a._CCW << ")";
                    return os;
                }

            private:
                Coordinate _center; /*!< Coordinate center of Arc */
                double _radius; /*!< Double _Radius of Arc */
                double _startAngle;/*!< Double startAngle of Arc */
                double _endAngle;/*!< Double endAngle of Arc */
                bool _CCW;
        };
    }
}
