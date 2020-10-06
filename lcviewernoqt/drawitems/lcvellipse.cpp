#include "lcvellipse.h"
#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"

using namespace lc::viewer;

LCVEllipse::LCVEllipse(const lc::entity::Ellipse_CSPtr& ellipse) :
    LCVDrawItem(ellipse, true),
    _ellipse(ellipse) {
}

void LCVEllipse::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
    if (_ellipse->minorRadius() != 0) {
        if(!_ellipse->isReversed())
            painter.ellipse(
                _ellipse->center().x(), _ellipse->center().y(),
                _ellipse->majorRadius(), _ellipse->minorRadius(),
                _ellipse->startAngle(), _ellipse->endAngle(),
                _ellipse->getAngle()
            );
        else //changed to unreversed form for display, i think painter support one form only
            painter.ellipse(
                _ellipse->center().x(), _ellipse->center().y(),
                _ellipse->majorRadius(), _ellipse->minorRadius(),
                _ellipse->startAngle(), 2*M_PI + _ellipse->endAngle(),
                // I think painter auto does this, but lets not be dependent on it
                _ellipse->getAngle()
            );
        if(autostroke())
            painter.stroke();
    }
}

lc::entity::CADEntity_CSPtr LCVEllipse::entity() const {
    return _ellipse;
}
