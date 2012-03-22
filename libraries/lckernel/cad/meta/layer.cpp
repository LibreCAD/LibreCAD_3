#include "layer.h"

using namespace lc;

Layer::Layer() {
    _color = NULL;
    _lineWidth = NULL;
}

Layer::~Layer() {
    if (_color != NULL) {
        delete _color;
    }
    if (_lineWidth != NULL) {
        delete _lineWidth;
    }
}

Layer::Layer(const QString& name, LineWidth *lineWidth, Color *color) {
    _name = name;
    _lineWidth = lineWidth;
    _color = color;
}

Color* Layer::color() const {
    return _color;
}
LineWidth* Layer::lineWidth() const {
    return _lineWidth;
}

QString Layer::name() const {
    return _name;
}

