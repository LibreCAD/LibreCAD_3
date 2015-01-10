#include "layer.h"

using namespace lc;

Layer::Layer() {
}

Layer::~Layer() {
}

Layer::Layer(const std::string name, const MetaLineWidth lineWidth, const Color color) : MetaType() {
    _name = name;
    _lineWidth = lineWidth;
    _color = color;
}

Layer::Layer(const std::string name, const MetaLineWidth lineWidth) : MetaType() {
    _name = name;
    _lineWidth = lineWidth;
}

Layer::Layer(const std::string name, const Color color) : MetaType() {
    _name = name;
    _color = color;
}

Color Layer::color() const {
    return _color;
}
MetaLineWidth Layer::lineWidth() const {
    return _lineWidth;
}

std::string Layer::name() const {
    return _name;
}

