#include "cad/primitive/circle.h"

#include <math.h>
#include <algorithm>
#include "cad/interface/metatype.h"
using namespace lc;

Circle::Circle(const geo::Coordinate& center, double radius, const Layer_CSPtr layer) : CADEntity(layer), geo::Circle(center, radius) {

}

Circle::Circle(const geo::Coordinate& center, double radius, const Layer_CSPtr layer, const std::list<MetaType_CSPtr>& metaTypes) : CADEntity(layer, metaTypes),  geo::Circle(center, radius) {
}


Circle::Circle(const Circle_CSPtr other, bool sameID) : CADEntity(other->layer(), other->metaTypes()),  geo::Circle(other->center(), other->radius()) {
    if (sameID) {
        this->setID(other->id());
    }
}


std::vector<EntityCoordinate> Circle::snapPoints(const geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const {
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
geo::Coordinate Circle::nearestPointOnPath(const geo::Coordinate& coord) const {
    double vl1 = (center() - coord).magnitude();
    const geo::Coordinate pointOnPath = geo::Circle::nearestPointOnPath(coord);
    double vl2 = (pointOnPath - coord).magnitude();

    if (vl1 < vl2) {
        return center();
    }

    return pointOnPath;
}

CADEntity_CSPtr Circle::move(const geo::Coordinate& offset) const {
    auto newCircle = std::make_shared<Circle>(this->center() + offset, this->radius(), layer());
    newCircle->setID(this->id());
    return newCircle;
}

CADEntity_CSPtr Circle::copy(const geo::Coordinate& offset) const {
    auto newCircle = std::make_shared<Circle>(this->center() + offset, this->radius(), layer());
    return newCircle;
}

CADEntity_CSPtr Circle::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto newCircle = std::make_shared<Circle>(this->center().rotate(rotation_center, rotation_angle), this->radius(), layer());
    newCircle->setID(this->id());
    return newCircle;
}

CADEntity_CSPtr Circle::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    // TODO return ellipse if scalefactor.x != scalefactor.y
    auto newCircle = std::make_shared<Circle>(this->center().scale(scale_center, scale_factor), this->radius() * fabs(scale_factor.x()), layer());
    newCircle->setID(this->id());
    return newCircle;
}

const geo::Area Circle::boundingBox() const {
    return geo::Area(geo::Coordinate(center().x() - radius(), center().y() - radius()), geo::Coordinate(center().x() + radius(), center().y() + radius()));
}
