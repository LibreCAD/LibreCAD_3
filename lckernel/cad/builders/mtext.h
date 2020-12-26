#pragma once

#include "cadentity.h"
#include "textbase.h"
#include <cad/primitive/textconst.h>

namespace lc {
    namespace builder {
        class MTextBuilder : public TextBaseBuilder
        {
        public:
            MTextBuilder();

            entity::MText_CSPtr build();

            MTextBuilder* setUnderlined(bool underline);

            bool underlined() const;

            MTextBuilder* setStrikethrough(bool strikethrough);

            bool strikethrough() const;

            MTextBuilder* setBold(bool bold);

            bool bold() const;

            MTextBuilder* setItalic(bool italic);

            bool italic() const;

            void copy(lc::entity::MText_CSPtr entity);

        private:
            bool _underlined;
            bool _strikethrough;
            bool _bold;
            bool _italic;
        };
    }
}
