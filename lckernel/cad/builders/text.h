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

    void copy(lc::entity::Text_CSPtr entity);

};
}
}
