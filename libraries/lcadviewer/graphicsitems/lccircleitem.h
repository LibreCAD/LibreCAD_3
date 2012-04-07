#ifndef LCCIRCLEITEM_H
#define LCCIRCLEITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QPainterPath>
#include "lcgraphicsitem.h"
#include <cad/primitive/circle.h>
#include <cad/meta/color.h>
#include <cad/interface/metatype.h>

class LCCircleItem : public LCGraphicsItem {
    public:
        LCCircleItem(const lc::Circle* circle);

        QRectF boundingRect() const;

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    private:
        QPainterPath shape() const;

    private:
        const lc::Circle* _circle;
};

#endif // LCCIRCLEITEM_H
