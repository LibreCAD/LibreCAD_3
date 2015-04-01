#include <cad/geometry/geoarc.h>
#include "lclwpolyline.h"
#include "../lcpainter.h"
#include "lcdrawoptions.h"

LCLWPolyline::LCLWPolyline(const lc::entity::LWPolyline_CSPtr lwpolyline) : LCVDrawItem(true), lc::entity::LWPolyline(lwpolyline, true) {
}

void LCLWPolyline::draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const {


    /* THis might be a bit slower because it will do memory allocations
    auto items = asGeometrics();
    for (auto geoItem : items) {
        if (auto vector = std::dynamic_pointer_cast<lc::geo::Vector>(geoItem)) {
            painter->move_to(vector->start().x(), vector->start().y());
            painter->line_to(vector->end().x(), vector->end().y());
            painter->stroke();
        } else if (auto arc = std::dynamic_pointer_cast<lc::geo::Arc>(geoItem)) {
            painter->arc(arc->center().x(), arc->center().y(), arc->radius(), arc->startAngle(), arc->endAngle());
            painter->stroke();
        } else {
        }
    } */

    bool modified = false;

    auto itr = vertex().begin();
    painter->move_to(itr->location().x(), itr->location().y());
    auto lastPoint=itr;
    itr++;
    while(itr != vertex().end()) {
        if (lastPoint->bulge()!=0.) {
            lc::geo::Arc a = lc::geo::Arc::createArcBulge(lastPoint->location(), itr->location(), lastPoint->bulge());
            // TODO find out why need a 'move_to' here
            painter->move_to(itr->location().x(), itr->location().y());
            if (a.reversed()) {
                painter->arcNegative(a.center().x(), a.center().y(), a.radius(), a.startAngle(), a.endAngle());
            } else {
                painter->arc(a.center().x(), a.center().y(), a.radius(), a.startAngle(), a.endAngle());
            }
            painter->move_to(itr->location().x(), itr->location().y());
        } else {
            painter->line_to(itr->location().x(), itr->location().y());
        }
        lastPoint=itr;
        itr++;
    }

    if (closed()) {
        auto firstP=vertex().begin();
        if (lastPoint->bulge()!=0.) {
            lc::geo::Arc a = lc::geo::Arc::createArcBulge(lastPoint->location(), firstP->location(), lastPoint->bulge());
            // TODO find out why need a 'move_to' here
            painter->move_to(firstP->location().x(), firstP->location().y());
            if (a.reversed()) {
                painter->arcNegative(a.center().x(), a.center().y(), a.radius(), a.startAngle(), a.endAngle());
            } else {
                painter->arc(a.center().x(), a.center().y(), a.radius(), a.startAngle(), a.endAngle());
            }
            painter->move_to(firstP->location().x(), firstP->location().y());
        } else {
            painter->line_to(firstP->location().x(), firstP->location().y());
        }
    }

    painter->stroke();

    /** Draw bounding box
    auto &&plb = this->boundingBox();
    painter->save();
    painter->source_rgba(0, 1., 0., 0.8);
    painter->move_to(plb.top().start().x(), plb.top().start().y());
    painter->line_to(plb.top().end().x(), plb.top().end().y());
    painter->line_to(plb.right().start().x(), plb.right().start().y());
    painter->line_to(plb.bottom().start().x(), plb.bottom().start().y());
    painter->line_to(plb.left().end().x(), plb.left().end().y());
    painter->stroke();
    painter->restore();
     */

    if (modified) {
        painter->restore();
    }
}

