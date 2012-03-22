#include "line.h"

using namespace lc;

Line::Line(Coordinate* start, Coordinate* end) : CADEntity() {
    this->_start = start;
    this->_end = end;
}
Line::Line(Coordinate* start, Coordinate* end, METAINFO_TYPELIST metaTypes) : CADEntity(metaTypes) {
    this->_start = start;
    this->_end = end;
}

Coordinate* Line::start() const {
    return _start;
}
Coordinate* Line::end() const {
    return _end;
}
