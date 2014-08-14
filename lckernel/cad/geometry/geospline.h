#pragma once

#include "cad/const.h"
#include "geocoordinate.h"

namespace lc {
    namespace geo {
        class Spline {
            public:
                Spline(const std::vector<Coordinate>& control_points, int degree, bool closed);


                /**
                 * @brief control_points, Returns Control points of spline
                 * @return vector<Coordinate> controlpoints
                 */
                const std::vector<Coordinate> control_points() const;

                /**
                 * @brief degree, Returns degree of spline
                 * @return int degree
                 */
                int degree() const;

                /**
                 * @brief closed, returns if spline is closed or not
                 * @return bool closed
                 */
                bool closed() const;

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
                const std::vector<Coordinate> _control_points; /*!< center of cirlce */
                const int _degree; /*!< Radius of Spline. */
                const bool _closed;
        };
    }
}
