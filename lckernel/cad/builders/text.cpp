#include "text.h"
#include <cad/primitive/text.h>

using namespace lc::builder;

TextBuilder::TextBuilder() {

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

lc::entity::Text_CSPtr TextBuilder::build() {
    checkValues(true);
    return entity::Text_CSPtr(new entity::Text(*this));
}
