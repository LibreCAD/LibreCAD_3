#include "metalinewidth.h"


using namespace lc;

MetaLineWidth::MetaLineWidth() : _width(0.), MetaType() {
}

std::string MetaLineWidth::lcMetaName() const {
    return MetaLineWidth::LCMETANAME();
}
std::string MetaLineWidth::LCMETANAME() {
    return "_LINEWIDTH";
}

MetaLineWidth::MetaLineWidth(const double width): _width(width), MetaType() {
    if (width < 0.0) {
        throw;
    }
}

double MetaLineWidth::width() const {
    return _width;
}
