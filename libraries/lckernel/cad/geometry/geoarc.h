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

                virtual QList<Coordinate> intersect(IntersectablePtr x) const ;
                virtual QList<Coordinate> intersect(const Vector& x) const;
                virtual QList<Coordinate> intersect(const Circle& x) const;
                virtual QList<Coordinate> intersect(const Arc& x) const;
                virtual QList<Coordinate> intersect(const Ellipse& x) const;
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
