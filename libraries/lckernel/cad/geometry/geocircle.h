#ifndef GEOCIRCLE_H
#define GEOCIRCLE_H

#include "cad/const.h"

#include "geointersectable.h"

#include "geoshape.h"
#include "geocoordinate.h"

namespace lc {
    namespace geo {
        // TODO: Should Circle be a arc with a start angle of 0 and a stop angle of 2PI ?
        class Circle : public Intersectable {
            public:
                Circle(const Coordinate& center, double radius);

                const Coordinate& center() const;
                double radius() const;

                geo::Coordinate nearestPointOnPath(const geo::Coordinate& coord) const;

                virtual QList<Coordinate> intersect(IntersectablePtr x) const ;
                virtual QList<Coordinate> intersect(const Vector& x) const;
                virtual QList<Coordinate> intersect(const Circle& x) const;
                virtual QList<Coordinate> intersect(const Arc& x) const;
                virtual QList<Coordinate> intersect(const Ellipse& x) const;

            private:
                const Coordinate _center;
                double _radius;
        };
        typedef shared_ptr<const Circle> CirclePtr;
    }
}

/**
  * Used for debugging for example qDebug() << "my Coordinate:" << point;
  * outputs:
  * My Coordinate:(1,2,3)
  *
  */
QDebug operator << (QDebug dbg, const lc::geo::Circle& c);

#endif // GEOCIRCLE_H
