#include "dimlinear.h"
#include <cad/primitive/dimlinear.h>

lc::builder::DimLinearBuilder::DimLinearBuilder() {
    _angle = 0;
    _oblique = 0;
}

lc::entity::DimLinear_CSPtr lc::builder::DimLinearBuilder::build() const {
    return lc::entity::DimLinear_CSPtr(new lc::entity::DimLinear(*this));
}

double lc::builder::DimLinearBuilder::angle() const {
    return _angle;
}

void lc::builder::DimLinearBuilder::setAngle(double angle) {
    _angle = angle;
}

double lc::builder::DimLinearBuilder::oblique() const {
    return _oblique;
}

void lc::builder::DimLinearBuilder::setOblique(double oblique) {
    _oblique = oblique;
}

const lc::geo::Coordinate& lc::builder::DimLinearBuilder::definitionPoint2() const {
    return _definitionPoint2;
}

void lc::builder::DimLinearBuilder::setDefinitionPoint2(const lc::geo::Coordinate& definitionPoint2) {
    _definitionPoint2 = definitionPoint2;
}

const lc::geo::Coordinate& lc::builder::DimLinearBuilder::definitionPoint3() const {
    return _definitionPoint3;
}

void lc::builder::DimLinearBuilder::setDefinitionPoint3(const lc::geo::Coordinate& definitionPoint3) {
    _definitionPoint3 = definitionPoint3;
}
