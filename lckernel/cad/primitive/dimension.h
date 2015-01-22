#pragma once

#include "cad/geometry/geocoordinate.h"
#include "cad/primitive/textconst.h"
#include "cad/base/cadentity.h"

namespace lc {
    class Dimension;
    typedef std::shared_ptr<Dimension> Dimension_SPtr;
    typedef std::shared_ptr<const Dimension> Dimension_CSPtr;

    /**
    * Base class for all dimensions
    * It takes in the values pretty much as given by the DXF reference
    * TODO: Should we move some variables to the implementation of a actualy Dimension like DimRadial and have all optional variables as part of the meta map ?
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

            Dimension(geo::Coordinate const& definitionPoint, geo::Coordinate const& middleOfText, TextConst::AttachmentPoint const& attachmentPoint, double const angle,
                      double  lineSpacingFactor, TextConst::LineSpacingStyle const& lineSpacingStyle, std::string const& explicitValue);

            /**
             * @brief Dimension
             * @param definitionPoint definition_point of the dimension
             * @param middleOfText Middle of text
             * @param attachmentPoint Where the text is located in reference to the text location
             * @param angle Angle of obliqueness
             */
            Dimension(geo::Coordinate const&   _definitionPoint,
                      geo::Coordinate const& _middleOfText,
                      TextConst::AttachmentPoint _attachPt,
                      double _angle);


            Dimension(Dimension const& other);
        public:
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
            geo::Coordinate definitionPoint() const;

            /**
            * Middle location of the text
            */
            geo::Coordinate middleOfText() const;


            TextConst::AttachmentPoint const& attachmentPoint() const;

            /**
            * Angle of witch to draw the text on
            */
            double angle() const;

            /**
            * Line spacing factor
            */
            double lineSpacingFactor() const;

            /**
            * Line spacing style
            */
            TextConst::LineSpacingStyle lineSpacingStyle() const;

            /**
            * Return's the explicit value of this dimension
            * The explicit value is the value has given by the ueaser instead of the measured one
            * default is the measurement. If "" or "<>" if a blank space it's surpressed
            */
            std::string explicitValue() const;

        private:
            const geo::Coordinate _definitionPoint;
            const geo::Coordinate _middleOfText;
            const TextConst::AttachmentPoint _attachmentPoint;
            const double _angle;
            const double _lineSpacingFactor;
            const TextConst::LineSpacingStyle _lineSpacingStyle;
            const std::string _explicitValue;
    };
}

// Dimension_H

