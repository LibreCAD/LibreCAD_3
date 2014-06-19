#ifndef LCVCOORDINATE_H
#define LCVCOORDINATE_H

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


#endif // COORDINATE_H
