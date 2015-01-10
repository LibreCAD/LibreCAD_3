#include "linewidth.h"


using namespace lc;

LineWidth::LineWidth() : MetaType(), _width(0.) {
}

std::string LineWidth::lcMetaName() const {
    return LineWidth::LCMETANAME();
}
std::string LineWidth::LCMETANAME() {
    return "_LINEWIDTH";
}

LineWidth::LineWidth(const double width): MetaType(), _width(width) {
    if (width < 0.0) {
        throw;
    }
}

double LineWidth::width() const {
    return _width;
}
