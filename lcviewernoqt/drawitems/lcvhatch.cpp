#include "lcvhatch.h"
#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/line.h"
#include "cad/primitive/ellipse.h"
#include "cad/primitive/spline.h"
#include "cad/primitive/lwpolyline.h"
#include "cad/base/visitor.h"

using namespace lc::viewer;

LCVHatch::LCVHatch(const lc::entity::Hatch_CSPtr& hatch) :
        LCVDrawItem(hatch, true),
        _hatch(hatch) {
}

class HatchHelper{
    public:
        HatchHelper(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect):_painter(painter),_options(options),_rect(rect),_processingLine(false){};
        void finish(){_painter.fill();_processingLine=false;}; //End of loop
        void operator()(const lc::entity::Arc&);//Only draw shapes in these functions
        void operator()(const lc::entity::Line&);
        void operator()(const lc::entity::LWPolyline&);
        void operator()(const lc::entity::Ellipse&);
        void operator()(const lc::entity::Spline&);
        //Other unnecessary operators
        void operator()(const lc::Visitable&){};
        void operator()(const lc::geo::Circle&){};
        void operator()(const lc::entity::Point&){};
        void operator()(const lc::entity::Circle&){};
        void operator()(const lc::entity::Image&){};
private:
    LcPainter& _painter;
    const LcDrawOptions &_options;
    const lc::geo::Area& _rect;
    bool _processingLine;
};

void HatchHelper::operator()(const lc::entity::Arc& arc){//Draw Arc
    if (arc.radius() != 0) {
        if (arc.CCW()) {
            _painter.arcNegative(arc.center().x(), arc.center().y(), arc.radius(), arc.startAngle(), arc.endAngle());
        } else {
            _painter.arc(arc.center().x(), arc.center().y(), arc.radius(), arc.startAngle(), arc.endAngle());
        }
    }
};// Tested

void HatchHelper::operator()(const lc::entity::Line& line){//Draw Line
    //Multiple lines should be connected to form polyline
    //``` painter.move_to(...); painter.line_to(...); painter.line_to(...); painter.line_to(...); ...```
    if(!_processingLine){
        _painter.move_to(line.start().x(), line.start().y());
        _processingLine = true;
    }
    _painter.line_to(line.end().x(), line.end().y());
};// Tested

void HatchHelper::operator()(const lc::entity::LWPolyline& polyline){//Draw Polyline
    //Create lines as in the lines
    for( auto &geoItem:polyline.asEntities()){//as geo::Vector or geo::Arc
        if (auto vector = std::dynamic_pointer_cast<const lc::geo::Vector>(geoItem)) {// Only two casts so this may be optimal
            if(!_processingLine){
                    _painter.move_to(vector->start().x(), vector->start().y());
                    _processingLine=true;
            }
            _painter.line_to(vector->end().x(),vector->end().y());
        } else if (auto arc = std::dynamic_pointer_cast<const lc::geo::Arc>(geoItem)) {
            _processingLine=false;//What if arc in middle of line?
            if (arc->CCW()) {
                _painter.arcNegative(arc->center().x(), arc->center().y(), arc->radius(), arc->startAngle(), arc->endAngle());
            } else {
                _painter.arc(arc->center().x(), arc->center().y(), arc->radius(), arc->startAngle(), arc->endAngle());
            }
        }
    }    
};//Untested: may be wrong, LibreCAD2 generated polyline hatch as line

void HatchHelper::operator()(const lc::entity::Ellipse& ellipse){//Draw Ellipse
    if (ellipse.minorRadius() != 0) {
        _painter.ellipse(
            ellipse.center().x(), ellipse.center().y(),
            ellipse.majorRadius(), ellipse.minorRadius(),
            ellipse.startAngle(), ellipse.endAngle(),
            ellipse.getAngle()
        );
    }
};//Tested

void HatchHelper::operator()(const lc::entity::Spline& spline){//Draw Spline
    for(const auto &bezier: spline.beziers()) {
        auto bez = bezier->getCP();
        if(!_processingLine){
            _painter.move_to(bez[0].x(), bez[0].y());
            _processingLine=true;
        }// as in the lines, end is start of new line or curve
        if(bez.size()==4) {
            _painter.curve_to(bez[1].x(), bez[1].y(), bez[2].x(), bez[2].y(), bez[3].x(), bez[3].y());
        } else if (bez.size()==3) {
            _painter.quadratic_curve_to(bez[1].x(), bez[1].y(), bez[2].x(), bez[2].y());
        } else if(bez.size()==2) {
            _painter.line_to(bez[1].x(), bez[1].y());
        }
    }    
};//Untested: I cannot get Spline hatch in LibreCAD2, may be wrong

void LCVHatch::draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const {
    HatchHelper h(painter,options,rect);//Save parameters to helpers
    for(auto &x:_hatch->_loopList){
        for(auto &y:x.objList){
            visitorDispatcher<void, GeoEntityVisitor>(h, *y.get());//Dispach visitor to entities
        }
        h.finish();//End of hatch loop
    }
}

lc::entity::CADEntity_CSPtr LCVHatch::entity() const {
    return _hatch;
}

