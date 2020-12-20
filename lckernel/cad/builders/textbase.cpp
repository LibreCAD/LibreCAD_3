#include "textbase.h"

using namespace lc::builder;

TextBaseBuilder::TextBaseBuilder()
    :
    _drawingDirection(lc::TextConst::DrawingDirection::None),
    _hAlign(lc::TextConst::HAlign::HACenter),
    _vAlign(lc::TextConst::VAlign::VAMiddle),
    _height(100),
    _angle(0)
{
}

const lc::geo::Coordinate& TextBaseBuilder::insertionPoint() const {
    return _insertionPoint;
}

TextBaseBuilder* TextBaseBuilder::setInsertionPoint(const lc::geo::Coordinate& coord) {
    _insertionPoint = coord;
    return this;
}

const std::string& TextBaseBuilder::textValue() const {
    return _textValue;
}

TextBaseBuilder* TextBaseBuilder::setTextValue(const std::string& text) {
    _textValue = text;
    return this;
}

double TextBaseBuilder::height() const {
    return _height;
}

TextBaseBuilder* TextBaseBuilder::setHeight(double heightIn) {
    _height = heightIn;
    return this;
}

double TextBaseBuilder::angle() const {
    return _angle;
}

TextBaseBuilder* TextBaseBuilder::setAngle(double angleIn) {
    _angle = angleIn;
    return this;
}

TextBaseBuilder* TextBaseBuilder::setTextFont(std::string font) {
    _textStyle = font;
    return this;
}

std::string TextBaseBuilder::textStyle() const {
    return _textStyle;
}

TextBaseBuilder* TextBaseBuilder::setDrawingDirection(lc::TextConst::DrawingDirection drawingDirectionIn) {
    _drawingDirection = drawingDirectionIn;
    return this;
}

lc::TextConst::DrawingDirection TextBaseBuilder::drawingDirection() const {
    return _drawingDirection;
}

TextBaseBuilder* TextBaseBuilder::setHorizontalAlign(lc::TextConst::HAlign halignin) {
    _hAlign = halignin;
    return this;
}

lc::TextConst::HAlign TextBaseBuilder::horizontalAlign() const {
    return _hAlign;
}

TextBaseBuilder* TextBaseBuilder::setVerticalAlign(lc::TextConst::VAlign valignin) {
    _vAlign = valignin;
    return this;
}

lc::TextConst::VAlign TextBaseBuilder::verticalAlign() const {
    return _vAlign;
}
