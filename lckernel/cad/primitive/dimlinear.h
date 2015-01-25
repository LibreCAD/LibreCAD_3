#pragma once

#include "cad/const.h"
#include "cad/interface/entityvisitor.h"
#include "cad/interface/entitydispatch.h"

#include "cad/primitive/dimension.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"
#include "cad/math/lcmath.h"
#include "coordinate.h"

namespace lc {
    class DimLinear;
    typedef std::shared_ptr<DimLinear> DimLinear_SPtr;
    typedef std::shared_ptr<const DimLinear> DimLinear_CSPtr;


    class DimLinear : public std::enable_shared_from_this<DimLinear>, public CADEntity, public Dimension {
        public:


            /**
             * @brief DimLinear, DimLinear constructor with metatypes
             * @param Dimension dimension
             * @param geo::Coordinate extension_point1
             * @param geo::Coordinate extension_point2
             * @param double oblique
             * @param Layer_CSPtr layer
             */
            DimLinear(geo::Coordinate const& definitionPoint, geo::Coordinate const& middleOfText, TextConst::AttachmentPoint const& attachmentPoint, double angle, double const lineSpacingFactor,
                      TextConst::LineSpacingStyle const& lineSpacingStyle, std::string const& explicitValue,
                      geo::Coordinate const& definitionPoint2,  geo::Coordinate const& definitionPoint3, const double oblique, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo);


            DimLinear(const DimLinear_CSPtr other, bool sameID = false);

            static DimLinear_SPtr dimAuto(geo::Coordinate const& p1, geo::Coordinate const& p2, TextConst::AttachmentPoint const& attachmentPoint, double angle, double const lineSpacingFactor,
                                          TextConst::LineSpacingStyle const& lineSpacingStyle, std::string const& explicitValue, const double oblique, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo);

        public:
            /**
             * @brief move, moves by an offset
             * @param geo::Coordinate offset
             * @return CADEntity_CSPtr moved entity
             */
            virtual CADEntity_CSPtr move(const geo::Coordinate& offset) const;

            /**
             * @brief copy, copies by an offset
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
             * @return
             */
            virtual CADEntity_CSPtr scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const;

            /**
             * @brief boundingBox of the entity
             * @return geo::Area area
             */
            virtual const geo::Area boundingBox() const;

            virtual CADEntity_CSPtr modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const;

            // Oblique angle http://www.cad-notes.com/autocad-isometric-text-and-dimension/
            double oblique() const;

            // Where p2 specifies the first point of the dimension, p3 specifies that second point ofthe dimension
            // defPoint specifies where the dimension is specified and notates the horizontal/vertical 'line' between the two points
            geo::Coordinate definitionPoint2() const;
            geo::Coordinate definitionPoint3() const;
        protected:
            const double _oblique;
            const geo::Coordinate _definitionPoint2;
            const geo::Coordinate _definitionPoint3;
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

// DIMLINEAR_H
