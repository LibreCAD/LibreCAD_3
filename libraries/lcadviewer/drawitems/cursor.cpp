#include "cursor.h"

Cursor::Cursor(int cursorSize, SnapManager* snapManager, const QColor& xAxisColor, const QColor& yAxisColor) : _snapManager(snapManager), _xAxisColor(xAxisColor), _yAxisColor(yAxisColor), _cursorSize(cursorSize) {
}

void Cursor::draw(const QGraphicsView* view, QPainter* painter, const QRectF& rect, const QPointF& _mousePosition) {
    Q_UNUSED(rect)

    QPointF zeroCorner = view->mapToScene(0, 0);
    QPointF minGridSpaceCorner = view->mapToScene(_cursorSize, 0);

    double minDistancePoints = (minGridSpaceCorner.x() - zeroCorner.x()) / 2;

    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setPen(_xAxisColor);
    painter->drawLine(-minDistancePoints + _mousePosition.x(), _mousePosition.y(), minDistancePoints + _mousePosition.x(), _mousePosition.y());

    painter->setPen(_yAxisColor);
    painter->drawLine(_mousePosition.x(), minDistancePoints + _mousePosition.y(), _mousePosition.x(), -minDistancePoints + _mousePosition.y());
}
