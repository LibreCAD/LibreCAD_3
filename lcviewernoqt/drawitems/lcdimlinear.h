#pragma once

#include "lcvdrawitem.h"
#include "lcdimension.h"
#include <cad/primitive/dimlinear.h>

namespace LCViewer {
    class LcDrawOptions;
    class LcPainter;

    class LCDimLinear : public LCVDrawItem, public LCDimension {
        public:
            LCDimLinear(const lc::entity::DimLinear_CSPtr dimLinear);

            /**
            * @brief draw, Draws the DimLinear
            * @param LcPainter painter, surface to be painted
            * @param LcDrawOptions options
            * @param geo::Area rect
            */
            virtual void draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const;

            lc::entity::CADEntity_CSPtr entity() const override;

        private:
            lc::entity::DimLinear_CSPtr _dimLinear;
    };
}
