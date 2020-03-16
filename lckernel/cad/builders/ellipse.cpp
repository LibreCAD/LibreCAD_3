#include "ellipse.h"
#include <cad/primitive/ellipse.h>

using namespace lc::builder;

lc::entity::Ellipse_CSPtr EllipseBuilder::build() {
    checkEntityConstraints();

    return lc::entity::Ellipse_CSPtr(new lc::entity::Ellipse(*this));
}

const lc::geo::Coordinate& EllipseBuilder::center() const {
    return _center;
}

void EllipseBuilder::setCenter(const lc::geo::Coordinate& center) {
    _center = center;
}

const lc::geo::Coordinate& EllipseBuilder::majorPoint() const {
    return _majorPoint;
}

void EllipseBuilder::setMajorPoint(const lc::geo::Coordinate& majorPoint) {
    _majorPoint = majorPoint;
}

double EllipseBuilder::minorRadius() const {
    return _minorRadius;
}

void EllipseBuilder::setMinorRadius(double minorRadius) {
    _minorRadius = minorRadius;
}

double EllipseBuilder::startAngle() const {
    return _startAngle;
}

void EllipseBuilder::setStartAngle(double startAngle) {
    _startAngle = startAngle;
}

double EllipseBuilder::endAngle() const {
    return _endAngle;
}

void EllipseBuilder::setEndAngle(double endAngle) {
    _endAngle = endAngle;
}

bool EllipseBuilder::isReversed() const {
    return _isReversed;
}

void EllipseBuilder::setIsReversed(bool isReversed) {
    _isReversed = isReversed;
}

