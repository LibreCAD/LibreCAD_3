#ifndef FTRIM_H
#define FTRIM_H

#include "cad/base/cadentity.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/interface/entityvisitor.h"
#include "cad/interface/entitydispatch.h"

namespace lc {

        class FTrim : public EntityDispatch {
            public:
                FTrim(QList<shared_ptr<const CADEntity> > limitingEntities, shared_ptr<const CADEntity> trimmedShape, geo::Coordinate trimPoint);

                virtual void visit(shared_ptr<const Line>);
                virtual void visit(shared_ptr<const Circle>);
                virtual void visit(shared_ptr<const Arc>);
                virtual void visit(shared_ptr<const Ellipse>);
                virtual void visit(shared_ptr<const Text>);
                QList<shared_ptr<const CADEntity> > result();
                shared_ptr<const CADEntity> trimmedShape() const;

            private:
                QList<shared_ptr<const CADEntity> > _limitingEntities;
                shared_ptr<const CADEntity> _trimmedShape;
                QList<shared_ptr<const CADEntity> > _newEntities;
                geo::Coordinate _trimPoint;
        };
}
#endif // FTRIM_H
