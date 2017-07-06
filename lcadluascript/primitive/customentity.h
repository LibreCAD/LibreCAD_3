#pragma once

#include "../builders/customentity.h"
#include <cad/primitive/customentity.h>
#include <cad/interface/snapable.h>

namespace lc {
    namespace entity {
        class LuaCustomEntity : public CustomEntity {
            public:
                LuaCustomEntity(const builder::CustomEntityBuilder& builder);

                virtual std::vector<EntityCoordinate> snapPoints(const geo::Coordinate& coord,
                                                                 const SimpleSnapConstrain& simpleSnapConstrain,
                                                                 double minDistanceToSnap,
                                                                 int maxNumberOfSnapPoints) const override;

                virtual geo::Coordinate nearestPointOnPath(const geo::Coordinate& coord) const override;

            private:
                LuaIntf::LuaRef _snapPoints;
                LuaIntf::LuaRef _nearestPoint;
        };

        DECLARE_SHORT_SHARED_PTR(LuaCustomEntity)
    }
}