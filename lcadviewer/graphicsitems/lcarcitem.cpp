#include <math.h>
#include "lcarcitem.h"

LCArcItem::LCArcItem(const std::shared_ptr<const lc::Arc> arc) : LCGraphicsItem(), _arc(arc) {
}


QRectF LCArcItem::boundingRect() const {
    return QRectF(_arc->center().x() - _arc->radius(),
                  _arc->center().y() - _arc->radius(),
                  2 * _arc->radius(),
                  2 * +_arc->radius());
}

void LCArcItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {

    LCGraphicsItem::paint(painter, option, widget);

    double factorForQt = (360.0 / M_PI * 2.0 * 8);


    painter->drawArc(QRectF(_arc->center().x() - _arc->radius(),
                            _arc->center().y() - _arc->radius(),
                            2 * _arc->radius(),
                            2 * _arc->radius()), _arc->startAngle() * factorForQt, _arc->endAngle() * factorForQt - _arc->startAngle() * factorForQt);

    // FIXME? Do we need to write end lines for arc's because of the limitation of the start/stop angle?
}


QPainterPath LCArcItem::shape() const {
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}



const std::shared_ptr<const lc::CADEntity> LCArcItem::entity() const {
    return _arc;
}
