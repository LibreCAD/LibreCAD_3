#include "geotext.h"

using namespace lc;
using namespace geo;

Text::Text(const Coordinate& insertion_point,
           const Coordinate& second_point, const double height,
           const std::string text_value, const double width_rel,
           const double angle, const std::string style,
           const TextGeneration textgeneration,
           const HAlign halign, const VAlign valign) :
    _insertion_point(insertion_point), _second_point(second_point) {

    _height = height;
    _text_value = text_value;
    _width_rel = width_rel;
    _angle = angle;
    _style = style;
    _textgeneration = textgeneration;
    _halign = halign;
    _valign = valign;

}

Text::Text(const Coordinate& insertion_point,
     const Coordinate& second_point, const double height,
     const std::string text_value, const double width_rel,
     const double angle, const std::string style,
     const int textgenvalue,
     const int halignvalue, const int valignvalue) :
    _insertion_point(insertion_point), _second_point(second_point) {

    _height = height;
    _text_value = text_value;
    _width_rel = width_rel;
    _angle = angle;
    _style = style;
    _textgeneration = static_cast <Text::TextGeneration>(textgenvalue);
    _halign = static_cast <Text::HAlign>(halignvalue);
    _valign = static_cast <Text::VAlign>(valignvalue);
}
const Coordinate Text::insertion_point() const {
    return _insertion_point;
}

const Coordinate Text::second_point() const {
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

std::string Text::text_value() const {
    return _text_value;
}

std::string Text::style() const {
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
