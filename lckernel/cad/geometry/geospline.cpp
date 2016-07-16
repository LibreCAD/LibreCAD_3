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

    //  std::cout << "Populating";
    //  std::cout << _flags;

    // convert LC Spline Control points to nurbs type coordinates.
    ON_3dPointArray points;
    for(const auto& p: _controlPoints) {
        points.Append(ON_3dPoint(p.x(), p.y(), p.z()));
    }
    auto cpcount = _controlPoints.size();

    // UNIFORM OPEN CURVE

    if(_flags == splineflag::PERIODIC) {
        _splineCurve.CreatePeriodicUniformNurbs(3, _degree, cpcount, points);
    }

    // UNIFORM BUT CLOSED ONE

    else if (_knotPoints.size()==0) {

        _splineCurve.Create(3, false, _degree+1, cpcount);
        auto i = 0;

        //  for(const auto & cp: _controlPoints) {
        //      _splineCurve.SetCV(i, ON_3dPoint(cp.x(), cp.y(), cp.z()));
        //      i++;
        //  }

        for(auto i = 0; i < cpcount; i++) {
            _splineCurve.SetCV(i, points[i]);
        }

        int knotcount = _degree+_controlPoints.size()-1;

        double* knots = new double[knotcount];
        ON_MakeClampedUniformKnotVector(_degree+1, _controlPoints.size(), knots);
        for (int i=0; i<knotcount; ++i) {
            _splineCurve.SetKnot(i, knots[i]);
        }
        // _splineCurve.CreateClam pedUniformNurbs(3, _degree+1, _controlPoints.size(), points);
    }

    //  NON UNIFORM NURBS.

    else if(knotPoints().size() > 0) {
        _splineCurve.Create(3, false, _degree+1, _controlPoints.size());

        //  auto i = 0;
        //  for(const auto & cp: _controlPoints) {
        //      _splineCurve.SetCV(i, ON_3dPoint(cp.x(), cp.y(), cp.z()));
        //      i++;
        //  }

        // SET CP's

        for(auto i = 0; i < cpcount; i++) {
            _splineCurve.SetCV(i, points[i]);
        }

        // SET Knot vectors
        auto i = 0;
        for(const auto& kp: _knotPoints) {
            _splineCurve.SetKnot(i, kp);
            i++;
        }

    }

}

std::vector<std::vector<lc::geo::Coordinate>> Spline::getBeziers() const {
    std::vector<std::vector<lc::geo::Coordinate>> ret;
    auto curve = _splineCurve.Duplicate();
//    std::cout << _splineCurve.CVSize();
    curve->MakePiecewiseBezier();
    int span_count = curve->SpanCount();
    int order = curve->m_order;
    for(int spani = 0; spani < span_count; spani++) {
        std::vector<geo::Coordinate> coords;
        ON_3dPoint *ctrl_points = new ON_3dPoint[order];
        //Load bezier control points
        for(int i = 0; i < order; i++ ){
            curve->GetCV(spani*(order-1) + i,ctrl_points[i]);
            coords.push_back(geo::Coordinate(ctrl_points[i].x, ctrl_points[i].y, ctrl_points[i].z));
        }

        std::cout << "DEBUG: Degree : "<< order - 1 << "\n";

        //Use control points to create bezier with our representation,
        // if the order is 3 elevate degree to make cubic bezier
        ret.push_back(coords);
    }
    return ret;
}
