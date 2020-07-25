#pragma once

#include "lcvdrawitem.h"
#include <cad/primitive/lwpolyline.h>

namespace lc {
    namespace viewer {
        class LCLWPolyline : public LCVDrawItem {
            public:
                LCLWPolyline(const lc::entity::LWPolyline_CSPtr& lwpolyline);

                virtual ~LCLWPolyline() = default;

                /**
                 * @brief draw, Draws the polyline
                 * @param LcPainter painter, surface to be painted
                 * @param LcDrawOptions options
                 * @param geo::Area rect
                 */
                void draw(LcPainter& painter, const LcDrawOptions& options, const lc::geo::Area& rect) const override;

                lc::entity::CADEntity_CSPtr entity() const override;

            private:
                lc::entity::LWPolyline_CSPtr _polyLine;
                std::vector<LCVDrawItem_SPtr> _drawItems;
        };
    }
}
