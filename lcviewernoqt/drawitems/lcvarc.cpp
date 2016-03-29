#include "lcvarc.h"
#include "../lcpainter.h"
#include "../lcdrawoptions.h"
using namespace LCViewer;
LCVArc::LCVArc(const lc::entity::Arc_CSPtr arc) : LCVDrawItem(true), lc::entity::Arc(arc, true) {
}

void LCVArc::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
    bool modified = false;


    if (radius() /** painter.scale() > 5 */) {
        if (CCW()) {
            painter.arc(center().x(), center().y(), radius(), startAngle(), endAngle());
        } else {
            painter.arcNegative(center().x(), center().y(), radius(), startAngle(), endAngle());
        }
        painter.stroke();
    }

    /*
    lc::geo::Arc const *arc = dynamic_cast<const lc::geo::Arc*>(this);
    lc::geo::Area a = arc->boundingBox();
    painter.save();
    painter.source_rgba(0, 1., 0., 0.8);
    painter.move_to(arc->boundingBox().top().start().x(), arc->boundingBox().top().start().y());
    painter.line_to(arc->boundingBox().top().end().x(), arc->boundingBox().top().end().y());
    painter.line_to(arc->boundingBox().right().start().x(), arc->boundingBox().right().start().y());
    painter.line_to(arc->boundingBox().bottom().start().x(), arc->boundingBox().bottom().start().y());
    painter.line_to(arc->boundingBox().left().end().x(), arc->boundingBox().left().end().y());
    painter.stroke();
    painter.restore();
    */


    if (modified) {
        painter.restore();
    }
}

