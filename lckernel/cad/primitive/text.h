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
#include "cad/math/lcmath.h"
namespace lc {
    class Text;
    typedef std::shared_ptr<Text> Text_SPtr;
    typedef std::shared_ptr<const Text> Text_CSPtr;


    class Text : public std::enable_shared_from_this<Text>, public CADEntity, public geo::Text {
        public:
            /**
             * @brief Text Entity
             * @param insertion_point insertion_point of the text
             * @param second_point second_point of text in UCS
             * @param height height of the text
             * @param text_value the text itself
             * @param width_rel X scale factor-width
             * @param angle angle of obliqueness
             * @param style name of text style
             * @param textgeneration Text drawing direction
             * @param halign Horizontal alignment
             * @param valign Vertical alignment
             * @param layer Layer of the entity
             */
            Text(const geo::Coordinate& insertion_point,
                 const geo::Coordinate& second_point,
                 const double height,
                 const std::string text_value,
                 const double width_rel,
                 const double angle,
                 const std::string style,
                 const TextGeneration textgeneration,
                 const HAlign halign, const VAlign valign, const Layer_CSPtr layer);

            /**
             * @brief Text Entity
             * @param insertion_point insertion_point of the text
             * @param second_point second_point of text in UCS
             * @param height height of the text
             * @param text_value the text itself
             * @param width_rel X scale factor-width
             * @param angle angle of obliqueness
             * @param style name of text style
             * @param textgeneration Text drawing direction
             * @param halign Horizontal alignment
             * @param valign Vertical alignment
             * @param layer Layer of the entity
             * @param metatypes metatypes of the cad entity
             */
            Text(const geo::Coordinate& insertion_point,
                 const geo::Coordinate& second_point,
                 const double height,
                 const std::string text_value,
                 const double width_rel,
                 const double angle,
                 const std::string style,
                 const TextGeneration textgeneration,
                 const HAlign halign, const VAlign valign, const Layer_CSPtr layer, const std::list<MetaType_CSPtr>& metaTypes);

        public:
            virtual CADEntity_CSPtr move(const geo::Coordinate& offset) const;
            virtual CADEntity_CSPtr copy(const geo::Coordinate& offset) const;
            virtual CADEntity_CSPtr rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const;
            virtual CADEntity_CSPtr scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const;
            virtual const geo::Area boundingBox() const;
        public:
            virtual void accept(const geo::Vector& o, EntityVisitor& ei) const {
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


#endif // TEXT_H
