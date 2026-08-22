#include "customentity.h"
#include "../primitive/customentity.h"

#include <stdexcept>

using namespace lc;
using namespace builder;

// --- Snap function ---
void CustomEntityBuilder::setSnapFunction(lc::scripting::ScriptCallback snapFunction) {
    _snapFunction = std::move(snapFunction);
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
    // Phase 6 PR-6.1 sub-piece 2b — the slot-shape check is `!isNil()`
    // (wrapper-only).  The pre-refactor LUA_TFUNCTION guard lives at
    // the Lua-side wrapper (`lc::lua::wrapAndSet*` helpers in
    // lcadluascript) — a non-callable LuaRef never reaches this builder
    // because those helpers refuse to wrap it.
    if (!throwExceptions) {
        return InsertBuilder::checkValues(throwExceptions) &&
               !_snapFunction.isNil() &&
               !_nearestPointFunction.isNil() &&
               !_dragPointsFunction.isNil() &&
               !_newDragPointFunction.isNil() &&
               !_dragPointsClickedFunction.isNil() &&
               !_dragPointsReleasedFunction.isNil();
    } else {
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

// --- Drag-points function ---
const lc::scripting::ScriptCallback& CustomEntityBuilder::dragPointsFunction() const {
    return _dragPointsFunction;
}

void CustomEntityBuilder::setDragPointsFunction(lc::scripting::ScriptCallback dragPointsFunction) {
    _dragPointsFunction = std::move(dragPointsFunction);
}

// --- New-drag-point function ---
const lc::scripting::ScriptCallback& CustomEntityBuilder::newDragPointFunction() const {
    return _newDragPointFunction;
}

void CustomEntityBuilder::setNewDragPointFunction(lc::scripting::ScriptCallback newDragPointFunction) {
    _newDragPointFunction = std::move(newDragPointFunction);
}

// --- Drag-points-clicked function ---
const lc::scripting::ScriptCallback& CustomEntityBuilder::dragPointsClickedFunction() const {
    return _dragPointsClickedFunction;
}

void CustomEntityBuilder::setDragPointsClickedFunction(lc::scripting::ScriptCallback dragPointsClickedFunction) {
    _dragPointsClickedFunction = std::move(dragPointsClickedFunction);
}

// --- Drag-points-released function ---
const lc::scripting::ScriptCallback& CustomEntityBuilder::dragPointsReleasedFunction() const {
    return _dragPointsReleasedFunction;
}

void CustomEntityBuilder::setDragPointsReleasedFunction(lc::scripting::ScriptCallback dragPointsReleasedFunction) {
    _dragPointsReleasedFunction = std::move(dragPointsReleasedFunction);
}
