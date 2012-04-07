#ifndef LCARCITEM_H
#define LCARCITEM_H


#include <QGraphicsItem>
#include <QPainter>
#include <QPainterPath>
#include "lcgraphicsitem.h"
#include <cad/primitive/arc.h>
#include <cad/meta/color.h>
#include <cad/interface/metatype.h>

class LCArcItem : public LCGraphicsItem {
    public:
        LCArcItem(const lc::Arc* arc);

        QRectF boundingRect() const;

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    private:
        QPainterPath shape() const;

    private:
        const lc::Arc* _arc;
};
#endif // LCARCITEM_H
