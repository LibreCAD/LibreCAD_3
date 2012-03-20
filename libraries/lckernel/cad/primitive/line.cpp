#include "line.h"

using namespace lc;

Line::Line(Point* start, Point* end) : CADEntity() {
    this->_start = start;
    this->_end = end;
}
Line::Line(Point* start, Point* end, METAINFO_TYPELIST metaTypes) : CADEntity(metaTypes) {
    this->_start = start;
    this->_end = end;
}

Point* Line::start() const {
    return _start;
}
Point* Line::end() const {
    return _end;
}
