#pragma once

#include "cad/geometry/geocoordinate.h"
#include "painters/lcpainter.h"


/**
  * Event that get's dispatched each time the mouse is moved.
  */
namespace LCViewer {
class MouseMoveEvent  {
    public:
        MouseMoveEvent(LcPainter* painter, const lc::geo::Coordinate& mousePosition) : _painter(painter), _mousePosition(mousePosition) {
        }

        lc::geo::Coordinate  mousePosition() const {
            return _mousePosition;
        }

        LcPainter* painter() const {
            return _painter;
        }

    private:
        LcPainter* _painter;
        lc::geo::Coordinate _mousePosition;
};
}
