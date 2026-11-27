#include "layer.h"
#include "cad/tools/string_helper.h"
#include <cassert>

using namespace lc;
using namespace lc::meta;

Layer::Layer(std::string name,
             const MetaLineWidthByValue& lineWidth,
             const Color& color,
             DxfLinePatternByValue_CSPtr linepattern,
             bool frozen) :
    Layer(std::move(name), lineWidth, color, std::move(linepattern), frozen, false) {
}

Layer::Layer(std::string name, const MetaLineWidthByValue& lineWidth, const Color& color,
             DxfLinePatternByValue_CSPtr linepattern, bool frozen, bool off) :
    Layer(std::move(name), lineWidth, color, std::move(linepattern), frozen, off, false) {
}

Layer::Layer(std::string name, const MetaLineWidthByValue& lineWidth, const Color& color,
             DxfLinePatternByValue_CSPtr linepattern, bool frozen, bool off, bool locked) :
    EntityMetaType(),
    DocumentMetaType(),
    _name(std::move(name)),
    _lineWidth(lineWidth),
    _color(color),
    _linepattern(std::move(linepattern)),
    _isFrozen(frozen),
    _isOff(off),
    _isLocked(locked) {
    assert(!tools::StringHelper::isBlank(_name) && "Name cannot be blank");
}

Layer::Layer(const builder::LayerBuilder& builder) :
    EntityMetaType(),
    DocumentMetaType(),
    _name(builder.name()),
    _lineWidth(builder.lineWidth()),
    _color(builder.color()),
    _linepattern(builder.linePattern()),
    _isFrozen(builder.isFrozen()),
    _isOff(builder.isOff()),
    _isLocked(builder.isLocked()) {
}

lc::Color Layer::color() const {
    return _color;
}

MetaLineWidthByValue Layer::lineWidth() const {
    return _lineWidth;
}

DxfLinePatternByValue_CSPtr Layer::linePattern() const {
    return _linepattern;
}

bool Layer::isFrozen() const {
    return _isFrozen;
}

bool Layer::isOff() const {
    return _isOff;
}

bool Layer::isLocked() const {
    return _isLocked;
}

const std::string Layer::name() const {
    return _name;
}

