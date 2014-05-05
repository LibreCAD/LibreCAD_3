#ifndef LCVIEWERDRAWITEM_H
#define LCVIEWERDRAWITEM_H

#include <QPainter>
#include <QGraphicsView>
#include <QVarLengthArray>

#include "const.h"

/**
  * LCADCairoViewerDrawItem is a abstract class that any class needs to implement if it want's to draw something on backgrounds or foregrounds
  * of a LCADCairoViewer document
  *
  * FIXME: Find a better name for this, or remove it in vafor of using events only
  */

class LCViewerDrawItem {
    public:
        virtual void draw(const QGraphicsView* view, QPainter* painter, const QRectF& rect) = 0;
};


#endif
