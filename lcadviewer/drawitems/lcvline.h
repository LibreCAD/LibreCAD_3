#ifndef LCVLINE_H
#define LCVLINE_H

#include "lcvdrawitem.h"
#include "cad/primitive/line.h"

class LcDrawOptions;
class LcPainter;
class QRectF;
class LCVLine : public LCVDrawItem, public lc::Line {
    public:
        LCVLine(const std::shared_ptr<const lc::Line> line);

        virtual void draw(LcPainter* painter, LcDrawOptions* options, const QRectF& rect) const;


};

#endif // LCVLINE_H
