#pragma once

#include "cad/const.h"
#include "cad/interface/entityvisitor.h"
#include "cad/interface/entitydispatch.h"


#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geodimension.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"
#include "cad/math/lcmath.h"

namespace lc {
    class Dimension;
    typedef std::shared_ptr<Dimension> Dimension_SPtr;
    typedef std::shared_ptr<const Dimension> Dimension_CSPtr;


    class Dimension : public std::enable_shared_from_this<Dimension>, public CADEntity, public geo::Dimension {
        public:

            /**
             * @brief Dimension
             * @param definition_point definition_point of the dimension
             * @param middle_of_text Middle of text
             * @param valign Vertical Alignment
             * @param halign Horizontal Alignment
             * @param lineSpacingStyle Line spacing style
             * @param lineSpacingFactor Spacing factor
             * @param text_value Text of dimension
             * @param style STyle name
             * @param angle Angle of obliqueness
             * @param layer Layer of entity
             */
            Dimension(const geo::Coordinate& definition_point,
                      const geo::Coordinate& middle_of_text,
                      const geo::MText::VAlign valign,
                      const geo::MText::HAlign halign,
                      const geo::MText::LineSpacingStyle lineSpacingStyle,
                      const double lineSpacingFactor,
                      const std::string& text_value,
                      const std::string& style,
                      const double angle, const Layer_CSPtr layer);

            /**
             * @brief Dimension
             * @param definition_point definition_point of the dimension
             * @param middle_of_text Middle of text
             * @param valign Vertical Alignment
             * @param halign Horizontal Alignment
             * @param lineSpacingStyle Line spacing style
             * @param lineSpacingFactor Spacing factor
             * @param text_value Text of dimension
             * @param style STyle name
             * @param angle Angle of obliqueness
             * @param layer Layer of entity
             * @param metatype of entity
             */
            Dimension(const geo::Coordinate& definition_point,
                      const geo::Coordinate& middle_of_text,
                      const geo::MText::VAlign valign,
                      const geo::MText::HAlign halign,
                      const geo::MText::LineSpacingStyle lineSpacingStyle,
                      const double lineSpacingFactor,
                      const std::string& text_value,
                      const std::string& style,
                      const double angle, const Layer_CSPtr layer,  MetaInfo_CSPtr metaInfo);

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

            virtual CADEntity_CSPtr modify(Layer_CSPtr layer, MetaInfo_CSPtr metaInfo) const;
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

// Dimension_H

