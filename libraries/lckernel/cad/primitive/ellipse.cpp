#include "ellipse.h"

using namespace lc;

Ellipse::Ellipse(const geo::Coordinate& center, const geo::Coordinate& majorP, double minorRadius, double startAngle, double endAngle)
    : geo::Ellipse(center, majorP, minorRadius, startAngle, endAngle) {
    if (startAngle < 0.0 || startAngle > PI2 || startAngle < endAngle) {
        throw "Invalid start angle";
    }

    if (endAngle < 0.0 || endAngle > PI2) {
        throw "Invalid end angle";
    }

    if (minorRadius < 0) {
        throw "Invalid minor radius";
    }
}

Ellipse::Ellipse(const geo::Coordinate& center, const geo::Coordinate& majorP, double minorRadius, double startAngle, double endAngle, const QList<MetaTypePtr>& metaTypes)
    : CADEntity(metaTypes),  geo::Ellipse(center, majorP, minorRadius, startAngle, endAngle) {
}
