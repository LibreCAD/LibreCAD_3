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
}

void LCVHatch::drawSolid(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
	// Hatch for solid fill based on approximation
    auto& reg = _hatch->getRegion();
    auto bbox = reg.boundingBox();
    auto minP = bbox.minP();
    auto maxP = bbox.maxP();
    unsigned int APPROX_POINTS = 300;
    float delta = (maxP.y() - minP.y())/(float)APPROX_POINTS;
    std::vector<float> oldI;
    float oldIY=minP.y();
    float extraSize=-delta;//Comensation if odd intersection is found, delta for first run
    std::vector<float> oldOk;
    for (float i=minP.y(); i<=maxP.y(); i+=delta)
    {
	    std::vector<lc::geo::Coordinate> newIc = reg.getLineIntersection(
	    		lc::geo::Vector(
	    			lc::geo::Coordinate(minP.x()-delta, i),
	    			lc::geo::Coordinate(maxP.x()+delta, i)
	    			)
	    	);
	    std::vector<float> newI;
	    for (auto &i : newIc){
	    	newI.push_back(i.x());
	    }//sort
	    sort(newI.begin(), newI.end()); 
	    if (newI.size()!=oldI.size() || newI.size()%2!=0){
	    	//Proper block not found, approximate with newerData
	    	extraSize += delta;
	    	oldI = newI;
	    	oldIY = i;
	    	continue;
	    }
	    //Counter for old and new loop
	    for(unsigned int x=0;x<newI.size();x+=2){
	    	    auto oldY = i-delta-extraSize;
		    painter.move_to(oldI[x], oldY);
		    painter.line_to(newI[x], i);
		    painter.line_to(newI[x+1], i);
		    painter.line_to(oldI[x+1], oldY);
		    painter.close_path();
		    painter.fill();
	    }
	    extraSize=0.;
	    oldI = newI;
	    oldIY = i;
	    oldOk = newI;
    }
    if(extraSize!=0){//Last size failed too
        for(unsigned int x=0;x<oldOk.size();x+=2){
    	    auto oldY = oldIY-delta;
    	    auto newY = oldY+extraSize;
	    painter.move_to(oldOk[x], oldY);
	    painter.line_to(oldOk[x], newY);
	    painter.line_to(oldOk[x+1], newY);
	    painter.line_to(oldOk[x+1], oldY);
	    painter.close_path();
	    painter.fill();
	}
    }
}

void LCVHatch::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
        	drawSolid(painter, options, rect);
}

lc::entity::CADEntity_CSPtr LCVHatch::entity() const {
    return _hatch;
}
