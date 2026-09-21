/**
* @file
* @section DESCRIPTION
*
* DimOrdinate builder
*/

#pragma once

#include "dimension.h"

namespace lc {
namespace builder {
class DimOrdinateBuilder : public DimensionBuilder {
public:
    DimOrdinateBuilder();

    const geo::Coordinate& featurePoint() const;
    void setFeaturePoint(const geo::Coordinate& featurePoint);

    const geo::Coordinate& leaderEndPoint() const;
    void setLeaderEndPoint(const geo::Coordinate& leaderEndPoint);

    bool xType() const;
    void setXType(bool xType);

    /**
     * @brief Place the dimension the way a user draws it: from the feature
     * point out to where the leader ends.
     *
     * As in AutoCAD, the leader's direction picks the axis. A leader that runs
     * mostly sideways from the feature measures Y, anything else measures X.
     * The text goes at the end of the leader, on the far side of it.
     * The datum, definitionPoint(), is left as it is.
     */
    void dimAuto(const geo::Coordinate& featurePoint, const geo::Coordinate& leaderEndPoint);

    /**
     * @brief Whether a leader from featurePoint to leaderEndPoint measures X.
     * The rule dimAuto() places a dimension by.
     */
    static bool measuresX(const geo::Coordinate& featurePoint, const geo::Coordinate& leaderEndPoint);

    entity::DimOrdinate_CSPtr build();

    void copy(entity::DimOrdinate_CSPtr entity);

private:
    geo::Coordinate _featurePoint;
    geo::Coordinate _leaderEndPoint;
    bool _xType;
};
}
}
