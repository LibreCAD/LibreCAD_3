#include "dimdiametric.h"
#include <cad/primitive/dimdiametric.h>

lc::builder::DimDiametricBuilder::DimDiametricBuilder() {
    _leader = 0;
}

double lc::builder::DimDiametricBuilder::leader() const {
    return _leader;
}

void lc::builder::DimDiametricBuilder::setLeader(double leader) {
    _leader = leader;
}

const lc::geo::Coordinate& lc::builder::DimDiametricBuilder::definitionPoint2() const {
    return _definitionPoint2;
}

void lc::builder::DimDiametricBuilder::setDefinitionPoint2(const lc::geo::Coordinate& definitionPoint2) {
    _definitionPoint2 = definitionPoint2;
}

lc::entity::DimDiametric_CSPtr lc::builder::DimDiametricBuilder::build() {
    checkValues(true);

    return lc::entity::DimDiametric_CSPtr(new lc::entity::DimDiametric(*this));
}
