#ifndef CURSOR_H
#define CURSOR_H

#include <QColor>
#include "lcviewercursoritem.h"

class Cursor : public LCViewerCursorItem {
    public:
        Cursor(int cursorSize, const QColor& xAxisColor, const QColor& yAxisColor);
        void draw(const QGraphicsView* view, QPainter* painter, const QRectF& rect, const QPointF& _mousePosition);

    private:
        const QColor _xAxisColor;
        const QColor _yAxisColor;
        const double _cursorSize;
};

#endif // CURSOR_H
