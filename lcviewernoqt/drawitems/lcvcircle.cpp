#include "lcvcircle.h"
#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"

using namespace lc::viewer;

LCVCircle::LCVCircle(const lc::entity::Circle_CSPtr& circle) :
        LCVDrawItem(circle, true),
        _circle(circle) {
}

void LCVCircle::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
    if (_circle->radius() != 0) {
        painter.circle(_circle->center().x(), _circle->center().y(), _circle->radius());
	if(autostroke())
	        painter.stroke();
    }
}

lc::entity::CADEntity_CSPtr LCVCircle::entity() const {
    return _circle;
}

