#pragma once

#include "lcvdrawitem.h"
#include "cad/primitive/circle.h"

namespace lc {
    namespace viewer {
        class LcDrawOptions;
        class LcPainter;

        class LCVCircle : public LCVDrawItem {
            public:
                LCVCircle(const lc::entity::Circle_CSPtr& circle);

                virtual ~LCVCircle() = default;

                /**
                 * @brief draw, Draws the circle
                 * @param LcPainter painter, surface to be painted
                 * @param LcDrawOptions options
                 * @param geo::Area rect
                 */
                void draw(LcPainter& painter, const LcDrawOptions& options, const lc::geo::Area& rect) const override;

                lc::entity::CADEntity_CSPtr entity() const override;

            private:
                lc::entity::Circle_CSPtr _circle;
        };
    }
}
