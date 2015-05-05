#include <cad/geometry/geoarc.h>
#include "lclwpolyline.h"
#include "../lcpainter.h"
#include "../lcdrawoptions.h"

LCLWPolyline::LCLWPolyline(const lc::entity::LWPolyline_CSPtr lwpolyline) : LCVDrawItem(true), lc::entity::LWPolyline(lwpolyline, true) {
}

void LCLWPolyline::draw(LcPainter &painter, const LcDrawOptions &options, const lc::geo::Area &rect) const {


    /* THis might be a bit slower because it will do memory allocations
    auto items = asGeometrics();
    for (auto geoItem : items) {
        if (auto vector = std::dynamic_pointer_cast<lc::geo::Vector>(geoItem)) {
            painter.move_to(vector->start().x(), vector->start().y());
            painter.line_to(vector->end().x(), vector->end().y());
            painter.stroke();
        } else if (auto arc = std::dynamic_pointer_cast<lc::geo::Arc>(geoItem)) {
            painter.arc(arc->center().x(), arc->center().y(), arc->radius(), arc->startAngle(), arc->endAngle());
            painter.stroke();
        } else {
        }
    } */

    bool modified = false;
    auto draw_arc = [&painter](lc::geo::Coordinate const & p1, lc::geo::Coordinate const & p2, double const bulge) {
       auto &&a = lc::geo::Arc::createArcBulge(p1, p2, bulge);
        painter.new_sub_path();
        if (a.CCW()) {
            painter.arcNegative(a.center().x(), a.center().y(), a.radius(), a.startAngle(), a.endAngle());
        } else {
            painter.arc(a.center().x(), a.center().y(), a.radius(), a.startAngle(), a.endAngle());
        }
    };

    auto drawTo = [&painter, &draw_arc](const decltype(vertex().begin()) &currentPoint, const decltype(vertex().begin()) &beforePoint) {

        lc::geo::Coordinate sWp1 = beforePoint->location()
                                   , sWp2 = beforePoint->location()
                                            , eWp1 = currentPoint->location()
                                                    , eWp2 = currentPoint->location();

        if (beforePoint->startWidth() > 0.) {
            sWp1 = ((beforePoint->location() - currentPoint->location())
                    .rotate(0.5 * M_PI)
                    .norm() * 0.5 * beforePoint->startWidth())
                   + beforePoint->location();

            sWp2 = ((beforePoint->location() - currentPoint->location())
                    .rotate(0.5 * M_PI)
                    .norm() * 0.5 * -beforePoint->startWidth())
                   + beforePoint->location();
        }

        if (beforePoint->endWidth() > 0.) {
            eWp2 = ((currentPoint->location() - beforePoint->location())
                    .rotate(0.5 * M_PI)
                    .norm() * 0.5 * beforePoint->endWidth())
                   + currentPoint->location();

            eWp1 = ((currentPoint->location() - beforePoint->location())
                    .rotate(0.5 * M_PI)
                    .norm() * 0.5 * -beforePoint->endWidth())
                   + currentPoint->location();
        }

        if (std::abs(beforePoint->bulge()) > 0.) {

            /* for arc */
            painter.new_path();
            if (beforePoint->startWidth())
                painter.line_to(sWp1.x(), sWp1.y());

            draw_arc(eWp1, sWp1, -beforePoint->bulge());
            painter.line_to(eWp1.x(), eWp1.y());

            if (beforePoint->endWidth())
                painter.line_to(eWp2.x(), eWp2.y());

            if (beforePoint->startWidth() || beforePoint->endWidth()) {
                // kinda we have to slice it out
                painter.line_to(sWp2.x(), sWp2.y());
                draw_arc(sWp2, eWp2, beforePoint->bulge());
                painter.close_path();
            }

        } else {
            /* for line */
            painter.move_to(sWp2.x(), sWp2.y());

            if (beforePoint->startWidth())
                painter.line_to(sWp1.x(), sWp1.y());

            painter.line_to(eWp1.x(), eWp1.y());

            if (beforePoint->endWidth())
                painter.line_to(eWp2.x(), eWp2.y());

            if (beforePoint->startWidth() || beforePoint->endWidth())
                painter.line_to(sWp2.x(), sWp2.y());
        }

        if (beforePoint->startWidth() || beforePoint->endWidth()) {
            painter.fill();
        } else {
            painter.stroke();
        }

    };// end drawTo

    // main part
    auto itr = vertex().begin();
    painter.move_to(itr->location().x(), itr->location().y());
    auto lastPoint = itr;
    itr++;
    while (itr != vertex().end()) {
        drawTo(itr, lastPoint);
        lastPoint = itr;
        itr++;
    }

    if (closed()) {
        drawTo(vertex().begin(), lastPoint);
    }
    /** Draw bounding box
    auto &&plb = this->boundingBox();
    painter.save();
    painter.source_rgba(0, 1., 0., 0.8);
    painter.move_to(plb.top().start().x(), plb.top().start().y());
    painter.line_to(plb.top().end().x(), plb.top().end().y());
    painter.line_to(plb.right().start().x(), plb.right().start().y());
    painter.line_to(plb.bottom().start().x(), plb.bottom().start().y());
    painter.line_to(plb.left().end().x(), plb.left().end().y());
    painter.stroke();
    painter.restore();
     */

    if (modified) {
        painter.restore();
    }
}



