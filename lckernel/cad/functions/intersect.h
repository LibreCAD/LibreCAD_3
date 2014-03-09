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
            virtual void visit(shared_ptr<const Line>, shared_ptr<const Line>);
            virtual void visit(shared_ptr<const Line>, shared_ptr<const Circle>);
            virtual void visit(shared_ptr<const Line>, shared_ptr<const Arc>);
            virtual void visit(shared_ptr<const Line>, shared_ptr<const Ellipse>);
            virtual void visit(shared_ptr<const Line>, shared_ptr<const Text>);

            virtual void visit(shared_ptr<const Circle>, shared_ptr<const Line>);
            virtual void visit(shared_ptr<const Circle>, shared_ptr<const Circle>);
            virtual void visit(shared_ptr<const Circle>, shared_ptr<const Arc>);
            virtual void visit(shared_ptr<const Circle>, shared_ptr<const Ellipse>);
            virtual void visit(shared_ptr<const Circle>, shared_ptr<const Text>);


            virtual void visit(shared_ptr<const Arc>, shared_ptr<const Line>);
            virtual void visit(shared_ptr<const Arc>, shared_ptr<const Circle>);
            virtual void visit(shared_ptr<const Arc>, shared_ptr<const Arc>);
            virtual void visit(shared_ptr<const Arc>, shared_ptr<const Ellipse>);
            virtual void visit(shared_ptr<const Arc>, shared_ptr<const Text>);


            virtual void visit(shared_ptr<const Ellipse>, shared_ptr<const Line>);
            virtual void visit(shared_ptr<const Ellipse>, shared_ptr<const Circle>);
            virtual void visit(shared_ptr<const Ellipse>, shared_ptr<const Arc>);
            virtual void visit(shared_ptr<const Ellipse>, shared_ptr<const Ellipse>);
            virtual void visit(shared_ptr<const Ellipse>, shared_ptr<const Text>);

            virtual void visit(shared_ptr<const Text>, shared_ptr<const Line>);
            virtual void visit(shared_ptr<const Text>, shared_ptr<const Circle>);
            virtual void visit(shared_ptr<const Text>, shared_ptr<const Arc>);
            virtual void visit(shared_ptr<const Text>, shared_ptr<const Ellipse>);
            virtual void visit(shared_ptr<const Text>, shared_ptr<const Text>);

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
            IntersectMany(QList<shared_ptr<const CADEntity> >,  Intersect::Method);
            QList<geo::Coordinate> result() const;
        private:
            QList<shared_ptr<const CADEntity> > _entities;
            Intersect::Method _method;
    };
}

#endif  //INTERSECT_H
