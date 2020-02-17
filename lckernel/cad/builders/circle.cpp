#include "circle.h"
#include <cad/primitive/circle.h>
#include<iostream>

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

lc::builder::CircleBuilder* lc::builder::CircleBuilder::threeTanConstructor(lc::entity::CADEntity_CSPtr circle0, lc::entity::CADEntity_CSPtr circle1, lc::entity::CADEntity_CSPtr circle2)
{
    lc::entity::Circle_CSPtr c0 = std::dynamic_pointer_cast<const lc::entity::Circle>(circle0);
    lc::entity::Circle_CSPtr c1 = std::dynamic_pointer_cast<const lc::entity::Circle>(circle0);
    lc::entity::Circle_CSPtr c2 = std::dynamic_pointer_cast<const lc::entity::Circle>(circle0);
}

lc::entity::Circle_CSPtr lc::builder::CircleBuilder::build() {
    return entity::Circle_CSPtr(new entity::Circle(*this));
}
