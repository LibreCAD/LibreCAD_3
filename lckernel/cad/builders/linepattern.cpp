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
    return _name != "";
}

DxfLinePattern_CSPtr LinePatternBuilder::build() {
    if(!checkValues()) {
        throw "Missing values";
    }

    return DxfLinePattern_CSPtr(new DxfLinePattern(*this));
}

LinePatternBuilder* LinePatternBuilder::addElement(double element) {
    _path.push_back(element);

    return this;
}
