#include "geospline.h"

using namespace lc;
using namespace geo;

Spline::Spline(const std::vector<Coordinate>& control_points, int degree, bool closed) : _controlPoints(control_points), _degree(degree), _closed(closed) {
}


Spline::Spline(const std::vector<Coordinate>& control_points,
               const std::vector<Coordinate>& knotPoints, int degree,
               bool closed) :  _controlPoints(control_points), _knotPoints(knotPoints),
    _degree(degree), _closed(closed) {
}

Spline::Spline(const std::vector<Coordinate>& control_points,
               const std::vector<Coordinate>& knotPoints,
               const std::vector<Coordinate>& fitPoints, int degree,
               bool closed) : _controlPoints(control_points), _knotPoints(knotPoints),
    _fitPoints(fitPoints),
    _degree(degree), _closed(closed) {
}

Spline::Spline(const std::vector<Coordinate>& control_points,
               const std::vector<Coordinate>& knotPoints,
               const std::vector<Coordinate>& fitPoints,
               int degree, bool closed, double stanx, double stany,
               double stanz, double etanx, double etany, double etanz)  :
    _controlPoints(control_points), _knotPoints(knotPoints),
    _fitPoints(fitPoints),
    _degree(degree), _closed(closed), _sTanX(stanx), _sTanY(stany),
    _sTanZ(stanz), _eTanX(etanx), _eTanY(etany),
    _eTanZ(etanz) {
}


const std::vector<Coordinate> Spline::controlPoints() const {
    return _controlPoints;
}

bool Spline::closed() const {
    return _closed;
}

int Spline::degree() const {
    return _degree;
}

const std::vector<Coordinate> Spline::knotPoints() const {
    return _knotPoints;
}

const std::vector<Coordinate> Spline::fitPoints() const {
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

int Spline::numberKnots() const {
    return _noKnots;
}

int Spline::numberFits() const {
    return _noFits;
}

int Spline::numberControlPoints() const {
    return _noControlPoints;
}

double Spline::knotTolerance() const {
    return _knotTolerance;
}

double Spline::controlPointTolerance() const {
    return _controlPointTolerance;
}

double Spline::fitTolerance() const {
    return _fitTolerance;
}


Coordinate Spline::nearestPointOnPath(const Coordinate& coord) const {
}

bool Spline::isCoordinateOnPath(const Coordinate& coord) const {
}
