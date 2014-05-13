#ifndef DRAWEVENT_H
#define DRAWEVENT_H

#include <QRectF>
#include <QPointF>

/**
  * Draw event for a object to draw something.
  *
  */
class QWidget;
class LcPainter;
class LCADViewer;
class DrawEvent {
    public:
        DrawEvent(LcPainter* painter, const QRectF& rect, const QPointF& mousePosition) :
            _painter(painter), _rect(rect), _mousePosition(mousePosition) {

        }

        LcPainter* painter() const {
            return _painter;
        }
        QRectF rect() const {
            return _rect;
        }
        QPointF mousePosition() const {
            return _mousePosition;
        }

    private:
        LcPainter* _painter;
        QRectF _rect;
        QPointF _mousePosition;
};

#endif // DRAWEVENT_H
