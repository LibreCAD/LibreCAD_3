#pragma once

#include "lcvdrawitem.h"
#include <cad/primitive/arc.h>

namespace lc {
namespace viewer {
class LCVArc : public LCVDrawItem {
public:
    LCVArc(const lc::entity::Arc_CSPtr& arc);

    virtual ~LCVArc() = default;

    /**
     * @brief draw, Draw the Arc
     * @param LcPainter painter, surface to be painted
     * @param LcDrawOptions options
     * @param geo::Area rect
     */
    void draw(LcPainter& painter, const LcDrawOptions& options, const lc::geo::Area& rect) const override;

    lc::entity::CADEntity_CSPtr entity() const override;

private:
    lc::entity::Arc_CSPtr _arc;
};
}
}
