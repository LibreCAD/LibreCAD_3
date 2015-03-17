#pragma once

#include "cad/const.h"
#include "geocoordinate.h"
#include "geobase.h"

namespace lc {
    namespace geo {
        class Area;

        class Arc  : public Base {
            public:
                Arc(const Coordinate& center, double radius, double startAngle, double endAngle);
                Arc(const Coordinate& center, double radius, double startAngle, double endAngle, bool reversed);


                static Arc createArc3P(const Coordinate& p1, const Coordinate& p2, const Coordinate& p3);
                static Arc createArcBulge(const Coordinate& p1, const Coordinate& p2, const double bulge);


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

            private:
                friend std::ostream& operator<<(std::ostream& os, const Arc& a) {
                    os << "Arc(center=" << a._center << " radius=" << a._radius << " startAngle=" << a._startAngle << " endAngle=" << a._endAngle << ")";
                    return os;
                }

            private:
                const Coordinate _center; /*!< Coordinate center of Arc */
                const double _radius; /*!< Double _Radius of Arc */
                const double _startAngle;/*!< Double startAngle of Arc */
                const double _endAngle;/*!< Double endAngle of Arc */
                const bool _reversed;
        };
    }
}
