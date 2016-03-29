#pragma once

#include "lcvdrawitem.h"
#include "cad/primitive/ellipse.h"

namespace LCViewer {

class LcDrawOptions;
class LcPainter;
class LCVEllipse : public LCVDrawItem, public lc::entity::Ellipse {
    public:
        /**
             * @brief LCVEllipse
             * @param Ellipse_CSPtr ellipse
             */
        LCVEllipse(const lc::entity::Ellipse_CSPtr ellipse);
        /**
         * @brief draw, Drawing the ellipse
         * @param LcPainter painter, surface to be painted
         * @param LcDrawOptions options
         * @param geo::Area rect
         */
        virtual void draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const;


};
}
