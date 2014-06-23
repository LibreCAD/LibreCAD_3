#include "cad/geometry/geodimension.h"

using namespace lc;
using namespace geo;

Dimension::Dimension(const Coordinate& definition_point,
                     const Coordinate& middle_of_text,
                     const MText::VAlign valign,
                     const MText::HAlign halign,
                     const MText::LineSpacingStyle lineSpacingStyle,
                     const double lineSpacingFactor,
                     const std::string& text_value,
                     const std::string& style,
                     const double angle) : _definition_point(definition_point),
    _middle_of_text(middle_of_text) {
    _valign = valign;
    _halign = halign;
    _linespacingstyle = lineSpacingStyle;
    _linespacingfactor = lineSpacingFactor;
    _text_value = text_value;
    _style = style;
    _angle = angle;
}

Dimension::Dimension() : _definition_point(0., 0.), _middle_of_text(0., 0.)
    , _angle(0.)
    , _linespacingstyle(MText::Exact), _linespacingfactor(1.)
    , _valign(MText::VATop), _halign(MText::HALeft)
    , _style("")
    , _text_value("") {

}

const Coordinate Dimension::definition_point() const {
    return _definition_point;
}

const Coordinate Dimension::middle_of_text()  const {
    return _middle_of_text;
}

double Dimension::angle() const {
    return _angle;
}

std::string Dimension::text_value() const {
    return _text_value;
}

std::string Dimension::style() const {
    return _style;
}

MText::HAlign Dimension::halign() const {
    return _halign;
}

MText::VAlign Dimension::valign() const {
    return _valign;
}

MText::LineSpacingStyle Dimension::lineSpacingStyle() const {
    return _linespacingstyle;
}

double Dimension::lineSpacingFactor() const {
    return _linespacingfactor;
}


Coordinate Dimension::nearestPointOnPath(const Coordinate& coord) const {

}
bool Dimension::isCoordinateOnPath(const Coordinate& coord) const {

}
