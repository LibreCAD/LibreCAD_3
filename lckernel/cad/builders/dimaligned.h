#pragma once

#include "dimension.h"

namespace lc {
namespace builder {
class DimAlignedBuilder : public DimensionBuilder {
public:
    DimAlignedBuilder() = default;

    const geo::Coordinate& definitionPoint2() const;
    void setDefinitionPoint2(const geo::Coordinate& definitionPoint2);

    const geo::Coordinate& definitionPoint3() const;
    void setDefinitionPoint3(const geo::Coordinate& definitionPoint3);

    void dimAuto(geo::Coordinate p2, geo::Coordinate p3, geo::Coordinate middleOfText);

    entity::DimAligned_CSPtr build();

private:
    geo::Coordinate _definitionPoint2;
    geo::Coordinate _definitionPoint3;
};
}
}