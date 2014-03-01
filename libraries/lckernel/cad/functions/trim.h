#ifndef TRIM_H
#define TRIM_H

#include "cad/base/cadentity.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/interface/entityvisitor.h"
#include "cad/interface/entitydispatch.h"

namespace lc {

    class Trim : public EntityDispatch {
        public:
            Trim(QList<shared_ptr<const lc::CADEntity> > limitingEntities, shared_ptr<const lc::CADEntity> trimmedShape, geo::Coordinate trimPoint);

            virtual void visit(shared_ptr<const lc::Line>);
            virtual void visit(shared_ptr<const lc::Circle>);
            virtual void visit(shared_ptr<const lc::Arc>);
            virtual void visit(shared_ptr<const lc::Ellipse>);
            QList<shared_ptr<const lc::CADEntity> > result();
            shared_ptr<const lc::CADEntity> trimmedShape() const;

        private:
            QList<shared_ptr<const lc::CADEntity> > _limitingEntities;
            shared_ptr<const lc::CADEntity> _trimmedShape;
            QList<shared_ptr<const lc::CADEntity> > _newEntities;
            geo::Coordinate _trimPoint;
    };
}
#endif // TRIM_H
