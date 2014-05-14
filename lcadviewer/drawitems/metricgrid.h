#ifndef METRICGRID_H
#define METRICGRID_H

#include <QPainter>
#include <QGraphicsView>
#include <QVarLengthArray>
#include "lcvdrawitem.h"
#include "cad/interface/snapable.h"
#include "cad/geometry/geocoordinate.h"
/**
  * Draw a metric grid on a LCGraphics View
  *
  */
class MetricGrid : public LCVDrawItem, public lc::Snapable {
    public:
        MetricGrid(int minimumGridSpacing, const QColor& major, const QColor& minor);
        virtual ~MetricGrid();

        virtual void draw(LcPainter* _painter, LcDrawOptions* options, const QRectF& updateRect) const;

        virtual QList<lc::EntityCoordinate> snapPoints(const lc::geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const;

        virtual lc::geo::Coordinate nearestPointOnPath(const lc::geo::Coordinate& coord) const;

    private:
        double gridSize();
    private:
        const QColor _majorColor;
        const QColor _minorColor;
        const int _minimumGridSpacing;

        // Not sure if ut belongs here, on the other hand we didn't want to pass the view and rect into snapPoints
        mutable double _lastGridSize;
};

#endif // METRICGRID_H
