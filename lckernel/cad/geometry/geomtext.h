#ifndef GEOMTEXT_H
#define GEOMTEXT_H

#include "cad/const.h"
#include "geocoordinate.h"
#include <string>

namespace lc {
    namespace geo {
        /**
         * @brief The Text class
         * Reference : http://www.autodesk.com/techpubs/autocad/acad2000/dxf/text_dxf_06.htm
         */
        class MText  {
            public:
                /**
                 * Vertical alignments.
                 */
                enum VAlign {
                    VATop,      /**< Top. */
                    VAMiddle,   /**< Middle */
                    VABottom    /**< Bottom */
                };

                /**
                 * Horizontal alignments.
                 */
                enum HAlign {
                    HALeft,     /**< Left */
                    HACenter,   /**< Centered */
                    HARight     /**< Right */
                };

                /**
                 * MText drawing direction.
                 */
                enum DrawingDirection {
                    LeftToRight,     /**< Left to right */
                    TopToBottom,     /**< Top to bottom */
                    ByStyle          /**< Inherited from associated text style */
                };

                /**
                 * Line spacing style for MTexts.
                 */
                enum LineSpacingStyle {
                    AtLeast,        /**< Taller characters will override */
                    Exact           /**< Taller characters will not override */
                };


                /**
                 * @brief MText
                 * @param insertionPoint Insertion point text
                 * @param height Height of text
                 * @param width Width of text
                 * @param valign Vertical alignment
                 * @param halign Horizontal alignment
                 * @param drawingDirection Drawing direction TopToBottom/LeftToRight
                 * @param lineSpacingStyle Line spacing style
                 * @param lineSpacingFactor Line spacing factor
                 * @param text Text value itself
                 * @param style style name of mtext
                 * @param angle angle of obliqueness
                 */
                MText(const Coordinate& insertion_point,
                      const double height,
                      const double width,
                      const VAlign valign,
                      const HAlign halign,
                      const DrawingDirection drawingDirection,
                      const LineSpacingStyle lineSpacingStyle,
                      const double lineSpacingFactor,
                      const std::string& text_value,
                      const std::string& style,
                      const double angle);

                /**
                 * @brief Insertion point of text
                 * First alignment point (in OCS)
                 * DXF: X value; APP: 3D point
                 * DXF: Y and Z values of first alignment point (in OCS)
                 * @return Coordinate insertion_point
                 */
                const Coordinate insertion_point() const;

                /**
                 * @brief Height of text
                 * Text height. code 40
                 * @return double height
                 */
                double height() const;

                /**
                 * @brief text value of the text
                 * Text value itself. code 1
                 * @return string text value.
                 */
                std::string text_value() const;

                /**
                 * @brief X factor relative width
                 * Relative X scale factor-width (optional; default = 1)
                 * This value is also adjusted when fit-type text is used.
                 * @return double X factor relative width
                 */
                double width() const;

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
                HAlign halign() const;

                /**
                 * @brief Vertical Alignment
                 * @return VAlign alignment
                 */
                VAlign valign() const;

                /**
                 * @brief drawingDirection
                 * @return MTextDrawingDirection
                 */
                DrawingDirection drawingDirection() const;

                /**
                 * @brief lineSpacingStyle
                 * @return MTextLineSpacingStyle
                 */
                LineSpacingStyle lineSpacingStyle() const;

                /**
                 * @brief lineSpacingFactor
                 * @return double
                 */
                double lineSpacingFactor() const;

                Coordinate nearestPointOnPath(const Coordinate& coord) const;
                bool isCoordinateOnPath(const Coordinate& coord) const;

            private:
                const Coordinate _insertion_point;
                double _height, _width, _angle, _linespacingfactor;
                DrawingDirection _drawingdirection;
                LineSpacingStyle _linespacingstyle;
                VAlign _valign;
                HAlign _halign;
                std::string _style, _text_value;

        };
    }
}

#endif // GEOMTEXT_H
