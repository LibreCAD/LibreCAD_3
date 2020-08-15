#pragma once

#include "cad/geometry/geocoordinate.h"
#include "painters/lcpainter.h"


/**
  * Event that gets dispatched each time the mouse is moved.
  */
namespace lc {
namespace viewer {
namespace event {
class MouseMoveEvent {
public:
    MouseMoveEvent(LcPainter* painter, lc::geo::Coordinate mousePosition) :
        _painter(painter),
        _mousePosition(std::move(mousePosition)) {
    }

    lc::geo::Coordinate mousePosition() const {
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
}
}
