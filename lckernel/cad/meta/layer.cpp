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

std::shared_ptr<const Layer> Layer::withState(const std::shared_ptr<const Layer>& source,
                                              bool frozen, bool off, bool locked) {
    if (!source) return nullptr;
    auto result = std::make_shared<Layer>(source->_name, source->_lineWidth, source->_color,
                                          source->_linepattern, frozen, off, locked);
    result->_colorSource = source->_colorSource ? source->_colorSource : source;
    return result;
}

bool Layer::retainsColorFrom(const std::shared_ptr<const Layer>& source) const {
    return source && (source.get() == this || _colorSource == source);
}

const std::string Layer::name() const {
    return _name;
}

