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
            _nearestPointFunction.isValid() && _nearestPointFunction.isFunction();
}

const LuaIntf::LuaRef& CustomEntityBuilder::nearestPointFunction() const {
    return _nearestPointFunction;
}

void CustomEntityBuilder::setNearestPointFunction(const LuaIntf::LuaRef& nearestPointFunction) {
    _nearestPointFunction = nearestPointFunction;
}


