#include "arc.h"

using namespace lc;

Arc::Arc(const geo::Coordinate& center, double radius, double startAngle, double endAngle, const shared_ptr<const Layer> layer) : CADEntity(layer), geo::Arc(center, radius, startAngle, endAngle) {

}

Arc::Arc(const geo::Coordinate& center, double radius, double startAngle, double endAngle, const shared_ptr<const Layer> layer, const QList<shared_ptr<const MetaType> >& metaTypes) : CADEntity(layer, metaTypes),  geo::Arc(center, radius, startAngle, endAngle) {
}


shared_ptr<const CADEntity> Arc::move(const geo::Coordinate& offset) const {
    Arc* newarc = new Arc(this->center() + offset, this->radius(), this->startAngle(), this->endAngle(), layer());
    newarc->setID(this->id());
    shared_ptr<const Arc> newArc = shared_ptr<const Arc>(newarc);
    return newArc;
}

shared_ptr<const CADEntity> Arc::copy(const geo::Coordinate& offset) const {
    Arc* newarc = new Arc(this->center() + offset, this->radius(), this->startAngle(), this->endAngle(), layer());
    shared_ptr<const Arc> newArc = shared_ptr<const Arc>(newarc);
    return newArc;
}

shared_ptr<const CADEntity> Arc::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    Arc* newarc = new Arc(this->center().rotate(rotation_center, rotation_angle),
                          this->radius(), this->startAngle() + rotation_angle,
                          this->endAngle() + rotation_angle, layer());
    newarc->setID(this->id());
    shared_ptr<const Arc> newArc = shared_ptr<const Arc>(newarc);
    return newArc;
}

shared_ptr<const CADEntity> Arc::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    Arc* newarc = new Arc(this->center().scale(scale_center, scale_factor), this->radius() * scale_factor.x(),
                          this->startAngle(), this->endAngle(), layer());
    newarc->setID(this->id());
    shared_ptr<const Arc> newArc = shared_ptr<const Arc>(newarc);
    return newArc;

}
