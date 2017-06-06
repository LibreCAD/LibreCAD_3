#include "line.h"
#include <cad/primitive/line.h>

using namespace lc::builder;

LineBuilder::LineBuilder() :
        _start(),
        _end() {
}

const lc::geo::Coordinate& LineBuilder::start() const {
    return _start;
}

LineBuilder* LineBuilder::setStart(const lc::geo::Coordinate& start) {
    _start = start;
    return this;
}

const lc::geo::Coordinate& LineBuilder::end() const {
    return _end;
}

LineBuilder* LineBuilder::setEnd(const lc::geo::Coordinate& end) {
    _end = end;
    return this;
}

lc::entity::Line_CSPtr LineBuilder::build() {
    return entity::Line_CSPtr(new entity::Line(*this));
}