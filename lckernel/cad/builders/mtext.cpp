#include "mtext.h"
#include <cad/primitive/mtext.h>

using namespace lc::builder;

MTextBuilder::MTextBuilder()
    :
    _underlined(false),
    _strikethrough(false),
    _bold(false),
    _italic(false),
    // 0 is the DXF "no reference rectangle, do not wrap"; 1 would be a wrap
    // column one drawing unit wide, which is what libdxfrw's DRW_Text default
    // used to make every MTEXT LibreCAD_3 wrote claim.
    _width(0.0),
    _mtextDrawingDirection(lc::TextConst::MTextDrawingDirection::ByStyle),
    _lineSpacingFactor(1.0),
    _lineSpacingStyle(lc::TextConst::LineSpacingStyle::AtLeast)
{
}

MTextBuilder* MTextBuilder::setUnderlined(bool underline) {
    _underlined = underline;
    return this;
}

bool MTextBuilder::underlined() const {
    return _underlined;
}

MTextBuilder* MTextBuilder::setStrikethrough(bool strikethrough) {
    _strikethrough = strikethrough;
    return this;
}

bool MTextBuilder::strikethrough() const {
    return _strikethrough;
}

MTextBuilder* MTextBuilder::setBold(bool bold) {
    _bold = bold;
    return this;
}

bool MTextBuilder::bold() const {
    return _bold;
}

MTextBuilder* MTextBuilder::setItalic(bool italic) {
    _italic = italic;
    return this;
}

bool MTextBuilder::italic() const {
    return _italic;
}

MTextBuilder* MTextBuilder::setWidth(double width) {
    _width = width;
    return this;
}

double MTextBuilder::width() const {
    return _width;
}

MTextBuilder* MTextBuilder::setMTextDrawingDirection(lc::TextConst::MTextDrawingDirection direction) {
    _mtextDrawingDirection = direction;
    return this;
}

lc::TextConst::MTextDrawingDirection MTextBuilder::mtextDrawingDirection() const {
    return _mtextDrawingDirection;
}

MTextBuilder* MTextBuilder::setLineSpacingFactor(double factor) {
    _lineSpacingFactor = factor;
    return this;
}

double MTextBuilder::lineSpacingFactor() const {
    return _lineSpacingFactor;
}

MTextBuilder* MTextBuilder::setLineSpacingStyle(lc::TextConst::LineSpacingStyle style) {
    _lineSpacingStyle = style;
    return this;
}

lc::TextConst::LineSpacingStyle MTextBuilder::lineSpacingStyle() const {
    return _lineSpacingStyle;
}

lc::entity::MText_CSPtr MTextBuilder::build() {
    checkValues(true);
    return entity::MText_CSPtr(new entity::MText(*this));
}

void MTextBuilder::copy(lc::entity::MText_CSPtr entity) {
    CADEntityBuilder::copy(entity);

    setInsertionPoint(entity->insertion_point());
    setTextValue(entity->text_value());
    setTextFont(entity->style());
    setHeight(entity->height());
    setAngle(entity->angle());
    setDrawingDirection(entity->textgeneration());
    setHorizontalAlign(entity->halign());
    setVerticalAlign(entity->valign());
    setUnderlined(entity->underlined());
    setStrikethrough(entity->strikethrough());
    setBold(entity->bold());
    setItalic(entity->italic());
    setWidth(entity->width());
    setMTextDrawingDirection(entity->drawingDirection());
    setLineSpacingFactor(entity->lineSpacingFactor());
    setLineSpacingStyle(entity->lineSpacingStyle());
}
