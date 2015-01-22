#pragma once

#include "geocoordinate.h"
#include <vector>

namespace lc {
    namespace geo {
        class Spline {
            public:

                Spline(const std::vector<Coordinate>& control_points, int degree, bool closed);


                Spline(const std::vector<Coordinate>& control_points,
                       const std::vector<Coordinate>& knotPoints, int degree,
                       bool closed);

                Spline(const std::vector<Coordinate>& control_points,
                       const std::vector<Coordinate>& knotPoints,
                       const std::vector<Coordinate>& fitPoints, int degree,
                       bool closed);

                Spline(const std::vector<Coordinate>& control_points,
                       const std::vector<Coordinate>& knotPoints,
                       const std::vector<Coordinate>& fitPoints,
                       int degree, bool closed, double stanx, double stany,
                       double stanz, double etanx, double etany, double etanz);

                /**
                 * @brief control_points, Returns Control points of spline
                 * @return vector<Coordinate> controlpoints
                 */
                const std::vector<Coordinate> controlPoints() const;

                /**
                 * @brief degree, Returns degree of spline
                 * @return int degree
                 */
                int degree() const;
                const std::vector<Coordinate> knotPoints() const;
                const std::vector<Coordinate> fitPoints() const;
                double startTanX() const;
                double startTanY() const;
                double startTanZ() const;
                double endTanX() const;
                double endTanY() const;
                double endTanZ() const;
                int numberKnots() const;
                int numberFits() const;
                int numberControlPoints() const;
                double knotTolerance() const; // (default = 0.0000001)
                double controlPointTolerance() const; // (default = 0.0000001)
                double fitTolerance() const; //(default = 0.0000000001)

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
                const std::vector<Coordinate> _knotPoints; /*!< center of cirlce */
                const std::vector<Coordinate> _fitPoints; /*!< center of cirlce */
                const std::vector<Coordinate> _controlPoints; /*!< center of cirlce */
                double _sTanX, _sTanY, _sTanZ;
                double _eTanX, _eTanY, _eTanZ;
                const int _degree; /*!< Radius of Spline. */
                int _noKnots;
                int _noFits;
                int _noControlPoints;
                double _knotTolerance; // (default = 0.0000001)
                double _controlPointTolerance; // (default = 0.0000001)
                double _fitTolerance; //(default = 0.0000000001)
                const bool _closed;
        };
    }
}
