#include "ellipse.h"

using namespace lc;

Ellipse::Ellipse(const geo::Coordinate& center, const geo::Coordinate& majorP, double minorRadius, double startAngle, double endAngle)
    : geo::Ellipse(center, majorP, minorRadius, startAngle, endAngle) {

}

Ellipse::Ellipse(const geo::Coordinate& center, const geo::Coordinate& majorP, double minorRadius, double startAngle, double endAngle, const QList<shared_ptr<const lc::MetaType> >& metaTypes)
    : CADEntity(metaTypes),  geo::Ellipse(center, majorP, minorRadius, startAngle, endAngle) {
}

shared_ptr<const lc::CADEntity> Ellipse::move(const geo::Coordinate& offset) const {
    Ellipse* newellipse = new lc::Ellipse(this->center()+ offset, this->majorP(), this->minorRadius(),
                                          this->startAngle(), this->endAngle());
    newellipse->setID(this->id());
    shared_ptr<const lc::Ellipse> newEllipse = shared_ptr<const lc::Ellipse>(newellipse);
    return newEllipse;
}

shared_ptr<const lc::CADEntity> Ellipse::copy(const geo::Coordinate& offset) const {
    Ellipse* newellipse = new lc::Ellipse(this->center()+ offset, this->majorP(), this->minorRadius(),
                                          this->startAngle(), this->endAngle());
    shared_ptr<const lc::Ellipse> newEllipse = shared_ptr<const lc::Ellipse>(newellipse);
    return newEllipse;
}

shared_ptr<const lc::CADEntity> Ellipse::rotate(const geo::Coordinate &rotation_center, const double& rotation_angle, const bool with_same_id) const {
    Ellipse* newellipse = new lc::Ellipse(this->center().rotate(rotation_center, rotation_angle),
                                         (this->majorP().rotate(geo::Coordinate(0., 0.), rotation_angle)),
                                          this->minorRadius() , this->startAngle(),
                                          this->endAngle());
    if (with_same_id == 1) {
        newellipse->setID(this->id());
    }
    shared_ptr<const lc::Ellipse> newEllipse = shared_ptr<const lc::Ellipse>(newellipse);
    return newEllipse;
}

shared_ptr<const lc::CADEntity> Ellipse::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    geo::Coordinate vp1(this->majorP());
    double a(vp1.magnitude());
    geo::Coordinate vp2(vp1.x() * 1./a, vp1.y() * 1./a);
    double ct = vp2.x();
    double ct2 = ct*ct; // cos^2 angle
    double st = vp2.y();
    double st2 = 1.0 - ct2; // sin^2 angle
    double kx2 = scale_factor.x() * scale_factor.x();
    double ky2 = scale_factor.y() * scale_factor.y();
    double b = (this->minorRadius()/this->majorP().magnitude()) * a;
    double cA = 0.5*a*a*(kx2 * ct2 + ky2 * st2);
    double cB = 0.5*b*b*(kx2 * st2 + ky2 * ct2);
    double cC = a * b * ct * st * (ky2 - kx2);
    geo::Coordinate vp(cA-cB,cC);
    geo::Coordinate vp3(a,b);
    vp3.scale(geo::Coordinate(vp*0.5));
    vp3.rotate(geo::Coordinate(ct,st));
    vp3.scale(scale_factor);
    Ellipse* newellipse = new lc::Ellipse(this->center().scale(scale_center, scale_factor), vp3,
                                          this->minorRadius(), this->startAngle(),this->endAngle());
    newellipse->setID(this->id());
    shared_ptr<const lc::Ellipse> newEllipse = shared_ptr<const lc::Ellipse>(newellipse);
    return newEllipse;

}
