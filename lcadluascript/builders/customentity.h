#pragma once

#include <lclua.h>
#include <cad/builders/insert.h>
#include <kaguya/include/kaguya/lua_ref.hpp>

namespace lc {
    namespace entity {
        class LuaCustomEntity;
        DECLARE_SHORT_SHARED_PTR(LuaCustomEntity)
    }
    namespace builder {
        class CustomEntityBuilder : public InsertBuilder {
            public:
                CustomEntityBuilder() = default;

                void setSnapFunction(kaguya::LuaRef snapFunction);
                const kaguya::LuaRef& snapFunction() const;

                const kaguya::LuaRef& nearestPointFunction() const;
                void setNearestPointFunction(const kaguya::LuaRef& nearestPointFunction);

                const kaguya::LuaRef& dragPointsFunction() const;
                void setDragPointsFunction(const kaguya::LuaRef& dragPointsFunction);

                const kaguya::LuaRef& newDragPointFunction() const;
                void setNewDragPointFunction(const kaguya::LuaRef& newDragPointFunction);

                const kaguya::LuaRef& dragPointsClickedFunction() const;
                void setDragPointsClickedFunction(const kaguya::LuaRef& dragPointsClickedFunction);

                const kaguya::LuaRef& dragPointsReleasedFunction() const;
                void setDragPointsReleasedFunction(const kaguya::LuaRef& dragPointsReleasedFunction);

                bool checkValues() override;

                entity::LuaCustomEntity_CSPtr build();

            private:
                kaguya::LuaRef _snapFunction;
                kaguya::LuaRef _nearestPointFunction;
                kaguya::LuaRef _dragPointsFunction;
                kaguya::LuaRef _newDragPointFunction;
                kaguya::LuaRef _dragPointsClickedFunction;
                kaguya::LuaRef _dragPointsReleasedFunction;
        };
    }
}