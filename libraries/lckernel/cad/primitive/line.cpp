#include "line.h"

using namespace lc;

Line::Line(const geo::Coordinate& start, const geo::Coordinate& end) : CADEntity(), Vector(start, end) {
}
Line::Line(const geo::Coordinate& start, const geo::Coordinate& end, METAINFO_TYPELIST metaTypes) : CADEntity(metaTypes), Vector(start, end) {
}


bool Line::inArea(const geo::Area& area) const {
    return true;
}
