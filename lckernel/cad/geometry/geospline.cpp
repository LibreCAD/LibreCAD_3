#include "geospline.h"
#include <tinysplinecxx.cxx>
#include <cad/logger/logger.h>
#include <algorithm>
#include <cmath>

using namespace lc;
using namespace geo;

const std::vector<Coordinate> &Spline::controlPoints() const {
    return _controlPoints;
}

bool Spline::closed() const {
    return _closed;
}

short Spline::degree() const {
    return _degree;
}

const std::vector<double> &Spline::knotPoints() const {
    return _knotPoints;
}

const std::vector<Coordinate> &Spline::fitPoints() const {
    return _fitPoints;
}


double Spline::startTanX() const {
    return _sTanX;
}

double Spline::startTanY() const {
    return _sTanY;
}

double Spline::startTanZ() const {
    return _sTanZ;

}

double Spline::endTanX() const {
    return _eTanX;
}

double Spline::endTanY() const {
    return _eTanY;
}

double Spline::endTanZ() const {
    return _eTanZ;
}

double Spline::nX() const {
    return _nX;
}
double Spline::nY() const {
    return _nY;
}
double Spline::nZ() const {
    return _nZ;
}
Spline::splineflag Spline::flags() const {
    return _flags;
}
double Spline::fitTolerance() const {
    return _fitTolerance;
}

Coordinate Spline::nearestPointOnPath(const Coordinate &coord) const {
    /// @todo implement
    /// @todo fix compiler warning
    return Coordinate(coord);//Force the intersection for now
}
Coordinate Spline::nearestPointOnEntity(const Coordinate &coord) const {
    /// @todo implement
    /// @todo fix compiler warning
    return Coordinate();
}

void Spline::populateCurve() {
    tinyspline::BSpline splineCurve;
    tinyspline::BSpline beziers;
    bool converted = false;

    try {
        auto nbControlPoints = _controlPoints.size();
        splineCurve = tinyspline::BSpline(nbControlPoints, 3, degree(), tinyspline::BSpline::Type::Clamped);
        //Set control points
        std::vector<tinyspline::real> ts_controlPoints;
        for (const auto& cp : _controlPoints) {
            ts_controlPoints.push_back(cp.x());
            ts_controlPoints.push_back(cp.y());
            ts_controlPoints.push_back(cp.z());
        }
        splineCurve.setControlPoints(ts_controlPoints);
        // Rational and periodic curve interpretation needs a separate model.
        // Use source knots for the bounded open-clamped profile. Preserve the
        // existing fallback for other profiles or TinySpline conversion errors.
        const bool ordinary = (_flags & (RATIONAL | PERIODIC)) == 0;
        const std::size_t order = static_cast<std::size_t>(degree()) + 1u;
        const bool validKnots = ordinary && !_knotPoints.empty()
            && _knotPoints.size() == nbControlPoints + order
            && std::all_of(_knotPoints.begin(), _knotPoints.end(),
                           [](double knot) { return std::isfinite(knot); })
            && std::is_sorted(_knotPoints.begin(), _knotPoints.end())
            && _knotPoints[order - 1u] < _knotPoints[nbControlPoints]
            && std::isfinite(_knotPoints[nbControlPoints] - _knotPoints[order - 1u])
            && _knotPoints.front() == _knotPoints[order - 1u]
            && _knotPoints[nbControlPoints] == _knotPoints.back()
            && std::adjacent_find(_knotPoints.begin(), _knotPoints.end(),
                 [](double a, double b) { return a < b &&
                     (!std::isfinite(b - a) || b - a < TS_KNOT_EPSILON); })
               == _knotPoints.end();
        if (validKnots) {
            try {
                auto candidate = splineCurve;
                candidate.setKnots(_knotPoints);
                beziers = candidate.toBeziers();
                splineCurve = candidate;
                converted = true;
            } catch (const std::runtime_error& e) {
                LOG_ERROR << "Unable to apply spline knots: " << e.what() << std::endl;
            }
        }
    }
    catch (std::runtime_error& e) {
        LOG_ERROR << e.what() << std::endl;
        splineCurve = tinyspline::BSpline();
    }

    if (!converted) {
        beziers = splineCurve.toBeziers();
    }

    int nbBeziers = beziers.numControlPoints() / splineCurve.order();
    int nbCoordinate = splineCurve.order() * splineCurve.dimension();

    auto controlPoints = beziers.controlPoints();

    if(splineCurve.degree() == 2) {
        for (int i = 0; i < nbBeziers; i++) {
            std::vector<geo::Coordinate> bez;

            auto j = nbCoordinate * i;
            auto end = nbCoordinate * (i+1);

            while(j < end) {
                lc::geo::Coordinate cp(controlPoints[j], controlPoints[j+1], controlPoints[j+2]);
                bez.push_back(cp);
                j = j + 3;
            }
            _beziers.push_back(std::make_shared<Bezier>(bez.at(0),bez.at(1),bez.at(2)));
        }
    } else if(splineCurve.degree() == 3) {
        for (int i = 0; i < nbBeziers; i++) {
            std::vector<geo::Coordinate> bez;

            auto j = nbCoordinate * i;
            auto end = nbCoordinate * (i+1);

            while(j < end) {
                lc::geo::Coordinate cp(controlPoints[j], controlPoints[j+1], controlPoints[j+2]);
                bez.push_back(cp);

                j = j + 3;
            }

            _beziers.push_back(std::make_shared<CubicBezier>(bez.at(0),bez.at(1),bez.at(2),bez.at(3)));
        }
    }
}

const std::vector<BB_CSPtr> Spline::beziers() const {
    return _beziers;
}

void Spline::trimAtPoint(const geo::Coordinate& c) {
    /// @todo implement
}
