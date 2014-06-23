#ifndef COORDINATE_H
#define COORDINATE_H

#include "cad/const.h"
#include "cad/interface/entityvisitor.h"
#include "cad/interface/entitydispatch.h"


#include "cad/geometry/geocoordinate.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"

#include <cad/meta/layer.h>

namespace lc {

    class Coordinate;
    typedef std::shared_ptr<Coordinate> Coordinate_SPtr;
    typedef std::shared_ptr<const Coordinate> Coordinate_CSPtr;

    class Coordinate : public std::enable_shared_from_this<Coordinate>, public CADEntity, public geo::Coordinate {
        public:
            Coordinate(const double x, const double y, const Layer_CSPtr layer);
            Coordinate(const double x, const double y, const Layer_CSPtr layer, const std::list<MetaType_CSPtr>& metaTypes);
            Coordinate(const Coordinate_CSPtr other, bool sameID = false);

        public:
            virtual CADEntity_CSPtr move(const geo::Coordinate& offset) const;
            virtual CADEntity_CSPtr copy(const geo::Coordinate& offset) const;
            virtual CADEntity_CSPtr rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const;
            virtual CADEntity_CSPtr scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const;
            virtual const geo::Area boundingBox() const;
            virtual Quadratic_CSPtr quadratic_equation() const;

        public:
            virtual void accept(const geo::Vector& o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(Coordinate_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
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
            virtual void accept(Spline_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(MText_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(Dimension_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(DimAligned_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(DimAngular_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(DimDiametric_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(DimLinear_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(DimRadial_CSPtr o, EntityVisitor& ei) const {
                ei.visit(shared_from_this(), o);
            }
            virtual void accept(CADEntity_CSPtr o, EntityVisitor& ei) const {
                o->accept(shared_from_this(), ei);
            }
            virtual void dispatch(EntityDispatch& ed) const {
                ed.visit(shared_from_this());
            }
    };
}

#endif // COORDINATE_H
