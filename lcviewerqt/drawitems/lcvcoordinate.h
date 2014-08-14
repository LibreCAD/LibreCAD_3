#pragma once

#include "lcvdrawitem.h"
#include "cad/primitive/coordinate.h"

class LcDrawOptions;
class LcPainter;
class QRectF;

class LCVCoordinate : public LCVDrawItem, public lc::Coordinate {
    public:
        LCVCoordinate(const lc::Coordinate_CSPtr Coordinate);

        virtual void draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const;
};
