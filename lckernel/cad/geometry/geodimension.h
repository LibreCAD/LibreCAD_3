#pragma once

#include "cad/const.h"
#include "geocoordinate.h"
#include <string>
#include "cad/geometry/geomtext.h"


namespace lc {
    namespace geo {

        class Dimension  {
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
                 */
                Dimension(const Coordinate& definition_point,
                          const Coordinate& middle_of_text,
                          const MText::VAlign valign,
                          const MText::HAlign halign,
                          const MText::LineSpacingStyle lineSpacingStyle,
                          const double lineSpacingFactor,
                          const std::string& text_value,
                          const std::string& style,
                          const double angle);
                Dimension();
                //virtual ~Dimension();
                /**
                 * @brief Insertion point of text
                 * First alignment point (in OCS)
                 * DXF: X value; APP: 3D point
                 * DXF: Y and Z values of first alignment point (in OCS)
                 * @return Coordinate insertion_point
                 */
                const Coordinate definition_point() const;

                /**
                 * @brief middle_of_text
                 * @return Coordinate middle of text
                 */
                const Coordinate middle_of_text() const;

                /**
                 * @brief text value of the text
                 * Text value itself. code 1
                 * @return string text value.
                 */
                std::string text_value() const;

                /**
                 * @brief angle
                 * Oblique angle (optional; default = 0)
                 * @return double oblique angle
                 */
                double angle() const;

                /**
                 * @brief style
                 * Text style name (optional, default = STANDARD)
                 * @return string style name
                 */
                std::string style() const;

                /**
                 * @brief Horizontal Alignment
                 * @return HAlign alignment
                 */
                MText::HAlign halign() const;

                /**
                 * @brief Vertical Alignment
                 * @return VAlign alignment
                 */
                MText::VAlign valign() const;

                /**
                 * @brief lineSpacingStyle
                 * @return DimensionLineSpacingStyle
                 */
                MText::LineSpacingStyle lineSpacingStyle() const;

                /**
                 * @brief lineSpacingFactor
                 * @return double
                 */
                double lineSpacingFactor() const;

                Coordinate nearestPointOnPath(const Coordinate& coord) const;
                bool isCoordinateOnPath(const Coordinate& coord) const;

                friend class DimAligned;
            private:
                const Coordinate _definition_point, _middle_of_text;
                double _angle, _linespacingfactor;
                MText::LineSpacingStyle _linespacingstyle;
                MText::VAlign _valign;
                MText::HAlign _halign;
                std::string _style, _text_value;

        };
    }
}

// GEODimension_H
