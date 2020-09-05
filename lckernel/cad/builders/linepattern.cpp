#include "linepattern.h"

#include <cad/meta/dxflinepattern.h>

using namespace lc;
using namespace builder;

LinePatternBuilder::LinePatternBuilder() :
    _name(),
    _description() {
}

const std::string& LinePatternBuilder::name() const {
    return _name;
}

const std::string& LinePatternBuilder::description() const {
    return _description;
}

const std::vector<double>& LinePatternBuilder::path() const {
    return _path;
}

LinePatternBuilder* LinePatternBuilder::setName(const std::string& name) {
    _name = name;
    return this;
}

LinePatternBuilder* LinePatternBuilder::setDescription(const std::string& description) {
    _description = description;
    return this;
}

LinePatternBuilder* LinePatternBuilder::setPath(const std::vector<double>& path) {
    _path = path;
    return this;
}

bool LinePatternBuilder::checkValues() {
    return !_name.empty();
}

meta::DxfLinePatternByValue_CSPtr LinePatternBuilder::build() {
    if(!checkValues()) {
        throw std::runtime_error("Missing values");
    }
    return meta::DxfLinePatternByValue_CSPtr(new meta::DxfLinePatternByValue(*this));
}

LinePatternBuilder* LinePatternBuilder::addElement(double element) {
    _path.push_back(element);
    return this;
}
