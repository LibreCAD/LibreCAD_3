#ifndef INTERSECT_H
#define INTERSECT_H

#include "cad/base/cadentity.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/interface/entityvisitor.h"

namespace lc {

    /**
      * @brief calculate the intersection pojnts of 2 entities
      * @sa IntersectMany
      */
    class Intersect : public EntityVisitor {
        public:
            enum Method {
                OnPath = 0,   // means that the paths must intersect
                Any = 1       // means that the paths may intersect outside of the real path.
                              // For example two lines of 2 in a slight angle might intersect outside of line's coordinate
                              // When method == Any is selected, the system will return that coordinate
                              // When Intersect is selected only a point is returned for a actual cross point
            };

            Intersect(Method method, double tolerance);
            virtual void visit(Line_CSPtr, Line_CSPtr);
            virtual void visit(Line_CSPtr, Circle_CSPtr);
            virtual void visit(Line_CSPtr, Arc_CSPtr);
            virtual void visit(Line_CSPtr, Ellipse_CSPtr);
            virtual void visit(Line_CSPtr, Text_CSPtr);
            virtual void visit(Line_CSPtr, Spline_CSPtr);
            virtual void visit(Line_CSPtr, MText_CSPtr);
            virtual void visit(Line_CSPtr, Dimension_CSPtr);

            virtual void visit(Circle_CSPtr, Line_CSPtr);
            virtual void visit(Circle_CSPtr, Circle_CSPtr);
            virtual void visit(Circle_CSPtr, Arc_CSPtr);
            virtual void visit(Circle_CSPtr, Ellipse_CSPtr);
            virtual void visit(Circle_CSPtr, Text_CSPtr);
            virtual void visit(Circle_CSPtr, Spline_CSPtr);
            virtual void visit(Circle_CSPtr, MText_CSPtr);
            virtual void visit(Circle_CSPtr, Dimension_CSPtr);

            virtual void visit(Arc_CSPtr, Line_CSPtr);
            virtual void visit(Arc_CSPtr, Circle_CSPtr);
            virtual void visit(Arc_CSPtr, Arc_CSPtr);
            virtual void visit(Arc_CSPtr, Ellipse_CSPtr);
            virtual void visit(Arc_CSPtr, Text_CSPtr);
            virtual void visit(Arc_CSPtr, Spline_CSPtr);
            virtual void visit(Arc_CSPtr, MText_CSPtr);
            virtual void visit(Arc_CSPtr, Dimension_CSPtr);

            virtual void visit(Ellipse_CSPtr, Line_CSPtr);
            virtual void visit(Ellipse_CSPtr, Circle_CSPtr);
            virtual void visit(Ellipse_CSPtr, Arc_CSPtr);
            virtual void visit(Ellipse_CSPtr, Ellipse_CSPtr);
            virtual void visit(Ellipse_CSPtr, Text_CSPtr);
            virtual void visit(Ellipse_CSPtr, Spline_CSPtr);
            virtual void visit(Ellipse_CSPtr, MText_CSPtr);
            virtual void visit(Ellipse_CSPtr, Dimension_CSPtr);

            virtual void visit(Text_CSPtr, Line_CSPtr);
            virtual void visit(Text_CSPtr, Circle_CSPtr);
            virtual void visit(Text_CSPtr, Arc_CSPtr);
            virtual void visit(Text_CSPtr, Ellipse_CSPtr);
            virtual void visit(Text_CSPtr, Text_CSPtr);
            virtual void visit(Text_CSPtr, Spline_CSPtr);
            virtual void visit(Text_CSPtr, MText_CSPtr);
            virtual void visit(Text_CSPtr, Dimension_CSPtr);

            virtual void visit(Spline_CSPtr, Line_CSPtr);
            virtual void visit(Spline_CSPtr, Circle_CSPtr);
            virtual void visit(Spline_CSPtr, Arc_CSPtr);
            virtual void visit(Spline_CSPtr, Ellipse_CSPtr);
            virtual void visit(Spline_CSPtr, Text_CSPtr);
            virtual void visit(Spline_CSPtr, Spline_CSPtr);
            virtual void visit(Spline_CSPtr, MText_CSPtr);
            virtual void visit(Spline_CSPtr, Dimension_CSPtr);

            virtual void visit(MText_CSPtr, Line_CSPtr);
            virtual void visit(MText_CSPtr, Circle_CSPtr);
            virtual void visit(MText_CSPtr, Arc_CSPtr);
            virtual void visit(MText_CSPtr, Ellipse_CSPtr);
            virtual void visit(MText_CSPtr, Text_CSPtr);
            virtual void visit(MText_CSPtr, Spline_CSPtr);
            virtual void visit(MText_CSPtr, MText_CSPtr);
            virtual void visit(MText_CSPtr, Dimension_CSPtr);

            virtual void visit(Dimension_CSPtr, Line_CSPtr);
            virtual void visit(Dimension_CSPtr, Circle_CSPtr);
            virtual void visit(Dimension_CSPtr, Arc_CSPtr);
            virtual void visit(Dimension_CSPtr, Ellipse_CSPtr);
            virtual void visit(Dimension_CSPtr, Text_CSPtr);
            virtual void visit(Dimension_CSPtr, Spline_CSPtr);
            virtual void visit(Dimension_CSPtr, MText_CSPtr);
            virtual void visit(Dimension_CSPtr, Dimension_CSPtr);

            std::vector<geo::Coordinate> result() const;
        private:
            std::vector<geo::Coordinate> _intersectionPoints;
            const Method _method;
            const double _tolerance;
    };

    /**
      * @brief calculate intersection points of many entities
      * @note Can we make this into a general template ???
      * @sa Intersect
      */
    class IntersectMany {
        public:
            IntersectMany(std::vector<CADEntity_CSPtr>,  Intersect::Method, double tolerance);
            std::vector<geo::Coordinate> result() const;
        private:
            std::vector<CADEntity_CSPtr> _entities;
            const Intersect::Method _method;
            const double _tolerance;
    };

    /**
      * @brief calculate intersection points of many entities
      * @note Can we make this into a general template ???
      * @sa Intersect
      */
    class IntersectAgainstOthers {
        public:
            IntersectAgainstOthers(std::vector<CADEntity_CSPtr>,  std::vector<CADEntity_CSPtr>, Intersect::Method, double tolerance);
            std::vector<geo::Coordinate> result() const;
        private:
            std::vector<CADEntity_CSPtr> _entities;
            std::vector<CADEntity_CSPtr> _others;
            const Intersect::Method _method;
            const double _tolerance;
    };

    /**
      * @brief calculate intersection points of many entities but beal out asap when a intersection point was found
      * @note Can we make this into a general template ???
      * @sa Intersect
      */
    class HasIntersectAgainstOthers {
        public:
            HasIntersectAgainstOthers(std::vector<CADEntity_CSPtr>,  std::vector<CADEntity_CSPtr>, Intersect::Method, double tolerance);
            bool result() const;
        private:
            std::vector<CADEntity_CSPtr> _entities;
            std::vector<CADEntity_CSPtr> _others;
            const Intersect::Method _method;
            const double _tolerance;
    };

}

#endif  //INTERSECT_H
