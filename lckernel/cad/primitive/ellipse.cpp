#include "ellipse.h"

using namespace lc;

Ellipse::Ellipse(const geo::Coordinate& center, const geo::Coordinate& majorP, double minorRadius, double startAngle, double endAngle)
    : geo::Ellipse(center, majorP, minorRadius, startAngle, endAngle) {

}

Ellipse::Ellipse(const geo::Coordinate& center, const geo::Coordinate& majorP, double minorRadius, double startAngle, double endAngle, const QList<shared_ptr<const lc::MetaType> >& metaTypes)
    : CADEntity(metaTypes),  geo::Ellipse(center, majorP, minorRadius, startAngle, endAngle) {
}

shared_ptr<const lc::CADEntity> Ellipse::move(const geo::Coordinate& offset) const {
    Ellipse* ell = new lc::Ellipse(this->center() + offset, this->majorP() + offset,
                                   this->minorRadius(), this->startAngle(), this ->endAngle());
    ell->setID(this->id());
    shared_ptr<const lc::Ellipse> newEllipse = shared_ptr<const lc::Ellipse>(ell);
    return newEllipse;
}

shared_ptr<const lc::CADEntity> Ellipse::rotate(const geo::Coordinate &rotation_point, const double& angle) const {
    Ellipse* ell = new lc::Ellipse(this->center().rotate(rotation_point, angle),
                                   this->majorP().rotate(rotation_point, angle),
                                   this->minorRadius(), this->startAngle(), this ->endAngle());
    ell->setID(this->id());
    shared_ptr<const lc::Ellipse> newEllipse = shared_ptr<const lc::Ellipse>(ell);
    return newEllipse;
}
