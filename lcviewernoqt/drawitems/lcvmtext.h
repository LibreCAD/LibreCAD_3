#pragma once

#include "lcvdrawitem.h"
#include "cad/primitive/mtext.h"

namespace lc {
    namespace viewer {
        class LcDrawOptions;

        class LcPainter;

        class LCVMText : public LCVDrawItem {
        public:
            LCVMText(const lc::entity::MText_CSPtr& text);

            virtual ~LCVMText() = default;

            /**
             * @brief draw, Drawing the text
             * @param LcPainter painter, surface to be painted
             * @param LcDrawOptions options
             * @param geo::Area rect
             */
            void draw(LcPainter& painter, const LcDrawOptions& options, const lc::geo::Area& rect) const override;

            lc::entity::CADEntity_CSPtr entity() const override;

        private:
            lc::entity::MText_CSPtr _mtext;
        };
    }
}
