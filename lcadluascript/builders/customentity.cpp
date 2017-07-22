#include "customentity.h"
#include "../primitive/customentity.h"

using namespace lc;
using namespace builder;

CustomEntityBuilder::CustomEntityBuilder() {

}

void CustomEntityBuilder::setSnapFunction(LuaIntf::LuaRef snapFunction) {
    _snapFunction = snapFunction;
}

entity::LuaCustomEntity_CSPtr CustomEntityBuilder::build() {
    if(!checkValues()) {
        throw "Missing or invalid values";
    }

    return entity::LuaCustomEntity_CSPtr(new entity::LuaCustomEntity(*this));
}

const LuaIntf::LuaRef& CustomEntityBuilder::snapFunction() const {
    return _snapFunction;
}

bool CustomEntityBuilder::checkValues() {
    return InsertBuilder::checkValues() &&
           _snapFunction.isValid() && _snapFunction.isFunction() &&
           _nearestPointFunction.isValid() && _nearestPointFunction.isFunction() &&
           _dragPointsFunction.isValid() && _dragPointsFunction.isFunction() &&
           _newDragPointFunction.isValid() && _newDragPointFunction.isFunction() &&
           _dragPointsClickedFunction.isValid() && _dragPointsClickedFunction.isFunction() &&
           _dragPointsReleasedFunction.isValid() && _dragPointsReleasedFunction.isFunction();
}

const LuaIntf::LuaRef& CustomEntityBuilder::nearestPointFunction() const {
    return _nearestPointFunction;
}

void CustomEntityBuilder::setNearestPointFunction(const LuaIntf::LuaRef& nearestPointFunction) {
    _nearestPointFunction = nearestPointFunction;
}

const LuaIntf::LuaRef& CustomEntityBuilder::dragPointsFunction() const {
    return _dragPointsFunction;
}

void CustomEntityBuilder::setDragPointsFunction(const LuaIntf::LuaRef& dragPointsFunction) {
    _dragPointsFunction = dragPointsFunction;
}

const LuaIntf::LuaRef& CustomEntityBuilder::newDragPointFunction() const {
    return _newDragPointFunction;
}

void CustomEntityBuilder::setNewDragPointFunction(const LuaIntf::LuaRef& setDragPointFunction) {
    _newDragPointFunction = setDragPointFunction;
}

const LuaIntf::LuaRef& CustomEntityBuilder::dragPointsClickedFunction() const {
    return _dragPointsClickedFunction;
}

void CustomEntityBuilder::setDragPointsClickedFunction(const LuaIntf::LuaRef& dragPointsClickedFunction) {
    _dragPointsClickedFunction = dragPointsClickedFunction;
}

const LuaIntf::LuaRef& CustomEntityBuilder::dragPointsReleasedFunction() const {
    return _dragPointsReleasedFunction;
}

void CustomEntityBuilder::setDragPointsReleasedFunction(const LuaIntf::LuaRef& dragPointsReleasedFunction) {
    _dragPointsReleasedFunction = dragPointsReleasedFunction;
}

