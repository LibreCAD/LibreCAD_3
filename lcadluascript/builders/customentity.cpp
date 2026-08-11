#include "customentity.h"
#include "../primitive/customentity.h"
#include "lua.h"

// Phase 6 PR-6.1 sub-piece 2a — Lua adapter for the LuaRef→ScriptCallback
// wrap at the setter overload sites.
#include <scriptadapter/luacallback.h>

using namespace lc;
using namespace builder;

// --- Snap function ---
void CustomEntityBuilder::setSnapFunction(lc::scripting::ScriptCallback snapFunction) {
    _snapFunction = std::move(snapFunction);
}

void CustomEntityBuilder::setSnapFunction(kaguya::LuaRef snapFunction) {
    // Phase 6 PR-6.1 sub-piece 2a — LuaRef overload wraps via the Lua
    // adapter's makeLuaCallback into a ScriptCallback with runtime tag
    // "lua"; ScriptCustomEntity's dispatch uses unwrapLuaCallback to
    // recover the LuaRef and call kaguya directly (preserving
    // concrete Insert type per the sub-piece 1 fixup lesson).
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
    // Phase 6 PR-6.1 sub-piece 2a — was `_snapFunction.type() ==
    // LUA_TFUNCTION` etc.; now checks whether the neutral ScriptCallback
    // is non-nil.  For Lua callbacks: a LuaRef wrapping a non-function
    // Lua value passes through makeLuaCallback but produces a ScriptCallback
    // that, on invoke, will fail — that's a runtime failure, not a
    // build-time one.  The pre-refactor build-time check was stricter
    // (LUA_TFUNCTION only); the new behavior accepts any non-nil callback
    // and defers the shape check to invoke time.  Acceptable because
    // callable-shape validation isn't a builder-level concern in the
    // neutral world.
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
    setDragPointsReleasedFunction(lc::lua::makeLuaCallback(dragPointsReleasedFunction));
}
