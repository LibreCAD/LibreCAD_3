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
            virtual void visit(std::shared_ptr<const Line>, std::shared_ptr<const Line>);
            virtual void visit(std::shared_ptr<const Line>, std::shared_ptr<const Circle>);
            virtual void visit(std::shared_ptr<const Line>, std::shared_ptr<const Arc>);
            virtual void visit(std::shared_ptr<const Line>, std::shared_ptr<const Ellipse>);
            virtual void visit(std::shared_ptr<const Line>, std::shared_ptr<const Text>);
            virtual void visit(std::shared_ptr<const Line>, std::shared_ptr<const Spline>);

            virtual void visit(std::shared_ptr<const Circle>, std::shared_ptr<const Line>);
            virtual void visit(std::shared_ptr<const Circle>, std::shared_ptr<const Circle>);
            virtual void visit(std::shared_ptr<const Circle>, std::shared_ptr<const Arc>);
            virtual void visit(std::shared_ptr<const Circle>, std::shared_ptr<const Ellipse>);
            virtual void visit(std::shared_ptr<const Circle>, std::shared_ptr<const Text>);
            virtual void visit(std::shared_ptr<const Circle>, std::shared_ptr<const Spline>);

            virtual void visit(std::shared_ptr<const Arc>, std::shared_ptr<const Line>);
            virtual void visit(std::shared_ptr<const Arc>, std::shared_ptr<const Circle>);
            virtual void visit(std::shared_ptr<const Arc>, std::shared_ptr<const Arc>);
            virtual void visit(std::shared_ptr<const Arc>, std::shared_ptr<const Ellipse>);
            virtual void visit(std::shared_ptr<const Arc>, std::shared_ptr<const Text>);
            virtual void visit(std::shared_ptr<const Arc>, std::shared_ptr<const Spline>);

            virtual void visit(std::shared_ptr<const Ellipse>, std::shared_ptr<const Line>);
            virtual void visit(std::shared_ptr<const Ellipse>, std::shared_ptr<const Circle>);
            virtual void visit(std::shared_ptr<const Ellipse>, std::shared_ptr<const Arc>);
            virtual void visit(std::shared_ptr<const Ellipse>, std::shared_ptr<const Ellipse>);
            virtual void visit(std::shared_ptr<const Ellipse>, std::shared_ptr<const Text>);
            virtual void visit(std::shared_ptr<const Ellipse>, std::shared_ptr<const Spline>);

            virtual void visit(std::shared_ptr<const Text>, std::shared_ptr<const Line>);
            virtual void visit(std::shared_ptr<const Text>, std::shared_ptr<const Circle>);
            virtual void visit(std::shared_ptr<const Text>, std::shared_ptr<const Arc>);
            virtual void visit(std::shared_ptr<const Text>, std::shared_ptr<const Ellipse>);
            virtual void visit(std::shared_ptr<const Text>, std::shared_ptr<const Text>);
            virtual void visit(std::shared_ptr<const Text>, std::shared_ptr<const Spline>);

            virtual void visit(std::shared_ptr<const Spline>, std::shared_ptr<const Line>);
            virtual void visit(std::shared_ptr<const Spline>, std::shared_ptr<const Circle>);
            virtual void visit(std::shared_ptr<const Spline>, std::shared_ptr<const Arc>);
            virtual void visit(std::shared_ptr<const Spline>, std::shared_ptr<const Ellipse>);
            virtual void visit(std::shared_ptr<const Spline>, std::shared_ptr<const Text>);
            virtual void visit(std::shared_ptr<const Spline>, std::shared_ptr<const Spline>);

            QList<geo::Coordinate> result() const;
        private:
            QList<geo::Coordinate> _intersectionPoints;
            Method _method;
    };

    /**
      * \brief calculate intersection points of many entities
      * \note Can we make this into a general template ???
      * \sa Intersect
      */
    class IntersectMany {
        public:
            IntersectMany(QList<std::shared_ptr<const CADEntity> >,  Intersect::Method);
            QList<geo::Coordinate> result() const;
        private:
            QList<std::shared_ptr<const CADEntity> > _entities;
            Intersect::Method _method;
    };
}

#endif  //INTERSECT_H
