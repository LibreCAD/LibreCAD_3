#include "lcvhatch.h"
#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/line.h"
#include "cad/primitive/ellipse.h"
#include "cad/primitive/spline.h"
#include "cad/primitive/lwpolyline.h"
#include <documentcanvas.h>

using namespace lc::viewer;

LCVHatch::LCVHatch(const lc::entity::Hatch_CSPtr& hatch) :
        LCVDrawItem(hatch, true),
        _hatch(hatch) {
    for(auto &x:_hatch->_loopList){
        for(auto &y:x->objList){
		auto di = DocumentCanvas::asDrawable(y);
		di->autostroke(false);
		_drawItems.push_back(di);
	}
	_drawItems.push_back(nullptr);
    }
}

void LCVHatch::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
   for(const auto& drawItem : _drawItems) {
	if(drawItem){
	        drawItem->draw(painter, options, rect);
	}else{
		painter.close_path();
		painter.fill();
	}
    }
    painter.fill();
}

lc::entity::CADEntity_CSPtr LCVHatch::entity() const {
    return _hatch;
}
