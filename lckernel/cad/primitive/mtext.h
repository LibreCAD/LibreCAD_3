#pragma once

#include "cad/const.h"
#include "cad/interface/entityvisitor.h"
#include "cad/interface/entitydispatch.h"


#include "cad/geometry/geocoordinate.h"
#include "cad/geometry/geomtext.h"
#include "cad/base/cadentity.h"
#include "cad/vo/entitycoordinate.h"

namespace lc {
    class MText;
    typedef std::shared_ptr<MText> MText_SPtr;
    typedef std::shared_ptr<const MText> MText_CSPtr;


    class MText : public std::enable_shared_from_this<MText>, public CADEntity, public geo::MText {
        public:

            /**
             * @brief MText
             * @param insertion_point insertion_point of the mtext
             * @param height height of the text
             * @param width width of text
             * @param valign vertical alignment
             * @param halign horizontal alignment
             * @param drawingDirection direction of drawing
             * @param lineSpacingStyle line spacing style
             * @param lineSpacingFactor spacing factor
             * @param text_value text value itself
             * @param style style name of text
             * @param angle angle of obliqueness
             * @param layer layer of entity
             */
            MText(const geo::Coordinate& insertion_point,
                  const double height,
                  const double width,
                  const VAlign valign,
                  const HAlign halign,
                  const DrawingDirection drawingDirection,
                  const LineSpacingStyle lineSpacingStyle,
                  const double lineSpacingFactor,
                  const std::string& text_value,
                  const std::string& style,
                  const double angle, const Layer_CSPtr layer);

            /**
             * @brief MText
             * @param insertion_point insertion_point of the mtext
             * @param height height of the text
             * @param width width of text
             * @param valign vertical alignment
             * @param halign horizontal alignment
             * @param drawingDirection direction of drawing
             * @param lineSpacingStyle line spacing style
             * @param lineSpacingFactor spacing factor
             * @param text_value text value itself
             * @param style style name of text
             * @param angle angle of obliqueness
             * @param layer layer of entity
             * @param metaTypes metatype of entity
             */
            MText(const geo::Coordinate& insertion_point,
                  const double height,
                  const double width,
                  const VAlign valign,
                  const HAlign halign,
                  const DrawingDirection drawingDirection,
                  const LineSpacingStyle lineSpacingStyle,
                  const double lineSpacingFactor,
                  const std::string& text_value,
                  const std::string& style,
                  const double angle, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo);


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
             * @return
             */
            virtual CADEntity_CSPtr scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const;

            virtual CADEntity_CSPtr modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const;
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

// MTEXT_H
