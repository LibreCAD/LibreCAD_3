#ifndef MTEXT_H
#define MTEXT_H

#include "cad/const.h"
#include "cad/interface/entityvisitor.h"
#include "cad/interface/entitydispatch.h"

#include "lckernel_global.h"
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
                  const double angle, const Layer_CSPtr layer, const std::list<MetaType_CSPtr>& metaTypes);


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
            virtual void accept(CADEntity_CSPtr o, EntityVisitor& ei) const {
                o->accept(shared_from_this(), ei);
            }
            virtual void dispatch(EntityDispatch& ed) const {
                ed.visit(shared_from_this());
            }
    };
}

#endif // MTEXT_H
