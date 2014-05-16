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
    class Text;
    typedef std::shared_ptr<Text> Text_SPtr;
    typedef std::shared_ptr<const Text> Text_CSPtr;

    class Text : public std::enable_shared_from_this<Text>, public CADEntity, public geo::Text {
        public:
            Text(const geo::Coordinate& insertion_point, double height, QString text_value, const Layer_CSPtr layer);
            Text(const geo::Coordinate& insertion_point, double height, QString text_value, const Layer_CSPtr layer, const std::list<MetaType_CSPtr>& metaTypes);

        public:
            virtual CADEntity_CSPtr move(const geo::Coordinate& offset) const;
            virtual CADEntity_CSPtr copy(const geo::Coordinate& offset) const;
            virtual CADEntity_CSPtr rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const;
            virtual CADEntity_CSPtr scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const;

        public:
            virtual void accept(Line_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(Circle_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(Arc_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(Ellipse_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(Text_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(CADEntity_CSPtr o, EntityVisitor& ei) const {
                o->accept(shared_from_this(), ei);
            }
            virtual void accept(Spline_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void dispatch(EntityDispatch& ed) const {
                ed.visit(shared_from_this());
            }

    };
}


#endif // TEXT_H
