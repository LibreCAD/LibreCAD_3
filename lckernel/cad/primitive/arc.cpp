#include "arc.h"

using namespace lc;

Arc::Arc(const geo::Coordinate& center, double radius, double startAngle, double endAngle, const Layer_CSPtr layer) : CADEntity(layer), geo::Arc(center, radius, startAngle, endAngle) {

}

Arc::Arc(const geo::Coordinate& center, double radius, double startAngle, double endAngle, const Layer_CSPtr layer, MetaInfo_CSPtr metaInfo) : CADEntity(layer, metaInfo),  geo::Arc(center, radius, startAngle, endAngle) {
}

Arc::Arc(const Arc_CSPtr other, bool sameID) : CADEntity(other, sameID),  geo::Arc(other->center(), other->radius(), other->startAngle(), other->endAngle()) {
}

CADEntity_CSPtr Arc::move(const geo::Coordinate& offset) const {
    auto newArc = std::make_shared<Arc>(this->center() + offset, this->radius(), this->startAngle(), this->endAngle(), layer());
    newArc->setID(this->id());
    return newArc;
}

CADEntity_CSPtr Arc::copy(const geo::Coordinate& offset) const {
    auto newArc = std::make_shared<Arc>(this->center() + offset, this->radius(), this->startAngle(), this->endAngle(), layer());
    return newArc;
}

CADEntity_CSPtr Arc::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto newArc = std::make_shared<Arc>(this->center().rotate(rotation_center, rotation_angle),
                                        this->radius(), this->startAngle() + rotation_angle,
                                        this->endAngle() + rotation_angle, layer());
    newArc->setID(this->id());
    return newArc;
}

CADEntity_CSPtr Arc::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newArc = std::make_shared<Arc>(this->center().scale(scale_center, scale_factor), this->radius() * fabs(scale_factor.x()),
                                        this->startAngle(), this->endAngle(), layer());
    newArc->setID(this->id());
    return newArc;

}

const geo::Area Arc::boundingBox() const {
    // TODO take start/end angle into consideration
    return geo::Area(geo::Coordinate(center().x() - radius(), center().y() - radius()), geo::Coordinate(center().x() + radius(), center().y() + radius()));
}

Quadratic Arc::quadratic() const {
    std::vector<double> ce(6, 0.);
    ce[0] = 1.;
    ce[2] = 1.;
    ce[5] = -this->radius() * this->radius();
    Quadratic ret(ce);
    ret.move(this->center());
    return ret;
}


