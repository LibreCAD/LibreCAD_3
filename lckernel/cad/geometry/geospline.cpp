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
    return Coordinate();
}
Coordinate Spline::nearestPointOnEntity(const Coordinate &coord) const {
    /* TODO implement
     * fix compiler warning
     */
    return Coordinate();
}

void Spline::populateCurve() {
    //  std::cout << "Populating";

    // convert LC Spline Control points to nurbs type coordinates.
    ON_3dPointArray points;
    for(const auto& p: _controlPoints) {
        points.Append(ON_3dPoint(p.x(), p.y(), p.z()));
    }
    auto cpcount = _controlPoints.size();

    // UNIFORM OPEN CURVE
    if(_flags == splineflag::PERIODIC) {
        _splineCurve.CreatePeriodicUniformNurbs(3, _degree+1, cpcount, points);
    }

    // UNIFORM BUT CLOSED ONE
    else if (_knotPoints.size()==0) {

        _splineCurve.Create(3, false, _degree+1, cpcount);
        auto i = 0;

        for(auto i = 0; i < cpcount; i++) {
            _splineCurve.SetCV(i, points[i]);
        }

        int knotcount = _degree+cpcount-1;

        double* knots = new double[knotcount];
        ON_MakeClampedUniformKnotVector(_degree+1, cpcount, knots);
        for (int i=0; i<knotcount; ++i) {
            _splineCurve.SetKnot(i, knots[i]);
        }
        delete knots;
    }

    //  NON UNIFORM NURBS.
    else if(knotPoints().size() > 0) {
        _splineCurve.Create(3, false, _degree+1, cpcount);

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

const std::vector<BB_CSPtr> Spline::beziers() const {
    return _beziers;
}

/*
 * Need to be updated to return bezier objects instead of returning coordinate vectors.
 * No external need to cast to bezier and then find intersections.
 */
void Spline::generateBeziers() {
    auto curve = _splineCurve.Duplicate();
    curve->MakePiecewiseBezier();
    ON_3dPoint p;

    int deg = curve->Degree();
    int cpcount = curve->CVCount();

    if(deg==2) {
        for (int i=0; i<deg+cpcount+2; ++i) {
            ON_BezierCurve bc;
            if (curve->ConvertSpanToBezier(i, bc)) {
                std::vector<geo::Coordinate> bez;
                for (int j=0; j<bc.CVCount(); j++) {
                    bc.GetCV(j, p);
                    bez.push_back(geo::Coordinate(p.x, p.y, p.z));
                }
                _beziers.push_back(std::make_shared<Bezier>(bez.at(0),bez.at(1),bez.at(2)));
            }
        }
    } else if(deg==3) {
        for (int i=0; i<deg+cpcount+2; ++i) {
            ON_BezierCurve bc;
            if (curve->ConvertSpanToBezier(i, bc)) {
                std::vector<geo::Coordinate> bez;
                for (int j=0; j<bc.CVCount(); j++) {
                    bc.GetCV(j, p);
                    bez.push_back(geo::Coordinate(p.x, p.y, p.z));
                }
                _beziers.push_back(std::make_shared<CubicBezier>(bez.at(0),bez.at(1),bez.at(2),bez.at(3)));
            }
        }
    }
}
