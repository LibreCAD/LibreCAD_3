#include "lccircleitem.h"


LCCircleItem::LCCircleItem(const lc::CirclePtr circle) : LCGraphicsItem(), _circle(circle) {
}





QRectF LCCircleItem::boundingRect() const {
    return QRectF(_circle->center().x() - _circle->radius(),
                  _circle->center().y() - _circle->radius(),
                  2 * _circle->radius(),
                  2 * +_circle->radius());
}

void LCCircleItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    LCGraphicsItem::paint(_circle, painter, option, widget);

    painter->drawArc(QRectF(_circle->center().x() - _circle->radius(),
                            _circle->center().y() - _circle->radius(),
                            2 * _circle->radius(),
                            2 * _circle->radius()), 0, 5760);

    // TODO: Add a little cross in the middle?
}


QPainterPath LCCircleItem::shape() const {
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}


const lc::CADEntityPtr LCCircleItem::entity() const {
    return _circle;
}
