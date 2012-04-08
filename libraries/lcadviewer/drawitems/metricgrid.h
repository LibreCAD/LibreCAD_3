#ifndef METRICGRID_H
#define METRICGRID_H

#include <QPainter>
#include <QGraphicsView>
#include <QVarLengthArray>
#include "lcviewerdrawitem.h"

/**
  * Draw a metric grid on a LCGraphics View
  *
  */
class MetricGrid : public LCViewerDrawItem {
    public:
        MetricGrid(int minimumGridSpacing, const QColor& major, const QColor& minor);


        virtual void draw(const QGraphicsView* view, QPainter* painter, const QRectF& rect);

    private:
        const QColor _majorColor;
        const QColor _minorColor;
        const int _minimumGridSpacing;
};

#endif // METRICGRID_H
