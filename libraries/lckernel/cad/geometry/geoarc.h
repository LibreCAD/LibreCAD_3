#ifndef GEOARC_H
#define GEOARC_H

#include "cad/const.h"

#include "geoshape.h"
#include "geointersectable.h"
#include "geocoordinate.h"

namespace lc {
    namespace geo {
        class Arc : public Intersectable {
            public:
                Arc(const Coordinate& center, double radius, double startAngle, double endAngle);

                const Coordinate& center() const;
                double radius() const;
                double startAngle() const;
                double endAngle() const;

                Coordinate nearestPointOnPath(const Coordinate& coord) const;
                bool isCoordinateOnPath(const Coordinate& coord) const;

                virtual QList<Coordinate> intersect(IntersectablePtr x, Intersectable::Coordinates intersect = Intersectable::Any) const ;
                virtual QList<Coordinate> intersect(const Vector& x, Intersectable::Coordinates intersect) const;
                virtual QList<Coordinate> intersect(const Circle& x, Intersectable::Coordinates intersect) const;
                virtual QList<Coordinate> intersect(const Arc& x, Intersectable::Coordinates intersect) const;
                virtual QList<Coordinate> intersect(const Ellipse& x, Intersectable::Coordinates intersect) const;
            private:
                const Coordinate _center;
                double _radius;
                double _startAngle;
                double _endAngle;
        };
        typedef shared_ptr<const Arc> ArcPtr;
    }
}
#endif // GEOARC_H
