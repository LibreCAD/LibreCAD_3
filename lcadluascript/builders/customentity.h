#pragma once

#include <lclua.h>
#include <cad/builders/insert.h>

namespace lc {
    namespace entity {
        class LuaCustomEntity;
        DECLARE_SHORT_SHARED_PTR(LuaCustomEntity)
    }
    namespace builder {
        class CustomEntityBuilder : public InsertBuilder {
            public:
                CustomEntityBuilder() = default;

                void setSnapFunction(LuaIntf::LuaRef snapFunction);
                const LuaIntf::LuaRef& snapFunction() const;

                const LuaIntf::LuaRef& nearestPointFunction() const;
                void setNearestPointFunction(const LuaIntf::LuaRef& nearestPointFunction);

                const LuaIntf::LuaRef& dragPointsFunction() const;
                void setDragPointsFunction(const LuaIntf::LuaRef& dragPointsFunction);

                const LuaIntf::LuaRef& newDragPointFunction() const;
                void setNewDragPointFunction(const LuaIntf::LuaRef& newDragPointFunction);

                const LuaIntf::LuaRef& dragPointsClickedFunction() const;
                void setDragPointsClickedFunction(const LuaIntf::LuaRef& dragPointsClickedFunction);

                const LuaIntf::LuaRef& dragPointsReleasedFunction() const;
                void setDragPointsReleasedFunction(const LuaIntf::LuaRef& dragPointsReleasedFunction);

                bool checkValues() override;

                entity::LuaCustomEntity_CSPtr build();

            private:
                LuaIntf::LuaRef _snapFunction;
                LuaIntf::LuaRef _nearestPointFunction;
                LuaIntf::LuaRef _dragPointsFunction;
                LuaIntf::LuaRef _newDragPointFunction;
                LuaIntf::LuaRef _dragPointsClickedFunction;
                LuaIntf::LuaRef _dragPointsReleasedFunction;
        };
    }
}