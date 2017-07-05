#pragma once

#include "lcvdrawitem.h"
#include "lcdimension.h"
#include <cad/primitive/dimangular.h>

namespace LCViewer {
        class LcDrawOptions;
        class LcPainter;

        class LCDimAngular : public LCVDrawItem, public LCDimension {
            public:
                LCDimAngular(const lc::entity::DimAngular_CSPtr dimAngular);

                /**
                * @brief draw, Draws the DimAngular
                * @param LcPainter painter, surface to be painted
                * @param LcDrawOptions options
                * @param geo::Area rect
                */
                virtual void draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const;

                lc::entity::CADEntity_CSPtr entity() const override;

            private:
                lc::entity::DimAngular_CSPtr _dimAngular;
        };
}
