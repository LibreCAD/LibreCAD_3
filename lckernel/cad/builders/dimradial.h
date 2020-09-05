/**
* @file
* @section DESCRIPTION
*
* Dimradial builder
*/

#pragma once

#include "dimension.h"

namespace lc {
namespace builder {
class DimRadialBuilder : public DimensionBuilder {
public:
    DimRadialBuilder();

    double leader() const;
    void setLeader(double leader);

    const geo::Coordinate& definitionPoint2() const;
    void setDefinitionPoint2(const geo::Coordinate& definitionPoint2);

    void dimAuto(lc::geo::Coordinate definitionPoint,
                 lc::geo::Coordinate definitionPoint2);

    lc::entity::DimRadial_CSPtr build() const;

private:
    double _leader;
    geo::Coordinate _definitionPoint2;
};
}
}
