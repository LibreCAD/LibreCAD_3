#include "metalinewidth.h"
#include <cassert>

using namespace lc;

MetaLineWidthByBlock::MetaLineWidthByBlock() : EntityMetaType() {
}

const std::string MetaLineWidthByBlock::metaTypeID() const {
    return MetaLineWidthByBlock::LCMETANAME();
}


MetaLineWidthByValue::MetaLineWidthByValue() : EntityMetaType(), DocumentMetaType(), _width(0.) {
    assert(_width >= 0. && "Line width must be >= 0.");
}

MetaLineWidthByValue::MetaLineWidthByValue(const double width): EntityMetaType(), _width(width) {
    assert(_width>=0. && "Line width must be >= 0.");
}

const std::string MetaLineWidthByValue::metaTypeID() const {
    return MetaLineWidthByValue::LCMETANAME();
}

double MetaLineWidthByValue::width() const {
    return _width;
}
