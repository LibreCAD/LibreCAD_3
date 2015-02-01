#include "endcaps.h"
#include <cmath>
#include "../lcpainter.h"
#include "lcdrawoptions.h"

EndCaps::EndCaps() {

    _arrows.insert(std::pair<unsigned int, std::function<void(LcPainter*, double , double , double, double, double)>>(NOCAP, [](LcPainter * painter, double, double, double endX, double endY, double size) {
    }));

    _arrows.insert(std::pair<unsigned int, std::function<void(LcPainter*, double , double , double, double, double)>>(OPENROUND, [](LcPainter * painter, double, double, double endX, double endY, double size) {
        painter->circle(endX, endY, size / cos(0.165) / 2);
    }));

    _arrows.insert(std::pair<unsigned int, std::function<void(LcPainter*, double , double , double, double, double)>>(CLOSEDROUND, [](LcPainter * painter, double, double, double endX, double endY, double size) {
        painter->circle(endX, endY, size / cos(0.165) / 2);
        painter->fill();
    }));

    _arrows.insert(std::pair<unsigned int, std::function<void(LcPainter*, double , double , double, double, double)>>(OPENARROW, [](LcPainter * painter, double startX, double startY, double endX, double endY, double size) {
        lc::geo::Coordinate start(startX, startY);
        double angle = start.angleTo(lc::geo::Coordinate(endX, endY));
        double cosv1, sinv1, cosv2, sinv2;
        double arrowSide = size / cos(0.165);

        cosv1 = cos(angle + 0.165) * arrowSide;
        sinv1 = sin(angle + 0.165) * arrowSide;
        cosv2 = cos(angle - 0.165) * arrowSide;
        sinv2 = sin(angle - 0.165) * arrowSide;

        painter->move_to(endX - cosv1, endY - sinv1);
        painter->line_to(endX, endY);
        painter->line_to(endX - cosv2, endY - sinv2);
        painter->stroke();

    }));

    _arrows.insert(std::pair<unsigned int, std::function<void(LcPainter*, double , double , double, double, double)>>(CLOSEDARROW, [](LcPainter * painter, double startX, double startY, double endX, double endY, double size) {
        lc::geo::Coordinate start(startX, startY);
        double angle = start.angleTo(lc::geo::Coordinate(endX, endY));
        double cosv1, sinv1, cosv2, sinv2;
        double arrowSide = size / cos(0.165);

        cosv1 = cos(angle + 0.165) * arrowSide;
        sinv1 = sin(angle + 0.165) * arrowSide;
        cosv2 = cos(angle - 0.165) * arrowSide;
        sinv2 = sin(angle - 0.165) * arrowSide;

        painter->move_to(endX - cosv1, endY - sinv1);
        painter->line_to(endX, endY);
        painter->line_to(endX - cosv2, endY - sinv2);
        painter->fill();
        painter->stroke();

    }));

    _arrows.insert(std::pair<unsigned int, std::function<void(LcPainter*, double , double , double, double, double)>>(OPENSQUARE, [](LcPainter * painter, double startX, double startY, double endX, double endY, double size) {
        lc::geo::Coordinate start(startX, startY);
        double angle = start.angleTo(lc::geo::Coordinate(endX, endY));

        double cosv1, sinv1, cosv2, sinv2, cosv3, sinv3;
        double arrowSide = size / cos(0.165);

        cosv1 = cos(angle + 0.165) * arrowSide;
        sinv1 = sin(angle + 0.165) * arrowSide;
        cosv2 = cos(angle - 0.165) * arrowSide;
        sinv2 = sin(angle - 0.165) * arrowSide;
        cosv3 = cos(angle) * 2 * arrowSide;
        sinv3 = sin(angle) * 2 * arrowSide;

        painter->move_to(endX, endY);
        painter->line_to(endX - cosv2, endY - sinv2);
        painter->line_to(endX - cosv3, endY - sinv3);
        painter->line_to(endX - cosv1, endY - sinv1);
        painter->line_to(endX, endY);
        painter->stroke();

    }));
    _arrows.insert(std::pair<unsigned int, std::function<void(LcPainter*, double , double , double, double, double)>>(CLOSEDSQUARE, [](LcPainter * painter, double startX, double startY, double endX, double endY, double size) {
        lc::geo::Coordinate start(startX, startY);
        double angle = start.angleTo(lc::geo::Coordinate(endX, endY));
        double cosv1, sinv1, cosv2, sinv2, cosv3, sinv3;
        double arrowSide = size / cos(0.165);

        cosv1 = cos(angle + 0.165) * arrowSide;
        sinv1 = sin(angle + 0.165) * arrowSide;
        cosv2 = cos(angle - 0.165) * arrowSide;
        sinv2 = sin(angle - 0.165) * arrowSide;
        cosv3 = cos(angle) * 2 * arrowSide;
        sinv3 = sin(angle) * 2 * arrowSide;

        painter->move_to(endX, endY);
        painter->line_to(endX - cosv2, endY - sinv2);
        painter->line_to(endX - cosv3, endY - sinv3);
        painter->line_to(endX - cosv1, endY - sinv1);
        painter->line_to(endX, endY);
        painter->fill();
        painter->stroke();

    }));
    _arrows.insert(std::pair<unsigned int, std::function<void(LcPainter*, double , double , double, double, double)>>(OPENTRIANGLE, [](LcPainter * painter, double startX, double startY, double endX, double endY, double size) {
        lc::geo::Coordinate start(startX, startY);
        double angle = start.angleTo(lc::geo::Coordinate(endX, endY));
        double cosv1, sinv1, cosv2, sinv2;
        double arrowSide = size / cos(0.165);

        cosv1 = cos(angle + 0.165) * arrowSide;
        sinv1 = sin(angle + 0.165) * arrowSide;
        cosv2 = cos(angle - 0.165) * arrowSide;
        sinv2 = sin(angle - 0.165) * arrowSide;

        painter->move_to(endX, endY);
        painter->line_to(endX - cosv1, endY - sinv1);
        painter->line_to(endX - cosv2, endY - sinv2);
        painter->line_to(endX, endY);
        painter->stroke();

    }));

    _arrows.insert(std::pair<unsigned int, std::function<void(LcPainter*, double , double , double, double, double)>>(CLOSEDTRIANGLE, [](LcPainter * painter, double startX, double startY, double endX, double endY, double size) {
        lc::geo::Coordinate start(startX, startY);
        double angle = start.angleTo(lc::geo::Coordinate(endX, endY));
        double cosv1, sinv1, cosv2, sinv2;
        double arrowSide = size / cos(0.165);

        cosv1 = cos(angle + 0.165) * arrowSide;
        sinv1 = sin(angle + 0.165) * arrowSide;
        cosv2 = cos(angle - 0.165) * arrowSide;
        sinv2 = sin(angle - 0.165) * arrowSide;

        painter->move_to(endX, endY);
        painter->line_to(endX - cosv1, endY - sinv1);
        painter->line_to(endX - cosv2, endY - sinv2);
        painter->line_to(endX, endY);
        painter->fill();
        painter->stroke();

    }));

}

void EndCaps::addCap(unsigned int type, std::function<void(LcPainter*, double startX, double startY, double endX, double endY, double size)> functor) {
    _arrows.insert(std::pair<unsigned int, std::function<void(LcPainter*, double , double , double, double, double)>>(type, functor));
}

void EndCaps::render(LcPainter* painter, unsigned int type, double startX, double startY, double endX, double endY, double size) const {
    _arrows.at(type)(painter, startX, startY, endX, endY, size);
}
