#ifndef ARC_H
#define ARC_H

#include "cad/const.h"
#include "cad/interface/entityvisitor.h"
#include "cad/interface/entitydispatch.h"

#include "lckernel_global.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geoarc.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"

#include <cad/meta/layer.h>

namespace lc {
    class Arc : public std::enable_shared_from_this<Arc>, public CADEntity, public geo::Arc {
        public:
            Arc(const geo::Coordinate& center, double radius, double startAngle, double endAngle, const std::shared_ptr<const Layer> layer);
            Arc(const geo::Coordinate& center, double radius, double startAngle, double endAngle, const std::shared_ptr<const Layer> layer, const QList<std::shared_ptr<const MetaType> >& metaTypes);

        public:
            virtual std::shared_ptr<const CADEntity> move(const geo::Coordinate& offset) const;
            virtual std::shared_ptr<const CADEntity> copy(const geo::Coordinate& offset) const;
            virtual std::shared_ptr<const CADEntity> rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const;
            virtual std::shared_ptr<const CADEntity> scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const;

        public:
            virtual void accept(std::shared_ptr<const Line> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(std::shared_ptr<const Circle> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(std::shared_ptr<const Arc> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(std::shared_ptr<const Ellipse> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(std::shared_ptr<const Text> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(std::shared_ptr<const Spline> o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(std::shared_ptr<const CADEntity> o, EntityVisitor& ei) const {
                o->accept(shared_from_this(), ei);
            }
            virtual void dispatch(EntityDispatch& ed) const {
                ed.visit(shared_from_this());
            }
    };
}

#endif // ARC_H
