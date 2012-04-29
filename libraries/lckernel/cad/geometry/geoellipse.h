#ifndef GEOELLIPSE_H
#define GEOELLIPSE_H

#include "cad/const.h"

#include "geoshape.h"
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

                virtual QList<Coordinate> intersect(IntersectablePtr x) const ;
                virtual QList<Coordinate> intersect(const Vector& x) const;
                virtual QList<Coordinate> intersect(const Circle& x) const;
                virtual QList<Coordinate> intersect(const Arc& x) const;
                virtual QList<Coordinate> intersect(const Ellipse& x) const;

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
