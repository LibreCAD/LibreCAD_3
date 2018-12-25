#include "geospline.h"

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
    try {
        auto nbControlPoints = _controlPoints.size();

        _splineCurve = ts::BSpline(degree(), 3, nbControlPoints, TS_CLAMPED);

        //Set control points
        std::vector<ts::rational> ts_controlPoints;
        for (const auto& cp : _controlPoints) {
            ts_controlPoints.push_back(cp.x());
            ts_controlPoints.push_back(cp.y());
            ts_controlPoints.push_back(cp.z());
        }

        _splineCurve.setCtrlp(ts_controlPoints);
    }
    catch (std::runtime_error& e) {
        _splineCurve = ts::BSpline();
    }
}

const std::vector<BB_CSPtr> Spline::beziers() const {
    return _beziers;
}

/*
 * Need to be updated to return bezier objects instead of returning coordinate vectors.
 * No external need to cast to bezier and then find intersections.
 */
void Spline::generateBeziers() {
    auto beziers = _splineCurve.toBeziers();

    int nbBeziers = beziers.nCtrlp() / _splineCurve.order();
    int nbCoordinate = _splineCurve.order() * _splineCurve.dim();

    if(_splineCurve.deg() == 2) {
        for (int i = 0; i < nbBeziers; i++) {
            std::vector<geo::Coordinate> bez;

            auto j = nbCoordinate * i;
            auto end = nbCoordinate * (i+1);

            while(j < end) {
                lc::geo::Coordinate cp(beziers.ctrlp()[j], beziers.ctrlp()[j+1], beziers.ctrlp()[j+2]);
                bez.push_back(cp);
                j = j + 3;
            }

            _beziers.push_back(std::make_shared<Bezier>(bez.at(0),bez.at(1),bez.at(2)));
        }
    } else if(_splineCurve.deg() == 3) {
        for (int i = 0; i < nbBeziers; i++) {
            std::vector<geo::Coordinate> bez;

            auto j = nbCoordinate * i;
            auto end = nbCoordinate * (i+1);

            while(j < end) {
                lc::geo::Coordinate cp(beziers.ctrlp()[j], beziers.ctrlp()[j+1], beziers.ctrlp()[j+2]);
                bez.push_back(cp);

                j = j + 3;
            }

            _beziers.push_back(std::make_shared<CubicBezier>(bez.at(0),bez.at(1),bez.at(2),bez.at(3)));
        }
    }
}

void Spline::trimAtPoint(const geo::Coordinate& c) {
    for(const auto & bez : _beziers) {
        if(bez->boundingBox().inArea(c)) {

        }
    }
}
