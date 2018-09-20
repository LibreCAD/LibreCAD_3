#include "lcvarc.h"
#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"

using namespace LCViewer;

LCVArc::LCVArc(const lc::entity::Arc_CSPtr& arc) :
        LCVDrawItem(arc, true),
        _arc(arc) {
}

void LCVArc::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
    if (_arc->radius() != 0) {
        if (_arc->CCW()) {
            painter.arcNegative(_arc->center().x(), _arc->center().y(), _arc->radius(), _arc->startAngle(), _arc->endAngle());
        } else {
            painter.arc(_arc->center().x(), _arc->center().y(), _arc->radius(), _arc->startAngle(), _arc->endAngle());
        }
        painter.stroke();
    }
}

lc::entity::CADEntity_CSPtr LCVArc::entity() const {
    return _arc;
}

