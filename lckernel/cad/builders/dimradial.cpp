#include "dimradial.h"
#include <cad/primitive/dimradial.h>

lc::builder::DimRadialBuilder::DimRadialBuilder() {
    _leader = 0;
}

double lc::builder::DimRadialBuilder::leader() const {
    return _leader;
}

void lc::builder::DimRadialBuilder::setLeader(double leader) {
    _leader = leader;
}

const lc::geo::Coordinate& lc::builder::DimRadialBuilder::definitionPoint2() const {
    return _definitionPoint2;
}

void lc::builder::DimRadialBuilder::setDefinitionPoint2(const lc::geo::Coordinate& definitionPoint2) {
    _definitionPoint2 = definitionPoint2;
}

lc::entity::DimRadial_CSPtr lc::builder::DimRadialBuilder::build() const {
    checkEntityConstraints();

    return lc::entity::DimRadial_CSPtr(new lc::entity::DimRadial(*this));
}

void lc::builder::DimRadialBuilder::dimAuto(lc::geo::Coordinate definitionPoint, lc::geo::Coordinate definitionPoint2) {
    _definitionPoint = definitionPoint;
    _definitionPoint2 = definitionPoint2;
    _middleOfText = definitionPoint.mid(definitionPoint2);
}
