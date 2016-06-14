#include "line.h"

#include <algorithm>
#include "cad/geometry/geoarea.h"

using namespace lc;
using namespace entity;


Line::Line(const geo::Coordinate& start, const geo::Coordinate& end, const Layer_CSPtr layer) : CADEntity(layer), geo::Vector(start, end) {
}

Line::Line(const geo::Coordinate& start, const geo::Coordinate& end, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) : CADEntity(layer, metaInfo), geo::Vector(start, end) {
}

Line::Line(const geo::Vector& vector, const Layer_CSPtr layer) : CADEntity(layer), geo::Vector(vector) {
}

Line::Line(const geo::Vector& vector, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) : CADEntity(layer, metaInfo), geo::Vector(vector) {
}

Line::Line(const Line_CSPtr other, bool sameID) : CADEntity(other, sameID), geo::Vector(other->start(), other->end()) {
}



std::vector<EntityCoordinate> Line::snapPoints(const geo::Coordinate& coord, const SimpleSnapConstrain & constrain, double minDistanceToSnap, int maxNumberOfSnapPoints) const {
    std::vector<EntityCoordinate> points;

    if (constrain.constrain() & SimpleSnapConstrain::LOGICAL) {
        points.emplace_back(start(), 0);
        points.emplace_back(end(), 1);
        points.emplace_back(end().mid(start()), 1);
    }


    const geo::Coordinate npoe = nearestPointOnPath(coord);
    if (constrain.constrain() & SimpleSnapConstrain::ON_ENTITYPATH) {
        points.emplace_back(npoe, 2);
    }

    if (constrain.constrain() & SimpleSnapConstrain::ON_ENTITY) {
        if (this->nearestPointOnEntity(coord).distanceTo(coord)<minDistanceToSnap) {
            points.emplace_back(npoe, 3);
        }
    }

    // Cleanup array of snappoints
    Snapable::snapPointsCleanup(points, coord, maxNumberOfSnapPoints, minDistanceToSnap);
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

CADEntity_CSPtr Line::mirror(const geo::Coordinate& axis1,
                             const geo::Coordinate& axis2) const {
    auto newLine = std::make_shared<Line>(this->start().mirror(axis1,
                                                               axis2),
                                          this->end().mirror(axis1,
                                                             axis2),
                                          layer());
    newLine->setID(this->id());
    return newLine;
}

const geo::Area Line::boundingBox() const {
    return geo::Area(start(), end());
}

CADEntity_CSPtr Line::modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const {
    auto newEntity = std::make_shared<Line>(this->start(), this->end(), layer, metaInfo);
    newEntity->setID(this->id());
    return newEntity;
}

std::map<unsigned int, lc::geo::Coordinate> Line::dragPoints() const {
    std::map<unsigned int, lc::geo::Coordinate> points;

	points[0] = start();
	points[1] = end();

	return points;
}

CADEntity_CSPtr Line::setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const {
    try {
	    auto newEntity = std::make_shared<Line>(dragPoints.at(0), dragPoints.at(1), layer(), metaInfo());
	    newEntity->setID(this->id());
	    return newEntity;
    }
    catch(const std::out_of_range& e) {
        //A point was not in the map, don't change the entity
        return shared_from_this();
    }
}
