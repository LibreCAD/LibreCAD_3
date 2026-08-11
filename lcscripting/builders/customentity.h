#pragma once

// Phase 6 PR-6.1 sub-piece 2b — CustomEntityBuilder physically moved
// from lcadluascript to lcscripting.  The neutral layer owns the
// class so both `lcadluascript` and `lcadpythonscript` can link it
// and bind it to their respective languages.
//
// The setters take `lc::scripting::ScriptCallback` ONLY.  Lua-side
// callers use `lc::lua::wrapAndSet*` free helpers in
// `lcadluascript/scriptadapter/customentitybuilderlua.{h,cpp}` — those
// wrap `kaguya::LuaRef` via `makeLuaCallback` with the LUA_TFUNCTION
// guard preserved from sub-piece 2a's fixup (rejects non-callable
// LuaRefs at set time so `checkValues()` correctly reports missing
// callbacks).

#include "../scriptcallback.h"

#include <cad/builders/insert.h>

namespace lc {
namespace entity {
// Forward-declare the moved class (see lcscripting/primitive/customentity.h).
class ScriptCustomEntity;
DECLARE_SHORT_SHARED_PTR(ScriptCustomEntity)
// Phase 6 PR-6.1 sub-piece 2b — API-compat typedefs for pre-refactor
// spellings.  Preserved from sub-piece 2a.
using LuaCustomEntity = ScriptCustomEntity;
using LuaCustomEntity_CSPtr = ScriptCustomEntity_CSPtr;
using LuaCustomEntity_SPtr = ScriptCustomEntity_SPtr;
}
namespace builder {

class CustomEntityBuilder : public InsertBuilder {
public:
    CustomEntityBuilder() = default;

    // Setters take neutral ScriptCallback only.  Callable-shape
    // validation:
    //   * Native callers (`nativeCallback`) — caller responsibility to
    //     pass a real callable.
    //   * Lua callers — use `lc::lua::wrapAndSet*` in lcadluascript,
    //     which applies the LUA_TFUNCTION guard.
    //   * Python callers — `makePythonCallback` doesn't fail on non-
    //     callable py::objects (they raise TypeError at invoke); sub-
    //     piece 3 will add binding-site validation.
    //
    // A default-constructed nil ScriptCallback leaves the slot empty,
    // and `checkValues()` reports it as missing — same as the
    // pre-refactor "no callback set" state.
    void setSnapFunction(lc::scripting::ScriptCallback snapFunction);
    const lc::scripting::ScriptCallback& snapFunction() const;

    const lc::scripting::ScriptCallback& nearestPointFunction() const;
    void setNearestPointFunction(lc::scripting::ScriptCallback nearestPointFunction);

    const lc::scripting::ScriptCallback& dragPointsFunction() const;
    void setDragPointsFunction(lc::scripting::ScriptCallback dragPointsFunction);

    const lc::scripting::ScriptCallback& newDragPointFunction() const;
    void setNewDragPointFunction(lc::scripting::ScriptCallback newDragPointFunction);

    const lc::scripting::ScriptCallback& dragPointsClickedFunction() const;
    void setDragPointsClickedFunction(lc::scripting::ScriptCallback dragPointsClickedFunction);

    const lc::scripting::ScriptCallback& dragPointsReleasedFunction() const;
    void setDragPointsReleasedFunction(lc::scripting::ScriptCallback dragPointsReleasedFunction);

    bool checkValues(bool throwExceptions=false) const override;

    entity::ScriptCustomEntity_CSPtr build();

private:
    // All 6 slots stored as the neutral ScriptCallback.  Dispatch in
    // ScriptCustomEntity uses the CustomEntityDispatchHook (installed
    // by lcadluascript) for the Lua concrete-type fast path and
    // falls through to `callback.invoke(...)` for non-Lua callbacks.
    lc::scripting::ScriptCallback _snapFunction;
    lc::scripting::ScriptCallback _nearestPointFunction;
    lc::scripting::ScriptCallback _dragPointsFunction;
    lc::scripting::ScriptCallback _newDragPointFunction;
    lc::scripting::ScriptCallback _dragPointsClickedFunction;
    lc::scripting::ScriptCallback _dragPointsReleasedFunction;
};
}
}
