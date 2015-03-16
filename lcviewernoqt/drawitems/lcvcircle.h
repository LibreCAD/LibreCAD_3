#pragma once

#include "lcvdrawitem.h"
#include "cad/primitive/circle.h"

class LcDrawOptions;
class LcPainter;

class LCVCircle  : public LCVDrawItem, public lc::Circle {
    public:
        LCVCircle(const lc::Circle_CSPtr circle);

        /**
         * @brief draw, Draws the circle
         * @param LcPainter painter, surface to be painted
         * @param LcDrawOptions options
         * @param geo::Area rect
         */
        virtual void draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const;
};
