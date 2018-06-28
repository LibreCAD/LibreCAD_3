#pragma once

#include "lcvdrawitem.h"
#include "lcdimension.h"
#include <cad/primitive/dimradial.h>

namespace LCViewer {
    class LcDrawOptions;
    class LcPainter;

    class LCDimRadial  : public LCVDrawItem, public LCDimension {
        public:
            LCDimRadial(const lc::entity::DimRadial_CSPtr& dimRadial);

            /**
            * @brief draw, Draws the DimRadial
            * @param LcPainter painter, surface to be painted
            * @param LcDrawOptions options
            * @param geo::Area rect
            */
            virtual void draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const override;

            lc::entity::CADEntity_CSPtr entity() const override;

        private:
            lc::entity::DimRadial_CSPtr _dimRadial;
    };
}
