#pragma once

#include "lcvdrawitem.h"
#include "cad/primitive/text.h"

namespace LCViewer {
    class LcDrawOptions;
    class LcPainter;

    class LCVText : public LCVDrawItem {
        public:
            LCVText(const lc::entity::Text_CSPtr& text);
            virtual ~LCVText() = default;

            /**
             * @brief draw, Drawing the text
             * @param LcPainter painter, surface to be painted
             * @param LcDrawOptions options
             * @param geo::Area rect
             */
            void draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const override;

            lc::entity::CADEntity_CSPtr entity() const override;

        private:
            lc::entity::Text_CSPtr _text;
    };
}
