#pragma once

#include "cad/const.h"
#include "cad/interface/entityvisitor.h"
#include "cad/interface/entitydispatch.h"


#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geodimdiametric.h"
#include "cad/geometry/geodimension.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"
#include "cad/math/lcmath.h"

namespace lc {
    class DimDiameteric;
    typedef std::shared_ptr<DimDiametric> DimDiametric_SPtr;
    typedef std::shared_ptr<const DimDiametric> DimDiametric_CSPtr;


    class DimDiametric : public std::enable_shared_from_this<DimDiametric>, public CADEntity, public geo::DimDiametric {
        public:

            /**
             * @brief DimAngular, DimDiametric default constructor
             * @param Dimension dimension
             * @param double leader
             * @param Layer_CSPtr layer
             */
            DimDiametric(const Dimension& dimension,
                         const double leader,
                         const Layer_CSPtr layer);

            /**
             * @brief DimDiametric, Dimdiametric constructor with metatypes
             * @param Dimension dimension
             * @param double leader
             * @param Layer_CSPtr layer
             * @param MetaTypes_CSPtr metaTypes
             */
            DimDiametric(const Dimension& dimension,
                         const double leader,
                         const Layer_CSPtr layer, MetaInfo_CSPtr metaInfo);

        public:
            /**
                             * @brief move, moves by an offset
                             * @param geo::Coordinate offset
                             * @return CADEntity_CSPtr moved entity
                             */
            virtual CADEntity_CSPtr move(const geo::Coordinate& offset) const;

            /**
                             * @brief copy, copies line by an offset
                             * @param geo::Coordinate offset
                             * @return CADEntity_CSPtr copied entity
                             */
            virtual CADEntity_CSPtr copy(const geo::Coordinate& offset) const;

            /**
                             * @brief rotate, rotate operation
                             * @param geo::Coordinate rotation_center
                             * @param double rotation_angle
                             * @return CADEntity_CSPtr rotated entity
                             */
            virtual CADEntity_CSPtr rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const;

            /**
                             * @brief scale, scales the entity
                             * @param geo::Coordinate scale_center
                             * @param double scale_factor
                             * @return CADEntity_CSPtr Scaled entity
                             */
            virtual CADEntity_CSPtr scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const;

            /**
                             * @brief boundingBox of the entity
                             * @return geo::Area area
                             */
            virtual const geo::Area boundingBox() const;

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

// DIMDIAMETRIC_H
