#include "geotext.h"

using namespace lc;
using namespace geo;

Text::Text(const Coordinate& insertion_point,
    const Coordinate& second_point, const double height,
    const QString text_value, const double width_rel,
    const double angle, const QString style,
    const TextGeneration textgeneration,
    const HAlign halign, const VAlign valign) :
    _insertion_point(insertion_point), /*! First alignment point code 10, 20, 30 */
    _second_point(second_point) { /*! Second alignment point in UCS ( optional ) code 11, 21, 31 */

    _height = height; /*! Text Height */
    _text_value = text_value; /*! Text Value */
    _width_rel = width_rel; /*! X scale width code 41 */
    _angle = angle; /*! Angle ( Optional ) Code 51 */
    _style = style; /*! Text style name code 7 */
    _textgeneration = textgeneration; /*! Text Generation flag code 71  */
    _halign = halign; /*! Horizontal alignment code 72 */
    _valign = valign; /*! Vertical alignment code 73  */

}

const Coordinate& Text::insertion_point() const {
    return _insertion_point;
}

const Coordinate& Text::second_point() const {
    return _second_point;
}

double Text::height() const {
    return _height;
}

double Text::width_rel() const {
    return _width_rel;
}

double Text::angle() const {
    return _angle;
}

QString Text::text_value() const {
    return _text_value;
}

QString Text::style() const {
    return _style;
}

Text::TextGeneration Text::textgeneration() const {
    return _textgeneration;
}

Text::HAlign Text::halign() const {
    return _halign;
}

Text::VAlign Text::valign() const {
    return _valign;
}

Coordinate Text::nearestPointOnPath(const Coordinate& coord) const {
    //return;
}


bool Text::isCoordinateOnPath(const Coordinate& coord) const {
    return (nearestPointOnPath(coord) - coord).magnitude() < 1.0e-4;
}
