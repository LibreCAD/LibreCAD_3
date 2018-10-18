#pragma once

#include "lcvdrawitem.h"
#include "lcdimension.h"
#include <cad/primitive/dimdiametric.h>

namespace lc {
    namespace viewer {
        class LcDrawOptions;

        class LcPainter;

        class LCDimDiametric : public LCVDrawItem, public LCDimension {
            public:
                LCDimDiametric(const lc::entity::DimDiametric_CSPtr& dimDiametric);

                virtual ~LCDimDiametric() = default;

                /**
                * @brief draw, Draws the DimDiametric
                * @param LcPainter painter, surface to be painted
                * @param LcDrawOptions options
                * @param geo::Area rect
                */
                void draw(LcPainter& painter, const LcDrawOptions& options, const lc::geo::Area& rect) const override;

                lc::entity::CADEntity_CSPtr entity() const override;

            private:
                lc::entity::DimDiametric_CSPtr _dimDiametric;
        };
    }
}
