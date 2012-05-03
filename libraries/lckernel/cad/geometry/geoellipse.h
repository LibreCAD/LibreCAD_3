#ifndef GEOELLIPSE_H
#define GEOELLIPSE_H

#include "cad/const.h"

#include "geointersectable.h"
#include "geocoordinate.h"


namespace lc {
    namespace geo {
        class Ellipse : public Intersectable {
            public:
                Ellipse(const Coordinate& center, const Coordinate& majorP, double minorRadius, double startAngle, double endAngle);

                const Coordinate& center() const;
                const Coordinate& majorP() const;
                double minorRadius() const;
                double startAngle() const;
                double endAngle() const;

                Coordinate nearestPointOnPath(const geo::Coordinate& coord) const;
                bool isCoordinateOnPath(const Coordinate& coord) const;

                virtual QList<Coordinate> intersect(IntersectablePtr x, Intersectable::Coordinates intersect = Intersectable::Any) const ;
                virtual QList<Coordinate> intersect(const Vector& x, Intersectable::Coordinates intersect) const;
                virtual QList<Coordinate> intersect(const Circle& x, Intersectable::Coordinates intersect) const;
                virtual QList<Coordinate> intersect(const Arc& x, Intersectable::Coordinates intersect) const;
                virtual QList<Coordinate> intersect(const Ellipse& x, Intersectable::Coordinates intersect) const;

            private:
                const Coordinate _center;
                const Coordinate _majorP;
                double _minorRadius;
                double _startAngle;
                double _endAngle;
        };
        typedef shared_ptr<const Ellipse> EllipsePtr;
    }
}

#endif // GEOELLIPSE_H
