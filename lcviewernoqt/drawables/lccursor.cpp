#include "lccursor.h"

#include "../documentcanvas.h"
#include "../events/snappointevent.h"
#include "../managers/snapmanager.h"
#include <string>
#include <cad/tools/string_helper.h>
#include <QtDebug>
using namespace lc;
using namespace lc::viewer;
using namespace lc::viewer::drawable;

Cursor::Cursor(int cursorSize,
               const std::shared_ptr<DocumentCanvas>& view,
               const lc::Color& xAxisColor,
               const lc::Color& yAxisColor) :
        _xAxisColor(xAxisColor),
        _yAxisColor(yAxisColor),
        _cursorSize(cursorSize),
        _locationEvent() {

}

void Cursor::onDraw(event::DrawEvent const & event) const {
    qDebug( "Cursor draw()");
    double zeroCornerX = 0.;
    double zeroCornerY = 0.;
    event.painter().device_to_user(&zeroCornerX, &zeroCornerY);

    double gridSPacingX = _cursorSize;
    double gridSPacingY = _cursorSize;
    event.painter().device_to_user(&gridSPacingX, &gridSPacingY);

    double minDistancePoints = (gridSPacingX - zeroCornerX) / 2.0;

    double x=0, y=0;

    // If we had a snao point, move the mouse to that area
    if (_lastSnapEvent.status()) {
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

    /** Cursor added temporarily until we have a better system for this **/
 /*   event.painter().source_rgb(1.,1.,1.);  //NOTE:TEMOPORARY UNABLED
    event.painter().move_to(x, y);
    std::string foo = lc::tools::StringHelper::string_format("%.2f,%.2f",x, y);
    event.painter().font_size(12, true);
    event.painter().text(foo.c_str());
    event.painter().stroke();
    */
    /** Cursor added temporarily until we have a better system for this **/

    event.painter().restore();
}

void Cursor::onSnapPointEvent(lc::viewer::event::SnapPointEvent const & event) {
    _lastSnapEvent = event;
}


Nano::Signal<void(const lc::viewer::event::LocationEvent&)>& Cursor::locationEvents() {
    return _locationEvent;
}

lc::geo::Coordinate Cursor::position() const {
	return { _lastSnapEvent.snapPoint().x(), _lastSnapEvent.snapPoint().y() };
}
