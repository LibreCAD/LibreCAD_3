#pragma once

#include "lcvdrawitem.h"
#include "lcdimension.h"
#include <cad/primitive/dimradial.h>


class LcDrawOptions;
class LcPainter;

class LCDimRadial  : public LCVDrawItem, public lc::entity::DimRadial, public LCDimension {
    public:
        LCDimRadial(const lc::entity::DimRadial_CSPtr dimRadial);

        /**
        * @brief draw, Draws the DimRadial
        * @param LcPainter painter, surface to be painted
        * @param LcDrawOptions options
        * @param geo::Area rect
        */
        virtual void draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const;
};
