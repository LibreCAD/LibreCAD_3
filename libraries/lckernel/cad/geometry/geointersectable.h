#ifndef GEOINTERSECTABLE_H
#define GEOINTERSECTABLE_H

#include <QList>
#include "cad/const.h"
#include "geomath.h"

namespace lc {
    namespace geo {

        class Vector;
        class Circle;
        class Arc;
        class Ellipse;
        class Intersectable;
        class Coordinate;
        typedef shared_ptr<const Vector> VectorPtr;
        typedef shared_ptr<const Circle> CirclePtr;
        typedef shared_ptr<const Arc> ArcPtr;
        typedef shared_ptr<const Ellipse> EllipsePtr;
        typedef shared_ptr<const Intersectable> IntersectablePtr;

        class Intersectable {
            public:

                enum Coordinates {
                    MustIntersect = 0,
                    Any = 1
                };

                /*!
                  * \brief Calculate intersection points for all know shapes within the engine
                  *
                  * \todo Decide if we should not return a list of QList<EntityCoordinate> so distance sorting is faster
                  */
                virtual QList<Coordinate> intersect(IntersectablePtr x, Intersectable::Coordinates intersect = Intersectable::Any) const = 0;
                virtual QList<Coordinate> intersect(const Vector& x, Intersectable::Coordinates intersect) const  = 0;
                virtual QList<Coordinate> intersect(const Circle& x, Intersectable::Coordinates intersect) const  = 0;
                virtual QList<Coordinate> intersect(const Arc& x, Intersectable::Coordinates intersect) const  = 0;
                virtual QList<Coordinate> intersect(const Ellipse& x, Intersectable::Coordinates intersect) const  = 0;

        };
        typedef shared_ptr<const Intersectable> IntersectablePtr;
    }
}
#endif // GEOINTERSECTABLE_H
