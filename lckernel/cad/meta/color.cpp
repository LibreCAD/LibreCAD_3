#include "color.h"

using namespace lc;

Color::Color() : MetaType() {
    _method = Color::Invalid;
}

Color::Color(const int r, const int g, const int b, const int a) : MetaType()  {
    _method = Color::ByEntity;
    _r = r;
    _g = g;
    _b = b;
    _a = a;
}

Color::Color(Color::Method method) {
    if (method == Color::Invalid) {
        throw;
    }

    if (method == Color::ByEntity) {
        throw;
    }

    this->_method = method;
}

Color::Method Color::method() const {
    return _method;
}
int Color::red() const {
    return _r;
}
int Color::green() const {
    return _g;
}
int Color::blue() const {
    return _b;
}
int Color::alpha() const {
    return _a;
}

//QColor Color::qColor() const {
//    return QColor(_r, _g, _b, _a);
//}
