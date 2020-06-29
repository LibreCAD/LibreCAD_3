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

void LCVHatch::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
    for(auto &x:_hatch->_loopList){
        bool processingLine = false; //To make polyline from lines for hatch
        for(auto &y:x.objList){
            if(auto arc = std::dynamic_pointer_cast<const lc::entity::Arc>(y)){//Tested
                if (arc->radius() != 0) {
                    if (arc->CCW()) {
                        painter.arcNegative(arc->center().x(), arc->center().y(), arc->radius(), arc->startAngle(), arc->endAngle());
                        //assert(arc->endAngle()>0.0);
                    } else {
                        painter.arc(arc->center().x(), arc->center().y(), arc->radius(), 0, M_PI);
                        //painter.fill();
                        //painter.arc(arc->center().x()+80, arc->center().y(), arc->radius(), 0, M_PI*2);
                    }
                }
            }else if(auto line = std::dynamic_pointer_cast<const lc::entity::Line>(y)){//Tested
                if(!processingLine){
                    painter.move_to(line->start().x(), line->start().y());
                    processingLine = true;
                }
                painter.line_to(line->end().x(), line->end().y());
            }else if(auto polyline = std::dynamic_pointer_cast<const lc::entity::LWPolyline>(y)){//Not tested, i used lcad2 for test file creation, it used line instead
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
                            painter.arc(arc->center().x(), arc->center().y(), arc->radius(), 0, M_PI);
                        }
                    }
                }
            }else if(auto ellipse = std::dynamic_pointer_cast<const lc::entity::Ellipse>(y)){//Tested
                if (ellipse->minorRadius() != 0) {
                    painter.ellipse(
                        ellipse->center().x(), ellipse->center().y(),
                        ellipse->majorRadius(), ellipse->minorRadius(),
                        ellipse->startAngle(), ellipse->endAngle(),
                        ellipse->getAngle()
                    );
                }
            }else if(auto spline = std::dynamic_pointer_cast<const lc::entity::Spline>(y)){// I cant get spline hatch on v2
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
