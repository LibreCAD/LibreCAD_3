#ifndef CIRCLE_H
#define CIRCLE_H

#include "qdebug.h"

#include "cad/const.h"
#include "cad/interface/entityinteraction.h"

#include "lckernel_global.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geocircle.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"
#include "cad/interface/snapable.h"

namespace lc {


    class Circle : public std::tr1::enable_shared_from_this<Circle>, public CADEntity, public geo::Circle, public Snapable {
        public:
            Circle(const geo::Coordinate& center, double radius);
            Circle(const geo::Coordinate& center, double radius, const QList<std::tr1::shared_ptr<const lc::MetaType> >& metaTypes);

            virtual QList<lc::EntityCoordinate> snapPoints(const geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const;
            virtual geo::Coordinate nearestPointOnPath(const geo::Coordinate& coord) const;

        public:
            virtual void accept(std::tr1::shared_ptr<const lc::Line> o, EntityInteraction& ei) const {
                ei.visitInteraction(shared_from_this(), o);
            }
            virtual void accept(std::tr1::shared_ptr<const lc::Circle> o, EntityInteraction& ei) const {
                ei.visitInteraction(shared_from_this(), o);
            }
            virtual void accept(std::tr1::shared_ptr<const lc::Arc> o, EntityInteraction& ei) const {
                ei.visitInteraction(shared_from_this(), o);
            }
            virtual void accept(std::tr1::shared_ptr<const lc::Ellipse> o, EntityInteraction& ei) const {
                ei.visitInteraction(shared_from_this(), o);
            }
            virtual void accept(std::tr1::shared_ptr<const lc::CADEntity> o, EntityInteraction& ei) const {
                o->accept(shared_from_this(), ei);
            }
    };
}
#endif // CIRCLE_H
