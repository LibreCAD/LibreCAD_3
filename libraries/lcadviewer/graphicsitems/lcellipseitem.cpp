#include "lcellipseitem.h"

LCEllipseItem::LCEllipseItem(const boost::shared_ptr<const lc::Ellipse> ellipse) : LCGraphicsItem(), _ellipse(ellipse) {
}


QRectF LCEllipseItem::boundingRect() const {
    return QRectF(_ellipse->center().x() - _ellipse->minorRadius(),
                  _ellipse->center().y() - _ellipse->minorRadius(),
                  2 * _ellipse->minorRadius(),
                  2 * +_ellipse->minorRadius());
}

void LCEllipseItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {

    /*

    // FIXME? Do we need to crite end lines for ellipse's because of the limitation of the start/stop angle?
    painter->drawEllipse(QRectF(_ellipse->center().x() - _ellipse->radius(),
                            _ellipse->center().y() - _ellipse->radius(),
                            2 * _ellipse->radius(),
                            2 * _ellipse->radius()), _ellipse->startAngle() * (360.0 / PI * 8), _ellipse->endAngle() * (360.0 / PI * 8) - _ellipse->startAngle() * (360.0 / PI * 8));
    */
    LCGraphicsItem::paint(_ellipse, painter, option, widget);

    // True ellipse drawing needs to be implemented
    painter->drawText(_ellipse->center().x(), _ellipse->center().y(), "Ellipse here");
}


QPainterPath LCEllipseItem::shape() const {
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}

const boost::shared_ptr<const lc::CADEntity> LCEllipseItem::entity() const {
    return _ellipse;
}
