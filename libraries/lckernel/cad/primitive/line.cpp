#include "line.h"

using namespace lc;

Line::Line(const geo::Coordinate& start, const geo::Coordinate& end) : CADEntity(), Vector(start, end) {
}

Line::Line(const geo::Coordinate& start, const geo::Coordinate& end, const QList<MetaTypePtr>& metaTypes) : CADEntity(metaTypes), Vector(start, end) {
}



QList<geo::Coordinate> Line::snapPoints(const geo::Coordinate& mousePointer, int maxNumberOfSnapPoints) {
    QList<geo::Coordinate> foo;
    foo << this->start() << this->end();
    return foo;
}
