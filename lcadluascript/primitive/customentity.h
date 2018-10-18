#pragma once

#include "../builders/customentity.h"
#include <cad/primitive/customentity.h>
#include <cad/interface/snapable.h>

namespace lc {
    namespace entity {
        class LuaCustomEntity : public CustomEntity {
            public:
                LuaCustomEntity(const builder::CustomEntityBuilder& builder);

                /**
                 * @brief Constructor used in operations which only affect the insert
                 * @param customEntity Custom entity to copy functions from
                 * @param insert Insert to copy data from, should be an instance of Insert
                 */
                LuaCustomEntity(const Insert_CSPtr& insert, const LuaCustomEntity_CSPtr& customEntity, bool sameID = false);

                std::vector<EntityCoordinate> snapPoints(const geo::Coordinate& coord,
                                                         const SimpleSnapConstrain& simpleSnapConstrain,
                                                         double minDistanceToSnap,
                                                         int maxNumberOfSnapPoints) const override;

                geo::Coordinate nearestPointOnPath(const geo::Coordinate& coord) const override;

                std::map<unsigned int, geo::Coordinate> dragPoints() const override;
                void setDragPoint(lc::geo::Coordinate position) const override;
                void onDragPointClick(lc::operation::Builder_SPtr builder, unsigned int point) const override;
                void onDragPointRelease(lc::operation::Builder_SPtr builder) const override;

                CADEntity_CSPtr modifyInsert(const CADEntity_CSPtr& insert) const;
                CADEntity_CSPtr move(const geo::Coordinate& offset) const override;
                CADEntity_CSPtr copy(const geo::Coordinate& offset) const override;
                CADEntity_CSPtr rotate(const geo::Coordinate& rotation_center, double rotation_angle) const override;
                CADEntity_CSPtr scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const override;
                CADEntity_CSPtr mirror(const geo::Coordinate& axis1, const geo::Coordinate& axis2) const override;
                CADEntity_CSPtr modify(meta::Layer_CSPtr layer, meta::MetaInfo_CSPtr metaInfo, meta::Block_CSPtr block) const override;

            private:
                LuaIntf::LuaRef _snapPoints;
                LuaIntf::LuaRef _nearestPoint;
                LuaIntf::LuaRef _dragPoints;
                LuaIntf::LuaRef _newDragPoint;
                LuaIntf::LuaRef _dragPointClick;
                LuaIntf::LuaRef _dragPointRelease;
        };

        DECLARE_SHORT_SHARED_PTR(LuaCustomEntity)
    }
}