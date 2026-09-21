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

            /// DXF group 41, the reference rectangle text wraps inside.
            /// 0 -- the default -- means there is none.
            MTextBuilder* setWidth(double width);

            double width() const;

            /// DXF group 72.  Named apart from TextBaseBuilder's
            /// drawingDirection(), which is TEXT's group-71 mirroring flag
            /// and means something else entirely.
            MTextBuilder* setMTextDrawingDirection(lc::TextConst::MTextDrawingDirection direction);

            lc::TextConst::MTextDrawingDirection mtextDrawingDirection() const;

            /// DXF group 44, a multiple of the single-line spacing.
            MTextBuilder* setLineSpacingFactor(double factor);

            double lineSpacingFactor() const;

            /// DXF group 73.
            MTextBuilder* setLineSpacingStyle(lc::TextConst::LineSpacingStyle style);

            lc::TextConst::LineSpacingStyle lineSpacingStyle() const;

            void copy(lc::entity::MText_CSPtr entity);

        private:
            bool _underlined;
            bool _strikethrough;
            bool _bold;
            bool _italic;
            double _width;
            lc::TextConst::MTextDrawingDirection _mtextDrawingDirection;
            double _lineSpacingFactor;
            lc::TextConst::LineSpacingStyle _lineSpacingStyle;
        };
    }
}
