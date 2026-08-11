#pragma once

#include <lclua.h>
#include <cad/builders/insert.h>

// Phase 6 PR-6.1 sub-piece 2a — CustomEntityBuilder's 6 function slots
// neutralized from kaguya::LuaRef to lc::scripting::ScriptCallback.
// Setters accept ScriptCallback directly; Lua-side callers wrap their
// LuaRef via `lc::lua::makeLuaCallback` at the binding site (mirrors
// the LuaCustomEntityManager pattern from sub-piece 1).  Physical move
// to lcscripting/ deferred to sub-piece 2b.
#include <lcscripting/scriptcallback.h>
#include <kaguya/kaguya.hpp>  // still needed for the LuaRef→ScriptCallback
                              // convenience-overload setters (kaguya-
                              // familiar Lua binding code can pass a raw
                              // LuaRef and let the setter wrap it).

namespace lc {
namespace entity {
// Phase 6 PR-6.1 sub-piece 2a — was `LuaCustomEntity`.  See
// primitive/customentity.h for the compat typedef.
class ScriptCustomEntity;
DECLARE_SHORT_SHARED_PTR(ScriptCustomEntity)
using LuaCustomEntity = ScriptCustomEntity;
using LuaCustomEntity_CSPtr = ScriptCustomEntity_CSPtr;
using LuaCustomEntity_SPtr = ScriptCustomEntity_SPtr;
}
namespace builder {
class CustomEntityBuilder : public InsertBuilder {
public:
    CustomEntityBuilder() = default;

    // Phase 6 PR-6.1 sub-piece 2a — setter overloads.  The primary
    // setter takes a neutral ScriptCallback; the LuaRef overload wraps
    // via lc::lua::makeLuaCallback and forwards.  Both Lua callers
    // (with kaguya::LuaRef) and future Python/native callers (with
    // ScriptCallback) work without change.
    void setSnapFunction(lc::scripting::ScriptCallback snapFunction);
    void setSnapFunction(kaguya::LuaRef snapFunction);
    const lc::scripting::ScriptCallback& snapFunction() const;

    const lc::scripting::ScriptCallback& nearestPointFunction() const;
    void setNearestPointFunction(lc::scripting::ScriptCallback nearestPointFunction);
    void setNearestPointFunction(const kaguya::LuaRef& nearestPointFunction);

    const lc::scripting::ScriptCallback& dragPointsFunction() const;
    void setDragPointsFunction(lc::scripting::ScriptCallback dragPointsFunction);
    void setDragPointsFunction(const kaguya::LuaRef& dragPointsFunction);

    const lc::scripting::ScriptCallback& newDragPointFunction() const;
    void setNewDragPointFunction(lc::scripting::ScriptCallback newDragPointFunction);
    void setNewDragPointFunction(const kaguya::LuaRef& newDragPointFunction);

    const lc::scripting::ScriptCallback& dragPointsClickedFunction() const;
    void setDragPointsClickedFunction(lc::scripting::ScriptCallback dragPointsClickedFunction);
    void setDragPointsClickedFunction(const kaguya::LuaRef& dragPointsClickedFunction);

    const lc::scripting::ScriptCallback& dragPointsReleasedFunction() const;
    void setDragPointsReleasedFunction(lc::scripting::ScriptCallback dragPointsReleasedFunction);
    void setDragPointsReleasedFunction(const kaguya::LuaRef& dragPointsReleasedFunction);

    bool checkValues(bool throwExceptions=false) const override;

    entity::ScriptCustomEntity_CSPtr build();

private:
    // Phase 6 PR-6.1 sub-piece 2a — was `kaguya::LuaRef`; now the
    // neutral ScriptCallback.  The setter overloads wrap Lua-side
    // LuaRefs via `lc::lua::makeLuaCallback` before storing.
    lc::scripting::ScriptCallback _snapFunction;
    lc::scripting::ScriptCallback _nearestPointFunction;
    lc::scripting::ScriptCallback _dragPointsFunction;
    lc::scripting::ScriptCallback _newDragPointFunction;
    lc::scripting::ScriptCallback _dragPointsClickedFunction;
    lc::scripting::ScriptCallback _dragPointsReleasedFunction;
};
}
}
