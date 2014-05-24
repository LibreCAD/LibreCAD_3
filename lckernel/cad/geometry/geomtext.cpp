#include "cad/geometry/geomtext.h"

using namespace lc;
using namespace geo;

//MText::Mtext(){

//}

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

MText::MTextDrawingDirection MText::drawingDirection() const {
    return _drawingdirection;
}

MText::MTextLineSpacingStyle MText::lineSpacingStyle() const {
    return _linespacingstyle;
}

double MText::lineSpacingFactor() const {
    return _linespacingfactor;
}
