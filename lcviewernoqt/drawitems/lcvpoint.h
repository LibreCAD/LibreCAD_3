#pragma once

#include "lcvdrawitem.h"
#include <cad/primitive/point.h>

namespace lc {
namespace viewer {
class LcDrawOptions;

class LcPainter;

class LCVPoint : public LCVDrawItem {
public:
    LCVPoint(const lc::entity::Point_CSPtr& coordinate);

    virtual ~LCVPoint() = default;

    /**
     * @brief draw, Draws the point
     * @param LcPainter painter, surface to be painted
     * @param LcDrawOptions options
     * @param geo::Area rect
     */
    void draw(LcPainter& painter, const LcDrawOptions& options, const lc::geo::Area& rect) const override;

    lc::entity::CADEntity_CSPtr entity() const override;

private:
    lc::entity::Point_CSPtr _point;
};
}
}
