#ifndef TEXT_H
#define TEXT_H

#include "cad/const.h"
#include "cad/interface/entityvisitor.h"
#include "cad/interface/entitydispatch.h"

#include "lckernel_global.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geotext.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"

namespace lc {
    class Text : public enable_shared_from_this<Text>, public CADEntity, public geo::Text {
        public:
            Text(const geo::Coordinate& insertion_point, double height, QString text_value);
            Text(const geo::Coordinate& insertion_point, double height, QString text_value, const QList<shared_ptr<const lc::MetaType> >& metaTypes);

        public:
            virtual shared_ptr<const lc::CADEntity> move(const geo::Coordinate& offset, const geo::Coordinate& rotation_center, const double& rotation_angle) const;
            virtual shared_ptr<const lc::CADEntity> copy(const geo::Coordinate& offset, const geo::Coordinate& rotation_center, const double& rotation_angle) const;
            virtual shared_ptr<const lc::CADEntity> rotate(const geo::Coordinate& offset, const geo::Coordinate &rotation_center, const double& rotation_angle, const bool with_same_id) const;

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


#endif // TEXT_H
