#include "geoellipse.h"

#include "cmath"


using namespace lc;
using namespace geo;

Ellipse::Ellipse(Coordinate center, Coordinate majorP, double minorRadius, double startAngle, double endAngle, bool reversed) :
    _center(std::move(center)),
    _majorP(std::move(majorP)),
    _minorRadius(minorRadius),
    _startAngle(startAngle),
    _endAngle(endAngle),
    _isReversed(reversed) {

}

const Coordinate Ellipse::center() const {
    return _center;
}

const Coordinate Ellipse::majorP() const {
    return _majorP;
}

double Ellipse::minorRadius() const {
    return _minorRadius;
}

double Ellipse::startAngle() const {
    return _startAngle;
}

double Ellipse::majorRadius() const {
    return _majorP.magnitude();
}

double Ellipse::getAngle() const {
    return _majorP.angle();
}

Ellipse Ellipse::geoscale(const Coordinate& center, const Coordinate &factor) const {
    geo::Coordinate vp1(this->majorP());
    double a(vp1.magnitude());
    geo::Coordinate vp2(vp1.x() * 1. / a, vp1.y() * 1. / a);
    geo::Coordinate startPoint;
    geo::Coordinate endPoint;

    if (isArc()) {
        startPoint = this->startPoint();
        endPoint = this->endPoint();
    }

    double ct = vp2.x();
    double ct2 = ct * ct; // cos^2 angle
    double st = vp2.y();
    double st2 = 1.0 - ct2; // sin^2 angle
    double kx2 = factor.x() * factor.x();
    double ky2 = factor.y() * factor.y();
    double b = (this->minorRadius() / this->majorP().magnitude()) * a;
    double cA = 0.5 * a * a * (kx2 * ct2 + ky2 * st2);
    double cB = 0.5 * b * b * (kx2 * st2 + ky2 * ct2);
    double cC = a * b * ct * st * (ky2 - kx2);
    geo::Coordinate vp(cA - cB, cC);
    geo::Coordinate vp3(a, b);
    geo::Coordinate vp4(vp3.scale(geo::Coordinate(vp.angle() * 0.5)));
    geo::Coordinate vp5(vp4.rotate(geo::Coordinate(ct, st)));
    geo::Coordinate vp6(vp5.scale(factor));
    double z = cA + cB;
    double x = vp.magnitude();
    double ratio = std::sqrt((z - x) / (z + x));
    double minor_ = vp6.magnitude() * ratio;

    return Ellipse(this->center().scale(center, factor), vp6,
                   minor_,
                   isArc() ? this->getEllipseAngle(startPoint) : 0.,
                   isArc() ? this->getEllipseAngle(endPoint) : 2.*M_PI);
}

Ellipse Ellipse::georotate(const geo::Coordinate& center, const double rotation_angle) const {
    return Ellipse(_center.rotate(center, rotation_angle),
                   majorP().rotate(center, rotation_angle),
                   minorRadius(), startAngle(), endAngle());
}

std::vector<Coordinate> Ellipse::findPotentialNearestPoints(const Coordinate &coord) const {
    std::vector<Coordinate> pnp;
    pnp.push_back(this->startPoint());
    pnp.push_back(this->endPoint());

    double ellipseAngle = this->getAngle();

    Coordinate trcoord = coord-_center;
    trcoord = trcoord.rotate(Coordinate(0, 0), -ellipseAngle);

    double b = _minorRadius;
    double a = _majorP.magnitude();

    double x=trcoord.x(),y=trcoord.y();

    double twoa2b2=2*(a*a-b*b);
    double twoax=2*a*x;
    double twoby=2*b*y;
    double a0=twoa2b2*twoa2b2;
    std::vector<double> ce(4,0.);
    std::vector<double> roots(0,0.);

    if(a0 > LCTOLERANCE) { // a != b , ellipse
        ce[0]=-2.*twoax/twoa2b2;
        ce[1]= (twoax*twoax+twoby*twoby)/a0-1.;
        ce[2]= - ce[0];
        ce[3]= -twoax*twoax/a0;
        //std::cout<<"1::find cosine, variable c, solve(c^4 +("<<ce[0]<<")*c^3+("<<ce[1]<<")*c^2+("<<ce[2]<<")*c+("<<ce[3]<<")=0,c)\n";
        roots = maths::Math::quarticSolver(ce);
    }
    else {//a=b, quadratic equation for circle
        a0=twoby/twoax;
        roots.push_back(sqrt(1./(1.+a0*a0)));
        roots.push_back(-roots[0]);
    }

    if(roots.empty()) {
        //this should not happen
        std::cout<<"(a= "<<a<<" b= "<<b<<" x= "<<x<<" y= "<<y<<" )\n";
        std::cout<<"finding minimum for ("<<x<<"-"<<a<<"*cos(t))^2+("<<y<<"-"<<b<<"*sin(t))^2\n";
        std::cout<<"2::find cosine, variable c, solve(c^4 +("<<ce[0]<<")*c^3+("<<ce[1]<<")*c^2+("<<ce[2]<<")*c+("<<ce[3]<<")=0,c)\n";
        std::cout<<ce[0]<<' '<<ce[1]<<' '<<ce[2]<<' '<<ce[3]<<std::endl;
        std::cerr<<"lcmath::geoellipse::findPotentialNearestPoints() finds no root from quartic, this should not happen\n";
        return pnp;
    }

    for(double root : roots) {
        double const s=twoby*root/(twoax-twoa2b2*root); //sine
        double const d2=twoa2b2+(twoax-2.*root*twoa2b2)*root+twoby*s;
        if (d2<0) {
            continue;
        } // fartherest
        Coordinate t(a*root, b*s);
        //double d=(t-trcoord).magnitude();
        //double angle = atan2(roots[i],s);
        // std::cout<<i<<" Find Point: cos= "<<roots[i]<<" sin= "<<s<<" angle= "<<angle<<" ds2= "<<d<<" d="<<d2<<std::endl;
        t = t.rotate(ellipseAngle);
        t = t + _center;
        pnp.push_back(t);
    }

    return pnp;
}

Coordinate Ellipse::nearestPointOnPath(const Coordinate& coord) const {
    Coordinate res;
    auto minDist = DBL_MAX;
    std::vector<Coordinate> potencialPoinst = this->findPotentialNearestPoints(coord);

    for (auto& verifiedPoint: potencialPoinst)
    {
        double d = verifiedPoint.distanceTo(coord);
        if (d < minDist) {
            minDist = verifiedPoint.distanceTo(coord);
            res = verifiedPoint;
        }
    };

    return res;
}

Coordinate Ellipse::nearestPointOnEntity(const Coordinate& coord) const {
    Coordinate res;
    double minDist = DBL_MAX;
    std::vector<Coordinate> potentialPoints = this->findPotentialNearestPoints(coord);

    for (const auto& verifiedPoint: potentialPoints) {
        if (this->isAngleBetween(this->center().angleTo(verifiedPoint))) {
            double d = verifiedPoint.distanceTo(coord);
            if (d < minDist) {
                minDist = verifiedPoint.distanceTo(coord);
                res = verifiedPoint;
            }
        }
    };

    return res;
}

double Ellipse::endAngle() const {
    return _endAngle;
}

Coordinate Ellipse::getPoint(const double& angle) const {
    const double a = majorP().magnitude();
    const double b = minorRadius();
    return _center + Coordinate(a * cos(angle), b * sin(angle)).rotate(Coordinate(0., 0.), majorP().angle());
}

Coordinate Ellipse::startPoint() const {
    return getPoint(startAngle());
}
Coordinate Ellipse::endPoint() const {
    return getPoint(endAngle());
}

bool Ellipse::isReversed() const {
    return _isReversed;
}

double Ellipse::ratio() const {
    return _majorP.magnitude() / _minorRadius;
}

bool Ellipse::isArc() const {
    return maths::Math::getAngleDifferenceShort(_startAngle, _endAngle, _isReversed) > LCARCTOLERANCE;
}

double Ellipse::getEllipseAngle(const Coordinate& coord) const {
    Coordinate offset = coord - _center;
    Coordinate point = offset.rotate(-_majorP.angle());
    return atan2(point.y() * _majorP.magnitude(), point.x() * _minorRadius);
}
