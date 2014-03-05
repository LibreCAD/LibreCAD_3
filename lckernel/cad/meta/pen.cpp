#include "pen.h"



using namespace lc;

Pen::Pen() {
    _color = NULL;
    _lineWidth = NULL;
}

Pen::Pen(Color* color, LineWidth* lineWidth) {
    _color = color;
    _lineWidth = lineWidth;
}

Pen::~Pen() {
    if (_color != NULL) {
        delete _color;
    }

    if (_lineWidth != NULL) {
        delete _lineWidth;
    }
}

LineWidth* Pen::lineWidth() const {
    return _lineWidth;
}

Color* Pen::color() const {
    return _color;
}
