#ifndef CIRCLE_H
#define CIRCLE_H

#include "qdebug.h"

#include "cad/const.h"
#include "cad/interface/entityvisitor.h"
#include "cad/interface/entitydispatch.h"

#include "lckernel_global.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geocircle.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"
#include "cad/interface/snapable.h"

namespace lc {


    class Circle : public enable_shared_from_this<Circle>, public CADEntity, public geo::Circle, public Snapable {
        public:
            Circle(const geo::Coordinate& center, double radius);
            Circle(const geo::Coordinate& center, double radius, const QList<shared_ptr<const lc::MetaType> >& metaTypes);
            virtual shared_ptr<const lc::CADEntity> move(const geo::Coordinate& offset) const;
            virtual QList<lc::EntityCoordinate> snapPoints(const geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const;
            virtual geo::Coordinate nearestPointOnPath(const geo::Coordinate& coord) const;
            virtual shared_ptr<const lc::CADEntity> rotate(const double& angle) const;
        public:
            virtual void accept(shared_ptr<const lc::Line> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(shared_ptr<const lc::Circle> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(shared_ptr<const lc::Arc> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(shared_ptr<const lc::Ellipse> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(shared_ptr<const lc::Text> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(shared_ptr<const lc::CADEntity> o, EntityVisitor& ei) const {
                o->accept(shared_from_this(), ei);
            }
            virtual void dispatch(EntityDispatch& ed) const {
                ed.visit(shared_from_this());
            }

    };
}
#endif // CIRCLE_H
