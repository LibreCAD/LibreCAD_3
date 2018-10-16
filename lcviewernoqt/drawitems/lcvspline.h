#pragma once

#include "lcvdrawitem.h"
#include "cad/primitive/spline.h"

namespace lc {
    namespace viewer {
        class LcDrawOptions;

        class LcPainter;

        class LCVSpline : public LCVDrawItem {
            public:
                LCVSpline(const lc::entity::Spline_CSPtr& spline);

                virtual ~LCVSpline() = default;

                /**
                 * @brief draw, Draws the point
                 * @param LcPainter painter, surface to be painted
                 * @param LcDrawOptions options
                 * @param geo::Area rect
                 */
                void draw(LcPainter& painter, const LcDrawOptions& options, const lc::geo::Area& rect) const override;

                lc::entity::CADEntity_CSPtr entity() const override;

            private:
                lc::entity::Spline_CSPtr _spline;
        };
    }
}
