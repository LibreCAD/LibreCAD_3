#ifndef GEOINTERSECTABLE_H
#define GEOINTERSECTABLE_H

#include <QList>
#include "cad/const.h"

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

                /*!
                  * \brief Calculate intersection points for all know shapes within the engine
                  *
                  * \todo Decide if we should not return a list of QList<EntityCoordinate> so distance sorting is faster
                  */
                virtual QList<Coordinate> intersect(IntersectablePtr x) const = 0;
                virtual QList<Coordinate> intersect(const Vector& x) const  = 0;
                virtual QList<Coordinate> intersect(const Circle& x) const  = 0;
                virtual QList<Coordinate> intersect(const Arc& x) const  = 0;
                virtual QList<Coordinate> intersect(const Ellipse& x) const  = 0;


                QList<Coordinate> lcgeoIntersectVectorVector(const Vector& a, const Vector& b) const;
                QList<Coordinate> geoIntersectArcLine(const Arc& arc, const Vector& line) const;
                QList<Coordinate> geoIntersectCircleLine(const Circle& circle, const Vector& line) const;
        };
        typedef shared_ptr<const Intersectable> IntersectablePtr;
    }
}
#endif // GEOINTERSECTABLE_H
