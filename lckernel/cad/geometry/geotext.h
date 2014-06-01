#ifndef GEOTEXT_H
#define GEOTEXT_H

#include "cad/const.h"
#include "geocoordinate.h"
#include <string>

namespace lc {
    namespace geo {
        /**
         * @brief The Text class
         * Reference : http://www.autodesk.com/techpubs/autocad/acad2000/dxf/text_dxf_06.htm
         */
        class Text  {
            public:
                /**
                 * Vertical alignments.
                 */
                enum VAlign {
                    VABaseline = 0, /**< Bottom */
                    VABottom = 1,   /**< Bottom */
                    VAMiddle = 2,   /**< Middle */
                    VATop = 3       /**< Top. */
                };

                /**
                 * Horizontal alignments.
                 */
                enum HAlign {
                    HALeft = 0,     /**< Left */
                    HACenter = 1,   /**< Centered */
                    HARight = 2,    /**< Right */
                    HAAligned = 3,  /**< Aligned */
                    HAMiddle = 4,   /**< Middle */
                    HAFit = 5       /**< Fit */
                };

                /**
                 * Text drawing direction.
                 */
                enum TextGeneration {
                    None = 0,      /**< Normal text */
                    Backward = 1,  /**< Mirrored in X */
                    UpsideDown = 2 /**< Mirrored in Y */
                };
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
                 */
                Text(const Coordinate& insertion_point,
                     const Coordinate& second_point, const double height,
                     const std::string text_value, const double width_rel,
                     const double angle, const std::string style,
                     const TextGeneration textgeneration,
                     const HAlign halign, const VAlign valign);

                Text(const Coordinate& insertion_point,
                     const Coordinate& second_point, const double height,
                     const std::string text_value, const double width_rel,
                     const double angle, const std::string style,
                     const int textgenvalue,
                     const int halignvalue, const int valignvalue);

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
                 * @brief Second point
                 * Second alignment point (in OCS) (optional)
                 * DXF: X value; APP: 3D point
                 * This value is meaningful only if the value of a 72 or 73 group is nonzero
                 * (if the justification is anything other than baseline/left).
                 * DXF: Y and Z values of second alignment point (in OCS) (optional)
                 * @return Coordinate second point.
                 */
                const Coordinate second_point() const;

                /**
                 * @brief X factor relative width
                 * Relative X scale factor-width (optional; default = 1)
                 * This value is also adjusted when fit-type text is used.
                 * @return double X factor relative width
                 */
                double width_rel() const;

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
                 * @brief textgeneration
                 * Text generation flags (optional, default = 0):
                 * 2 = Text is backward (mirrored in X).
                 * 4 = Text is upside down (mirrored in Y).
                 * @return
                 */
                TextGeneration textgeneration() const;

                /**
                 * @brief halign
                 * Horizontal text justification type (optional, default = 0) integer codes (not bit-coded)
                 * 0 = Left; 1= Center; 2 = Right
                 * 3 = Aligned (if vertical alignment = 0)
                 * 4 = Middle (if vertical alignment = 0)
                 * 5 = Fit (if vertical alignment = 0)
                 * See the Group 72 and 73 integer codes table for clarification.
                 * @return
                 */
                HAlign halign() const;

                /**
                 * @brief valign
                 * Vertical text justification type (optional, default = 0): integer codes (not bit- coded):
                 * 0 = Baseline; 1 = Bottom; 2 = Middle; 3 = Top
                 * See the Group 72 and 73 integer codes table for clarification.
                 * @return
                 */
                VAlign valign() const;

                Coordinate nearestPointOnPath(const Coordinate& coord) const;
                bool isCoordinateOnPath(const Coordinate& coord) const;

            private:
                const Coordinate _insertion_point, _second_point;
                double _height, _width_rel, _angle;
                VAlign _valign;
                HAlign _halign;
                TextGeneration _textgeneration;
                std::string _style, _text_value;

        };
    }
}

#endif // GEOTEXT_H
