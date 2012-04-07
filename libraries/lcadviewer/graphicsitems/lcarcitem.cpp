#include "lcarcitem.h"


LCArcItem::LCArcItem(const lc::Arc* arc) : LCGraphicsItem(), _arc(arc) {
}


QRectF LCArcItem::boundingRect() const {
    return QRectF(_arc->center().x() - _arc->radius(),
                  _arc->center().y() - _arc->radius(),
                  2 * _arc->radius(),
                  2 * +_arc->radius());
}

void LCArcItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {

    LCGraphicsItem::paint(_arc, painter, option, widget);

    // FIXME? Do we need to crite end lines for arc's because of the limitation of the start/stop angle?
    painter->drawArc(QRectF(_arc->center().x() - _arc->radius(),
                            _arc->center().y() - _arc->radius(),
                            2 * _arc->radius(),
                            2 * _arc->radius()), _arc->startAngle() * (360.0 / PI * 8), _arc->endAngle() * (360.0 / PI * 8) - _arc->startAngle() * (360.0 / PI * 8));
}


QPainterPath LCArcItem::shape() const {
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}


