#include "layer.h"
#include "cad/functions/string_helper.h"
#include <cassert>

using namespace lc;

Layer::Layer() {
}

Layer::Layer(const std::string name, const MetaLineWidthByValue lineWidth, const Color color) :
        EntityMetaType(),
        DocumentMetaType(),
        _name(name),
        _lineWidth(lineWidth),
        _color(color),
        _linepattern(nullptr),
        _isFrozen(false) {
    assert(!StringHelper::isBlank(name) && "Name cannot be blank");
}

Layer::Layer(const std::string name,
             const MetaLineWidthByValue lineWidth,
             const Color color,
             DxfLinePatternByValue_CSPtr linepattern,
             const bool frozen) :
        EntityMetaType(),
        DocumentMetaType(),
        _name(name),
        _lineWidth(lineWidth),
        _color(color),
        _linepattern(linepattern),
        _isFrozen(frozen) {
    assert(!StringHelper::isBlank(name) && "Name cannot be blank");
}

Layer::Layer(const std::string name, const MetaLineWidthByValue lineWidth) :
        EntityMetaType(),
        DocumentMetaType(),
        _name(name),
        _lineWidth(lineWidth),
        _linepattern(nullptr),
        _isFrozen(false) {
    assert(!StringHelper::isBlank(name) && "Name cannot be blank");
}

Layer::Layer(const std::string name, const Color color) :
        EntityMetaType(),
        DocumentMetaType(),
        _name(name),
        _color(color),
        _linepattern(nullptr),
        _isFrozen(false) {
    assert(!StringHelper::isBlank(name) && "Name cannot be blank"); // Name must be set
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

