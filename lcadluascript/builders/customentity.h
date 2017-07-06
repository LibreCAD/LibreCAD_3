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
                CustomEntityBuilder();

                void setSnapFunction(LuaIntf::LuaRef snapFunction);
                const LuaIntf::LuaRef& snapFunction() const;

                const LuaIntf::LuaRef& nearestPointFunction() const;
                void setNearestPointFunction(const LuaIntf::LuaRef& nearestPointFunction);

                bool checkValues() override;

                entity::LuaCustomEntity_CSPtr build();

            private:
                LuaIntf::LuaRef _snapFunction;
                LuaIntf::LuaRef _nearestPointFunction;
        };
    }
}