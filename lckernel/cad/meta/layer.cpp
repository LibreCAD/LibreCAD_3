#include "layer.h"
#include "cad/functions/string_helper.h"
#include <cassert>

using namespace lc;

Layer::Layer(std::string name,
             const MetaLineWidthByValue& lineWidth,
             const Color& color,
             DxfLinePatternByValue_CSPtr linepattern,
             bool frozen) :
        EntityMetaType(),
        DocumentMetaType(),
        _name(std::move(name)),
        _lineWidth(lineWidth),
        _color(color),
        _linepattern(std::move(linepattern)),
        _isFrozen(frozen) {
    assert(!StringHelper::isBlank(_name) && "Name cannot be blank");
}

Layer::Layer(const builder::LayerBuilder& builder) :
    EntityMetaType(),
    DocumentMetaType(),
    _name(builder.name()),
    _lineWidth(builder.lineWidth()),
    _color(builder.color()),
    _linepattern(builder.linePattern()),
    _isFrozen(builder.isFrozen()) {
}

Color Layer::color() const {
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

const std::string Layer::name() const {
    return _name;
}

