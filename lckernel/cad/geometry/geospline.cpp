#include "geospline.h"

using namespace lc;
using namespace geo;

/*
Spline::Spline(const std::vector<Coordinate>& control_points, int degree, bool closed) : _controlPoints(control_points), _degree(degree), _closed(closed), _sTanX(0.), _sTanY(0.),
                                                                                         _sTanZ(0.), _eTanX(0.), _eTanY(0.),
                                                                                         _eTanZ(0.) {
}


Spline::Spline(const std::vector<Coordinate>& control_points,
               const std::vector<double>& knotPoints, int degree,
               bool closed) :  _controlPoints(control_points), _knotPoints(knotPoints),
    _degree(degree), _closed(closed), _sTanX(0.), _sTanY(0.),
    _sTanZ(0.), _eTanX(0.), _eTanY(0.),
    _eTanZ(0.) {
}

Spline::Spline(const std::vector<Coordinate>& control_points,
               const std::vector<double>& knotPoints,
               const std::vector<Coordinate>& fitPoints, int degree,
               bool closed) : _controlPoints(control_points), _knotPoints(knotPoints),
    _fitPoints(fitPoints),
    _degree(degree), _closed(closed), _sTanX(0.), _sTanY(0.),
    _sTanZ(0.), _eTanX(0.), _eTanY(0.),
    _eTanZ(0.) {
}
*/
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
    return Coordinate();
}
Coordinate Spline::nearestPointOnEntity(const Coordinate &coord) const {
    /* TODO implement
     * fix compiler warning
     */
    return Coordinate();
}

void Spline::populateCurve() {
    // Periodic
    ON_3dPointArray points;
    for(const auto& p: _controlPoints) {
        points.Append(ON_3dPoint(p.x(), p.y(), p.z()));
    }
    ON_NurbsCurve* nc = ON_NurbsCurve::New();
    if(_flags == splineflag::PERIODIC) {
        nc->CreatePeriodicUniformNurbs(3, _degree, _controlPoints.size(), points);
    }
}

std::vector<std::vector<lc::geo::Coordinate>> Spline::getBeziers() const {
    auto curve = _splineCurve.Duplicate();
    curve->MakePiecewiseBezier();
    int span_count = curve->SpanCount();
    int order = curve->m_order;
    for(int spani = 0; spani < span_count; spani++) {
        ON_3dPoint *ctrl_points = new ON_3dPoint[order];
        //Load bezier control points
        for(int i = 0; i < order; i++ ){
            curve->GetCV(spani*(order-1) + i,ctrl_points[i]);
        }
        //Use control points to create bezier with our representation,
        // if the order is 3 elevate degree to make cubic bezier
    }

}
