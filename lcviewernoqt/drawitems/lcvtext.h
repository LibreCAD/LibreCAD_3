#pragma once

#include "lcvdrawitem.h"
#include "cad/primitive/text.h"
namespace LCViewer {

class LcDrawOptions;
class LcPainter;

class LCVText : public LCVDrawItem, public lc::entity::Text {
    public:
        LCVText(const lc::entity::Text_CSPtr Text);
        /**
         * @brief draw, Drawing the text
         * @param LcPainter painter, surface to be painted
         * @param LcDrawOptions options
         * @param geo::Area rect
         */

        virtual void draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const;


};
}
