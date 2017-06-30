#include "customentitystorage.h"

using namespace lc;

//TODO: get a real ID

CustomEntityStorage::CustomEntityStorage(const std::string& pluginName,
                                         const std::string& entityName,
                                         const geo::Coordinate& base,
                                         const std::map<std::string, std::string> params) :
    Block("LC_" + std::to_string(rand()), base),
    _pluginName(pluginName),
    _entityName(entityName),
    _params(params) {

}

const std::string& CustomEntityStorage::pluginName() const {
    return _pluginName;
}

const std::string& CustomEntityStorage::entityName() const {
    return _entityName;
}

void CustomEntityStorage::setParam(const std::string& param, const std::string& value) {
    _params[param] = value;
}

std::string CustomEntityStorage::param(const std::string& param) const {
    try {
        return _params.at(param);
    }
    catch (std::out_of_range& e) {
        return "";
    }
}

const std::map<std::string, std::string>& CustomEntityStorage::params() const {
    return _params;
}
