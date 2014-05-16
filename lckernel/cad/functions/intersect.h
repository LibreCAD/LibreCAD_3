#ifndef INTERSECT_H
#define INTERSECT_H

#include "cad/base/cadentity.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/interface/entityvisitor.h"

namespace lc {

    /**
      * \brief calculate the intersection pojnts of 2 entities
      * \sa IntersectMany
      */
    class Intersect : public EntityVisitor {
        public:
            enum Method {
                MustIntersect,
                Any = 1
            };

            Intersect(Method method);
            virtual void visit(Line_CSPtr, Line_CSPtr);
            virtual void visit(Line_CSPtr, Circle_CSPtr);
            virtual void visit(Line_CSPtr, Arc_CSPtr);
            virtual void visit(Line_CSPtr, Ellipse_CSPtr);
            virtual void visit(Line_CSPtr, Text_CSPtr);
            virtual void visit(Line_CSPtr, Spline_CSPtr);

            virtual void visit(Circle_CSPtr, Line_CSPtr);
            virtual void visit(Circle_CSPtr, Circle_CSPtr);
            virtual void visit(Circle_CSPtr, Arc_CSPtr);
            virtual void visit(Circle_CSPtr, Ellipse_CSPtr);
            virtual void visit(Circle_CSPtr, Text_CSPtr);
            virtual void visit(Circle_CSPtr, Spline_CSPtr);

            virtual void visit(Arc_CSPtr, Line_CSPtr);
            virtual void visit(Arc_CSPtr, Circle_CSPtr);
            virtual void visit(Arc_CSPtr, Arc_CSPtr);
            virtual void visit(Arc_CSPtr, Ellipse_CSPtr);
            virtual void visit(Arc_CSPtr, Text_CSPtr);
            virtual void visit(Arc_CSPtr, Spline_CSPtr);

            virtual void visit(Ellipse_CSPtr, Line_CSPtr);
            virtual void visit(Ellipse_CSPtr, Circle_CSPtr);
            virtual void visit(Ellipse_CSPtr, Arc_CSPtr);
            virtual void visit(Ellipse_CSPtr, Ellipse_CSPtr);
            virtual void visit(Ellipse_CSPtr, Text_CSPtr);
            virtual void visit(Ellipse_CSPtr, Spline_CSPtr);

            virtual void visit(Text_CSPtr, Line_CSPtr);
            virtual void visit(Text_CSPtr, Circle_CSPtr);
            virtual void visit(Text_CSPtr, Arc_CSPtr);
            virtual void visit(Text_CSPtr, Ellipse_CSPtr);
            virtual void visit(Text_CSPtr, Text_CSPtr);
            virtual void visit(Text_CSPtr, Spline_CSPtr);

            virtual void visit(Spline_CSPtr, Line_CSPtr);
            virtual void visit(Spline_CSPtr, Circle_CSPtr);
            virtual void visit(Spline_CSPtr, Arc_CSPtr);
            virtual void visit(Spline_CSPtr, Ellipse_CSPtr);
            virtual void visit(Spline_CSPtr, Text_CSPtr);
            virtual void visit(Spline_CSPtr, Spline_CSPtr);

            std::vector<geo::Coordinate> result() const;
        private:
            std::vector<geo::Coordinate> _intersectionPoints;
            Method _method;
    };

    /**
      * \brief calculate intersection points of many entities
      * \note Can we make this into a general template ???
      * \sa Intersect
      */
    class IntersectMany {
        public:
            IntersectMany(std::vector<CADEntity_CSPtr>,  Intersect::Method);
            std::vector<geo::Coordinate> result() const;
        private:
            std::vector<CADEntity_CSPtr> _entities;
            Intersect::Method _method;
    };
}

#endif  //INTERSECT_H
