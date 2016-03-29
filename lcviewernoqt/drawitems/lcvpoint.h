#pragma once

#include "lcvdrawitem.h"
#include <cad/primitive/point.h>
namespace LCViewer {

class LcDrawOptions;
class LcPainter;

class LCVPoint : public LCVDrawItem, public lc::entity::Point  {
    public:
        LCVPoint(const lc::entity::Point_CSPtr Coordinate);
        /**
         * @brief draw, Draws the point
         * @param LcPainter painter, surface to be painted
         * @param LcDrawOptions options
         * @param geo::Area rect
         */
        virtual void draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const;
};
}
