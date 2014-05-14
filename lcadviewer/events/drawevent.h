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
        DrawEvent(LcPainter* painter, const QPointF& mousePosition) : _painter(painter), _mousePosition(mousePosition) {

        }
        LcPainter* painter() const {
            return _painter;
        }
        QPointF mousePosition() const {
            return _mousePosition;
        }

    private:
        LcPainter* _painter;
        QPointF _mousePosition;
};

#endif // DRAWEVENT_H
