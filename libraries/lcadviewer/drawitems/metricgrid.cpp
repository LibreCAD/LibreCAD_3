#include "metricgrid.h"

#include <QDebug>
MetricGrid::MetricGrid(int minimumGridSpacing, const QColor& major, const QColor& minor) :  _majorColor(major), _minorColor(minor), _minimumGridSpacing(minimumGridSpacing) {
}


void MetricGrid::draw(const QGraphicsView* view, QPainter* painter, const QRectF& rect) {

    QRectF sceneRect = rect.normalized();
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

    // Start Drawing
    painter->setRenderHint(QPainter::Antialiasing, false);

    // Major lines
    qreal left = rect.left() - fmod(rect.left(), gridSize);
    qreal top = rect.top() - fmod(rect.top(), gridSize);

    QVarLengthArray<QLineF, 100> lines;
    int i = 0;

    for (qreal x = left; x < rect.right(); x += gridSize) {
        lines.append(QLineF(x, rect.top(), x, rect.bottom()));
    }

    for (qreal y = top; y < rect.bottom(); y += gridSize) {
        lines.append(QLineF(rect.left(), y, rect.right(), y));
    }

    painter->setPen(QPen(QBrush(_majorColor), 0.0, Qt::DotLine));
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

    painter->setPen(_minorColor);
    painter->drawLines(lines.data(), lines.size());
}
