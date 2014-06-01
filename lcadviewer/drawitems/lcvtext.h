#ifndef LCVTEXT_H
#define LCVTEXT_H

#include "lcvdrawitem.h"
#include "cad/primitive/text.h"

class LcDrawOptions;
class LcPainter;
class QRectF;
class LCVText : public LCVDrawItem, public lc::Text {
    public:
        LCVText(const lc::Text_CSPtr Text);

        virtual void draw(LcPainter* painter, LcDrawOptions* options, const lc::geo::Area& rect) const;


};

#endif // LCVTEXT_H
