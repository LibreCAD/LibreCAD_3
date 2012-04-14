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
        LCArcItem(const lc::ArcPtr arc);

        QRectF boundingRect() const;

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
        virtual const lc::CADEntityPtr entity() const;

    private:
        QPainterPath shape() const;

    private:
        const lc::ArcPtr _arc;
};
#endif // LCARCITEM_H
