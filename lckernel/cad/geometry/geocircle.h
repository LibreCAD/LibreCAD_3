#pragma once

#include "cad/const.h"
#include "geocoordinate.h"

namespace lc {
    namespace geo {
        // TODO: Should Circle be a arc with a start angle of 0 and a stop angle of 2PI ?
        class Circle {
            public:
                Circle(const Coordinate& center, double radius);

                /*!
                 * \brief Returns the Center of circle.
                 * \return lc::geo::Coordinate Center.
                 */
                const Coordinate center() const;

                /*!
                 * \brief returns the radius of the circle.
                 * \return Double radius.
                 */
                double radius() const;

                /*!
                 * \brief returns the nearest Point On Path
                 * \param lc::geo::Coordinate coord
                 * \return lc::geo::Coordinate
                 */
                Coordinate nearestPointOnPath(const Coordinate& coord) const;

                /*!
                 * \brief checks if Coordinate is on path
                 * \param lc::geo::Coordinate coord
                 * \return bool
                 */
                bool isCoordinateOnPath(const Coordinate& coord) const;

            private:
                const Coordinate _center; /*!< center of cirlce */
                const double _radius; /*!< Radius of circle. */
        };
    }
}
