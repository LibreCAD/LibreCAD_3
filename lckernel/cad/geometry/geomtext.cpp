#include "cad/geometry/geomtext.h"

using namespace lc;
using namespace geo;

MText::MText(const Coordinate& insertion_point,
             const double height,
             const double width,
             const VAlign valign,
             const HAlign halign,
             const DrawingDirection drawingDirection,
             const LineSpacingStyle lineSpacingStyle,
             const double lineSpacingFactor,
             const std::string& text_value,
             const std::string& style,
             const double angle) : _insertion_point(insertion_point) {
    _height = height;
    _width = width;
    _valign = valign;
    _halign = halign;
    _drawingdirection = drawingDirection;
    _linespacingstyle = lineSpacingStyle;
    _linespacingfactor = lineSpacingFactor;
    _text_value = text_value;
    _style = style;
    _angle = angle;
}

const Coordinate MText::insertion_point() const {
    return _insertion_point;
}

double MText::height() const {
    return _height;
}

double MText::width() const {
    return _width;
}

double MText::angle() const {
    return _angle;
}

std::string MText::text_value() const {
    return _text_value;
}

std::string MText::style() const {
    return _style;
}

MText::HAlign MText::halign() const {
    return _halign;
}

MText::VAlign MText::valign() const {
    return _valign;
}

MText::DrawingDirection MText::drawingDirection() const {
    return _drawingdirection;
}

MText::LineSpacingStyle MText::lineSpacingStyle() const {
    return _linespacingstyle;
}

double MText::lineSpacingFactor() const {
    return _linespacingfactor;
}
