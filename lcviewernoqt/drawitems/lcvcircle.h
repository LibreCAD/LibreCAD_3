#pragma once

#include "lcvdrawitem.h"
#include "cad/primitive/circle.h"

namespace LCViewer {
    class LcDrawOptions;
    class LcPainter;

    class LCVCircle  : public LCVDrawItem {
        public:
            LCVCircle(const lc::entity::Circle_CSPtr circle);

            /**
             * @brief draw, Draws the circle
             * @param LcPainter painter, surface to be painted
             * @param LcDrawOptions options
             * @param geo::Area rect
             */
            virtual void draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const override;

            lc::entity::CADEntity_CSPtr entity() const override;

        private:
            lc::entity::Circle_CSPtr _circle;
    };
}
