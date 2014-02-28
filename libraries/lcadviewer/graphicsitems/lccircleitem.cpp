#include "lccircleitem.h"


LCCircleItem::LCCircleItem(const boost::shared_ptr<const lc::Circle> circle) : LCGraphicsItem(), _circle(circle) {
}





QRectF LCCircleItem::boundingRect() const {
    return QRectF(_circle->center().x() - _circle->radius(),
                  _circle->center().y() - _circle->radius(),
                  2. * _circle->radius(),
                  2. * +_circle->radius());
}

void LCCircleItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    LCGraphicsItem::paint(_circle, painter, option, widget);

    // See qt_curves_for_arc, at high zoom levels the arc renders inaccurate when the arc os much larger then the screen
    // and I think this can be corrected by adding more control points.

    painter->drawArc(QRectF(_circle->center().x() - _circle->radius(),
                            _circle->center().y() - _circle->radius(),
                            2. * _circle->radius(),
                            2. * _circle->radius()), 0, 5760);
}


QPainterPath LCCircleItem::shape() const {
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}


const boost::shared_ptr<const lc::CADEntity> LCCircleItem::entity() const {
    return _circle;
}
