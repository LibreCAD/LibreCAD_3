#pragma once

#include "lcvdrawitem.h"
#include "cad/primitive/circle.h"

class LcDrawOptions;
class LcPainter;

class LCVCircle  : public LCVDrawItem, public lc::entity::Circle {
    public:
        LCVCircle(const lc::entity::Circle_CSPtr circle);

        /**
         * @brief draw, Draws the circle
         * @param LcPainter painter, surface to be painted
         * @param LcDrawOptions options
         * @param geo::Area rect
         */
        virtual void draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const;
};
