#include "lcgraphicsitem.h"

#include <QStyleOptionGraphicsItem>

LCGraphicsItem::LCGraphicsItem() {
}

QColor LCGraphicsItem::getScreenColor(const QColor& color) const {
    int r, g, b, a;
    color.getRgb(&r, &g, &b, &a);

    // FIXME: If the background of the screen is dark, or light swap or not swap collors accordingly for dark and light pens
    if (r < 0x10 && g < 0x10 && b < 0x10) {
        return QColor(0xff - r, 0xff - g, 0xff - b);
    }

    return color;
}


void LCGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(widget)

    std::shared_ptr<const lc::CADEntity> cadEntity = entity();

    // If the current item is selected set a selected pen
    if (this->isSelected()) {
        // FIXME: Needs to be a configurable color
        painter->setPen(QColor(50, 240, 50));
        return;
    }

    // Get the pen color of this entity
    const lc::Color* lccolor = dynamic_cast<const lc::Color*>(cadEntity->metaType(lc::MetaType::COLOR).get());
    QColor entityColor;

    if (lccolor != NULL) {
        entityColor = getScreenColor(lccolor->qColor());
    } else {
        // FIXME: Needs to be a configurable color
        entityColor = getScreenColor(QColor(0, 0, 0));
    }

    // Get line width of this entity
    double lineWidth = 0.0;
    const lc::LineWidth* lcLineWidth = dynamic_cast<const lc::LineWidth*>(cadEntity->metaType(lc::MetaType::LINEWIDTH).get());

    if (lcLineWidth != NULL) {
        lineWidth = lcLineWidth->width();
    }

    // FIXME 1 : Test if the current entity is BYLAYER or BYBLOCK

    // Set correct Pen
    QPen pen(QBrush(entityColor), 0.0);
    pen.setCapStyle(Qt::FlatCap);
    pen.setJoinStyle(Qt::MiterJoin);
    painter->setPen(pen);
}

