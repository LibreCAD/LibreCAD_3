#pragma once

#include "lcvdrawitem.h"
#include <cad/primitive/lwpolyline.h>

class LCLWPolyline : public LCVDrawItem, public lc::entity::LWPolyline {
    public:
    LCLWPolyline(const lc::entity::LWPolyline_CSPtr lwpolyline);

    /**
     * @brief draw, Draws the polyline
     * @param LcPainter painter, surface to be painted
     * @param LcDrawOptions options
     * @param geo::Area rect
     */
    virtual void draw(LcPainter& painter, const LcDrawOptions &options, const lc::geo::Area& rect) const;
};
