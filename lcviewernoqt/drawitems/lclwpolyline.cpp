#include <cad/geometry/geoarc.h>
#include <documentcanvas.h>
#include "lclwpolyline.h"
#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"
#include "cad/primitive/arc.h"
#include "cad/primitive/line.h"

using namespace lc::viewer;

LCLWPolyline::LCLWPolyline(const lc::entity::LWPolyline_CSPtr& lwpolyline) :
    LCVDrawItem(lwpolyline, true),
    _polyLine(lwpolyline) {

    for(const auto& entity : _polyLine->asEntities()) {
        _drawItems.push_back(DocumentCanvas::asDrawable(entity));
    }
}

void LCLWPolyline::draw(LcPainter &painter, const LcDrawOptions &options, const lc::geo::Area &rect) const {
    bool autoStroke = autostroke();
    auto vertices = _polyLine->vertex();

    if(!autoStroke) { // We need shape only
        for(auto drawItem : _drawItems) {
            drawItem->autostroke(false);
            drawItem->draw(painter, options, rect);
        }
    } else { // We are in rendering
        if(vertices.size()<2) return;
        auto totalSteps = vertices.size()-1;;
        if(_polyLine->closed())totalSteps++;
        for(unsigned int n=0; n<totalSteps; n++) {
            auto n1=(n+1)%vertices.size();
            double startWidth = vertices[n].startWidth(), endWidth=vertices[n].endWidth();
            if(startWidth==0 && endWidth==0) {
                _drawItems[n]->draw(painter, options, rect);
            } else if(vertices[n].bulge()==0) {
                auto vert1 = vertices[n];
                auto vert2 = vertices[n1];
                auto angle = vert1.location().angleTo(vert2.location());
                auto rot = geo::Coordinate(cos(angle+M_PI/2), sin(angle+M_PI/2));
                //4 points
                auto point1 = vert1.location()+rot*startWidth/2;
                auto point2 = vert2.location()+rot*endWidth/2;
                auto point3 = vert2.location()-rot*endWidth/2;
                auto point4 = vert1.location()-rot*startWidth/2;

                painter.move_to(point1.x(), point1.y());
                painter.line_to(point2.x(), point2.y());
                painter.line_to(point3.x(), point3.y());
                painter.line_to(point4.x(), point4.y());
                painter.close_path();
                painter.fill();
            } else {
                //We need arc center
                auto arcOld = std::dynamic_pointer_cast<const lc::entity::Arc>(_drawItems[n]->entity());//Old arc
                double start=arcOld->startAngle(), end=arcOld->endAngle();
                float n, angle1, angle2, delta, points=100, intp1, intp2, rd1, rd2;
                if(!arcOld->CCW()) {
                    if(start<end)
                        end-=2*M_PI;
                    n=1;
                } else {
                    if(start>end)
                        end+=2*M_PI;
                    n=-1;
                }
                delta=std::abs(end-start);
                for(int i=0; i<points; i++) {
                    intp1 = ((float)i)/points;
                    intp2 = ((float)(i+1))/points;
                    angle1=start - n*( intp1*delta);
                    angle2=start - n*( intp2*delta);
                    rd1=((1-intp1)*startWidth + intp1*endWidth)/2;
                    rd2=((1-intp2)*startWidth + intp2*endWidth)/2;
                    painter.move_to(arcOld->center().x() + (arcOld->radius()+rd1)*cos(angle1), arcOld->center().y()+ (arcOld->radius()+rd1)*sin(angle1));
                    painter.line_to(arcOld->center().x() + (arcOld->radius()+rd2)*cos(angle2), arcOld->center().y()+ (arcOld->radius()+rd2)*sin(angle2));
                    painter.line_to(arcOld->center().x() + (arcOld->radius()-rd2)*cos(angle2), arcOld->center().y()+ (arcOld->radius()-rd2)*sin(angle2));
                    painter.line_to(arcOld->center().x() + (arcOld->radius()-rd1)*cos(angle1), arcOld->center().y()+ (arcOld->radius()-rd1)*sin(angle1));
                    painter.close_path();
                    painter.fill();
                }
            }
        }
    }
}

lc::entity::CADEntity_CSPtr LCLWPolyline::entity() const {
    return _polyLine;
}



