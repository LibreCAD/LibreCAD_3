#include "layer.h"
#include <cad/meta/layer.h>

using namespace lc;
using namespace builder;

LayerBuilder::LayerBuilder() :
    _name(),
    _lineWidth(1),
    _color(),
    _linePattern(nullptr),
    _isFrozen(false) {

}

const std::string& LayerBuilder::name() const {
    return _name;
}

const meta::MetaLineWidthByValue& LayerBuilder::lineWidth() const {
    return _lineWidth;
}

const Color& LayerBuilder::color() const {
    return _color;
}

meta::DxfLinePatternByValue_CSPtr LayerBuilder::linePattern() const {
    return _linePattern;
}

bool LayerBuilder::isFrozen() const {
    return _isFrozen;
}

LayerBuilder* LayerBuilder::setName(const std::string& name) {
    _name = name;
    
    return this;
}

LayerBuilder* LayerBuilder::setLineWidth(const meta::MetaLineWidthByValue& lineWidth) {
    _lineWidth = lineWidth;

    return this;
}

LayerBuilder* LayerBuilder::setColor(const Color& color) {
    _color = color;

    return this;
}

LayerBuilder* LayerBuilder::setLinePattern(const meta::DxfLinePatternByValue_CSPtr& linePattern) {
    _linePattern = linePattern;

    return this;
}

LayerBuilder* LayerBuilder::setIsFrozen(bool isFrozen) {
    _isFrozen = isFrozen;

    return this;
}

meta::Layer_CSPtr LayerBuilder::build() {
    if(!checkValues()) {
        throw std::runtime_error("Missing values");
    }

    return meta::Layer_CSPtr(new meta::Layer(*this));
}

bool LayerBuilder::checkValues() {
    return !_name.empty();
}
