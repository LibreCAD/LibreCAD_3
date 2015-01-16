#pragma once

#include "cad/geometry/geocoordinate.h"
#include "cad/primitive/textconst.h"

namespace lc {
    class Dimension;
    typedef std::shared_ptr<Dimension> Dimension_SPtr;
    typedef std::shared_ptr<const Dimension> Dimension_CSPtr;


    class Dimension {

            enum AttachmentPoint {
                Top_left,
                Top_center,
                Top_right,
                Middle_left,
                Middle_center,
                Middle_right,
                Bottom_left,
                Bottom_center,
                Bottom_right
            };

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
                      const AttachmentPoint attachPt,
                      const geo::TextConst::LineSpacingStyle lineSpacingStyle,
                      const double lineSpacingFactor,
                      const std::string& text_value,
                      const std::string& style,
                      const double angle);

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
                      const AttachmentPoint attachPt,
                      const geo::TextConst::LineSpacingStyle lineSpacingStyle,
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

            virtual CADEntity_CSPtr modify(Layer_CSPtr layer, const MetaInfo_CSPtr metaInfo) const;
        public:

        private:
            const Coordinate _definition_point, _middle_of_text;
            double _angle, _linespacingfactor;
            TextConst::LineSpacingStyle _linespacingstyle;
            TextConst::VAlign _valign;
            TextConst::HAlign _halign;

    };
}

// Dimension_H

