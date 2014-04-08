#include "cad/primitive/circle.h"

using namespace lc;

Circle::Circle(const geo::Coordinate& center, double radius) : CADEntity(), geo::Circle(center, radius) {

}

Circle::Circle(const geo::Coordinate& center, double radius, const QList<shared_ptr<const lc::MetaType> >& metaTypes) : CADEntity(metaTypes),  geo::Circle(center, radius) {
}


QList<lc::EntityCoordinate> Circle::snapPoints(const geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const {
    QList<EntityCoordinate> points;

    points.append(EntityCoordinate(center(), (center() - coord).magnitude(), 0));

    geo::Coordinate npoe = nearestPointOnPath(coord);
    geo::Coordinate rVector = npoe - coord;

    double distance = rVector.magnitude();

    if (distance < minDistanceToSnap) {
        points.append(EntityCoordinate(npoe, distance, -1));
    }

    // Sort by distance
    qSort(points.begin() , points.end(), EntityCoordinate::sortAscending);
    return points.mid(0, maxNumberOfSnapPoints);
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

shared_ptr<const lc::CADEntity> Circle::move(const geo::Coordinate& offset, const geo::Coordinate& rotation_center, const double& rotation_angle) const {
//    Circle *newcircle = new lc::Circle((this->center().rotate(rotation_center, rotation_angle)) + offset, this->radius());
//    newcircle->setID(this->id());
//    shared_ptr<const lc::Circle> newCircle = shared_ptr<const lc::Circle>(newcircle);
//    return newCircle;
    return rotate(offset, rotation_center, rotation_angle, 1);
}

shared_ptr<const lc::CADEntity> Circle::copy(const geo::Coordinate& offset, const geo::Coordinate& rotation_center, const double& rotation_angle) const {
//    Circle *newcircle = new lc::Circle((this->center().rotate(rotation_center, rotation_angle)) + offset, this->radius());
//    shared_ptr<const lc::Circle> newCircle = shared_ptr<const lc::Circle>(newcircle);
//    return newCircle;
    return rotate(offset, rotation_center, rotation_angle, 0);
}

shared_ptr<const lc::CADEntity> Circle::rotate(const geo::Coordinate& offset, const geo::Coordinate &rotation_center, const double& rotation_angle, const bool with_same_id) const {
    Circle *newcircle = new lc::Circle((this->center().rotate(rotation_center, rotation_angle)) + offset, this->radius());
    if ( with_same_id == 1) {
        newcircle->setID(this->id());
    }
    shared_ptr<const lc::Circle> newCircle = shared_ptr<const lc::Circle>(newcircle);
    return newCircle;
}
