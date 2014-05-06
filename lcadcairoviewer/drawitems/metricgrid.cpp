#include <math.h>
#include "metricgrid.h"

#include <QDebug>
MetricGrid::MetricGrid(int minimumGridSpacing, const QColor& major, const QColor& minor) :  _majorColor(major), _minorColor(minor), _minimumGridSpacing(minimumGridSpacing) {
}

MetricGrid::~MetricGrid() {
    qDebug() << "MetricGrid destroyed";
}


void MetricGrid::draw(cairo_t* cr, const QRectF& rect)  {
/*
    QPointF zeroCorner = view->mapToScene(0, 0);
    QPointF minGridSpaceCorner = view->mapToScene(_minimumGridSpacing, 0);

    // This brings the distance always between 10 and 100, need to have some math behind this
    double minDistancePoints = minGridSpaceCorner.x() - zeroCorner.x();
    double factor = 1.0;

    while (minDistancePoints < 10.0) {
        minDistancePoints *= 10.0;
        factor = factor * 10.0;
    }

    while (minDistancePoints > 100.0) {
        minDistancePoints = minDistancePoints / 10.0;
        factor = factor / 10.0;
    }

    // determine the grid spacing
    double gridSize;

    if (minDistancePoints < 10.0) {
        gridSize = (10.0 / factor);
    } else if (minDistancePoints < 20.0) {
        gridSize = (20.0 / factor);
    } else if (minDistancePoints < 50.0) {
        gridSize = (50.0 / factor);
    } else {
        gridSize = (100.0 / factor);
    }

    _lastGridSize = gridSize;

    // Start Drawing
    painter->setRenderHint(QPainter::Antialiasing, false);

    // Major lines
    qreal left = rect.left() - fmod(rect.left(), gridSize);
    qreal top = rect.top() - fmod(rect.top(), gridSize);

    QVarLengthArray<QLineF, 100> lines;

    for (qreal x = left; x < rect.right(); x += gridSize) {
        lines.append(QLineF(x, rect.top(), x, rect.bottom()));
    }

    for (qreal y = top; y < rect.bottom(); y += gridSize) {
        lines.append(QLineF(rect.left(), y, rect.right(), y));
    }

    painter->setPen(QPen(QBrush(_majorColor), 0.0, Qt::SolidLine));
    painter->drawLines(lines.data(), lines.size());

    // Draw minor lines
    lines.clear();
    gridSize *= 10;
    left = rect.left() - fmod(rect.left(), gridSize);
    top = rect.top() - fmod(rect.top(), gridSize);

    for (qreal x = left; x < rect.right(); x += gridSize) {
        lines.append(QLineF(x, rect.top(), x, rect.bottom()));
    }

    for (qreal y = top; y < rect.bottom(); y += gridSize) {
        lines.append(QLineF(rect.left(), y, rect.right(), y));
    }

    painter->setPen(QPen(QBrush(_minorColor), 0.0, Qt::SolidLine));
    painter->drawLines(lines.data(), lines.size());
            */
}


/**
  * Return a number of snap points, grid will always return 1
  *
  */
QList<lc::EntityCoordinate> MetricGrid::snapPoints(const lc::geo::Coordinate& coord, double minDistanceToSnap, int maxNumberOfSnapPoints) const {

    QList<lc::EntityCoordinate> points;
    double mx = coord.x() * 1.0;
    double my = coord.y() * 1.0;
    double gs = this->_lastGridSize * 1.0;


    double x, y;

    if (mx < 0.0) {
        x = (mx - gs / 2) - fmod(mx - gs / 2, -gs);
    } else {
        x = (mx + gs / 2) - fmod(mx + gs / 2, gs);
    }

    if (my < 0.0) {
        y = (my - gs / 2) - fmod(my - gs / 2, -gs);
    } else {
        y = (my + gs / 2) - fmod(my + gs / 2, gs);
    }

    points << lc::EntityCoordinate(lc::geo::Coordinate(x, y), (lc::geo::Coordinate(x, y) - coord).magnitude(), 0);
    return points;
}


lc::geo::Coordinate MetricGrid::nearestPointOnPath(const lc::geo::Coordinate& coord) const {
    throw "nearestPointOnPath not available for grid";
}
