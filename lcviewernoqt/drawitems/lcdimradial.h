#pragma once

#include "lcvdrawitem.h"
#include <cad/primitive/dimradial.h>


class LcDrawOptions;
class LcPainter;
class QRectF;

class LCDimRadial  : public LCVDrawItem, public lc::DimRadial {
public:
    LCDimRadial(const lc::DimRadial_CSPtr dimRadial);

    /**
    * @brief draw, Draws the circle
    * @param LcPainter painter, surface to be painted
    * @param LcDrawOptions options
    * @param geo::Area rect
    */
    virtual void draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const;
};
