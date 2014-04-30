#ifndef FTRIM_H
#define FTRIM_H

#include "cad/base/cadentity.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/interface/entityvisitor.h"
#include "cad/interface/entitydispatch.h"

namespace lc {

    class FTrim : public EntityDispatch {
        public:
            FTrim(QList<std::shared_ptr<const CADEntity> > limitingEntities, std::shared_ptr<const CADEntity> trimmedShape, geo::Coordinate trimPoint);

            virtual void visit(std::shared_ptr<const Line>);
            virtual void visit(std::shared_ptr<const Circle>);
            virtual void visit(std::shared_ptr<const Arc>);
            virtual void visit(std::shared_ptr<const Ellipse>);
            virtual void visit(std::shared_ptr<const Text>);
            QList<std::shared_ptr<const CADEntity> > result();
            std::shared_ptr<const CADEntity> trimmedShape() const;

        private:
            QList<std::shared_ptr<const CADEntity> > _limitingEntities;
            std::shared_ptr<const CADEntity> _trimmedShape;
            QList<std::shared_ptr<const CADEntity> > _newEntities;
            geo::Coordinate _trimPoint;
    };
}
#endif // FTRIM_H
