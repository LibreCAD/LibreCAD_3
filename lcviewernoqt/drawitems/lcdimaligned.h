#pragma once

#include "lcvdrawitem.h"
#include "lcdimension.h"
#include <cad/primitive/dimaligned.h>

namespace LCViewer {
        class LcDrawOptions;
        class LcPainter;

        class LCDimAligned : public LCVDrawItem, public LCDimension {
            public:
                LCDimAligned(const lc::entity::DimAligned_CSPtr dimAligned);

                /**
                * @brief draw, Draws the LCDimAligned
                * @param LcPainter painter, surface to be painted
                * @param LcDrawOptions options
                * @param geo::Area rect
                */
                virtual void draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const override;

                lc::entity::CADEntity_CSPtr entity() const override;

            private:
                lc::entity::DimAligned_CSPtr _dimAligned;
        };
}
