#ifndef DIMLINEAR_H
#define DIMLINEAR_H

#include "cad/const.h"
#include "cad/interface/entityvisitor.h"
#include "cad/interface/entitydispatch.h"

#include "lckernel_global.h"
#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geodimlinear.h"
#include "cad/geometry/geodimension.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"
#include "cad/math/lcmath.h"

namespace lc {
    class DimLinear;
    typedef std::shared_ptr<DimLinear> DimLinear_SPtr;
    typedef std::shared_ptr<const DimLinear> DimLinear_CSPtr;


    class DimLinear : public std::enable_shared_from_this<DimLinear>, public CADEntity, public geo::DimLinear {
        public:

            /**
                 * @brief DimAngular
                 * @param dimension
                 * @param extension_point1
                 * @param extension_point2
                 * @param layer
                 */
            DimLinear(const Dimension& dimension,
                      const geo::Coordinate& extension_point1,
                      const geo::Coordinate& extension_point2,
                      const double oblique,
                      const Layer_CSPtr layer);

            /**
             * @brief DimAngular
             * @param dimension
             * @param extension_point1
             * @param extension_point2
             * @param layer
             * @param metaTypes
             */
            DimLinear(const Dimension& dimension,
                      const geo::Coordinate& extension_point1,
                      const geo::Coordinate& extension_point2,
                      const double oblique,
                      const Layer_CSPtr layer, const std::list<MetaType_CSPtr>& metaTypes);

        public:
            virtual CADEntity_CSPtr move(const geo::Coordinate& offset) const;
            virtual CADEntity_CSPtr copy(const geo::Coordinate& offset) const;
            virtual CADEntity_CSPtr rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const;
            virtual CADEntity_CSPtr scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const;
            virtual const geo::Area boundingBox() const;

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

#endif // DIMLINEAR_H
