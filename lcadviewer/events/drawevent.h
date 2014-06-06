#ifndef DRAWEVENT_H
#define DRAWEVENT_H

#include <cad/geometry/geoarea.h>

/**
  * Draw event for a object to draw something.
  *
  */
class LcPainter;
class LCADViewer;
class DrawEvent {
    public:
        DrawEvent(LcPainter* painter, const lc::geo::Coordinate& mousePosition) : _painter(painter), _mousePosition(mousePosition) {

        }
        LcPainter* painter() const {
            return _painter;
        }
        lc::geo::Coordinate mousePosition() const {
            return _mousePosition;
        }

    private:
        const LcPainter* _painter;
        const lc::geo::Coordinate _mousePosition;
};

#endif // DRAWEVENT_H
