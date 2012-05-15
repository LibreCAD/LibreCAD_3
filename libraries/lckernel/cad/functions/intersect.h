#ifndef INTERSECT_H
#define INTERSECT_H

#include "cad/geometry/geocoordinate.h"
#include "cad/interface/entityinteraction.h"

namespace lc {

    class Intersect : public EntityInteraction {
        public:
            enum Method {
                MustIntersect,
                Any = 1
            };

            Intersect(Method method);
            virtual void visitInteraction(std::tr1::shared_ptr<const Line>, std::tr1::shared_ptr<const Line>);
            virtual void visitInteraction(std::tr1::shared_ptr<const Line>, std::tr1::shared_ptr<const Circle>);
            virtual void visitInteraction(std::tr1::shared_ptr<const Line>, std::tr1::shared_ptr<const Arc>);
            virtual void visitInteraction(std::tr1::shared_ptr<const Line>, std::tr1::shared_ptr<const Ellipse>);

            virtual void visitInteraction(std::tr1::shared_ptr<const Circle>, std::tr1::shared_ptr<const Line>);
            virtual void visitInteraction(std::tr1::shared_ptr<const Circle>, std::tr1::shared_ptr<const Circle>);
            virtual void visitInteraction(std::tr1::shared_ptr<const Circle>, std::tr1::shared_ptr<const Arc>);
            virtual void visitInteraction(std::tr1::shared_ptr<const Circle>, std::tr1::shared_ptr<const Ellipse>);

            virtual void visitInteraction(std::tr1::shared_ptr<const Arc>, std::tr1::shared_ptr<const Line>);
            virtual void visitInteraction(std::tr1::shared_ptr<const Arc>, std::tr1::shared_ptr<const Circle>);
            virtual void visitInteraction(std::tr1::shared_ptr<const Arc>, std::tr1::shared_ptr<const Arc>);
            virtual void visitInteraction(std::tr1::shared_ptr<const Arc>, std::tr1::shared_ptr<const Ellipse>);

            virtual void visitInteraction(std::tr1::shared_ptr<const Ellipse>, std::tr1::shared_ptr<const Line>);
            virtual void visitInteraction(std::tr1::shared_ptr<const Ellipse>, std::tr1::shared_ptr<const Circle>);
            virtual void visitInteraction(std::tr1::shared_ptr<const Ellipse>, std::tr1::shared_ptr<const Arc>);
            virtual void visitInteraction(std::tr1::shared_ptr<const Ellipse>, std::tr1::shared_ptr<const Ellipse>);

            QList<geo::Coordinate> result() const;
        private:
            QList<geo::Coordinate> _intersectionPoints;
            Method _method;
    };

}

#endif  INTERSECT_H
