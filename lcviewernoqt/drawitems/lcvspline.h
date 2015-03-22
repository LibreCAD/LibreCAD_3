#pragma once

#include "lcvdrawitem.h"
#include <cad/primitive/spline.h>

class LcDrawOptions;
class LcPainter;
class QRectF;

class LCVSpline : public LCVDrawItem, public lc::Spline  {
    public:
        LCVSpline(const lc::Spline_CSPtr Coordinate);
        /**
         * @brief draw, Draws the point
         * @param LcPainter painter, surface to be painted
         * @param LcDrawOptions options
         * @param geo::Area rect
         */
        virtual void draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const;
};
