#pragma once

#include "cad/const.h"
#include "cad/interface/entityvisitor.h"
#include "cad/interface/entitydispatch.h"


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
             * @brief Text, default constructor
             * @param geo::Coordinate insertion_point, insertion_point of the text
             * @param geo::Coordinate second_point, second_point of text in UCS
             * @param double height, height of the text
             * @param string text_value, the text itself
             * @param double width_rel, X scale factor-width
             * @param double angle, angle of obliqueness
             * @param string style, name of text style
             * @param int textgeneration, Text drawing direction
             * @param int halign, Horizontal alignment
             * @param int valign, Vertical alignment
             * @param Layer_CSPtr layer, Layer of the entity
             */
            Text(const geo::Coordinate& insertion_point,
                 const geo::Coordinate& second_point,
                 const double height,
                 const std::string text_value,
                 const double width_rel,
                 const double angle,
                 const std::string style,
                 const int textgeneration,
                 const int halign, const int valign, const Layer_CSPtr layer);

            /**
             * @brief Text, Text constructor with metatypes
             * @param geo::Coordinate insertion_point, insertion_point of the text
             * @param geo::Coordinate second_point, second_point of text in UCS
             * @param double height, height of the text
             * @param string text_value, the text itself
             * @param double width_rel, X scale factor-width
             * @param double angle, angle of obliqueness
             * @param string style, name of text style
             * @param int textgeneration, Text drawing direction
             * @param int halign, Horizontal alignment
             * @param int valign, Vertical alignment
             * @param Layer_CSPtr layer, Layer of the entity
             * @param metatypes metatypes of the cad entity
             */
            Text(const geo::Coordinate& insertion_point,
                 const geo::Coordinate& second_point,
                 const double height,
                 const std::string text_value,
                 const double width_rel,
                 const double angle,
                 const std::string style,
                 const int textgeneration,
                 const int halign, const int valign, const Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo);

            Text(const Text_CSPtr other, bool sameID);

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

            /**
             * @brief boundingBox of the entity
             * @return geo::Area area
             */
            virtual const geo::Area boundingBox() const;

            virtual CADEntity_CSPtr modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const;

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


// TEXT_H
