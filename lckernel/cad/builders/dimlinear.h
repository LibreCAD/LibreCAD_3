/**
* @file
* @section DESCRIPTION
*
* Dimlinear builder
*/

#pragma once

#include "dimension.h"

namespace lc {
namespace builder {
class DimLinearBuilder : public DimensionBuilder {
public:
    DimLinearBuilder();

    double angle() const;
    void setAngle(double angle);

    double oblique() const;
    void setOblique(double oblique);

    const geo::Coordinate& definitionPoint2() const;
    void setDefinitionPoint2(const geo::Coordinate& definitionPoint2);

    const geo::Coordinate& definitionPoint3() const;
    void setDefinitionPoint3(const geo::Coordinate& definitionPoint3);

    lc::entity::DimLinear_CSPtr build() const;

private:
    double _angle;
    double _oblique;
    geo::Coordinate _definitionPoint2;
    geo::Coordinate _definitionPoint3;
};
}
}
