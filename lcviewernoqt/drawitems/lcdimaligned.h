#pragma once

#include "lcvdrawitem.h"
#include "lcdimension.h"
#include <cad/primitive/dimaligned.h>


class LcDrawOptions;
class LcPainter;

class LCDimAligned : public LCVDrawItem, public lc::entity::DimAligned, public LCDimension {
    public:
        LCDimAligned(const lc::entity::DimAligned_CSPtr dimAligned);

        /**
        * @brief draw, Draws the LCDimAligned
        * @param LcPainter painter, surface to be painted
        * @param LcDrawOptions options
        * @param geo::Area rect
        */
        virtual void draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const;
};
