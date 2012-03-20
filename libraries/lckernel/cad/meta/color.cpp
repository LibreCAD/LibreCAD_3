#include "color.h"
#include <QColor>


using namespace lc;

Color::Color() {
    _method = Color::Invalid;
}

Color::Color(const int r, const int g, const int b, const int a) {
    _method = Color::ByEntity;
    _r = r;
    _g = g;
    _b = b;
    _a = a;
}

Color::Color(Color::Method method) {
    if (_method == Color::ByEntity || _method == Color::ByLayer) {
        throw;
    }

    this->_method = method;
}

Color::Method Color::method() const {
    return _method;
}
int Color::red() const {
    if (_method != Color::ByEntity) {
        throw;
    }

    return _r;
}
int Color::green() const {
    if (_method != Color::ByEntity) {
        throw;
    }

    return _g;
}
int Color::blue() const {
    if (_method != Color::ByEntity) {
        throw;
    }

    return _b;
}
int Color::alpha() const {
    if (_method != Color::ByEntity) {
        throw;
    }

    return _a;
}

QColor Color::qColor() const {
    return QColor(_r, _g, _b, _a);
}
