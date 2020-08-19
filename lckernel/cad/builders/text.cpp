#include "text.h"
#include <cad/primitive/text.h>

using namespace lc::builder;

TextBuilder::TextBuilder() 
    :
    _height(100),
    _angle(0)
{

}

const lc::geo::Coordinate& TextBuilder::insertionPoint() const {
    return _insertionPoint;
}

TextBuilder* TextBuilder::setInsertionPoint(const lc::geo::Coordinate& coord) {
    _insertionPoint = coord;
    return this;
}

const std::string& TextBuilder::textValue() const {
    return _textValue;
}

TextBuilder* TextBuilder::setTextValue(const std::string& text) {
    _textValue = text;
    return this;
}

double TextBuilder::height() const {
    return _height;
}

TextBuilder* TextBuilder::setHeight(double heightIn) {
    _height = heightIn;
    return this;
}

double TextBuilder::angle() const {
    return _angle;
}

TextBuilder* TextBuilder::setAngle(double angleIn) {
    _angle = angleIn;
    return this;
}

lc::entity::Text_CSPtr TextBuilder::build() {
    checkValues(true);
    return entity::Text_CSPtr(new entity::Text(*this));
}
