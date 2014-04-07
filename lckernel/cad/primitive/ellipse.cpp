#include "ellipse.h"

using namespace lc;

Ellipse::Ellipse(const geo::Coordinate& center, const geo::Coordinate& majorP, double minorRadius, double startAngle, double endAngle)
    : geo::Ellipse(center, majorP, minorRadius, startAngle, endAngle) {

}

Ellipse::Ellipse(const geo::Coordinate& center, const geo::Coordinate& majorP, double minorRadius, double startAngle, double endAngle, const QList<shared_ptr<const lc::MetaType> >& metaTypes)
    : CADEntity(metaTypes),  geo::Ellipse(center, majorP, minorRadius, startAngle, endAngle) {
}

//shared_ptr<const lc::CADEntity> Ellipse::move(const geo::Coordinate& offset) const {
//    Ellipse* ell = new lc::Ellipse(this->center() + offset, this->majorP() + offset,
//                                   this->minorRadius(), this->startAngle(), this ->endAngle());
//    ell->setID(this->id());
//    shared_ptr<const lc::Ellipse> newEllipse = shared_ptr<const lc::Ellipse>(ell);
//    return newEllipse;
//}

//shared_ptr<const lc::CADEntity> Ellipse::rotate(const geo::Coordinate &rotation_point, const double& rotation_angle) const {
//    Ellipse* ell = new lc::Ellipse(this->center().rotate(rotation_point, rotation_angle),
//                                   this->majorP().rotate(rotation_point, rotation_angle),
//                                   this->minorRadius(), this->startAngle(), this ->endAngle());
//    ell->setID(this->id());
//    shared_ptr<const lc::Ellipse> newEllipse = shared_ptr<const lc::Ellipse>(ell);
//    return newEllipse;
//}

//shared_ptr<const lc::CADEntity> Ellipse::copy(const lc::geo::Coordinate& offset, const double& rotation_angle) const {

//}

shared_ptr<const lc::CADEntity> Ellipse::move(const geo::Coordinate& offset, const geo::Coordinate& rotation_center, const double& rotation_angle) const {
//    Ellipse* newellipse = new lc::Ellipse((this->center().rotate(rotation_center, rotation_angle)) + offset,
//                                          (this->majorP().rotate(rotation_center, rotation_angle)) + offset,
//                                          this->minorRadius() , this->startAngle() + rotation_angle,
//                                          this->endAngle() + rotation_angle);
//    newellipse->setID(this->id());
//    shared_ptr<const lc::Ellipse> newEllipse = shared_ptr<const lc::Ellipse>(newellipse);
//    return newEllipse;
    return rotate(offset, rotation_center, rotation_angle, 1);
}

shared_ptr<const lc::CADEntity> Ellipse::copy(const geo::Coordinate& offset, const geo::Coordinate& rotation_center, const double& rotation_angle) const {
//    Ellipse* newellipse = new lc::Ellipse((this->center().rotate(rotation_center, rotation_angle)) + offset,
//                                          (this->majorP().rotate(rotation_center, rotation_angle)) + offset,
//                                          this->minorRadius() , this->startAngle() + rotation_angle,
//                                          this->endAngle() + rotation_angle);
//    shared_ptr<const lc::Ellipse> newEllipse = shared_ptr<const lc::Ellipse>(newellipse);
//    return newEllipse;
        return rotate(offset, rotation_center, rotation_angle, 0);
}

shared_ptr<const lc::CADEntity> Ellipse::rotate(const geo::Coordinate& offset, const geo::Coordinate &rotation_center, const double& rotation_angle, const bool with_same_id) const {
    Ellipse* newellipse = new lc::Ellipse((this->center().rotate(rotation_center, rotation_angle)) + offset,
                                          (this->majorP().rotate(rotation_center, rotation_angle)) + offset,
                                          this->minorRadius() , this->startAngle() + rotation_angle,
                                          this->endAngle() + rotation_angle);
    if (with_same_id == 1) {
        newellipse->setID(this->id());
    }
    shared_ptr<const lc::Ellipse> newEllipse = shared_ptr<const lc::Ellipse>(newellipse);
    return newEllipse;
}
