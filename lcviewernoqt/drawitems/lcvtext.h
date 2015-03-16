#pragma once

#include "lcvdrawitem.h"
#include "cad/primitive/text.h"

class LcDrawOptions;
class LcPainter;

class LCVText : public LCVDrawItem, public lc::Text {
    public:
        LCVText(const lc::Text_CSPtr Text);
        /**
         * @brief draw, Drawing the text
         * @param LcPainter painter, surface to be painted
         * @param LcDrawOptions options
         * @param geo::Area rect
         */

        virtual void draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const;


};
