#ifndef GEOINTERSECT_H
#define GEOINTERSECT_H

#include <QList>

namespace lc {
    namespace geo {
        class Vector;
        class Arc;
        class Circle;
        class Coordinate;

        class GeoIntersect {
            public:
                enum Intersect {
                    MustIntersect = 0,
                    Any = 1
                };

                static QList<Coordinate> intersectVectorVector(const Vector& a, const Vector& b, GeoIntersect::Intersect intersect = GeoIntersect::Any);
                static QList<Coordinate> intersectArcLine(const Arc& arc, const Vector& line, GeoIntersect::Intersect intersect = GeoIntersect::Any);
                static QList<Coordinate> intersectCircleLine(const Circle& circle, const Vector& line, GeoIntersect::Intersect intersect = GeoIntersect::Any);

        };
    }
}

#endif // GEOMATH_H
