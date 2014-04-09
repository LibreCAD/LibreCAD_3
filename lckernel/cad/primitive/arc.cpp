#include "arc.h"

using namespace lc;

Arc::Arc(const geo::Coordinate& center, double radius, double startAngle, double endAngle) : CADEntity(), geo::Arc(center, radius, startAngle, endAngle) {

}

Arc::Arc(const geo::Coordinate& center, double radius, double startAngle, double endAngle, const QList<shared_ptr<const lc::MetaType> >& metaTypes) : CADEntity(metaTypes),  geo::Arc(center, radius, startAngle, endAngle) {
}

shared_ptr<const lc::CADEntity> Arc::move(const geo::Coordinate& offset, const geo::Coordinate& rotation_center, const double& rotation_angle) const {
    return rotate(offset, rotation_center, rotation_angle, 1);
}

shared_ptr<const lc::CADEntity> Arc::copy(const geo::Coordinate& offset, const geo::Coordinate& rotation_center, const double& rotation_angle) const {
    return rotate(offset, rotation_center, rotation_angle, 0);
}

shared_ptr<const lc::CADEntity> Arc::rotate(const geo::Coordinate& offset, const geo::Coordinate &rotation_center, const double& rotation_angle, const bool with_same_id) const {
    Arc *newarc = new lc::Arc((this->center().rotate(rotation_center, rotation_angle)) + offset,
                              this->radius(), this->startAngle() + rotation_angle,
                              this->endAngle() + rotation_angle);
    if(with_same_id == 1) {
        newarc->setID(this->id());
    }
    shared_ptr<const lc::Arc> newArc = shared_ptr<const lc::Arc>(newarc);
    return newArc;
}

shared_ptr<const lc::CADEntity> Arc::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    Arc *newarc = new lc::Arc(this->center().scale(scale_center, scale_factor), this->radius() * scale_factor.x(),
                              this->startAngle(), this->endAngle());
    newarc->setID(this->id());
    shared_ptr<const lc::Arc> newArc = shared_ptr<const lc::Arc>(newarc);
    return newArc;

}
