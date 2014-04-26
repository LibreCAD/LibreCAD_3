#include "cad/primitive/circle.h"

using namespace lc;

Circle::Circle(const geo::Coordinate& center, double radius, const shared_ptr<const Layer> layer) : CADEntity(layer), geo::Circle(center, radius) {

}

Circle::Circle(const geo::Coordinate& center, double radius, const shared_ptr<const Layer> layer, const QList<shared_ptr<const MetaType> >& metaTypes) : CADEntity(layer, metaTypes),  geo::Circle(center, radius) {
}


QList<EntityCoordinate> Circle::snapPoints(const geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const {
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

shared_ptr<const CADEntity> Circle::move(const geo::Coordinate& offset) const {
    Circle* newcircle = new Circle(this->center() + offset, this->radius(), layer());
    newcircle->setID(this->id());
    shared_ptr<const Circle> newCircle = shared_ptr<const Circle>(newcircle);
    return newCircle;
}

shared_ptr<const CADEntity> Circle::copy(const geo::Coordinate& offset) const {
    Circle* newcircle = new Circle(this->center() + offset, this->radius(), layer());
    shared_ptr<const Circle> newCircle = shared_ptr<const Circle>(newcircle);
    return newCircle;
}

shared_ptr<const CADEntity> Circle::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    Circle* newcircle = new Circle(this->center().rotate(rotation_center, rotation_angle), this->radius(), layer());
    newcircle->setID(this->id());
    shared_ptr<const Circle> newCircle = shared_ptr<const Circle>(newcircle);
    return newCircle;
}

shared_ptr<const CADEntity> Circle::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    Circle* newcircle = new Circle(this->center().scale(scale_center, scale_factor), this->radius() * scale_factor.x(), layer());
    newcircle->setID(this->id());
    shared_ptr<const Circle> newCircle = shared_ptr<const Circle>(newcircle);
    return newCircle;
}
