#pragma once

#include <cad/builders/dimension.h>
#include "cad/geometry/geocoordinate.h"
#include "cad/primitive/textconst.h"
#include "cad/base/cadentity.h"

namespace lc {
    namespace entity {
        /**
        * Base class for all dimensions
        * It takes in the values pretty much as given by the DXF reference
        */
        class Dimension {

        public:

            /**
             * @brief Dimension
             * @param definitionPoint definition_point of the dimension
             * @param middleOfText Middle of text
             * @param attachmentPoint Where the text is located in reference to the text location
             * @param angle Angle of obliqueness
             * @param lineSpacingFactor Spacing factor
             * @param lineSpacingStyle name
             * @param explicitValue given value
             */

            Dimension(geo::Coordinate definitionPoint,
                      geo::Coordinate middleOfText,
                      TextConst::AttachmentPoint attachmentPoint,
                      double textAngle,
                      double lineSpacingFactor,
                      TextConst::LineSpacingStyle lineSpacingStyle,
                      std::string explicitValue);

            /**
             * @brief Dimension
             * @param definitionPoint definition_point of the dimension
             * @param middleOfText Middle of text
             * @param attachmentPoint Where the text is located in reference to the text location
             * @param angle Angle of obliqueness
             */
            Dimension(geo::Coordinate definitionPoint,
                      geo::Coordinate middleOfText,
                      TextConst::AttachmentPoint attachmentPoint,
                      double textAngle);


            Dimension(const Dimension& other);

        protected:
            Dimension(const builder::DimensionBuilder& builder);

        public:
            //TODO: re-enable those methods
            /**
             * @brief move, moves by an offset
             * @param geo::Coordinate offset
             * @return CADEntity_CSPtr moved entity
             */
            //            CADEntity_CSPtr move(const geo::Coordinate& offset) const;

            /**
             * @brief copy, copies by an offset
             * @param geo::Coordinate offset
             * @return CADEntity_CSPtr copied entity
             */
            //            CADEntity_CSPtr copy(const geo::Coordinate& offset) const;

            /**
             * @brief rotate, rotate operation
             * @param geo::Coordinate rotation_center
             * @param double rotation_angle
             * @return CADEntity_CSPtr rotated entity
             */
            //            CADEntity_CSPtr rotate(const geo::Coordinate& rotation_center, const double rotation_angle) const;

            /**
             * @brief scale, scales the entity
             * @param geo::Coordinate scale_center
             * @param double scale_factor
             * @return
             */
            //            CADEntity_CSPtr scale(const geo::Coordinate& scale_center, const geo::Coordinate& scale_factor) const;


            /**
            * Definition point of this dimension
            */
            const geo::Coordinate &definitionPoint() const noexcept;

            /**
            * Middle location of the text
            */
            const geo::Coordinate &middleOfText() const noexcept;

            TextConst::AttachmentPoint const &attachmentPoint() const;

            /**
            * Angle of witch to draw the text on
            */
            double textAngle() const;

            /**
            * Line spacing factor
            */
            double lineSpacingFactor() const;

            /**
            * Line spacing style
            */
            TextConst::LineSpacingStyle lineSpacingStyle() const;

            /**
            * Returns the explicit value of this dimension
            * The explicit value is the value given by the user instead of the measured one
            * default is the measurement. If "" or "<>" if a blank space it's suppressed
            */
            std::string explicitValue() const;

            void getDimensionProperties(PropertiesMap& propertyValues) const;

            void setDimensionProperties(const PropertiesMap& propertiesMap, lc::geo::Coordinate& definitionPointp, lc::geo::Coordinate& middleOfTextp,
                double& textAnglep, double& lineSpacingFactorp, std::string& explicitValuep) const;

        private:
            const geo::Coordinate _definitionPoint;
            const geo::Coordinate _middleOfText;
            const TextConst::AttachmentPoint _attachmentPoint;
            const double _textAngle;
            const double _lineSpacingFactor;
            const TextConst::LineSpacingStyle _lineSpacingStyle;
            const std::string _explicitValue;
        };

        DECLARE_SHORT_SHARED_PTR(Dimension)
    }
}
