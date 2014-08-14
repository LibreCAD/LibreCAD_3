#pragma once

#include "lcvdrawitem.h"
#include "cad/primitive/circle.h"

class LcDrawOptions;
class LcPainter;
class QRectF;

class LCVCircle  : public LCVDrawItem, public lc::Circle {
    public:
        LCVCircle(const lc::Circle_CSPtr circle);

        virtual void draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const;
};
