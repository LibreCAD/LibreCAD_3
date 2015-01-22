#pragma once

#include "lcvdrawitem.h"
#include <cad/primitive/dimdiametric.h>


class LcDrawOptions;
class LcPainter;
class QRectF;

class LCDimDiametric  : public LCVDrawItem, public lc::DimDiametric {
    public:
        LCDimDiametric(const lc::DimDiametric_CSPtr dimDiametric);

        /**
        * @brief draw, Draws the circle
        * @param LcPainter painter, surface to be painted
        * @param LcDrawOptions options
        * @param geo::Area rect
        */
        virtual void draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const;
};
