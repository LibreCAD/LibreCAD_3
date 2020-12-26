#include "text.h"
#include <cad/primitive/text.h>

using namespace lc::builder;

TextBuilder::TextBuilder()
    :
    TextBaseBuilder()
{}

lc::entity::Text_CSPtr TextBuilder::build() {
    checkValues(true);
    return entity::Text_CSPtr(new entity::Text(*this));
}

void TextBuilder::copy(lc::entity::Text_CSPtr entity) {
    CADEntityBuilder::copy(entity);

    setInsertionPoint(entity->insertion_point());
    setTextValue(entity->text_value());
    setTextFont(entity->style());
    setHeight(entity->height());
    setAngle(entity->angle());
    setDrawingDirection(entity->textgeneration());
    setHorizontalAlign(entity->halign());
    setVerticalAlign(entity->valign());
}
