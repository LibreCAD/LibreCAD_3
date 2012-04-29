#ifndef LCGRAPHICSITEM_H
#define LCGRAPHICSITEM_H

#include <QColor>
#include <QGraphicsItem>

#include <QPainter>
#include <cad/meta/color.h>
#include <cad/meta/linewidth.h>
#include <cad/interface/metatype.h>
#include <cad/base/cadentity.h>


/**
  * Every primitive graphics item needs to implement LCGraphicsItem
  * THis class will take care of setting correct pen and other relevant items
  *
  * \note At some point we will not inherit from Graphicsitem anmyre because possible we will remove the depedency
  * from graphicsview and graphics scene for the reason that it contains a lot of bloat (from a CAD perspective)
  * and doesn't support layered drawings.
  */
class LCGraphicsItem : public QGraphicsItem {
    public:
        LCGraphicsItem();

        /**
          * Return a new Pen based on the pen for the current entity.
          * The pen that get returned might have reversed color based on layer information and current background color of the screen
          */
        virtual QColor getScreenColor(const QColor& color) const;

        /**
          * Paint the entity, the default implementation will just set the color
          */
        virtual void paint(lc::CADEntityPtr, QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

        /**
          * Return the CADentity that this GrapgicsItem manages
          */
        virtual const lc::CADEntityPtr entity() const = 0;
};

#endif // LCGRAPHICSITEM_H
