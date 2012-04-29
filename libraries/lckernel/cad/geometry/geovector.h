#ifndef GEOVECTOR_H
#define GEOVECTOR_H

#include "cad/const.h"

#include "geoshape.h"
#include "geocoordinate.h"
#include "geointersectable.h"

namespace lc {
    namespace geo {
        class Vector : public Intersectable {
            public:
                Vector(const Coordinate& start, const Coordinate& end);

                const Coordinate& start() const;
                const Coordinate& end() const;

                geo::Coordinate nearestPointOnPath(const geo::Coordinate& coord) const;

                virtual QList<Coordinate> intersect(IntersectablePtr x) const ;
                virtual QList<Coordinate> intersect(const Vector& x) const;
                virtual QList<Coordinate> intersect(const Circle& x) const;
                virtual QList<Coordinate> intersect(const Arc& x) const;
                virtual QList<Coordinate> intersect(const Ellipse& x) const;

            private:
                const Coordinate _start;
                const Coordinate _end;

        };
        typedef shared_ptr<const Vector> VectorPtr;
    }
}


/**
  * Used for debugging for example qDebug() << "my Coordinate:" << point;
  * outputs:
  * My Coordinate:(1,2,3)
  *
  */
QDebug operator << (QDebug dbg, const lc::geo::Vector& c);

#endif // GEOVECTOR_H
