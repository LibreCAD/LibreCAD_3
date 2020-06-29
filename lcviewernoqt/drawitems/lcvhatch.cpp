#include "lcvhatch.h"
#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/line.h"
#include "cad/primitive/ellipse.h"
#include "cad/primitive/spline.h"
#include "cad/primitive/lwpolyline.h"

using namespace lc::viewer;

LCVHatch::LCVHatch(const lc::entity::Hatch_CSPtr& hatch) :
        LCVDrawItem(hatch, true),
        _hatch(hatch) {
}

#include <iostream>
void LCVHatch::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
std::cout << "Found loops: "<<_hatch->_loopList.size() <<std::endl;
    for(auto &x:_hatch->_loopList){
	std::cout << x << " Found objects: "<<x->objList.size() <<std::endl;
        bool processingLine = false; //To make polyline from lines for hatch
        for(auto &y:x->objList){
            if(auto arc = std::dynamic_pointer_cast<const lc::entity::Arc>(y)){//Tested
		std::cout << "Arc" <<std::endl;
                if (arc->radius() != 0) {
		std::cout << arc->CCW() << 'x' <<arc->center().x() << 'y' <<arc->center().y() << 'r' <<arc->radius() << 's' <<arc->startAngle() << 'e' <<arc->endAngle() <<std::endl;
                    if (arc->CCW()) {
                        painter.arcNegative(arc->center().x(), arc->center().y(), arc->radius(), arc->startAngle(), arc->endAngle());
                    } else {
                        painter.arc(arc->center().x(), arc->center().y(), arc->radius(), arc->startAngle(), arc->endAngle());
                    }
                }
            }else if(auto line = std::dynamic_pointer_cast<const lc::entity::Line>(y)){//Tested
		std::cout << "Line" <<std::endl;
                if(!processingLine){
                    painter.move_to(line->start().x(), line->start().y());
                    processingLine = true;
                }
                painter.line_to(line->end().x(), line->end().y());
            }else if(auto polyline = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(y)){//Not tested, i used lcad2 for test file creation, it used line instead
		std::cout << "Polyline" <<std::endl;
                for( auto &geoItem:polyline->asEntities()){//as geo::Vector or geo::Arc
                    if (auto vector = std::dynamic_pointer_cast<const lc::geo::Vector>(geoItem)) {
                        if(!processingLine){
                                painter.move_to(vector->start().x(), vector->start().y());
                                processingLine=true;
                        }
                        painter.line_to(vector->end().x(),vector->end().y());
                    } else if (auto arc = std::dynamic_pointer_cast<const lc::geo::Arc>(geoItem)) {// copy paste above code
                        processingLine=false;//reset, guess
                        if (arc->CCW()) {
                            painter.arcNegative(arc->center().x(), arc->center().y(), arc->radius(), arc->startAngle(), arc->endAngle());
                        } else {
	                        painter.arc(arc->center().x(), arc->center().y(), arc->radius(), arc->startAngle(), arc->endAngle());
                        }
                    }
                }
            }else if(auto ellipse = std::dynamic_pointer_cast<const lc::entity::Ellipse>(y)){//Tested
		std::cout << "Ellipse" <<std::endl;
                if (ellipse->minorRadius() != 0) {
                    painter.ellipse(
                        ellipse->center().x(), ellipse->center().y(),
                        ellipse->majorRadius(), ellipse->minorRadius(),
                        ellipse->startAngle(), ellipse->endAngle(),
                        ellipse->getAngle()
                    );
                }
            }else if(auto spline = std::dynamic_pointer_cast<const lc::entity::Spline>(y)){// I cant get spline hatch on v2
		std::cout << "Spline" <<std::endl;
                //copy paste again, i don't know what it means
                for(const auto &bezier: spline->beziers()) {
                    auto bez = bezier->getCP();
                    if(!processingLine){
                        painter.move_to(bez[0].x(), bez[0].y());
                        processingLine=true;
                    }//old style required maybe
                    if(bez.size()==4) {
                        painter.curve_to(bez[1].x(), bez[1].y(), bez[2].x(), bez[2].y(), bez[3].x(), bez[3].y());
                    } else if (bez.size()==3) {
                        painter.quadratic_curve_to(bez[1].x(), bez[1].y(), bez[2].x(), bez[2].y());
                    } else if(bez.size()==2) {
                        painter.line_to(bez[1].x(), bez[1].y());
                    }
                }
            }
        }
        painter.fill(); //not reset processingLine, assume only lines are in layer
    }
}

lc::entity::CADEntity_CSPtr LCVHatch::entity() const {
    return _hatch;
}
