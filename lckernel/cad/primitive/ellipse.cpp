#include <memory>
#include "ellipse.h"

#include <cmath>

using namespace lc;

Ellipse::Ellipse(const geo::Coordinate& center, const geo::Coordinate& majorP, double minorRadius, double startAngle, double endAngle, const Layer_CSPtr layer)
    : CADEntity(layer), geo::Ellipse(center, majorP, minorRadius, startAngle, endAngle) {

}

Ellipse::Ellipse(const geo::Coordinate& center, const geo::Coordinate& majorP, double minorRadius, double startAngle, double endAngle, const Layer_CSPtr layer, const std::list<MetaType_CSPtr>& metaTypes)
    : CADEntity(layer, metaTypes),  geo::Ellipse(center, majorP, minorRadius, startAngle, endAngle) {
}

Ellipse::Ellipse(const Ellipse_CSPtr other, bool sameID) : CADEntity(other->layer(), other->metaTypes()),  geo::Ellipse(other->center(), other->majorP(), other->minorRadius(), other->startAngle(), other->endAngle()) {
    if (sameID) {
        this->setID(other->id());
    }
}


CADEntity_CSPtr Ellipse::move(const geo::Coordinate& offset) const {
    auto newellipse = std::make_shared<Ellipse>(this->center() + offset, this->majorP(), this->minorRadius(),
                                                this->startAngle(), this->endAngle(), layer());
    newellipse->setID(this->id());
    return newellipse;
}

CADEntity_CSPtr Ellipse::copy(const geo::Coordinate& offset) const {
    auto newEllipse = std::make_shared<Ellipse>(this->center() + offset, this->majorP(), this->minorRadius(),
                                                this->startAngle(), this->endAngle(), layer());
    return newEllipse;
}

CADEntity_CSPtr Ellipse::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto newEllipse = std::make_shared<Ellipse>(this->center().rotate(rotation_center, rotation_angle),
                                                (this->majorP().rotate(geo::Coordinate(0., 0.), rotation_angle)),
                                                this->minorRadius() , this->startAngle(),
                                                this->endAngle(), layer());
    newEllipse->setID(this->id());
    return newEllipse;
}

CADEntity_CSPtr Ellipse::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
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
    double kx2 = scale_factor.x() * scale_factor.x();
    double ky2 = scale_factor.y() * scale_factor.y();
    double b = (this->minorRadius() / this->majorP().magnitude()) * a;
    double cA = 0.5 * a * a * (kx2 * ct2 + ky2 * st2);
    double cB = 0.5 * b * b * (kx2 * st2 + ky2 * ct2);
    double cC = a * b * ct * st * (ky2 - kx2);
    geo::Coordinate vp(cA - cB, cC);
    geo::Coordinate vp3(a, b);
    geo::Coordinate vp4(vp3.scale(geo::Coordinate(vp.angle() * 0.5)));
    geo::Coordinate vp5(vp4.rotate(geo::Coordinate(ct, st)));
    geo::Coordinate vp6(vp5.scale(scale_factor));
    double z=cA+cB;
    double x=vp.magnitude();
    double ratio = sqrt((z - x)/(z + x));
    double minor_ = vp6.magnitude() * ratio;

    auto newEllipse = std::make_shared<Ellipse>(this->center().scale(scale_center, scale_factor), vp6,
                                                minor_,
                                                isArc() ? this->getEllipseAngle(startPoint) : 0.,
                                                isArc() ? this->getEllipseAngle(endPoint) : 360., layer());

    newEllipse->setID(this->id());
    return newEllipse;

}

const geo::Area Ellipse::boundingBox() const {

    geo::Coordinate p = geo::Coordinate(this->center().x(), this->center().y() + this->minorRadius()).rotate(this->majorP() - this->center());

    double r_min = this->minorRadius();
    double r_max = p.distanceTo(this->majorP());
    double rot = (this->majorP() - this->center()).angle();

    double t_nil = atan(-r_min * tan(rot) / r_max);
    double t_inf = atan(r_min * (cos(rot) / sin(rot)) / r_max);
    double rect_width = 2 * (r_max * cos(t_nil) * cos(rot) - r_min * sin(t_nil) * sin(rot));
    double rect_height = 2 * (r_min * sin(t_inf) * cos(rot) + r_max * cos(t_inf) * sin(rot));

    return geo::Area(geo::Coordinate(0., 0.), geo::Coordinate(0., 0.));
}
