#include "metalinewidth.h"
#include <cassert>

using namespace lc;

const std::string MetaLineWidth::LCMETANAME() {
    return "_LINEWIDTH";
}

const std::string MetaLineWidth::id() const {
    return LCMETANAME();
}

MetaLineWidthByBlock::MetaLineWidthByBlock() : MetaLineWidth() {
}

const std::string MetaLineWidthByBlock::metaTypeID() const {
    return MetaLineWidthByBlock::LCMETANAME();
}


MetaLineWidthByValue::MetaLineWidthByValue() :
        MetaLineWidth(),
        DocumentMetaType(),
        _width(0.) {

    assert(_width >= 0. && "Line width must be >= 0.");
}

MetaLineWidthByValue::MetaLineWidthByValue(const double width):
        MetaLineWidth(),
        DocumentMetaType(),
        _width(width) {

    assert(_width>=0. && "Line width must be >= 0.");
}

const std::string MetaLineWidthByValue::metaTypeID() const {
    return MetaLineWidthByValue::LCMETANAME();
}

double MetaLineWidthByValue::width() const {
    return _width;
}

const std::string MetaLineWidthByValue::name() const {
    return LCMETANAME();
}
