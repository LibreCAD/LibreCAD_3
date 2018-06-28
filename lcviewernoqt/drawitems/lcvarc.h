#pragma once

#include "lcvdrawitem.h"
#include <cad/primitive/arc.h>

namespace LCViewer {
    class LCVArc: public LCVDrawItem {
        public:
            LCVArc(const lc::entity::Arc_CSPtr& arc);
            /**
             * @brief draw, Draw the Arc
             * @param LcPainter painter, surface to be painted
             * @param LcDrawOptions options
             * @param geo::Area rect
             */
            virtual void draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const override;

            lc::entity::CADEntity_CSPtr entity() const override;

        private:
            lc::entity::Arc_CSPtr _arc;
    };
}
