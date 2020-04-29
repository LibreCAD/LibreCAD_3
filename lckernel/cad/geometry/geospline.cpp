#include "geospline.h"
#include <tinysplinecxx.cxx>
#include <cad/logger/logger.h>

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
    /* TODO implement
     * fix compiler warning
     */
    return Coordinate(coord);//Force the intersection for now
}
Coordinate Spline::nearestPointOnEntity(const Coordinate &coord) const {
    /* TODO implement
     * fix compiler warning
     */
    return Coordinate();
}

void Spline::populateCurve() {
    tinyspline::BSpline splineCurve;

    try {
        auto nbControlPoints = _controlPoints.size();

        splineCurve = tinyspline::BSpline(nbControlPoints, 3, degree(), TS_CLAMPED);

        //Set control points
        std::vector<tinyspline::real> ts_controlPoints;
        for (const auto& cp : _controlPoints) {
            ts_controlPoints.push_back(cp.x());
            ts_controlPoints.push_back(cp.y());
            ts_controlPoints.push_back(cp.z());
        }

        splineCurve.setControlPoints(ts_controlPoints);
    }
    catch (std::runtime_error& e) {
	LOG_ERROR << e.what() << std::endl;
        splineCurve = tinyspline::BSpline();
    }
    
    auto beziers = splineCurve.toBeziers();

    int nbBeziers = ts_bspline_num_control_points(beziers.data()) / splineCurve.order();
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
    for(const auto & bez : _beziers) {
        if(bez->boundingBox().inArea(c)) {

        }
    }
}
