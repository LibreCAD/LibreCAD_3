#pragma once

#include "lcvdrawitem.h"
#include <cad/primitive/arc.h>

class LCVArc: public LCVDrawItem, public lc::Arc {
    public:
        LCVArc(const lc::Arc_CSPtr circle);
        /**
         * @brief draw, Draw the Arc
         * @param LcPainter painter, surface to be painted
         * @param LcDrawOptions options
         * @param geo::Area rect
         */
        virtual void draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const;
};
