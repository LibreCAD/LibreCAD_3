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
  * FIXME: Find a better name for this, or remove it in favor of using events only
  */

typedef struct _cairo cairo_t;

class LCViewerDrawItem {
    public:
        virtual void draw(cairo_t* cr, const QRectF& rect) = 0;
};


#endif
