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

Point::Point(const Point_CSPtr& other, bool sameID) : CADEntity(other, sameID),  geo::Coordinate(other->x(), other->y(), other->z()) {
}

Point::Point(const builder::PointBuilder& builder) :
    CADEntity(builder),
    geo::Coordinate(builder.coordinate()) {
}

CADEntity_CSPtr Point::move(const geo::Coordinate& offset) const {
    auto newCoordinate = std::make_shared<Point>(geo::Coordinate(x(), y(), z()) + offset, layer(), metaInfo(), block());
    newCoordinate->setID(this->id());
    return newCoordinate;
}

CADEntity_CSPtr Point::copy(const geo::Coordinate& offset) const {
    auto newCoordinate = std::make_shared<Point>(geo::Coordinate(x(), y(), z()) + offset, layer(), metaInfo(), block());
    return newCoordinate;
}

CADEntity_CSPtr Point::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    const auto rotated = geo::Coordinate(x(), y()).rotate(
        geo::Coordinate(rotation_center.x(), rotation_center.y()), rotation_angle);
    auto newCoordinate = std::make_shared<Point>(geo::Coordinate(rotated.x(), rotated.y(), z()), layer(), metaInfo(), block());
    newCoordinate->setID(this->id());
    return newCoordinate;
}

CADEntity_CSPtr Point::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    const auto scaled = geo::Coordinate(x(), y(), z()).scale(scale_center, scale_factor);
    auto newCoordinate = std::make_shared<Point>(scaled, layer(), metaInfo(), block());
    newCoordinate->setID(this->id());
    return newCoordinate;
}

CADEntity_CSPtr Point::mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const {
    const auto direction = axis2 - axis1;
    if (direction.x() == 0.0 && direction.y() == 0.0 && direction.z() == 0.0) {
        auto unchanged = std::make_shared<Point>(geo::Coordinate(x(), y(), z()), layer(), metaInfo(), block());
        unchanged->setID(this->id());
        return unchanged;
    }
    const bool planarAxis = axis1.z() == axis2.z();
    const auto reflected = (planarAxis ? geo::Coordinate(x(), y(), axis1.z())
                                       : geo::Coordinate(x(), y(), z())).mirror(axis1, axis2);
    const auto position = planarAxis ? geo::Coordinate(reflected.x(), reflected.y(), z()) : reflected;
    auto newCoordinate = std::make_shared<Point>(position, layer(), metaInfo(), block());
    newCoordinate->setID(this->id());
    return newCoordinate;
}

const geo::Area Point::boundingBox() const {
    const geo::Coordinate position(x(), y(), z());
    return geo::Area(position, position);
}

CADEntity_CSPtr Point::modify(meta::Layer_CSPtr layer, const meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const {
    auto newEntity = std::make_shared<Point>(geo::Coordinate(x(), y(), z()),
                     layer,
                     metaInfo,
                     block
                                            );
    newEntity->setID(this->id());

    return newEntity;
}

// additional options
geo::Coordinate Point::nearestPointOnPath(const Coordinate& coord) const {
    return geo::Coordinate(x(), y(), z());
}

geo::Coordinate Point::nearestPointOnEntity(const Coordinate& coord) const {
    return geo::Coordinate(x(), y(), z());
}


std::vector<EntityCoordinate> Point::snapPoints(const geo::Coordinate& coord,
        const SimpleSnapConstrain & constrain,
        double minDistanceToSnap,
        int maxNumberOfSnapPoints) const {
    std::vector<EntityCoordinate> points;
    if ((bool) (constrain.constrain() & SimpleSnapConstrain::LOGICAL)) {
        points.emplace_back(geo::Coordinate(x(), y(), z()),0);
    }

    Snapable::snapPointsCleanup(points, coord, maxNumberOfSnapPoints, minDistanceToSnap);
    return points;
}

std::map<unsigned int, lc::geo::Coordinate> Point::dragPoints() const {
    std::map<unsigned int, lc::geo::Coordinate> points;
    points[0]=(geo::Coordinate(x(), y(), z()));
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

PropertiesMap Point::availableProperties() const {
    PropertiesMap propertyValues;
    propertyValues["coordinate"] = lc::geo::Coordinate(this->x(), this->y(), this->z());
    return propertyValues;
}

CADEntity_CSPtr Point::setProperties(const PropertiesMap& propertiesMap) const {
    lc::geo::Coordinate coordp = lc::geo::Coordinate(this->x(), this->y(), this->z());

    for (auto iter = propertiesMap.begin(); iter != propertiesMap.end(); ++iter) {
        if (iter->first == "coordinate") {
            coordp = boost::get<lc::geo::Coordinate>(iter->second);
        }
    }

    auto pointEntity = std::make_shared<Point>(coordp, layer(), metaInfo(), block());
    pointEntity->setID(this->id());
    return pointEntity;
}
