#pragma once

#include "insert.h"

namespace lc {
    namespace entity {
        class CustomEntity : public Insert {
            public:
                CustomEntity(const builder::InsertBuilder& builder);

                virtual std::vector<EntityCoordinate> snapPoints(const geo::Coordinate& coord,
                                                                 const SimpleSnapConstrain& simpleSnapConstrain,
                                                                 double minDistanceToSnap,
                                                                 int maxNumberOfSnapPoints) const override = 0;

                virtual geo::Coordinate nearestPointOnPath(const geo::Coordinate& coord) const override = 0;
        };
    }
}