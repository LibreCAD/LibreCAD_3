#ifndef LCADVIEWERDRAWITEM_H
#define LCADVIEWERDRAWITEM_H

#include <QPainter>
#include <QGraphicsView>
#include <QVarLengthArray>

#include "const.h"

/**
  * LCADViewerDrawItem is a abstract class that any class needs to implement if it want's to draw something on backgrounds or foregrounds
  * of a LCADViewer document
  *
  * FIXME: Find a better name for this
  */

class LCADViewerDrawItem {
    public:
        virtual void draw(const QGraphicsView* view, QPainter* painter, const QRectF& rect) = 0;
};

typedef shared_ptr<LCADViewerDrawItem> LCADViewerDrawItemPtr;

#endif // LCADVIEWERDRAWITEM_H
