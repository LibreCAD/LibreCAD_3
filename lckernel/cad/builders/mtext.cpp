#include "mtext.h"
#include <cad/primitive/mtext.h>

using namespace lc::builder;

MTextBuilder::MTextBuilder()
    :
    _underlined(false),
    _strikethrough(false),
    _bold(false),
    _italic(false)
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
}
