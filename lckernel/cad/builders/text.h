#pragma once

#include "cadentity.h"
#include "textbase.h"
#include <cad/primitive/textconst.h>

namespace lc {
namespace builder {
class TextBuilder : public TextBaseBuilder
{
public:
    TextBuilder();

    entity::Text_CSPtr build();

    TextBuilder* setDrawingDirection(lc::TextConst::DrawingDirection drawingDirectionIn);

    lc::TextConst::DrawingDirection drawingDirection() const;

    TextBuilder* setHorizontalAlign(lc::TextConst::HAlign halignin);

    lc::TextConst::HAlign horizontalAlign() const;

    TextBuilder* setVerticalAlign(lc::TextConst::VAlign valignin);

    lc::TextConst::VAlign verticalAlign() const;

    TextBuilder* setUnderlined(bool underline);

    bool underlined() const;

    TextBuilder* setStrikethrough(bool strikethrough);

    bool strikethrough() const;

    TextBuilder* setBold(bool bold);

    bool bold() const;

    TextBuilder* setItalic(bool italic);

    bool italic() const;

    void copy(lc::entity::Text_CSPtr entity);

private:
    lc::TextConst::DrawingDirection _drawingDirection;
    lc::TextConst::HAlign _hAlign;
    lc::TextConst::VAlign _vAlign;
    bool _underlined;
    bool _strikethrough;
    bool _bold;
    bool _italic;
};
}
}
