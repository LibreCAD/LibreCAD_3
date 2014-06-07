#include "line.h"

using namespace lc;

#include <algorithm>
#include "cad/geometry/geoarea.h"

Line::Line(const geo::Coordinate& start, const geo::Coordinate& end, const Layer_CSPtr layer) : CADEntity(layer), Vector(start, end) {
}

Line::Line(const geo::Coordinate& start, const geo::Coordinate& end, const Layer_CSPtr layer, const std::list<MetaType_CSPtr>& metaTypes) : CADEntity(layer, metaTypes), Vector(start, end) {
}

Line::Line(const geo::Vector& vector, const Layer_CSPtr layer) : CADEntity(layer), Vector(vector) {
}

Line::Line(const geo::Vector& vector, const Layer_CSPtr layer, const std::list<MetaType_CSPtr>& metaTypes) : CADEntity(layer, metaTypes), Vector(vector) {
}

Line::Line(const Line_CSPtr other, bool sameID) : CADEntity(other->layer(), other->metaTypes()), Vector(other->start(), other->end()) {
    if (sameID) {
        this->setID(other->id());
    }
}



std::vector<EntityCoordinate> Line::snapPoints(const geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const {
    std::vector<EntityCoordinate> points;

    points.push_back(EntityCoordinate(start(), (start() - coord).magnitude(), 0));
    points.push_back(EntityCoordinate(end(), (end() - coord).magnitude(), 1));

    geo::Coordinate npoe = nearestPointOnPath(coord);
    geo::Coordinate rVector = npoe - coord;

    double distance = rVector.magnitude();

    if (distance < minDistanceToSnap) {
        points.push_back(EntityCoordinate(npoe, distance, -1));
    }

    // Sort by distance and take first XX elements
    std::sort(points.begin() , points.end(), EntityCoordinate::sortAscending);
    points.erase(points.begin() + maxNumberOfSnapPoints, points.end());
    return points;
}

geo::Coordinate Line::nearestPointOnPath(const geo::Coordinate& coord) const {
    return geo::Vector::nearestPointOnPath(coord);
}

CADEntity_CSPtr Line::move(const geo::Coordinate& offset) const {
    auto newLine = std::make_shared<Line>(this->start() + offset, this->end() + offset, layer());
    newLine->setID(this->id());
    return newLine;
}

CADEntity_CSPtr Line::copy(const geo::Coordinate& offset) const {
    auto newLine = std::make_shared<Line>(this->start() + offset, this->end() + offset, layer());
    return newLine;
}

CADEntity_CSPtr Line::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto newLine = std::make_shared<Line>(this->start().rotate(rotation_center, rotation_angle),
                                          this->end().rotate(rotation_center, rotation_angle), layer());
    newLine->setID(this->id());
    return newLine;
}

CADEntity_CSPtr Line::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto newLine = std::make_shared<Line>(this->start().scale(scale_center, scale_factor),
                                          this->end().scale(scale_center, scale_factor), layer());
    newLine->setID(this->id());
    return newLine;
}

const geo::Area Line::boundingBox() const {
    return geo::Area(start(), end());
}
