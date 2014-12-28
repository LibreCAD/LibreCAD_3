#include "linewidth.h"


using namespace lc;

LineWidth::LineWidth() : _width(0.), MetaType() {
}

LineWidth::LineWidth(const double width): _width(width), MetaType() {
    if (width < 0.0) {
        throw;
    }
}

double LineWidth::width() const {
    return _width;
}
