#pragma once

#include "dimension.h"

namespace lc {
namespace builder {
class DimDiametricBuilder : public DimensionBuilder {
public:
    DimDiametricBuilder();

    double leader() const;
    void setLeader(double leader);

    const geo::Coordinate& definitionPoint2() const;
    void setDefinitionPoint2(const geo::Coordinate& definitionPoint2);

    lc::entity::DimDiametric_CSPtr build();

private:
    double _leader;
    geo::Coordinate _definitionPoint2;
};
}
}