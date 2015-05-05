#include "cad/primitive/circle.h"
#include "cad/primitive/ellipse.h"
#include <cmath>
#include <algorithm>
#include "cad/interface/metatype.h"

using namespace lc;
using namespace entity;

Circle::Circle(const geo::Coordinate &center, double radius, const Layer_CSPtr layer) : CADEntity(layer),
                                                                                        geo::Circle(center, radius) {

}

Circle::Circle(const geo::Coordinate &center, double radius, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo)
        : CADEntity(layer, metaInfo), geo::Circle(center, radius) {
}


Circle::Circle(const Circle_CSPtr other, bool sameID) : CADEntity(other, sameID),
                                                        geo::Circle(other->center(), other->radius()) {
}


std::vector<EntityCoordinate> Circle::snapPoints(const geo::Coordinate &coord, const SimpleSnapConstrain &constrain,
                                                 double minDistanceToSnap, int maxNumberOfSnapPoints) const {

    std::vector<EntityCoordinate> points;
    if (constrain.constrain() & SimpleSnapConstrain::LOGICAL) {
        // Add center
        lc::geo::Coordinate coord = center();
        points.emplace_back(coord, 0);
        // Add 4 coordinates
        // Top Point
        coord = center() + lc::geo::Coordinate(0., radius());
        points.emplace_back(coord, 1);
        // Right Point
        coord = center() + lc::geo::Coordinate(radius(), 0.);
        points.emplace_back(coord, 2);
        // Left Point
        coord = center() + lc::geo::Coordinate(-radius(), 0.);
        points.emplace_back(coord, 3);
        // Bottom Point
        coord = center() + lc::geo::Coordinate(0., -radius());
        points.emplace_back(coord, 4);
    }

    if (constrain.constrain() & SimpleSnapConstrain::ON_ENTITY ||
        constrain.constrain() & SimpleSnapConstrain::ON_ENTITYPATH) {
        geo::Coordinate npoe = nearestPointOnPath(coord);
        points.emplace_back(npoe, -1);
    }

    // Cleanup array of snappoints
    Snapable::snapPointsCleanup(points, coord, maxNumberOfSnapPoints, minDistanceToSnap);
    return points;
}

geo::Coordinate Circle::nearestPointOnPath(const geo::Coordinate &coord) const {
    const geo::Coordinate pointOnPath = geo::Circle::nearestPointOnPath(coord);
    return pointOnPath;
}

CADEntity_CSPtr Circle::move(const geo::Coordinate &offset) const {
    auto newCircle = std::make_shared<Circle>(this->center() + offset, this->radius(), layer(), metaInfo());
    newCircle->setID(this->id());
    return newCircle;
}

CADEntity_CSPtr Circle::copy(const geo::Coordinate &offset) const {
    auto newCircle = std::make_shared<Circle>(this->center() + offset, this->radius(), layer(), metaInfo());
    return newCircle;
}

CADEntity_CSPtr Circle::rotate(const geo::Coordinate &rotation_center, const double rotation_angle) const {
    auto newCircle = std::make_shared<Circle>(this->center().rotate(rotation_center, rotation_angle), this->radius(),
                                              layer(), metaInfo());
    newCircle->setID(this->id());
    return newCircle;
}

CADEntity_CSPtr Circle::scale(const geo::Coordinate &scale_center, const geo::Coordinate &scale_factor) const {
    // TODO return ellipse if scalefactor.x != scalefactor.y

    auto newCircle = std::make_shared<Circle>(this->center().scale(scale_center, scale_factor),
                                              this->radius() * fabs(scale_factor.x()), layer(), metaInfo());
    newCircle->setID(this->id());
    return newCircle;
}

const geo::Area Circle::boundingBox() const {
    return geo::Area(geo::Coordinate(center().x() - radius(), center().y() - radius()),
                     geo::Coordinate(center().x() + radius(), center().y() + radius()));
}

CADEntity_CSPtr Circle::modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const {
    auto newEntity = std::make_shared<Circle>(this->center(), this->radius(), layer, metaInfo);
    newEntity->setID(this->id());
    return newEntity;
}


