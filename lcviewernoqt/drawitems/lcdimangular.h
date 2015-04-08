#pragma once

#include "lcvdrawitem.h"
#include "lcdimension.h"
#include <cad/primitive/dimangular.h>


class LcDrawOptions;
class LcPainter;

class LCDimAngular : public LCVDrawItem, public lc::entity::DimAngular, public LCDimension {
    public:
        LCDimAngular(const lc::entity::DimAngular_CSPtr dimAngular);

        /**
        * @brief draw, Draws the DimAngular
        * @param LcPainter painter, surface to be painted
        * @param LcDrawOptions options
        * @param geo::Area rect
        */
        virtual void draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const;
};
