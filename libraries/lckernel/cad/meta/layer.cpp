#include "layer.h"

using namespace lc;

Layer::Layer() {
    _pen = NULL;
}

Layer::~Layer() {
    if (_pen != NULL) {
        delete _pen;
    }
}

Layer::Layer(const QString& name, Pen* Pen) {
    _name = name;
    _pen = Pen;
}

Pen* Layer::pen() const {
    return _pen;
}

QString Layer::name() const {
    return _name;
}

