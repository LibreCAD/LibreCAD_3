#include "line.h"

using namespace lc;

Line::Line(const geo::Coordinate& start, const geo::Coordinate& end, const std::shared_ptr<const Layer> layer) : CADEntity(layer), Vector(start, end) {
}

Line::Line(const geo::Coordinate& start, const geo::Coordinate& end, const std::shared_ptr<const Layer> layer, const QList<std::shared_ptr<const MetaType> >& metaTypes) : CADEntity(layer, metaTypes), Vector(start, end) {
}

Line::Line(const geo::Vector& vector, const std::shared_ptr<const Layer> layer, const QList<std::shared_ptr<const MetaType> >& metaTypes) : CADEntity(layer, metaTypes), Vector(vector) {
}

Line::Line(const std::shared_ptr<const Line> other, bool sameID) : CADEntity(other->layer(), other->metaTypes()), Vector(other->start(), other->end()) {
    if (sameID) {
        this->setID(other->id());
    }
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

std::shared_ptr<const CADEntity> Line::move(const geo::Coordinate& offset) const {
    auto newLine = std::make_shared<Line>(this->start() + offset, this->end() + offset, layer());
    newLine->setID(this->id());
    return newLine;
}

std::shared_ptr<const CADEntity> Line::copy(const geo::Coordinate& offset) const {
    auto newLine = std::make_shared<Line>(this->start() + offset, this->end() + offset, layer());
    return newLine;
}

std::shared_ptr<const CADEntity> Line::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto newLine = std::make_shared<Line>(this->start().rotate(rotation_center, rotation_angle),
                                          this->end().rotate(rotation_center, rotation_angle), layer());
    newLine->setID(this->id());
    return newLine;
}

std::shared_ptr<const CADEntity> Line::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newLine = std::make_shared<Line>(this->start().scale(scale_center, scale_factor),
                                          this->end().scale(scale_center, scale_factor), layer());
    newLine->setID(this->id());
    return newLine;
}
