#ifndef TRIM_H
#define TRIM_H

#include "cad/base/cadentity.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/interface/entityvisitor.h"
#include "cad/interface/entitydispatch.h"

namespace lc {

    class Trim : public EntityDispatch {
        public:
            Trim(QList<boost::shared_ptr<const lc::CADEntity> > limitingEntities, boost::shared_ptr<const lc::CADEntity> trimmedShape, geo::Coordinate trimPoint);

            virtual void visit(boost::shared_ptr<const lc::Line>);
            virtual void visit(boost::shared_ptr<const lc::Circle>);
            virtual void visit(boost::shared_ptr<const lc::Arc>);
            virtual void visit(boost::shared_ptr<const lc::Ellipse>);
            QList<boost::shared_ptr<const lc::CADEntity> > result();
            boost::shared_ptr<const lc::CADEntity> trimmedShape() const;

        private:
            QList<boost::shared_ptr<const lc::CADEntity> > _limitingEntities;
            boost::shared_ptr<const lc::CADEntity> _trimmedShape;
            QList<boost::shared_ptr<const lc::CADEntity> > _newEntities;
            geo::Coordinate _trimPoint;
    };
}
#endif // TRIM_H
