#pragma once

#include "lcvdrawitem.h"
#include "cad/primitive/ellipse.h"

class LcDrawOptions;
class LcPainter;

class LCVEllipse : public LCVDrawItem, public lc::Ellipse {
    public:
        /**
             * @brief LCVEllipse
             * @param Ellipse_CSPtr ellipse
             */
        LCVEllipse(const lc::Ellipse_CSPtr ellipse);
        /**
         * @brief draw, Drawing the ellipse
         * @param LcPainter painter, surface to be painted
         * @param LcDrawOptions options
         * @param geo::Area rect
         */
        virtual void draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const;


};
