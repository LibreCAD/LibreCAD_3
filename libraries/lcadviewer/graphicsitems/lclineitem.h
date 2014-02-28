#ifndef LCLINEITEM_H
#define LCLINEITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include "lcgraphicsitem.h"
#include <cad/primitive/line.h>
#include <cad/meta/color.h>
#include <cad/interface/metatype.h>


// http://www.davidwdrell.net/wordpress/?page_id=46
// http://qt-project.org/forums/viewthread/4570

class LCLineItem : public LCGraphicsItem {
    public:
        LCLineItem(boost::shared_ptr<const lc::Line> l);

        QRectF boundingRect() const;

        void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
        virtual const boost::shared_ptr<const lc::CADEntity> entity() const;

    private:
        QPainterPath shape() const;

    private:
        const boost::shared_ptr<const lc::Line> _line;

};

#endif // LCLINEITEM_H
