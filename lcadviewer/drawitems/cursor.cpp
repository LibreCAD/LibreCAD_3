#include "cursor.h"

#include "lcadviewer.h"
#include "helpers/snapmanager.h"

Cursor::Cursor(int cursorSize, LCADViewer* graphicsView, std::shared_ptr<SnapManager>  snapManager, const QColor& xAxisColor, const QColor& yAxisColor) : _xAxisColor(xAxisColor), _yAxisColor(yAxisColor), _cursorSize(cursorSize) {

    connect(graphicsView, SIGNAL(drawEvent(const DrawEvent&)),
            this, SLOT(on_Draw_Event(const DrawEvent&)));

    connect(snapManager.get(), SIGNAL(snapPointEvent(const SnapPointEvent&)),
            this, SLOT(on_SnapPoint_Event(const SnapPointEvent&)));

    connect(graphicsView, SIGNAL(mouseReleaseEvent(const MouseReleaseEvent&)),
            this, SLOT(on_MouseRelease_Event(const MouseReleaseEvent&)));

}

void Cursor::on_Draw_Event(const DrawEvent& event) {

    /*
        QPointF zeroCorner = event.view()->mapToScene(0, 0);
        QPointF minGridSpaceCorner = event.view()->mapToScene(_cursorSize, 0);

        double minDistancePoints = (minGridSpaceCorner.x() - zeroCorner.x()) / 2.0;

        double x, y;

        // If we had a snao point, move the mouse to that area
        if (_lastSnapEvent.status() == true) {
            x = _lastSnapEvent.snapPoint().x();
            y = _lastSnapEvent.snapPoint().y();
        } else {
            x = event.mousePosition().x();
            y = event.mousePosition().y();
        }

        event.painter()->setRenderHint(QPainter::Antialiasing, false);
        event.painter()->setPen(QPen(QBrush(_xAxisColor), 0.0, Qt::SolidLine));
        event.painter()->drawLine(QPointF(-minDistancePoints + x, y), QPointF(minDistancePoints + x, y));

        event.painter()->setPen(QPen(QBrush(_yAxisColor), 0.0, Qt::SolidLine));
        event.painter()->drawLine(QPointF(x, minDistancePoints + y), QPointF(x, -minDistancePoints + y));
    */
}

void Cursor::on_SnapPoint_Event(const SnapPointEvent& event) {
    _lastSnapEvent = event;
}

void Cursor::on_MouseRelease_Event(const MouseReleaseEvent& event) {
    if (_lastSnapEvent.status() == true) {
        MouseReleaseEvent snappedLocation(_lastSnapEvent.snapPoint(), event.mouseEvent(), event.entities());
        emit mouseReleaseEvent(snappedLocation);
    } else {
        emit mouseReleaseEvent(event);
    }
}
