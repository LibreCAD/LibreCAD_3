#include "customentity.h"
#include "../primitive/customentity.h"
#include "lua.h"

// Phase 6 PR-6.1 sub-piece 2a — Lua adapter for the LuaRef→ScriptCallback
// wrap at the setter overload sites.
#include <scriptadapter/luacallback.h>

using namespace lc;
using namespace builder;

// Phase 6 PR-6.1 sub-piece 2a review fixup — pre-refactor `checkValues()`
// rejected any LuaRef whose type wasn't LUA_TFUNCTION at BUILD time
// (`_snapFunction.type() != LUA_TFUNCTION` etc.).  Post-refactor,
// `ScriptCallback::isNil()` only checks whether the wrapper was
// constructed at all — it can't peek at the underlying LuaRef's Lua-level
// type.  A `setSnapFunction(kaguya::LuaRef())` (nil LuaRef) would call
// `makeLuaCallback`, get a wrapper around the nil ref, store as
// non-nil ScriptCallback, and PASS checkValues() — dispatching only
// silently no-ops at invoke time.
//
// The fix here: the LuaRef-overload setters check
// `luaRef.type() == LUA_TFUNCTION` before wrapping.  Non-function
// LuaRefs are silently REJECTED (the slot stays whatever it was —
// initially nil, so checkValues() correctly fails).  This preserves
// the pre-refactor "reject non-callable at set time" semantic.
//
// For the ScriptCallback-overload setters, we cannot peek deeper — the
// caller is responsible for passing a real callable.  Documented in
// the header + inline note where the setters are defined.

// --- Snap function ---
void CustomEntityBuilder::setSnapFunction(lc::scripting::ScriptCallback snapFunction) {
    _snapFunction = std::move(snapFunction);
}

void CustomEntityBuilder::setSnapFunction(kaguya::LuaRef snapFunction) {
    // Phase 6 PR-6.1 sub-piece 2a review fixup — reject non-function
    // LuaRefs before wrapping.  Matches pre-refactor `checkValues()`
    // behavior which rejected any non-LUA_TFUNCTION LuaRef.
    if (snapFunction.type() != LUA_TFUNCTION) {
        return;
    }
    setSnapFunction(lc::lua::makeLuaCallback(std::move(snapFunction)));
}

entity::ScriptCustomEntity_CSPtr CustomEntityBuilder::build() {
    if(!checkValues()) {
        throw std::runtime_error("Missing or invalid values");
    }

    return entity::ScriptCustomEntity_CSPtr(new entity::ScriptCustomEntity(*this));
}

const lc::scripting::ScriptCallback& CustomEntityBuilder::snapFunction() const {
    return _snapFunction;
}

bool CustomEntityBuilder::checkValues(bool throwExceptions) const {
    // Phase 6 PR-6.1 sub-piece 2a review fixup — `ScriptCallback::isNil()`
    // is a wrapper-only check; the LuaRef-shape check that used to live
    // here now lives in the LuaRef-overload setters (which refuse to
    // wrap non-LUA_TFUNCTION refs).  Result: the slot stays default-
    // constructed if the caller passes a bad LuaRef, so `isNil()`
    // correctly reports the missing callback.  For ScriptCallback-
    // overload callers (native / Python), the isNil check is all we
    // can do — caller responsibility to pass a real callable.
    if (!throwExceptions) {
        return InsertBuilder::checkValues(throwExceptions) &&
               !_snapFunction.isNil() &&
               !_nearestPointFunction.isNil() &&
               !_dragPointsFunction.isNil() &&
               !_newDragPointFunction.isNil() &&
               !_dragPointsClickedFunction.isNil() &&
               !_dragPointsReleasedFunction.isNil();
    } else
    {
        if (_snapFunction.isNil()) {
            throw std::runtime_error("Snap function callback MUST be set");
        }
        if (_nearestPointFunction.isNil()) {
            throw std::runtime_error("Nearest point function callback MUST be set");
        }
        if (_dragPointsFunction.isNil()) {
            throw std::runtime_error("Drag points function callback MUST be set");
        }
        if (_newDragPointFunction.isNil()) {
            throw std::runtime_error("New drag point function callback MUST be set");
        }
        if (_dragPointsClickedFunction.isNil()) {
            throw std::runtime_error("Drag points clicked function callback MUST be set");
        }
        if (_dragPointsReleasedFunction.isNil()) {
            throw std::runtime_error("Drag points released function callback MUST be set");
        }

        return InsertBuilder::checkValues(throwExceptions);
    }
}

// --- Nearest-point function ---
const lc::scripting::ScriptCallback& CustomEntityBuilder::nearestPointFunction() const {
    return _nearestPointFunction;
}

void CustomEntityBuilder::setNearestPointFunction(lc::scripting::ScriptCallback nearestPointFunction) {
    _nearestPointFunction = std::move(nearestPointFunction);
}

void CustomEntityBuilder::setNearestPointFunction(const kaguya::LuaRef& nearestPointFunction) {
    // See setSnapFunction's rationale for the LUA_TFUNCTION guard.
    if (nearestPointFunction.type() != LUA_TFUNCTION) {
        return;
    }
    setNearestPointFunction(lc::lua::makeLuaCallback(nearestPointFunction));
}

// --- Drag-points function ---
const lc::scripting::ScriptCallback& CustomEntityBuilder::dragPointsFunction() const {
    return _dragPointsFunction;
}

void CustomEntityBuilder::setDragPointsFunction(lc::scripting::ScriptCallback dragPointsFunction) {
    _dragPointsFunction = std::move(dragPointsFunction);
}

void CustomEntityBuilder::setDragPointsFunction(const kaguya::LuaRef& dragPointsFunction) {
    if (dragPointsFunction.type() != LUA_TFUNCTION) {
        return;
    }
    setDragPointsFunction(lc::lua::makeLuaCallback(dragPointsFunction));
}

// --- New-drag-point function ---
const lc::scripting::ScriptCallback& CustomEntityBuilder::newDragPointFunction() const {
    return _newDragPointFunction;
}

void CustomEntityBuilder::setNewDragPointFunction(lc::scripting::ScriptCallback newDragPointFunction) {
    _newDragPointFunction = std::move(newDragPointFunction);
}

void CustomEntityBuilder::setNewDragPointFunction(const kaguya::LuaRef& newDragPointFunction) {
    if (newDragPointFunction.type() != LUA_TFUNCTION) {
        return;
    }
    setNewDragPointFunction(lc::lua::makeLuaCallback(newDragPointFunction));
}

// --- Drag-points-clicked function ---
const lc::scripting::ScriptCallback& CustomEntityBuilder::dragPointsClickedFunction() const {
    return _dragPointsClickedFunction;
}

void CustomEntityBuilder::setDragPointsClickedFunction(lc::scripting::ScriptCallback dragPointsClickedFunction) {
    _dragPointsClickedFunction = std::move(dragPointsClickedFunction);
}

void CustomEntityBuilder::setDragPointsClickedFunction(const kaguya::LuaRef& dragPointsClickedFunction) {
    if (dragPointsClickedFunction.type() != LUA_TFUNCTION) {
        return;
    }
    setDragPointsClickedFunction(lc::lua::makeLuaCallback(dragPointsClickedFunction));
}

// --- Drag-points-released function ---
const lc::scripting::ScriptCallback& CustomEntityBuilder::dragPointsReleasedFunction() const {
    return _dragPointsReleasedFunction;
}

void CustomEntityBuilder::setDragPointsReleasedFunction(lc::scripting::ScriptCallback dragPointsReleasedFunction) {
    _dragPointsReleasedFunction = std::move(dragPointsReleasedFunction);
}

void CustomEntityBuilder::setDragPointsReleasedFunction(const kaguya::LuaRef& dragPointsReleasedFunction) {
    if (dragPointsReleasedFunction.type() != LUA_TFUNCTION) {
        return;
    }
    setDragPointsReleasedFunction(lc::lua::makeLuaCallback(dragPointsReleasedFunction));
}
