#pragma once

#include "cad/const.h"
#include "geocoordinate.h"
#include "geobase.h"
#include <vector>
#include "cad/math/quadratic_math.h"


namespace lc {
    namespace geo {
        class Area;

        class Arc  : public Base {
            public:
                Arc(const Coordinate& center, double radius, double startAngle, double endAngle);
                Arc(const Coordinate& center, double radius, double startAngle, double endAngle, bool reversed);

                static Arc createArc3P(const Coordinate& p1, const Coordinate& p2, const Coordinate& p3);
                static Arc createArcBulge(const Coordinate& p1, const Coordinate& p2, const double bulge);

                Arc(const Arc &c) : _center(c._center), _radius(c._radius), _startAngle(c._startAngle), _endAngle(c._endAngle), _reversed(c._reversed) {}
                Arc(Arc &&c) noexcept {
                    std::swap(_center, c._center);
                    std::swap(_radius, c._radius);
                    std::swap(_startAngle, c._startAngle);
                    std::swap(_endAngle, c._endAngle);
                    std::swap(_reversed, c._reversed);
                }

                Arc& operator = (Arc&& c) noexcept {
                    std::swap(_center, c._center);
                    std::swap(_radius, c._radius);
                    std::swap(_startAngle, c._startAngle);
                    std::swap(_endAngle, c._endAngle);
                    std::swap(_reversed, c._reversed);
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
                double reversed() const;

                Coordinate nearestPointOnPath(const Coordinate& coord) const;
                bool isCoordinateOnPath(const Coordinate& coord) const;

                /**
                 * @brief quadratic, Returns quadratic for the entity
                 * @return Quadratic quadratic equation
                 */
                Quadratic quadratic() const {
                    std::vector<double> ce(6, 0.);
                    ce[0] = 1.;
                    ce[2] = 1.;
                    ce[5] = -this->radius() * this->radius();
                    Quadratic ret(ce);
                    ret.move(this->center());
                    return ret;
                }
            private:
                friend std::ostream& operator<<(std::ostream& os, const Arc& a) {
                    os << "Arc(center=" << a._center << " radius=" << a._radius << " startAngle=" << a._startAngle << " endAngle=" << a._endAngle << ")";
                    return os;
                }

            private:
                Coordinate _center; /*!< Coordinate center of Arc */
                double _radius; /*!< Double _Radius of Arc */
                double _startAngle;/*!< Double startAngle of Arc */
                double _endAngle;/*!< Double endAngle of Arc */
                bool _reversed;
        };
    }
}
