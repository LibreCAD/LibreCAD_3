#include "textbase.h"

using namespace lc::builder;

TextBaseBuilder::TextBaseBuilder()
    :
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

/*void TextBaseBuilder::copy(lc::entity::TextBase_CSPtr entity) {
    CADEntityBuilder::copy(entity);

    setInsertionPoint(entity->insertion_point());
    setTextValue(entity->text_value());
    setTextFont(entity->style());
    setHeight(entity->height());
    setRotation(entity->rotation());
}*/
