#include "circle.h"
#include <cad/primitive/circle.h>

lc::builder::CircleBuilder::CircleBuilder() {

}

lc::builder::CircleBuilder::~CircleBuilder() {

}

const lc::geo::Coordinate& lc::builder::CircleBuilder::center() const {
    return _center;
}

lc::builder::CircleBuilder* lc::builder::CircleBuilder::setCenter(const lc::geo::Coordinate& center) {
    _center = center;
    return this;
}

double lc::builder::CircleBuilder::radius() const {
    return _radius;
}

lc::builder::CircleBuilder* lc::builder::CircleBuilder::setRadius(double radius) {
    _radius = radius;
    return this;
}

lc::entity::Circle_CSPtr lc::builder::CircleBuilder::build() {
    return entity::Circle_CSPtr(new entity::Circle(*this));
}
