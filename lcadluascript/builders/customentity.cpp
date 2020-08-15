#include "customentity.h"
#include "../primitive/customentity.h"
#include "lua.h"

using namespace lc;
using namespace builder;

void CustomEntityBuilder::setSnapFunction(kaguya::LuaRef snapFunction) {
    _snapFunction = std::move(snapFunction);
}

entity::LuaCustomEntity_CSPtr CustomEntityBuilder::build() {
    if(!checkValues()) {
        throw std::runtime_error("Missing or invalid values");
    }

    return entity::LuaCustomEntity_CSPtr(new entity::LuaCustomEntity(*this));
}

const kaguya::LuaRef& CustomEntityBuilder::snapFunction() const {
    return _snapFunction;
}

bool CustomEntityBuilder::checkValues(bool throwExceptions) const {
    if (!throwExceptions) {
        return InsertBuilder::checkValues(throwExceptions) &&
               _snapFunction.type() == LUA_TFUNCTION &&
               _nearestPointFunction.type() == LUA_TFUNCTION &&
               _dragPointsFunction.type() == LUA_TFUNCTION &&
               _newDragPointFunction.type() == LUA_TFUNCTION &&
               _dragPointsClickedFunction.type() == LUA_TFUNCTION &&
               _dragPointsReleasedFunction.type() == LUA_TFUNCTION;
    } else
    {
        if (_snapFunction.type() != LUA_TFUNCTION) {
            throw std::runtime_error("Snap function callback MUST be a function");
        }
        if (_nearestPointFunction.type() != LUA_TFUNCTION) {
            throw std::runtime_error("Nearest point function callback MUST be a function");
        }
        if (_dragPointsFunction.type() != LUA_TFUNCTION) {
            throw std::runtime_error("Drag points function callback MUST be a function");
        }
        if (_newDragPointFunction.type() != LUA_TFUNCTION) {
            throw std::runtime_error("New drag point function callback MUST be a function");
        }
        if (_dragPointsClickedFunction.type() != LUA_TFUNCTION) {
            throw std::runtime_error("Drag points clicked function callback MUST be a function");
        }
        if (_dragPointsReleasedFunction.type() != LUA_TFUNCTION) {
            throw std::runtime_error("Drag points released function callback MUST be a function");
        }

        return InsertBuilder::checkValues(throwExceptions);
    }
}

const kaguya::LuaRef& CustomEntityBuilder::nearestPointFunction() const {
    return _nearestPointFunction;
}

void CustomEntityBuilder::setNearestPointFunction(const kaguya::LuaRef& nearestPointFunction) {
    _nearestPointFunction = nearestPointFunction;
}

const kaguya::LuaRef& CustomEntityBuilder::dragPointsFunction() const {
    return _dragPointsFunction;
}

void CustomEntityBuilder::setDragPointsFunction(const kaguya::LuaRef& dragPointsFunction) {
    _dragPointsFunction = dragPointsFunction;
}

const kaguya::LuaRef& CustomEntityBuilder::newDragPointFunction() const {
    return _newDragPointFunction;
}

void CustomEntityBuilder::setNewDragPointFunction(const kaguya::LuaRef& newDragPointFunction) {
    _newDragPointFunction = newDragPointFunction;
}

const kaguya::LuaRef& CustomEntityBuilder::dragPointsClickedFunction() const {
    return _dragPointsClickedFunction;
}

void CustomEntityBuilder::setDragPointsClickedFunction(const kaguya::LuaRef& dragPointsClickedFunction) {
    _dragPointsClickedFunction = dragPointsClickedFunction;
}

const kaguya::LuaRef& CustomEntityBuilder::dragPointsReleasedFunction() const {
    return _dragPointsReleasedFunction;
}

void CustomEntityBuilder::setDragPointsReleasedFunction(const kaguya::LuaRef& dragPointsReleasedFunction) {
    _dragPointsReleasedFunction = dragPointsReleasedFunction;
}

