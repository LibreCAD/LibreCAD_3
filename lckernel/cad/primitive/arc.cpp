#include "arc.h"

using namespace lc;
using namespace entity;

Arc::Arc(const geo::Coordinate& center, double radius, double startAngle, double endAngle, const Layer_CSPtr layer) : CADEntity(layer), geo::Arc(center, radius, startAngle, endAngle) {

}

Arc::Arc(const geo::Coordinate& center, double radius, double startAngle, double endAngle, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) : CADEntity(layer, metaInfo),  geo::Arc(center, radius, startAngle, endAngle) {
}

Arc::Arc(const Arc_CSPtr other, bool sameID) : CADEntity(other, sameID),  geo::Arc(other->center(), other->radius(), other->startAngle(), other->endAngle()) {
}

std::vector<EntityCoordinate> Arc::snapPoints(const geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const {
    std::vector<EntityCoordinate> points;

    points.push_back(EntityCoordinate(center(), (center() - coord).magnitude(), 0));

    geo::Coordinate npoe = nearestPointOnPath(coord);
    geo::Coordinate rVector = npoe - coord;

    double distance = rVector.magnitude();

    if (distance < minDistanceToSnap) {
        points.push_back(EntityCoordinate(npoe, distance, -1));
    }

    // Sort by distance and keep maxNumberOfSnapPoints
    std::sort(points.begin() , points.end(), EntityCoordinate::sortAscending);
    points.erase(points.begin() + maxNumberOfSnapPoints, points.end());
    return points;
}

// TODO: Decide if a point like center should be returned by a function nearestPointOnPath
geo::Coordinate Arc::nearestPointOnPath(const geo::Coordinate& coord) const {
    double vl1 = (center() - coord).magnitude();
    const geo::Coordinate pointOnPath = geo::Arc::nearestPointOnPath(coord);
    double vl2 = (pointOnPath - coord).magnitude();

    /*
    if (vl1 < vl2) {
        return center();
    } */

    return pointOnPath;
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
    return geo::Arc::boundingBox();
}

CADEntity_CSPtr Arc::modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const {
    auto newArc = std::make_shared<Arc>(this->center(), this->radius(), this->startAngle(), this->endAngle(), layer, metaInfo);
    newArc->setID(this->id());
    return newArc;
}


