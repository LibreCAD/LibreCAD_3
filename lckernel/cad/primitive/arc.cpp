#include "arc.h"

using namespace lc;
using namespace entity;

Arc::Arc(const geo::Coordinate& center, double radius, double startAngle, double endAngle, bool isCCW, const Layer_CSPtr layer) : CADEntity(layer), geo::Arc(center, radius, startAngle, endAngle, isCCW) {

}

Arc::Arc(const geo::Coordinate& center, double radius, double startAngle, double endAngle, bool isCCW, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) : CADEntity(layer, metaInfo),  geo::Arc(center, radius, startAngle, endAngle, isCCW) {
}

Arc::Arc(const Arc_CSPtr other, bool sameID) : CADEntity(other, sameID),  geo::Arc(other->center(), other->radius(), other->startAngle(), other->endAngle(), other->CCW()) {
}

std::vector<EntityCoordinate> Arc::snapPoints(const geo::Coordinate& coord, const SimpleSnapConstrain & constrain, double minDistanceToSnap, int maxNumberOfSnapPoints) const {
    std::vector<EntityCoordinate> points;


    if (constrain.constrain() & SimpleSnapConstrain::ON_ENTITY ||
        constrain.constrain() & SimpleSnapConstrain::ON_ENTITYPATH) {
        geo::Coordinate npoe = nearestPointOnPath(coord);
        points.emplace_back(npoe, -1);
        std::cerr << "Arc::snapPoints" << "\n";
    }

    // Cleanup array of snappoints
    Snapable::snapPointsCleanup(points, coord, maxNumberOfSnapPoints, minDistanceToSnap);
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
    auto newArc = std::make_shared<Arc>(this->center() + offset, this->radius(), this->startAngle(), this->endAngle(), this->CCW(), layer());
    newArc->setID(this->id());
    return newArc;
}

CADEntity_CSPtr Arc::copy(const geo::Coordinate& offset) const {
    auto newArc = std::make_shared<Arc>(this->center() + offset, this->radius(), this->startAngle(), this->endAngle(), this->CCW(), layer());
    return newArc;
}

CADEntity_CSPtr Arc::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto newArc = std::make_shared<Arc>(this->center().rotate(rotation_center, rotation_angle),
                                        this->radius(), this->startAngle() + rotation_angle,
                                        this->endAngle() + rotation_angle, this->CCW(), layer());
    newArc->setID(this->id());
    return newArc;
}

CADEntity_CSPtr Arc::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newArc = std::make_shared<Arc>(this->center().scale(scale_center, scale_factor), this->radius() * fabs(scale_factor.x()),
                                        this->startAngle(), this->endAngle(), this->CCW(), layer());
    newArc->setID(this->id());
    return newArc;

}

const geo::Area Arc::boundingBox() const {
    return geo::Arc::boundingBox();
}

CADEntity_CSPtr Arc::modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const {
    auto newArc = std::make_shared<Arc>(this->center(), this->radius(), this->startAngle(), this->endAngle(), this->CCW(), layer, metaInfo);
    newArc->setID(this->id());
    return newArc;
}


