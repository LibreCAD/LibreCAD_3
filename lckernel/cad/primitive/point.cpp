#include <memory>
#include <cad/primitive/point.h>

using namespace lc;
using namespace entity;

Point::Point(const double x, const double y,
             Layer_CSPtr layer, MetaInfo_CSPtr metaInfo, Block_CSPtr block) :
        CADEntity(std::move(layer), std::move(metaInfo), std::move(block)),
        geo::Coordinate(x, y) {
}

Point::Point(geo::Coordinate const& coord,
             Layer_CSPtr layer, MetaInfo_CSPtr metaInfo, Block_CSPtr block) :
        CADEntity(std::move(layer), std::move(metaInfo), std::move(block)),
        geo::Coordinate(coord) {
}

Point::Point(const Point_CSPtr other, bool sameID) : CADEntity(other, sameID),  geo::Coordinate(other->x(), other->y()) {
}

Point::Point(const builder::PointBuilder& builder) :
    CADEntity(builder),
    geo::Coordinate(builder.coordinate()) {
}

CADEntity_CSPtr Point::move(const geo::Coordinate& offset) const {
    auto newCoordinate = std::make_shared<Point>(this->x() + offset.x(), this->y() + offset.y(), layer());
    newCoordinate->setID(this->id());
    return newCoordinate;
}

CADEntity_CSPtr Point::copy(const geo::Coordinate& offset) const {
    auto newCoordinate = std::make_shared<Point>(this->x() + offset.x(), this->y() + offset.y(), layer());
    return newCoordinate;
}

CADEntity_CSPtr Point::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto rotcord = geo::Coordinate(this->x(), this->y()).rotate(rotation_center, rotation_angle);
    auto newCoordinate = std::make_shared<Point>(rotcord.x(), rotcord.y(), layer());
    newCoordinate->setID(this->id());
    return newCoordinate;
}

CADEntity_CSPtr Point::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto rotcord = geo::Coordinate(this->x(), this->y()).scale(scale_center, scale_factor);
    auto newCoordinate = std::make_shared<Point>(rotcord.x(), rotcord.y(), layer());
    newCoordinate->setID(this->id());
    return newCoordinate;
}

CADEntity_CSPtr Point::mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const {
    auto rotcord = geo::Coordinate(this->x(), this->y()).rotate(axis1, axis2);
    auto newCoordinate = std::make_shared<Point>(rotcord.x(), rotcord.y(), layer());
    newCoordinate->setID(this->id());
    return newCoordinate;
}

const geo::Area Point::boundingBox() const {
    return geo::Area(geo::Coordinate(this->x(), this->y()), 0., 0.);
}

CADEntity_CSPtr Point::modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo, Block_CSPtr block) const {
    auto newEntity = std::make_shared<Point>(this->x(), this->y(),
                                             layer,
                                             metaInfo,
                                             block
    );
    newEntity->setID(this->id());

    return newEntity;
}


