#include "cursor.h"

#include "lcadviewer.h"
#include "helpers/snapmanager.h"
#include "lcpainter.h"
#include "events/drawevent.h"

Cursor::Cursor(int cursorSize, LCADViewer* graphicsView, SnapManager_SPtr  snapManager, const QColor& xAxisColor, const QColor& yAxisColor) : _xAxisColor(xAxisColor), _yAxisColor(yAxisColor), _cursorSize(cursorSize) {

    connect(graphicsView, SIGNAL(drawEvent(const DrawEvent&)),
            this, SLOT(on_Draw_Event(const DrawEvent&)));

    connect(snapManager.get(), SIGNAL(snapPointEvent(const SnapPointEvent&)),
            this, SLOT(on_SnapPoint_Event(const SnapPointEvent&)));

    connect(graphicsView, SIGNAL(mouseReleaseEvent(const MouseReleaseEvent&)),
            this, SLOT(on_MouseRelease_Event(const MouseReleaseEvent&)));

}

void Cursor::on_Draw_Event(const DrawEvent& event)  {

    double zeroCornerX = 0.;
    double zeroCornerY = 0.;
    event.painter()->device_to_user(&zeroCornerX, &zeroCornerY);

    double gridSPacingX = _cursorSize;
    double gridSPacingY = _cursorSize;
    event.painter()->device_to_user(&gridSPacingX, &gridSPacingY);

    double minDistancePoints = (gridSPacingX - zeroCornerX) / 2.0;

    double x, y;

    // If we had a snao point, move the mouse to that area
    if (_lastSnapEvent.status() == true) {
        x = _lastSnapEvent.snapPoint().x();
        y = _lastSnapEvent.snapPoint().y();
    } else {
        x = event.mousePosition().x();
        y = event.mousePosition().y();
    }

    event.painter()->save();
    event.painter()->disable_antialias();

    event.painter()->move_to(-minDistancePoints + x, y);
    event.painter()->line_to(minDistancePoints + x, y);
    event.painter()->source_rgba(_xAxisColor.redF(), _xAxisColor.greenF(), _xAxisColor.blueF(), _xAxisColor.alphaF());
    event.painter()->stroke();

    event.painter()->move_to(x, -minDistancePoints + y);
    event.painter()->line_to(x, minDistancePoints + y);
    event.painter()->source_rgba(_yAxisColor.redF(), _yAxisColor.greenF(), _yAxisColor.blueF(), _yAxisColor.alphaF());
    event.painter()->stroke();


    event.painter()->restore();

}

void Cursor::on_SnapPoint_Event(const SnapPointEvent& event) {
    _lastSnapEvent = event;
}

void Cursor::on_MouseRelease_Event(const MouseReleaseEvent& event) {
    if (_lastSnapEvent.status() == true) {
        MouseReleaseEvent snappedLocation(_lastSnapEvent.snapPoint(), event.entities());
        emit mouseReleaseEvent(snappedLocation);
    } else {
        emit mouseReleaseEvent(event);
    }
}
