#include "lcvhatch.h"
#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"

using namespace lc::viewer;

LCVHatch::LCVHatch(const lc::entity::Hatch_CSPtr& hatch) :
        LCVDrawItem(hatch, true),
        _hatch(hatch) {
}

void LCVHatch::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
    painter.circle(0, 0, 20);
    painter.stroke();
}

lc::entity::CADEntity_CSPtr LCVHatch::entity() const {
    return _hatch;
}

