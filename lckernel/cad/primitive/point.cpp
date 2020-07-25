#include <memory>
#include <cad/primitive/point.h>

using namespace lc;
using namespace entity;

Point::Point(double x, double y,
             meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) :
        CADEntity(std::move(layer), std::move(metaInfo), std::move(block)),
        geo::Coordinate(x, y) {
}

Point::Point(geo::Coordinate coord,
             meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) :
        CADEntity(std::move(layer), std::move(metaInfo), std::move(block)),
        geo::Coordinate(std::move(coord)) {
}

Point::Point(const Point_CSPtr& other, bool sameID) : CADEntity(other, sameID),  geo::Coordinate(other->x(), other->y()) {
}

Point::Point(const builder::PointBuilder& builder) :
    CADEntity(builder),
    geo::Coordinate(builder.coordinate()) {
}

CADEntity_CSPtr Point::move(const geo::Coordinate& offset) const {
    auto newCoordinate = std::make_shared<Point>(this->x() + offset.x(), this->y() + offset.y(), layer(), metaInfo(), block());
    newCoordinate->setID(this->id());
    return newCoordinate;
}

CADEntity_CSPtr Point::copy(const geo::Coordinate& offset) const {
    auto newCoordinate = std::make_shared<Point>(this->x() + offset.x(), this->y() + offset.y(), layer(), metaInfo(), block());
    return newCoordinate;
}

CADEntity_CSPtr Point::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto rotcord = geo::Coordinate(this->x(), this->y()).rotate(rotation_center, rotation_angle);
    auto newCoordinate = std::make_shared<Point>(rotcord.x(), rotcord.y(), layer(), metaInfo(), block());
    newCoordinate->setID(this->id());
    return newCoordinate;
}

CADEntity_CSPtr Point::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto rotcord = geo::Coordinate(this->x(), this->y()).scale(scale_center, scale_factor);
    auto newCoordinate = std::make_shared<Point>(rotcord.x(), rotcord.y(), layer(), metaInfo(), block());
    newCoordinate->setID(this->id());
    return newCoordinate;
}

CADEntity_CSPtr Point::mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const {
    auto rotcord = geo::Coordinate(this->x(), this->y()).rotate(axis1, axis2);
    auto newCoordinate = std::make_shared<Point>(rotcord.x(), rotcord.y(), layer(), metaInfo(), block());
    newCoordinate->setID(this->id());
    return newCoordinate;
}

const geo::Area Point::boundingBox() const {
    return geo::Area(geo::Coordinate(this->x(), this->y()), 0., 0.);
}

CADEntity_CSPtr Point::modify(meta::Layer_CSPtr layer, const meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const {
    auto newEntity = std::make_shared<Point>(this->x(), this->y(),
                                             layer,
                                             metaInfo,
                                             block
    );
    newEntity->setID(this->id());

    return newEntity;
}

// additional options
geo::Coordinate Point::nearestPointOnPath(const Coordinate& coord) const {
    return geo::Coordinate(this->x(), this->y());
}

geo::Coordinate Point::nearestPointOnEntity(const Coordinate& coord) const {
    return geo::Coordinate(this->x(), this->y());
}


std::vector<EntityCoordinate> Point::snapPoints(const geo::Coordinate& coord,
                                               const SimpleSnapConstrain & constrain,
                                               double minDistanceToSnap,
                                               int maxNumberOfSnapPoints) const {
    std::vector<EntityCoordinate> points;
    if ((bool) (constrain.constrain() & SimpleSnapConstrain::LOGICAL)) {
	    points.emplace_back(geo::Coordinate(this->x(), this->y()),0);
    }

    Snapable::snapPointsCleanup(points, coord, maxNumberOfSnapPoints, minDistanceToSnap);
    return points;
}

std::map<unsigned int, lc::geo::Coordinate> Point::dragPoints() const {
	std::map<unsigned int, lc::geo::Coordinate> points;
	points[0]=(geo::Coordinate(this->x(), this->y()));
	return points;
}

CADEntity_CSPtr Point::setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const {
    try {
	    auto newEntity = std::make_shared<Point>(dragPoints.at(0),
                                                layer(),
                                                metaInfo(),
                                                block()
        );
	    newEntity->setID(this->id());
	    return newEntity;
    }
    catch(const std::out_of_range& e) {
        //A point was not in the map, don't change the entity
        return shared_from_this();
    }
}
