#pragma once

#include <cad/geometry/geocoordinate.h>

namespace lc {
namespace entity {
class Tangentable {
public:
    /**
     * @brief Give line tangent points on entity
     * @param extPoint First point of line tangent
     * @return Vector of tangent end points
     */
    virtual std::vector<lc::geo::Coordinate> lineTangentPointsOnEntity(const lc::geo::Coordinate& extPoint) = 0;
};
}
}