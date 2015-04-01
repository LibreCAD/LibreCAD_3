#pragma once

#include "geobase.h"
#include "geocoordinate.h"
#include <vector>
#include "cad/base/visitor.h"

namespace lc {
    namespace geo {
        class Spline : public Base, virtual public Visitable {
            public:

                Spline(const std::vector<Coordinate> &control_points,
                       const std::vector<double> &knotPoints,
                       const std::vector<Coordinate> &fitPoints,
                       int degree, bool closed, double fitTolerance,
                       double stanx, double stany, double stanz,
                       double etanx, double etany, double etanz,
                       double nx, double ny, double nz) :
                        Base(),
                        _controlPoints(control_points),
                        _knotPoints(knotPoints),
                        _fitPoints(fitPoints),
                        _degree(degree), _closed(closed),
                        _fitTolerance(fitTolerance),
                        _sTanX(stanx), _sTanY(stany), _sTanZ(stanz),
                        _eTanX(etanx), _eTanY(etany), _eTanZ(etanz),
                        _nX(nx), _nY(ny), _nZ(nz) {}

                /**
                 * @brief control_points, Returns Control points of spline
                 * @return vector<Coordinate> controlpoints
                 */
                const std::vector<Coordinate> &controlPoints() const;

                /**
                 * @brief degree, Returns degree of spline
                 * @return int degree
                 */
                short degree() const;
                /**
                * @brief Return a vector of knotpoints
                * @return vector of double
                */
                const std::vector<double> &knotPoints() const;

                /**
                * @brief Return a vector of fitpoints
                * @return vector of Coordinate
                */
                const std::vector<Coordinate> &fitPoints() const;

                double fitTolerance() const;

            // start tangent x coordinate
                double startTanX() const;
                // start tangent y coordinate
                double startTanY() const;
                // start tangent z coordinate
                double startTanZ() const;

                // end tangent x coordinate
                double endTanX() const;
                // end tangent y coordinate
                double endTanY() const;
                // end tangent z coordinate
                double endTanZ() const;

                // normal vector x coordinate
                double nX() const;
                // normal vector y coordinate
                double nY() const;
                // normal vector z coordinate
                double nZ() const;
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
                Coordinate nearestPointOnPath(const Coordinate &coord) const;

                /*!
                 * \brief checks if Coordinate is on path
                 * \param lc::geo::Coordinate coord
                 * \return bool
                 */
                bool isCoordinateOnPath(const Coordinate &coord) const;

                virtual void accept(GeoEntityVisitor &v) const override { v.visit(*this); }

            private:
                const std::vector<Coordinate> _controlPoints;
                const std::vector<double> _knotPoints;
                const std::vector<Coordinate> _fitPoints;

                const short _degree;
                const bool _closed;

                const double _fitTolerance;

                const double _sTanX; // start tangent x coordinate
                const double _sTanY; // start tangent y coordinate
                const double _sTanZ; // start tangent z coordinate

                const double _eTanX; // end tangent x coordinate
                const double _eTanY; // end tangent y coordinate
                const double _eTanZ; // end tangent z coordinate

                const double _nX;  // normal vector x coordinate
                const double _nY;  // normal vector y coordinate
                const double _nZ;  // normal vector z coordinate

        };
    }
}
