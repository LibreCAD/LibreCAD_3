#include <cad/geometry/geoarc.h>
#include <documentcanvas.h>
#include "lclwpolyline.h"
#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"
using namespace LCViewer;
LCLWPolyline::LCLWPolyline(const lc::entity::LWPolyline_CSPtr lwpolyline) :
        LCVDrawItem(lwpolyline, true),
        _polyLine(lwpolyline) {

    for(auto entity : _polyLine->asEntities()) {
        _drawItems.push_back(DocumentCanvas::asDrawable(entity));
    }
}

void LCLWPolyline::draw(LcPainter &painter, const LcDrawOptions &options, const lc::geo::Area &rect) const {
    for(auto drawItem : _drawItems) {
        drawItem->draw(painter, options, rect);
    }
}

lc::entity::CADEntity_CSPtr LCLWPolyline::entity() const {
    return _polyLine;
}



