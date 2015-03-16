#include <cad/geometry/geoarc.h>
#include "lclwpolyline.h"
#include "../lcpainter.h"
#include "lcdrawoptions.h"

LCLWPolyline::LCLWPolyline(const lc::LWPolyline_CSPtr lwpolyline) : LCVDrawItem(true), lc::LWPolyline(lwpolyline, true) {
}

void LCLWPolyline::draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const {
    bool modified = false;

    auto itr = vertex().begin();
    painter->move_to(itr->location().x(), itr->location().y());
    auto lastPoint=itr;
    itr++;
    while(itr != vertex().end()) {
        if (lastPoint->bulge()!=0.) {
            lc::geo::Arc a = lc::geo::Arc::createArcBulge(lastPoint->location(), itr->location(), lastPoint->bulge());
            // TODO find out we need a 'move_to' here
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
            // TODO find out we need a 'move_to' here
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

    if (modified) {
        painter->restore();
    }
}

