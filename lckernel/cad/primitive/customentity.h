#pragma once

#include <cad/interface/unmanageddraggable.h>
#include "insert.h"

namespace lc {
    namespace entity {
        class CustomEntity : public Insert, public UnmanagedDraggable {
            public:
                CustomEntity(const builder::InsertBuilder& builder);
                CustomEntity(const Insert_CSPtr& insert, bool sameID = false);

                std::vector<EntityCoordinate> snapPoints(const geo::Coordinate& coord,
                                                                 const SimpleSnapConstrain& simpleSnapConstrain,
                                                                 double minDistanceToSnap,
                                                                 int maxNumberOfSnapPoints) const override = 0;

                geo::Coordinate nearestPointOnPath(const geo::Coordinate& coord) const override = 0;

                std::map<unsigned int, geo::Coordinate> dragPoints() const override = 0;
                CADEntity_CSPtr setDragPoints(std::map<unsigned int, lc::geo::Coordinate> dragPoints) const override;

                CADEntity_CSPtr move(const geo::Coordinate& offset) const override = 0;
                CADEntity_CSPtr copy(const geo::Coordinate& offset) const override = 0;
                CADEntity_CSPtr rotate(const geo::Coordinate& rotation_center, double rotation_angle) const override = 0;
                CADEntity_CSPtr scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const override = 0;
                CADEntity_CSPtr mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const override = 0;
                CADEntity_CSPtr modify(Layer_CSPtr layer, MetaInfo_CSPtr metaInfo, Block_CSPtr block) const override = 0;
        };
    }
}