#ifndef LCVELLIPSE_H
#define LCVELLIPSE_H

#include "lcvdrawitem.h"
#include "cad/primitive/ellipse.h"

class LcDrawOptions;
class LcPainter;
class QRectF;
class LCVEllipse : public LCVDrawItem, public lc::Ellipse {
    public:
        LCVEllipse(const lc::Ellipse_CSPtr ellipse);

        virtual void draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const;


};

#endif // LCVELLIPSE_H
