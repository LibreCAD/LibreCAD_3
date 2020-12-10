#include "text.h"
#include <cad/primitive/text.h>

using namespace lc::builder;

TextBuilder::TextBuilder()
    :
    _drawingDirection(lc::TextConst::DrawingDirection::None),
    _hAlign(lc::TextConst::HAlign::HACenter),
    _vAlign(lc::TextConst::VAlign::VAMiddle),
    _underlined(false),
    _strikethrough(false),
    _bold(false),
    _italic(false)
{
}

TextBuilder* TextBuilder::setDrawingDirection(lc::TextConst::DrawingDirection drawingDirectionIn) {
    _drawingDirection = drawingDirectionIn;
    return this;
}

lc::TextConst::DrawingDirection TextBuilder::drawingDirection() const {
    return _drawingDirection;
}

TextBuilder* TextBuilder::setHorizontalAlign(lc::TextConst::HAlign halignin) {
    _hAlign = halignin;
    return this;
}

lc::TextConst::HAlign TextBuilder::horizontalAlign() const {
    return _hAlign;
}

TextBuilder* TextBuilder::setVerticalAlign(lc::TextConst::VAlign valignin) {
    _vAlign = valignin;
    return this;
}

lc::TextConst::VAlign TextBuilder::verticalAlign() const {
    return _vAlign;
}

TextBuilder* TextBuilder::setUnderlined(bool underline) {
    _underlined = underline;
    return this;
}

bool TextBuilder::underlined() const {
    return _underlined;
}

TextBuilder* TextBuilder::setStrikethrough(bool strikethrough) {
    _strikethrough = strikethrough;
    return this;
}

bool TextBuilder::strikethrough() const {
    return _strikethrough;
}

TextBuilder* TextBuilder::setBold(bool bold) {
    _bold = bold;
    return this;
}

bool TextBuilder::bold() const {
    return _bold;
}

TextBuilder* TextBuilder::setItalic(bool italic) {
    _italic = italic;
    return this;
}

bool TextBuilder::italic() const {
    return _italic;
}

lc::entity::Text_CSPtr TextBuilder::build() {
    checkValues(true);
    return entity::Text_CSPtr(new entity::Text(*this));
}

void TextBuilder::copy(lc::entity::Text_CSPtr entity) {
    TextBaseBuilder::copy(entity);

    setDrawingDirection(entity->textgeneration());
    setHorizontalAlign(entity->halign());
    setVerticalAlign(entity->valign());
    setUnderlined(entity->underlined());
    setStrikethrough(entity->strikethrough());
    setBold(entity->bold());
    setItalic(entity->italic());
}
