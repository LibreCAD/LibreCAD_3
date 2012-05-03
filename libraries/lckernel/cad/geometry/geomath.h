#ifndef GEOMATH_H
#define GEOMATH_H

#include <QList>

namespace lc {
    namespace geo {
        class Vector;
        class Arc;
        class Circle;
        class Coordinate;

        class GeoMath {
            public:
                enum Intersect {
                    MustIntersect = 0,
                    Any = 1
                };

                static QList<Coordinate> intersectVectorVector(const Vector& a, const Vector& b, GeoMath::Intersect intersect = GeoMath::Any);
                static QList<Coordinate> intersectArcLine(const Arc& arc, const Vector& line, GeoMath::Intersect intersect = GeoMath::Any);
                static QList<Coordinate> intersectCircleLine(const Circle& circle, const Vector& line, GeoMath::Intersect intersect = GeoMath::Any);

        };
    }
}

#endif // GEOMATH_H
