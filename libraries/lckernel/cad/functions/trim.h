#ifndef TRIM_H
#define TRIM_H

#include "cad/geometry/geocoordinate.h"
#include "cad/interface/entityinteraction.h"

namespace lc {
    class Trim : public EntityInteraction {
        public:
            Trim(geo::Coordinate trimPoint);
            virtual void visitInteraction(std::shared_ptr<const Line>, std::shared_ptr<const Line>);
            virtual void visitInteraction(std::shared_ptr<const Line>, std::shared_ptr<const Circle>);
            virtual void visitInteraction(std::shared_ptr<const Line>, std::shared_ptr<const Arc>);
            virtual void visitInteraction(std::shared_ptr<const Line>, std::shared_ptr<const Ellipse>);

            virtual void visitInteraction(std::shared_ptr<const Circle>, std::shared_ptr<const Line>);
            virtual void visitInteraction(std::shared_ptr<const Circle>, std::shared_ptr<const Circle>);
            virtual void visitInteraction(std::shared_ptr<const Circle>, std::shared_ptr<const Arc>);
            virtual void visitInteraction(std::shared_ptr<const Circle>, std::shared_ptr<const Ellipse>);

            virtual void visitInteraction(std::shared_ptr<const Arc>, std::shared_ptr<const Line>);
            virtual void visitInteraction(std::shared_ptr<const Arc>, std::shared_ptr<const Circle>);
            virtual void visitInteraction(std::shared_ptr<const Arc>, std::shared_ptr<const Arc>);
            virtual void visitInteraction(std::shared_ptr<const Arc>, std::shared_ptr<const Ellipse>);

            virtual void visitInteraction(std::shared_ptr<const Ellipse>, std::shared_ptr<const Line>);
            virtual void visitInteraction(std::shared_ptr<const Ellipse>, std::shared_ptr<const Circle>);
            virtual void visitInteraction(std::shared_ptr<const Ellipse>, std::shared_ptr<const Arc>);
            virtual void visitInteraction(std::shared_ptr<const Ellipse>, std::shared_ptr<const Ellipse>);

        private:
            QList<std::tr1::shared_ptr<const lc::CADEntity> > _newEntities;
            geo::Coordinate _trimPoint;
    };
}
#endif // TRIM_H
