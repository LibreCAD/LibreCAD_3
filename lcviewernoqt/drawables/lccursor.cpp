#include "lccursor.h"

#include "../documentcanvas.h"
#include "../events/snappointevent.h"
#include "../managers/snapmanager.h"

using namespace LCViewer;

Cursor::Cursor(int cursorSize, std::shared_ptr<DocumentCanvas> view, const lc::Color& xAxisColor, const lc::Color& yAxisColor) : _xAxisColor(xAxisColor), _yAxisColor(yAxisColor), _cursorSize(cursorSize) {
/*
    connect(graphicsView, SIGNAL(drawEvent(const DrawEvent&)),
            this, SLOT(on_Draw_Event(const DrawEvent&)));

    connect(snapManager.get(), SIGNAL(snapPointEvent(const SnapPointEvent&)),
            this, SLOT(on_SnapPoint_Event(const SnapPointEvent&)));

    connect(graphicsView, SIGNAL(mouseReleaseEvent(const MouseReleaseEvent&)),
            this, SLOT(on_MouseRelease_Event(const MouseReleaseEvent&)));
*/
}

void Cursor::onDraw(DrawEvent const & event) const {
    double zeroCornerX = 0.;
    double zeroCornerY = 0.;
    event.painter().device_to_user(&zeroCornerX, &zeroCornerY);

    double gridSPacingX = _cursorSize;
    double gridSPacingY = _cursorSize;
    event.painter().device_to_user(&gridSPacingX, &gridSPacingY);

    double minDistancePoints = (gridSPacingX - zeroCornerX) / 2.0;

    double x=0, y=0;

    // If we had a snao point, move the mouse to that area
    if (_lastSnapEvent.status() == true) {
        x = _lastSnapEvent.snapPoint().x();
        y = _lastSnapEvent.snapPoint().y();
    } else {
        return;
//        x = event.mousePosition().x();
//        y = event.mousePosition().y();
    }

    event.painter().save();
    event.painter().disable_antialias();

    event.painter().move_to(-minDistancePoints + x, y);
    event.painter().line_to(minDistancePoints + x, y);
    event.painter().source_rgba(_xAxisColor.red(), _xAxisColor.green(), _xAxisColor.blue(), _xAxisColor.alpha());
    event.painter().stroke();

    event.painter().move_to(x, -minDistancePoints + y);
    event.painter().line_to(x, minDistancePoints + y);
    event.painter().source_rgba(_yAxisColor.red(), _yAxisColor.green(), _yAxisColor.blue(), _yAxisColor.alpha());
    event.painter().stroke();

    event.painter().restore();
}

void Cursor::onSnapPointEvent(SnapPointEvent const & event) {
    _lastSnapEvent = event;
}


Nano::Signal<void(const LocationEvent&)> Cursor::locationEvents() const {
    return _locationEvent;
}
