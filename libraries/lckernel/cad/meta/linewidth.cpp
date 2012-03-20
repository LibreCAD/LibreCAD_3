#include "linewidth.h"


using namespace lc;

LineWidth::LineWidth() {
    _method = LineWidth::Invalid;
}

LineWidth::LineWidth(const double width) {
    if (width < 0.0) {
        throw;
    }

    _method = LineWidth::ByEntity;
    _width = width;
}

LineWidth::LineWidth(LineWidth::Method method) {
    _width = 0.0;
    _method = method;
}

LineWidth::Method LineWidth::method() const {
    return _method;
}

double LineWidth::width() const {
    if (_method != LineWidth::ByEntity) {
        throw;
    }

    return _width;
}
