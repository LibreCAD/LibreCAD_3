#pragma once

#include "lcvdrawitem.h"
#include "lcdimension.h"
#include <cad/primitive/dimlinear.h>


class LcDrawOptions;
class LcPainter;
class QRectF;

class LCDimLinear : public LCVDrawItem, public lc::DimLinear, public LCDimension {
    public:
        LCDimLinear(const lc::DimLinear_CSPtr dimLinear);

        /**
        * @brief draw, Draws the circle
        * @param LcPainter painter, surface to be painted
        * @param LcDrawOptions options
        * @param geo::Area rect
        */
        virtual void draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const;
};
