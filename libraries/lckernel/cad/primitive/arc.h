#ifndef ARC_H
#define ARC_H

#include "cad/const.h"
#include "cad/interface/entityinteraction.h"

#include "lckernel_global.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geoarc.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"

namespace lc {
    class Arc : public std::tr1::enable_shared_from_this<Arc>, public CADEntity, public geo::Arc {
        public:
            Arc(const geo::Coordinate& center, double radius, double startAngle, double endAngle);
            Arc(const geo::Coordinate& center, double radius, double startAngle, double endAngle, const QList<std::tr1::shared_ptr<const lc::MetaType> >& metaTypes);

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

#endif // ARC_H
