#include <cad/geometry/geoarc.h>
#include "lclwpolyline.h"
#include "../painters/lcpainter.h"
#include "../lcdrawoptions.h"
using namespace LCViewer;
LCLWPolyline::LCLWPolyline(const lc::entity::LWPolyline_CSPtr lwpolyline) :
        LCVDrawItem(lwpolyline, true),
        _polyLine(lwpolyline) {
}

void LCLWPolyline::draw(LcPainter &painter, const LcDrawOptions &options, const lc::geo::Area &rect) const {
    auto draw_arc = [&painter](lc::geo::Coordinate const & p1, lc::geo::Coordinate const & p2, double const bulge) {
       auto &&a = lc::geo::Arc::createArcBulge(p1, p2, bulge);
        painter.new_sub_path();
        if (a.CCW()) {
            painter.arc(a.center().x(), a.center().y(), a.radius(), a.startAngle(), a.endAngle());
        } else {
            painter.arcNegative(a.center().x(), a.center().y(), a.radius(), a.startAngle(), a.endAngle());
        }
    };

    auto drawTo = [&painter, &draw_arc](const decltype(_polyLine->vertex().begin()) &currentPoint, const decltype(_polyLine->vertex().begin()) &beforePoint) {

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

        if (beforePoint->bulge() != 0.) {

            /* for arc */
            painter.new_path();
            if (beforePoint->startWidth()) {
                painter.line_to(sWp1.x(), sWp1.y());
            }

            draw_arc(eWp1, sWp1, -beforePoint->bulge());
            painter.line_to(eWp1.x(), eWp1.y());

            if (beforePoint->endWidth()) {
                painter.line_to(eWp2.x(), eWp2.y());
            }

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
    auto itr = _polyLine->vertex().begin();
    painter.move_to(itr->location().x(), itr->location().y());
    auto lastPoint = itr;
    itr++;
    while (itr != _polyLine->vertex().end()) {
        drawTo(itr, lastPoint);
        lastPoint = itr;
        itr++;
    }

    if (_polyLine->closed()) {
        drawTo(_polyLine->vertex().begin(), lastPoint);
    }
}

lc::entity::CADEntity_CSPtr LCLWPolyline::entity() const {
    return _polyLine;
}



