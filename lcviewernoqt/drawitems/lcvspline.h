#pragma once

#include "lcvdrawitem.h"
#include "cad/primitive/spline.h"

namespace LCViewer {
    class LcDrawOptions;

    class LcPainter;

    class QRectF;

    class LCVSpline : public LCVDrawItem, public lc::entity::Spline {
    public:
        LCVSpline(const lc::entity::Spline_CSPtr Coordinate);

        /**
         * @brief draw, Draws the point
         * @param LcPainter painter, surface to be painted
         * @param LcDrawOptions options
         * @param geo::Area rect
         */
        virtual void draw(LcPainter &painter, const LcDrawOptions &options, const lc::geo::Area &rect) const;
    };
}