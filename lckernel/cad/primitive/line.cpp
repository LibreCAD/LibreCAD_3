#include "line.h"

using namespace lc;

Line::Line(const geo::Coordinate& start, const geo::Coordinate& end) : CADEntity(), Vector(start, end) {
}

Line::Line(const geo::Coordinate& start, const geo::Coordinate& end, const QList<shared_ptr<const MetaType> >& metaTypes) : CADEntity(metaTypes), Vector(start, end) {
}

Line::Line(const geo::Vector& vector, const QList<shared_ptr<const MetaType> >& metaTypes) : CADEntity(metaTypes), Vector(vector) {
}



QList<EntityCoordinate> Line::snapPoints(const geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const {
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

shared_ptr<const CADEntity> Line::move(const geo::Coordinate& offset) const {
    Line *newline = new Line(this->start() + offset, this->end() + offset);
    newline->setID(this->id());
    shared_ptr<const Line> newLine = shared_ptr<const Line>(newline);
    return newLine;
}

shared_ptr<const CADEntity> Line::copy(const geo::Coordinate& offset) const {
    Line *newline = new Line(this->start() + offset, this->end() + offset);
    shared_ptr<const Line> newLine = shared_ptr<const Line>(newline);
    return newLine;
}

shared_ptr<const CADEntity> Line::rotate(const geo::Coordinate &rotation_center, const double rotation_angle) const {
    Line *newline = new Line(this->start().rotate(rotation_center, rotation_angle),
                             this->end().rotate(rotation_center, rotation_angle));
    newline->setID(this->id());
    shared_ptr<const Line> newLine = shared_ptr<const Line>(newline);
    return newLine;
}

shared_ptr<const CADEntity> Line::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    Line *newline = new Line(this->start().scale(scale_center, scale_factor),
                                 this->end().scale(scale_center, scale_factor));
    newline->setID(this->id());
    shared_ptr<const Line> newLine = shared_ptr<const Line>(newline);
    return newLine;
}
