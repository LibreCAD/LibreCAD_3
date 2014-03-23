#include "arc.h"

using namespace lc;

Arc::Arc(const geo::Coordinate& center, double radius, double startAngle, double endAngle) : CADEntity(), geo::Arc(center, radius, startAngle, endAngle) {

}

Arc::Arc(const geo::Coordinate& center, double radius, double startAngle, double endAngle, const QList<shared_ptr<const lc::MetaType> >& metaTypes) : CADEntity(metaTypes),  geo::Arc(center, radius, startAngle, endAngle) {
}

shared_ptr<const lc::CADEntity> Arc::move(const geo::Coordinate& offset) const {
    Arc *ar = new lc::Arc(this->center() + offset, this->radius(), this->startAngle(), this->endAngle());
    ar->setID(this->id());
    shared_ptr<const lc::Arc> newArc = shared_ptr<const lc::Arc>(ar);
    return newArc;
}

shared_ptr<const lc::CADEntity> Arc::rotate(const double& angle) const {
    //return;
}
