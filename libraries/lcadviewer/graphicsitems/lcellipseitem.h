#ifndef LCELLIPSEITEM_H
#define LCELLIPSEITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QPainterPath>
#include "lcgraphicsitem.h"
#include <cad/primitive/ellipse.h>
#include <cad/meta/color.h>
#include <cad/interface/metatype.h>


class LCEllipseItem : public LCGraphicsItem {
    public:
        LCEllipseItem(const lc::EllipsePtr ellipse);

        QRectF boundingRect() const;

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
        virtual const lc::CADEntityPtr entity() const;

    private:
        QPainterPath shape() const;

    private:
        const lc::EllipsePtr _ellipse;
};

#endif // LCELLIPSEITEM_H
