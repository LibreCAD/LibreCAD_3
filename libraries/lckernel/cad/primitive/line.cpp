#include "line.h"

using namespace lc;

Line::Line(const geo::Coordinate& start, const geo::Coordinate& end) : CADEntity(), Vector(start, end) {
}

Line::Line(const geo::Coordinate& start, const geo::Coordinate& end, const QList<shared_ptr<const lc::MetaType> >& metaTypes) : CADEntity(metaTypes), Vector(start, end) {
}

Line::Line(const geo::Vector& vector, const QList<shared_ptr<const lc::MetaType> >& metaTypes) : CADEntity(metaTypes), Vector(vector) {
}



QList<lc::EntityCoordinate> Line::snapPoints(const geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const {
    QList<EntityCoordinate> points;

    points.append(EntityCoordinate(start(), (start() - coord).magnitude(), 0));

    points.append(EntityCoordinate(end(), (end() - coord).magnitude(), 1));


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

geo::Coordinate Line::nearestPointOnPath(const geo::Coordinate& coord) const {
    return geo::Vector::nearestPointOnPath(coord);
}



