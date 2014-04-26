#ifndef CIRCLE_H
#define CIRCLE_H

#include <QDebug>

#include <cad/meta/layer.h>

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
            Circle(const geo::Coordinate& center, double radius, const shared_ptr<const Layer> _layer);
            Circle(const geo::Coordinate& center, double radius, const shared_ptr<const Layer> _layer, const QList<shared_ptr<const MetaType> >& metaTypes);

        public:
            virtual QList<EntityCoordinate> snapPoints(const geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const;
            virtual geo::Coordinate nearestPointOnPath(const geo::Coordinate& coord) const;

        public:
            virtual shared_ptr<const CADEntity> move(const geo::Coordinate& offset) const;
            virtual shared_ptr<const CADEntity> copy(const geo::Coordinate& offset) const;
            virtual shared_ptr<const CADEntity> rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const;
            virtual shared_ptr<const CADEntity> scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const;
        public:
            virtual void accept(shared_ptr<const Line> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(shared_ptr<const Circle> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(shared_ptr<const Arc> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(shared_ptr<const Ellipse> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(shared_ptr<const Text> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(shared_ptr<const Spline> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(shared_ptr<const CADEntity> o, EntityVisitor& ei) const {
                o->accept(shared_from_this(), ei);
            }
            virtual void dispatch(EntityDispatch& ed) const {
                ed.visit(shared_from_this());
            }

    };
}
#endif // CIRCLE_H
