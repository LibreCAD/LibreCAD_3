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
    bool _underlined;
    bool _strikethrough;
    bool _bold;
    bool _italic;
};
}
}
