#include "metalinewidth.h"


using namespace lc;

MetaLineWidth::MetaLineWidth() : MetaType(), _width(0.) {
}

std::string MetaLineWidth::lcMetaName() const {
    return MetaLineWidth::LCMETANAME();
}
std::string MetaLineWidth::LCMETANAME() {
    return "_LINEWIDTH";
}

MetaLineWidth::MetaLineWidth(const double width): MetaType(), _width(width) {
    if (width < 0.0) {
        throw;
    }
}

double MetaLineWidth::width() const {
    return _width;
}
