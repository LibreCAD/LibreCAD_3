#include <memory>
#include "coordinate.h"
#include <cmath>

using namespace lc;

Coordinate::Coordinate(const double x, const double y, const Layer_CSPtr layer)
    : CADEntity(layer), geo::Coordinate(x, y) {
}

Coordinate::Coordinate(const double x, const double y, const Layer_CSPtr layer, const std::list<MetaType_CSPtr>& metaTypes)
    : CADEntity(layer, metaTypes),  geo::Coordinate(x, y) {
}

Coordinate::Coordinate(const Coordinate_CSPtr other, bool sameID) : CADEntity(other->layer(), other->metaTypes()),  geo::Coordinate(other->x(), other->y()) {
    if (sameID) {
        this->setID(other->id());
    }
}

CADEntity_CSPtr Coordinate::move(const geo::Coordinate& offset) const {
    auto newCoordinate = std::make_shared<Coordinate>(this->x() + offset.x(), this->y() + offset.y(), layer());
    newCoordinate->setID(this->id());
    return newCoordinate;
}

CADEntity_CSPtr Coordinate::copy(const geo::Coordinate& offset) const {
    auto newCoordinate = std::make_shared<Coordinate>(this->x() + offset.x(), this->y() + offset.y(), layer());
    return newCoordinate;
}

CADEntity_CSPtr Coordinate::rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const {
    auto rotcord = geo::Coordinate(this->x(), this->y()).rotate(rotation_center, rotation_angle);
    auto newCoordinate = std::make_shared<Coordinate>(rotcord.x(), rotcord.y(), layer());
    newCoordinate->setID(this->id());
    return newCoordinate;
}

CADEntity_CSPtr Coordinate::scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const {
    auto rotcord = geo::Coordinate(this->x(), this->y()).scale(scale_center, scale_factor);
    auto newCoordinate = std::make_shared<Coordinate>(rotcord.x(), rotcord.y(), layer());
    newCoordinate->setID(this->id());
    return newCoordinate;
}

const geo::Area Coordinate::boundingBox() const {

}

Quadratic_CSPtr Coordinate::quadratic_equation() const {

}
