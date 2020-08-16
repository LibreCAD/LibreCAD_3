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

double LineBuilder::getLineAngle(lc::entity::CADEntity_CSPtr entity) const {
    lc::entity::Line_CSPtr line = std::dynamic_pointer_cast<const lc::entity::Line>(entity);
    if (line == nullptr) {
        return -1;
    }
    return line->Angle1();
}

lc::entity::Line_CSPtr LineBuilder::build() {
    checkValues(true);
    return entity::Line_CSPtr(new entity::Line(*this));
}

void LineBuilder::copy(entity::Line_CSPtr entity) {
    setStart(entity->start());
    setEnd(entity->end());
    CADEntityBuilder::copy(entity);
}
