#include "arc.h"
#include <cad/primitive/arc.h>

using namespace lc::builder;

const lc::geo::Coordinate& ArcBuilder::center() const {
    return _center;
}

ArcBuilder* ArcBuilder::setCenter(const lc::geo::Coordinate& center) {
    _center = center;
    return this;
}

double ArcBuilder::radius() const {
    return _radius;
}

ArcBuilder* ArcBuilder::setRadius(double radius) {
    _radius = radius;
    return this;
}

double ArcBuilder::startAngle() const {
    return _startAngle;
}

ArcBuilder* ArcBuilder::setStartAngle(double startAngle) {
    _startAngle = startAngle;
    return this;
}

double ArcBuilder::endAngle() const {
    return _endAngle;
}

ArcBuilder* ArcBuilder::setEndAngle(double endAngle) {
    _endAngle = endAngle;
    return this;
}

bool ArcBuilder::isCCW() const {
    return _isCCW;
}

ArcBuilder* ArcBuilder::setIsCCW(bool isCCW) {
    _isCCW = isCCW;
    return this;
}

lc::entity::Arc_CSPtr ArcBuilder::build() {
    checkValues(true);

    return entity::Arc_CSPtr(new entity::Arc(*this));
}
