#include "gradientbackground.h"

#include <QRectF>
#include <QGraphicsView>
#include <QPainter>

GradientBackground::GradientBackground(const QColor& topColor, const QColor& bottomColor) : _topColor(topColor), _bottomColor(bottomColor) {
}


void GradientBackground::draw(const QGraphicsView* view, QPainter* painter, const QRectF& rect) {
    QPolygonF psceneRect = view->mapToScene(view->rect());

    QRectF sceneRect = psceneRect.boundingRect();

    QLinearGradient gradient(
        (sceneRect.right() - sceneRect.left()) / 2.0, sceneRect.top(),
        (sceneRect.right() - sceneRect.left()) / 2.0, sceneRect.bottom());
    gradient.setColorAt(0, _topColor);
    gradient.setColorAt(1, _bottomColor);
    painter->fillRect(rect.intersected(sceneRect), gradient);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(sceneRect);
}
