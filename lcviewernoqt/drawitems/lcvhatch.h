#pragma once

#include "lcvdrawitem.h"
#include "cad/primitive/hatch.h"

namespace lc {
    namespace viewer {
        class LcDrawOptions;
        class LcPainter;

        class LCVHatch : public LCVDrawItem {
            public:
                LCVHatch(const lc::entity::Hatch_CSPtr& circle);

                virtual ~LCVHatch() = default;

                /**
                 * @brief draw, Draws the circle
                 * @param LcPainter painter, surface to be painted
                 * @param LcDrawOptions options
                 * @param geo::Area rect
                 */
                void draw(LcPainter& painter, const LcDrawOptions& options, const lc::geo::Area& rect) const override;

                lc::entity::CADEntity_CSPtr entity() const override;

            private:
                lc::entity::Hatch_CSPtr _hatch;
                std::vector<LCVDrawItem_CSPtr> _drawItems;
        };
    }
}
